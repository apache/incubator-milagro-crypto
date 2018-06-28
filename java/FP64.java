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

package org.apache.milagro.amcl.XXX;

public final class FP {

	public static final int NOT_SPECIAL=0;
	public static final int PSEUDO_MERSENNE=1;
	public static final int MONTGOMERY_FRIENDLY=2;
	public static final int GENERALISED_MERSENNE=3;

	public static final int MODBITS=@NBT@; /* Number of bits in Modulus */
	public static final int MOD8=@M8@;  /* Modulus mod 8 */
	public static final int MODTYPE=@MT@;

	public static final int FEXCESS =((int)1<<@SH@);  // BASEBITS*NLEN-MODBITS or 2^30 max!
	public static final long OMASK=(long)(-1)<<(MODBITS%BIG.BASEBITS);
	public static final int TBITS=MODBITS%BIG.BASEBITS; // Number of active bits in top word 
	public static final long TMASK=((long)1<<TBITS)-1;


	public final BIG x;
	public static BIG p=new BIG(ROM.Modulus);
	public static BIG r2modp=new BIG(ROM.R2modp);
	public int XES;

/**************** 64-bit specific ************************/

/* reduce a DBIG to a BIG using the appropriate form of the modulus */
	public static BIG mod(DBIG d)
	{
		if (MODTYPE==PSEUDO_MERSENNE)
		{
			BIG b;		
			long v,tw;
			BIG t=d.split(MODBITS);
			b=new BIG(d);

			v=t.pmul((int)ROM.MConst);

			t.add(b);
			t.norm();

			tw=t.w[BIG.NLEN-1];
			t.w[BIG.NLEN-1]&=FP.TMASK;
			t.w[0]+=(ROM.MConst*((tw>>TBITS)+(v<<(BIG.BASEBITS-TBITS))));

			t.norm();
			return t;			
		}
		if (FP.MODTYPE==MONTGOMERY_FRIENDLY)
		{
			BIG b;		
			long[] cr=new long[2];
			for (int i=0;i<BIG.NLEN;i++)
			{
				cr=BIG.muladd(d.w[i],ROM.MConst-1,d.w[i],d.w[BIG.NLEN+i-1]);
				d.w[BIG.NLEN+i]+=cr[0];
				d.w[BIG.NLEN+i-1]=cr[1];
			}
			
			b=new BIG(0);
			for (int i=0;i<BIG.NLEN;i++ )
				b.w[i]=d.w[BIG.NLEN+i];
			b.norm();
			return b;		
		}
		if (MODTYPE==GENERALISED_MERSENNE)
		{ // GoldiLocks Only
			BIG b;		
			BIG t=d.split(MODBITS);
			b=new BIG(d);
			b.add(t);
			DBIG dd=new DBIG(t);
			dd.shl(MODBITS/2);

			BIG tt=dd.split(MODBITS);
			BIG lo=new BIG(dd);
			b.add(tt);
			b.add(lo);
			b.norm();
			tt.shl(MODBITS/2);
			b.add(tt);

			long carry=b.w[BIG.NLEN-1]>>TBITS;
			b.w[BIG.NLEN-1]&=FP.TMASK;
			b.w[0]+=carry;
			
			b.w[224/BIG.BASEBITS]+=carry<<(224%BIG.BASEBITS);
			b.norm();
			return b;		
		}
		if (MODTYPE==NOT_SPECIAL)
		{
			return BIG.monty(p,ROM.MConst,d);
		}

		return new BIG(0);
	}



/*********************************************************/


/* Constructors */
	public FP(int a)
	{
		x=new BIG(a);
		nres();
	}

	public FP()
	{
		x=new BIG(0);
		XES=1;
	}

	public FP(BIG a)
	{
		x=new BIG(a);
		nres();
	}
	
	public FP(FP a)
	{
		x=new BIG(a.x);
		XES=a.XES;
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
		if (MODTYPE!=PSEUDO_MERSENNE && MODTYPE!=GENERALISED_MERSENNE)
		{
			DBIG d=BIG.mul(x,r2modp);  /*** Change ***/
			x.copy(mod(d));
			XES=2;
		}
		else XES=1;
	}

/* convert back to regular form */
	public BIG redc()
	{
		if (MODTYPE!=PSEUDO_MERSENNE && MODTYPE!=GENERALISED_MERSENNE)
		{
			DBIG d=new DBIG(x);
			return mod(d);
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
		XES=b.XES;
	}

/* set this=0 */
	public void zero()
	{
		x.zero();
		XES=1;
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
		int t,c=d;
		c=~(c-1);
		t=c&(XES^b.XES);
		XES^=t;
		b.XES^=t;
	}

/* copy FPs depending on d */
	public void cmove(FP b,int d)
	{
		x.cmove(b.x,d);
		XES^=(XES^b.XES)&(-d);

	}

/* this*=b mod Modulus */
	public void mul(FP b)
	{
		if ((long)XES*b.XES>(long)FEXCESS) reduce();

		DBIG d=BIG.mul(x,b.x);
		x.copy(mod(d));
		XES=2;
	}

/* this*=c mod Modulus, where c is a small int */
	public void imul(int c)
	{
//		norm();
		boolean s=false;
		if (c<0)
		{
			c=-c;
			s=true;
		}

		if (MODTYPE==PSEUDO_MERSENNE || MODTYPE==GENERALISED_MERSENNE)
		{
			DBIG d=x.pxmul(c);
			x.copy(mod(d));
			XES=2;
		}
		else
		{
			if (XES*c<=FEXCESS)
			{
				x.pmul(c);
				XES*=c;
			}
			else
			{  // this is not good
				FP n=new FP(c);
				mul(n);
			}
		}
		
/*
		if (c<=BIG.NEXCESS && XES*c<=FEXCESS)
		{
			x.imul(c);
			XES*=c;
			x.norm();
		}
		else
		{
			DBIG d=x.pxmul(c);
			x.copy(mod(d));
			XES=2;
		}
*/
		if (s) {neg(); norm();}

	}

/* this*=this mod Modulus */
	public void sqr()
	{
		DBIG d;
		if ((long)XES*XES>(long)FEXCESS) reduce();

		d=BIG.sqr(x);	
		x.copy(mod(d));
		XES=2;
	}

/* this+=b */
	public void add(FP b) {
		x.add(b.x);
		XES+=b.XES;
		if (XES>FEXCESS) reduce();
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
		return r;
	}

/* this = -this mod Modulus */
	public void neg()
	{
		int sb;
		BIG m=new BIG(p);

		sb=logb2(XES-1);
		m.fshl(sb);
		x.rsub(m);		

		XES=(1<<sb);
		if (XES>FEXCESS) reduce();
	}

/* this-=b */
	public void sub(FP b)
	{
		FP n=new FP(b);
		n.neg();
		this.add(n);
	}

	public void rsub(FP b)
	{
		FP n=new FP(this);
		n.neg();
		this.copy(b);
		this.add(n);
	}

/* this/=2 mod Modulus */
	public void div2()
	{
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
/*
		BIG r=redc();
		r.invmodp(p);
		x.copy(r);
		nres();
*/
		BIG m2=new BIG(p);
		m2.dec(2); m2.norm();
		copy(pow(m2));

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
		XES=1;
	}

	public FP pow(BIG e)
	{
		byte[] w=new byte[1+(BIG.NLEN*BIG.BASEBITS+3)/4];
		FP [] tb=new FP[16];
		BIG t=new BIG(e);
		t.norm();
		int nb=1+(t.nbits()+3)/4;

		for (int i=0;i<nb;i++)
		{
			int lsbs=t.lastbits(4);
			t.dec(lsbs);
			t.norm();
			w[i]=(byte)lsbs;
			t.fshr(4);
		}
		tb[0]=new FP(1);
		tb[1]=new FP(this);
		for (int i=2;i<16;i++)
		{
			tb[i]=new FP(tb[i-1]);
			tb[i].mul(this);
		}
		FP r=new FP(tb[w[nb-1]]);
		for (int i=nb-2;i>=0;i--)
		{
			r.sqr();
			r.sqr();
			r.sqr();
			r.sqr();
			r.mul(tb[w[i]]);
		}
		r.reduce();
		return r;
	}

/* return this^e mod Modulus 
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
	} */

/* return sqrt(this) mod Modulus */
	public FP sqrt()
	{
		reduce();
		BIG b=new BIG(p);
		if (MOD8==5)
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
