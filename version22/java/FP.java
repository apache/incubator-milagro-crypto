/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

/* Finite Field arithmetic */
/* AMCL mod p functions */

public final class FP {
	private final BIG x;
	private static BIG p=new BIG(ROM.Modulus);

/* Constructors */
	public FP(int a)
	{
		x=new BIG(a);
		nres();
	}

	public FP()
	{
		x=new BIG(0);
	}

	public FP(BIG a)
	{
		x=new BIG(a);
		nres();
	}
	
	public FP(FP a)
	{
		x=new BIG(a.x);
	}

/* convert to string */
	public String toString() 
	{
		String s=redc().toString();
		return s;
	}

	public String toRawString() 
	{
		String s=x.toRawString();
		return s;
	}

/* convert to Montgomery n-residue form */
	public void nres()
	{
		if (ROM.MODTYPE!=ROM.PSEUDO_MERSENNE && ROM.MODTYPE!=ROM.GENERALISED_MERSENNE)
		{
			DBIG d=new DBIG(x);
			d.shl(ROM.NLEN*ROM.BASEBITS);
			x.copy(d.mod(p));
		}
	}

/* convert back to regular form */
	public BIG redc()
	{
		if (ROM.MODTYPE!=ROM.PSEUDO_MERSENNE && ROM.MODTYPE!=ROM.GENERALISED_MERSENNE)
		{
			DBIG d=new DBIG(x);
			return BIG.mod(d);
		}
		else 
		{
			BIG r=new BIG(x);
			return r;
		}
	}

/* test this=0? */
	public boolean iszilch() {
		reduce();
		return x.iszilch();
	}

/* copy from FP b */
	public void copy(FP b)
	{
		x.copy(b.x);
	}

/* set this=0 */
	public void zero()
	{
		x.zero();
	}
	
/* set this=1 */
	public void one()
	{
		x.one(); nres();
	}

/* normalise this */
	public void norm()
	{
		x.norm();
	}

/* swap FPs depending on d */
	public void cswap(FP b,int d)
	{
		x.cswap(b.x,d);
	}

/* copy FPs depending on d */
	public void cmove(FP b,int d)
	{
		x.cmove(b.x,d);
	}

/* this*=b mod Modulus */
	public void mul(FP b)
	{
		norm();
		b.norm();

		if (BIG.pexceed(x,b.x)) reduce();

		DBIG d=BIG.mul(x,b.x);
		x.copy(BIG.mod(d));
	}

/* this*=c mod Modulus, where c is a small int */
	public void imul(int c)
	{
		norm();
		boolean s=false;
		if (c<0)
		{
			c=-c;
			s=true;
		}
		if (c<ROM.NEXCESS && ((BIG.EXCESS(x)+1)*(c+1)+1)<ROM.FEXCESS)
		{
			x.imul(c);
		}
		else
		{
			if (((BIG.EXCESS(x)+1)*(c+1)+1)<ROM.FEXCESS) x.pmul(c);
			else
			{
				DBIG d=x.pxmul(c);
				x.copy(d.mod(p));
			}
		}
		if (s) neg();
		norm();
	}

/* this*=this mod Modulus */
	public void sqr()
	{
		DBIG d;
		norm();

		if (BIG.sexceed(x)) reduce();

		d=BIG.sqr(x);	
		x.copy(BIG.mod(d));
	}

/* this+=b */
	public void add(FP b) {
		x.add(b.x);
		if (BIG.EXCESS(x)+2>=ROM.FEXCESS) reduce();
	}

// https://graphics.stanford.edu/~seander/bithacks.html
// constant time log to base 2 (or number of bits in)

	private static int logb2(int v)
	{
		int r;
		v |= v >>> 1;
		v |= v >>> 2;
		v |= v >>> 4;
		v |= v >>> 8;
		v |= v >>> 16;

		v = v - ((v >>> 1) & 0x55555555);                  
		v = (v & 0x33333333) + ((v >>> 2) & 0x33333333);  
		r = ((v + (v >>> 4) & 0xF0F0F0F) * 0x1010101) >>> 24; 
		return r+1;
	}

/* this = -this mod Modulus */
	public void neg()
	{
		int sb;
		BIG m=new BIG(p);

		norm();
		sb=logb2((int)BIG.EXCESS(x));
/*
		ov=BIG.EXCESS(x); 
		sb=1; while(ov!=0) {sb++;ov>>=1;} 
*/
		m.fshl(sb);
		x.rsub(m);		

		if (BIG.EXCESS(x)>=ROM.FEXCESS) reduce();
	}

/* this-=b */
	public void sub(FP b)
	{
		FP n=new FP(b);
		n.neg();
		this.add(n);
	}

/* this/=2 mod Modulus */
	public void div2()
	{
		x.norm();
		if (x.parity()==0)
			x.fshr(1);
		else
		{
			x.add(p);
			x.norm();
			x.fshr(1);
		}
	}

/* this=1/this mod Modulus */
	public void inverse()
	{
		BIG r=redc();
		r.invmodp(p);
		x.copy(r);
		nres();
	}

/* return TRUE if this==a */
	public boolean equals(FP a)
	{
		a.reduce();
		reduce();
		if (BIG.comp(a.x,x)==0) return true;
		return false;
	}

/* reduce this mod Modulus */
	public void reduce()
	{
		x.mod(p);
	}

/* return this^e mod Modulus */
	public FP pow(BIG e)
	{
		int bt;
		FP r=new FP(1);
		e.norm();
		x.norm();
		FP m=new FP(this);
		while (true)
		{
			bt=e.parity();
			e.fshr(1);
			if (bt==1) r.mul(m);
			if (e.iszilch()) break;
			m.sqr();
		}
		r.x.mod(p);
		return r;
	}

/* return sqrt(this) mod Modulus */
	public FP sqrt()
	{
		reduce();
		BIG b=new BIG(p);
		if (ROM.MOD8==5)
		{
			b.dec(5); b.norm(); b.shr(3);
			FP i=new FP(this); i.x.shl(1);
			FP v=i.pow(b);
			i.mul(v); i.mul(v);
			i.x.dec(1);
			FP r=new FP(this);
			r.mul(v); r.mul(i); 
			r.reduce();
			return r;
		}
		else
		{
			b.inc(1); b.norm(); b.shr(2);
			return pow(b);
		}
	}

/* return jacobi symbol (this/Modulus) */
	public int jacobi()
	{
		BIG w=redc();
		return w.jacobi(p);
	}
/*
	public static void main(String[] args) {
		BIG m=new BIG(ROM.Modulus);
		BIG x=new BIG(3);
		BIG e=new BIG(m);
		e.dec(1);

		System.out.println("m= "+m.nbits());	


		BIG r=x.powmod(e,m);

		System.out.println("m= "+m.toString());	
		System.out.println("r= "+r.toString());	

		BIG.cswap(m,r,0);

		System.out.println("m= "+m.toString());	
		System.out.println("r= "+r.toString());	

//		FP y=new FP(3);
//		FP s=y.pow(e);
//		System.out.println("s= "+s.toString());	

	} */
}
