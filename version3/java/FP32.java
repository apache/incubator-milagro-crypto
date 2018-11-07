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

	public static final int FEXCESS = (((int)1<<@SH@)-1); // BASEBITS*NLEN-MODBITS or 2^30 max!
	public static final int OMASK=(int)(-1)<<(MODBITS%BIG.BASEBITS);
	public static final int TBITS=MODBITS%BIG.BASEBITS; // Number of active bits in top word 
	public static final int TMASK=((int)1<<TBITS)-1;


	public final BIG x;
	//public BIG p=new BIG(ROM.Modulus);
	//public BIG r2modp=new BIG(ROM.R2modp);   /*** Change ***/
	public int XES;

/**************** 32-bit specific ************************/


/* reduce a DBIG to a BIG using the appropriate form of the modulus */
	public static BIG mod(DBIG d)
	{
		if (MODTYPE==PSEUDO_MERSENNE)
		{
			BIG b;
			int v,tw;
			BIG t=d.split(MODBITS);
			b=new BIG(d);

			v=t.pmul((int)ROM.MConst);

			t.add(b);
			t.norm();

			tw=t.w[BIG.NLEN-1];
			t.w[BIG.NLEN-1]&=TMASK;
			t.w[0]+=(ROM.MConst*((tw>>TBITS)+(v<<(BIG.BASEBITS-TBITS))));

			t.norm();
			return t;

		}
		if (MODTYPE==MONTGOMERY_FRIENDLY)
		{
			BIG b;
			int[] cr=new int[2];				
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

			int carry=b.w[BIG.NLEN-1]>>TBITS;
			b.w[BIG.NLEN-1]&=TMASK;
			b.w[0]+=carry;
			
			b.w[224/BIG.BASEBITS]+=carry<<(224%BIG.BASEBITS);
			b.norm();
			return b;
		}
		if (MODTYPE==NOT_SPECIAL)
		{
			return BIG.monty(new BIG(ROM.Modulus),ROM.MConst,d);
		}

		return new BIG(0);
	}

	private static int quo(BIG n,BIG m)
	{
		int sh;
		int num,den;
		int hb=BIG.CHUNK/2;
		if (TBITS<hb)
		{
			sh=hb-TBITS;
			num=(n.w[BIG.NLEN-1]<<sh)|(n.w[BIG.NLEN-2]>>(BIG.BASEBITS-sh));
			den=(m.w[BIG.NLEN-1]<<sh)|(m.w[BIG.NLEN-2]>>(BIG.BASEBITS-sh));
		}
		else
		{
			num=n.w[BIG.NLEN-1];
			den=m.w[BIG.NLEN-1];
		}
		return (int)(num/(den+1));
	}

/* reduce this mod Modulus */
	public void reduce()
	{
		BIG m=new BIG(ROM.Modulus);
		BIG r=new BIG(ROM.Modulus);
		int sr,sb,q,carry;
		x.norm();

		if (XES>16)
		{
			q=quo(x,m);
			carry=r.pmul(q);
			r.w[BIG.NLEN-1]+=(carry<<BIG.BASEBITS); // correction - put any carry out back in again
			x.sub(r);
			x.norm();
			sb=2;
		}
		else  sb=logb2(XES-1);

		m.fshl(sb);
		while (sb>0)
		{
// constant time...
			sr=BIG.ssn(r,x,m);  // optimized combined shift, subtract and norm
			x.cmove(r,1-sr);
			sb--;
		}

		XES=1;
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
			DBIG d=BIG.mul(x,new BIG(ROM.R2modp));  /*** Change ***/
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
		FP z=new FP(this);
		z.reduce();
		return z.x.iszilch();
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
		} */
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
		BIG m=new BIG(ROM.Modulus);

		sb=logb2(XES-1);
		m.fshl(sb);
		x.rsub(m);		

		XES=(1<<sb)+1;
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
			x.add(new BIG(ROM.Modulus));
			x.norm();
			x.fshr(1);
		}
	}

// return this^(p-3)/4 or this^(p-5)/8
	private FP fpow()
	{
		int i,j,k,bw,w,c,nw,lo,m,n;
		FP [] xp=new FP[11];
		int[]  ac={1,2,3,6,12,15,30,60,120,240,255};
// phase 1
			
		xp[0]=new FP(this);	// 1 
		xp[1]=new FP(this); xp[1].sqr(); // 2
		xp[2]=new FP(xp[1]); xp[2].mul(this);  //3
		xp[3]=new FP(xp[2]); xp[3].sqr();  // 6 
		xp[4]=new FP(xp[3]); xp[4].sqr();  // 12
		xp[5]=new FP(xp[4]); xp[5].mul(xp[2]);  // 15
		xp[6]=new FP(xp[5]); xp[6].sqr();  // 30
		xp[7]=new FP(xp[6]); xp[7].sqr();  // 60
		xp[8]=new FP(xp[7]); xp[8].sqr();  // 120
		xp[9]=new FP(xp[8]); xp[9].sqr();  // 240
		xp[10]=new FP(xp[9]); xp[10].mul(xp[5]);  // 255		
			
		if (MOD8==5)
		{
			n=MODBITS-3;
			c=(ROM.MConst+5)/8;
		} else {
			n=MODBITS-2;
			c=(ROM.MConst+3)/4;
		}

		bw=0; w=1; while (w<c) {w*=2; bw+=1;}
		k=w-c;

		FP key=new FP(); i=10;
		if (k!=0)
		{
			while (ac[i]>k) i--;
			key.copy(xp[i]); 
			k-=ac[i];
		}
		while (k!=0)
		{
			i--;
			if (ac[i]>k) continue;
			key.mul(xp[i]);
			k-=ac[i]; 
		}

// phase 2 
		xp[1].copy(xp[2]);
		xp[2].copy(xp[5]);
		xp[3].copy(xp[10]);

		j=3; m=8;
		nw=n-bw;
		FP t=new FP();

		while (2*m<nw)
		{
			t.copy(xp[j++]);
			for (i=0;i<m;i++)
				t.sqr(); 
			xp[j].copy(xp[j-1]);
			xp[j].mul(t);
			m*=2;
		}

		lo=nw-m;
		FP r=new FP(xp[j]);

		while (lo!=0)
		{
			m/=2; j--;
			if (lo<m) continue;
			lo-=m;
			t.copy(r);
			for (i=0;i<m;i++)
				t.sqr();
			r.copy(t);
			r.mul(xp[j]);
		}

// phase 3
		for (i=0;i<bw;i++ )
			r.sqr();

		if (w-c!=0)
			r.mul(key); 
		return r;
	}

/* this=1/this mod Modulus */
	public void inverse()
	{

		if (MODTYPE==PSEUDO_MERSENNE)
		{
			FP y=fpow();
			if (MOD8==5)
			{
				FP t=new FP(this);
				t.sqr();
				mul(t);
				y.sqr();

			} 
			y.sqr();
			y.sqr();
			mul(y);
		} else {
			BIG m2=new BIG(ROM.Modulus);
			m2.dec(2); m2.norm();
			copy(pow(m2));
		}
	}

/* return TRUE if this==a */
	public boolean equals(FP a)
	{
		FP f=new FP(this);
		FP s=new FP(a);
		f.reduce();
		s.reduce();
		if (BIG.comp(f.x,s.x)==0) return true;
		return false;
	}

	private FP pow(BIG e)
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
		if (MOD8==5)
		{
			FP i=new FP(this); i.x.shl(1);
			FP v;
			if (MODTYPE==PSEUDO_MERSENNE)
			{
				v=i.fpow();
			} else {
				BIG b=new BIG(ROM.Modulus);
				b.dec(5); b.norm(); b.shr(3);
				v=i.pow(b);
			}
			i.mul(v); i.mul(v);
			i.x.dec(1);
			FP r=new FP(this);
			r.mul(v); r.mul(i); 
			r.reduce();
			return r;
		}
		else
		{
			if (MODTYPE==PSEUDO_MERSENNE)
			{
				FP r=fpow();
				r.mul(this);
				return r;
			} else {
				BIG b=new BIG(ROM.Modulus);
				b.inc(1); b.norm(); b.shr(2);
				return pow(b);
			}
		}
	}

/* return jacobi symbol (this/Modulus) */
	public int jacobi()
	{
		BIG w=redc();
		return w.jacobi(new BIG(ROM.Modulus));
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
