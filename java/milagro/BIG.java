package milagro;
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

/* AMCL BIG number class */

public class BIG {
	private int[] w=new int[ROM.NLEN];
/* Constructors */

	public BIG()
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=0;
	}

	public BIG(int x)
	{
		w[0]=x;
		for (int i=1;i<ROM.NLEN;i++)
			w[i]=0;
	}

	public BIG(BIG x)
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x.w[i];
	}

	public BIG(DBIG x)
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x.w[i];
	}

	public BIG(int[] x)
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x[i];
	}

	public int get(int i)
	{
		return w[i];
	}

	public void set(int i,int x)
	{
		w[i]=x;
	}

	public void xortop(int x)
	{
		w[ROM.NLEN-1]^=x;
	}

	public void ortop(int x)
	{
		w[ROM.NLEN-1]|=x;
	}

/* calculate Field Excess */
	public static int EXCESS(BIG a)
	{
		return ((a.w[ROM.NLEN-1]&ROM.OMASK)>>(ROM.ROM.modCurve.getModBits()%ROM.BASEBITS));
	}

/* test for zero */
	public boolean iszilch() {
		for (int i=0;i<ROM.NLEN;i++)
			if (w[i]!=0) return false;
		return true;
	}

/* set to zero */
	public void zero()
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=0;
	}

/* set to one */
	public void one()
	{
		w[0]=1;
		for (int i=1;i<ROM.NLEN;i++)
			w[i]=0;
	}

/* Test for equal to one */
	public boolean isunity()
	{
		for (int i=1;i<ROM.NLEN;i++)
			if (w[i]!=0) return false;
		if (w[0]!=1) return false;
		return true;
	}

/* Copy from another BIG */
	public void copy(BIG x)
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x.w[i];
	}

	public void copy(DBIG x)
	{
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x.w[i];
	}


/* Conditional swap of two bigs depending on d using XOR - no branches */
	public void cswap(BIG b,int d)
	{
		int i;
		int t,c=d;
		c=~(c-1);

		for (i=0;i<ROM.NLEN;i++)
		{
			t=c&(w[i]^b.w[i]);
			w[i]^=t;
			b.w[i]^=t;
		}
	}

	public void cmove(BIG g,int d)
	{
		int i;
		int b=-d;

		for (i=0;i<ROM.NLEN;i++)
		{
			w[i]^=(w[i]^g.w[i])&b;
		}
	}

/* normalise BIG - force all digits < 2^BASEBITS */
	public int norm() {
		int d,carry=0;
		for (int i=0;i<ROM.NLEN-1;i++)
		{
			d=w[i]+carry;
			w[i]=d&ROM.MASK;
			carry=d>>ROM.BASEBITS;
		}
		w[ROM.NLEN-1]=(w[ROM.NLEN-1]+carry);

		return (w[ROM.NLEN-1]>>((8*ROM.MODBYTES)%ROM.BASEBITS));
	}

/* Shift right by less than a word */
	public int fshr(int k) {
		int r=w[0]&(((int)1<<k)-1); /* shifted out part */
		for (int i=0;i<ROM.NLEN-1;i++)
			w[i]=(w[i]>>k)|((w[i+1]<<(ROM.BASEBITS-k))&ROM.MASK);
		w[ROM.NLEN-1]=w[ROM.NLEN-1]>>k;
		return r;
	}

/* general shift right */
	public void shr(int k) {
		int n=k%ROM.BASEBITS;
		int m=k/ROM.BASEBITS;
		for (int i=0;i<ROM.NLEN-m-1;i++)
			w[i]=(w[m+i]>>n)|((w[m+i+1]<<(ROM.BASEBITS-n))&ROM.MASK);
		w[ROM.NLEN-m-1]=w[ROM.NLEN-1]>>n;
		for (int i=ROM.NLEN-m;i<ROM.NLEN;i++) w[i]=0;
	}

/* Shift right by less than a word */
	public int fshl(int k) {
		w[ROM.NLEN-1]=((w[ROM.NLEN-1]<<k))|(w[ROM.NLEN-2]>>(ROM.BASEBITS-k));
		for (int i=ROM.NLEN-2;i>0;i--)
			w[i]=((w[i]<<k)&ROM.MASK)|(w[i-1]>>(ROM.BASEBITS-k));
		w[0]=(w[0]<<k)&ROM.MASK;

		return (w[ROM.NLEN-1]>>((8*ROM.MODBYTES)%ROM.BASEBITS)); /* return excess - only used in FF.java */
	}

/* general shift left */
	public void shl(int k) {
		int n=k%ROM.BASEBITS;
		int m=k/ROM.BASEBITS;

		w[ROM.NLEN-1]=((w[ROM.NLEN-1-m]<<n))|(w[ROM.NLEN-m-2]>>(ROM.BASEBITS-n));
		for (int i=ROM.NLEN-2;i>m;i--)
			w[i]=((w[i-m]<<n)&ROM.MASK)|(w[i-m-1]>>(ROM.BASEBITS-n));
		w[m]=(w[0]<<n)&ROM.MASK;
		for (int i=0;i<m;i++) w[i]=0;
	}

/* return number of bits */
	public int nbits() {
		int bts,k=ROM.NLEN-1;
		int c;
		norm();
		while (k>=0 && w[k]==0) k--;
		if (k<0) return 0;
		bts=ROM.BASEBITS*k;
		c=w[k];
		while (c!=0) {c/=2; bts++;}
		return bts;
	}

	public String toRawString()
	{
		BIG b=new BIG(this);
		String s="(";
		for (int i=0;i<ROM.NLEN-1;i++)
		{
			s+=Integer.toHexString(b.w[i]); s+=",";
		}
		s+=Integer.toHexString(b.w[ROM.NLEN-1]); s+=")";
		return s;
	}

/* Convert to Hex String */
	public String toString() {
		BIG b;
		String s="";
		int len=nbits();

		if (len%4==0) len/=4;
		else {len/=4; len++;}
		if (len<ROM.MODBYTES*2) len=ROM.MODBYTES*2;

		for (int i=len-1;i>=0;i--)
		{
			b=new BIG(this);
			b.shr(i*4);
			s+=Integer.toHexString(b.w[0]&15);
		}
		return s;
	}

/* return this+x */
	public BIG plus(BIG x) {
		BIG s=new BIG(0);
		for (int i=0;i<ROM.NLEN;i++)
			s.w[i]=w[i]+x.w[i];
		return s;
	}

/* this+=x */
	public void add(BIG x) {
		for (int i=0;i<ROM.NLEN;i++)
			w[i]+=x.w[i];
	}

/* this+=x, where x is int */
	public void inc(int x) {
		norm();
		w[0]+=x;
	}

/* return this.x */
	public BIG minus(BIG x) {
		BIG d=new BIG(0);
		for (int i=0;i<ROM.NLEN;i++)
			d.w[i]=w[i]-x.w[i];
		return d;
	}

/* this-=x */
	public void sub(BIG x) {
		for (int i=0;i<ROM.NLEN;i++)
			w[i]-=x.w[i];
	}

/* reverse subtract this=x-this */
	public void rsub(BIG x) {
		for (int i=0;i<ROM.NLEN;i++)
			w[i]=x.w[i]-w[i];
	}

/* this-=x where x is int */
	public void dec(int x) {
		norm();
		w[0]-=x;
	}

/* this*=x, where x is small int<NEXCESS */
	public void imul(int c)
	{
		for (int i=0;i<ROM.NLEN;i++) w[i]*=c;
	}

/* convert this BIG to byte array */
	public void tobytearray(byte[] b,int n)
	{
		norm();
		BIG c=new BIG(this);

		for (int i=ROM.MODBYTES-1;i>=0;i--)
		{
			b[i+n]=(byte)c.w[0];
			c.fshr(8);
		}
	}

/* convert from byte array to BIG */
	public static BIG frombytearray(byte[] b,int n)
	{
		BIG m=new BIG(0);

		for (int i=0;i<ROM.MODBYTES;i++)
		{
			m.fshl(8); m.w[0]+=(int)b[i+n]&0xff;
			//m.inc((int)b[i]&0xff);
		}
		return m;
	}

	public void toBytes(byte[] b)
	{
		tobytearray(b,0);
	}

	public static BIG fromBytes(byte[] b)
	{
		return frombytearray(b,0);
	}

/* set this[i]+=x*y+c, and return high part */
	public int muladd(int x,int y,int c,int i)
	{
		long prod=(long)x*y+c+w[i];
		w[i]=(int)prod&ROM.MASK;
		return (int)(prod>>ROM.BASEBITS);
	}

/* this*=x, where x is >NEXCESS */
	public int pmul(int c)
	{
		int ak,carry=0;
		norm();
		for (int i=0;i<ROM.NLEN;i++)
		{
			ak=w[i];
			w[i]=0;
			carry=muladd(ak,c,carry,i);
		}
		return carry;
	}

/* this*=c and catch overflow in DBIG */
	public DBIG pxmul(int c)
	{
		DBIG m=new DBIG(0);
		int carry=0;
		for (int j=0;j<ROM.NLEN;j++)
			carry=m.muladd(w[j],c,carry,j);
		m.w[ROM.NLEN]=carry;
		return m;
	}

/* divide by 3 */
	public int div3()
	{
		int ak,base,carry=0;
		norm();
		base=((int)1<<ROM.BASEBITS);
		for (int i=ROM.NLEN-1;i>=0;i--)
		{
			ak=(carry*base+w[i]);
			w[i]=ak/3;
			carry=ak%3;
		}
		return (int)carry;
	}

/* return a*b where result fits in a BIG */
	public static BIG smul(BIG a,BIG b)
	{
		int carry;
		BIG c=new BIG(0);
		for (int i=0;i<ROM.NLEN;i++)
		{
			carry=0;
			for (int j=0;j<ROM.NLEN;j++)
				if (i+j<ROM.NLEN) carry=c.muladd(a.w[i],b.w[j],carry,i+j);
		}
		return c;
	}

/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
	public static int comp(BIG a,BIG b)
	{
		for (int i=ROM.NLEN-1;i>=0;i--)
		{
			if (a.w[i]==b.w[i]) continue;
			if (a.w[i]>b.w[i]) return 1;
			else  return -1;
		}
		return 0;
	}

/* set x = x mod 2^m */
	public void mod2m(int m)
	{
		int i,wd,bt;
		int msk;

		wd=m/ROM.BASEBITS;
		bt=m%ROM.BASEBITS;
		msk=((int)1<<bt)-1;
		w[wd]&=msk;
		for (i=wd+1;i<ROM.NLEN;i++) w[i]=0;
	}

/* Arazi and Qi inversion mod 256 */
	public static int invmod256(int a)
	{
		int U,t1,t2,b,c;
		t1=0;
		c=(a>>1)&1;
		t1+=c;
		t1&=1;
		t1=2-t1;
		t1<<=1;
		U=t1+1;

// i=2
		b=a&3;
		t1=U*b; t1>>=2;
		c=(a>>2)&3;
		t2=(U*c)&3;
		t1+=t2;
		t1*=U; t1&=3;
		t1=4-t1;
		t1<<=2;
		U+=t1;

// i=4
		b=a&15;
		t1=U*b; t1>>=4;
		c=(a>>4)&15;
		t2=(U*c)&15;
		t1+=t2;
		t1*=U; t1&=15;
		t1=16-t1;
		t1<<=4;
		U+=t1;

		return U;
	}

/* a=1/a mod 2^256. This is very fast! */
	public void invmod2m()
	{
		int i;
		BIG U=new BIG(0);
		BIG b=new BIG(0);
		BIG c=new BIG(0);

		U.inc(invmod256(lastbits(8)));

		for (i=8;i<256;i<<=1)
		{
			b.copy(this); b.mod2m(i);
			BIG t1=BIG.smul(U,b); t1.shr(i);
			c.copy(this); c.shr(i); c.mod2m(i);

			BIG t2=BIG.smul(U,c); t2.mod2m(i);
			t1.add(t2);
			b=BIG.smul(t1,U); t1.copy(b);
			t1.mod2m(i);

			t2.one(); t2.shl(i); t1.rsub(t2); t1.norm();
			t1.shl(i);
			U.add(t1);
		}
		this.copy(U);
	}

/* reduce this mod m */
	public void mod(BIG m)
	{
		int k=0;

		norm();
		if (comp(this,m)<0) return;
		do
		{
			m.fshl(1);
			k++;
		} while (comp(this,m)>=0);

		while (k>0)
		{
			m.fshr(1);
			if (comp(this,m)>=0)
			{
				sub(m);
				norm();
			}
			k--;
		}
	}

/* divide this by m */
	public void div(BIG m)
	{
		int k=0;
		norm();
		BIG e=new BIG(1);
		BIG b=new BIG(this);
		zero();

		while (comp(b,m)>=0)
		{
			e.fshl(1);
			m.fshl(1);
			k++;
		}

		while (k>0)
		{
			m.fshr(1);
			e.fshr(1);
			if (comp(b,m)>=0)
			{
				add(e);
				norm();
				b.sub(m);
				b.norm();
			}
			k--;
		}
	}

/* return parity */
	public int parity()
	{
		return w[0]%2;
	}

/* return n-th bit */
	public int bit(int n)
	{
		if ((w[n/ROM.BASEBITS]&((int)1<<(n%ROM.BASEBITS)))>0) return 1;
		else return 0;
	}

/* return n last bits */
	public int lastbits(int n)
	{
		int msk=(1<<n)-1;
		norm();
		return ((int)w[0])&msk;
	}

/* get 8*MODBYTES size random number */
	public static BIG random(RAND rng)
	{
		BIG m=new BIG(0);
		int i,b,j=0,r=0;

/* generate random BIG */
		for (i=0;i<8*ROM.MODBYTES;i++)
		{
			if (j==0) r=rng.getByte();
			else r>>=1;

			b=r&1;
			m.shl(1); m.w[0]+=b;// m.inc(b);
			j++; j&=7;
		}
		return m;
	}

/* Create random BIG in portable way, one bit at a time */
	public static BIG randomnum(BIG q,RAND rng)
	{
		DBIG d=new DBIG(0);
		int i,b,j=0,r=0;
		for (i=0;i<2*ROM.ROM.modCurve.getModBits();i++)
		{
			if (j==0) r=rng.getByte();
			else r>>=1;

			b=r&1;
			d.shl(1); d.w[0]+=b;// m.inc(b);
			j++; j&=7;
		}
		BIG m=d.mod(q);
		return m;
	}

/* return NAF value as +/- 1, 3 or 5. x and x3 should be normed.
nbs is number of bits processed, and nzs is number of trailing 0s detected */
	public static int[] nafbits(BIG x,BIG x3,int i)
	{
		int[] n=new int[3];
		int nb=x3.bit(i)-x.bit(i);
		int j;
		n[1]=1;
		n[0]=0;
		if (nb==0) {n[0]=0; return n;}
		if (i==0) {n[0]=nb; return n;}
	    if (nb>0) n[0]=1;
		else      n[0]=(-1);

		for (j=i-1;j>0;j--)
		{
			n[1]++;
			n[0]*=2;
			nb=x3.bit(j)-x.bit(j);
			if (nb>0) n[0]+=1;
			if (nb<0) n[0]-=1;
			if (n[0]>5 || n[0]<-5) break;
		}

		if (n[0]%2!=0 && j!=0)
		{ /* backtrack */
			if (nb>0) n[0]=(n[0]-1)/2;
			if (nb<0) n[0]=(n[0]+1)/2;
			n[1]--;
		}
		while (n[0]%2==0)
		{ /* remove trailing zeros */
			n[0]/=2;
			n[2]++;
			n[1]--;
		}
		return n;
	}

/* return a*b as DBIG */
	public static DBIG mul(BIG a,BIG b)
	{
		long t,co;
		DBIG c=new DBIG(0);
		a.norm();
		b.norm();

		t=(long)a.w[0]*b.w[0]; c.w[0]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[1]*b.w[0]+(long)a.w[0]*b.w[1]+co; c.w[1]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[2]*b.w[0]+(long)a.w[1]*b.w[1]+(long)a.w[0]*b.w[2]+co; c.w[2]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[3]*b.w[0]+(long)a.w[2]*b.w[1]+(long)a.w[1]*b.w[2]+(long)a.w[0]*b.w[3]+co; c.w[3]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[4]*b.w[0]+(long)a.w[3]*b.w[1]+(long)a.w[2]*b.w[2]+(long)a.w[1]*b.w[3]+(long)a.w[0]*b.w[4]+co; c.w[4]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[5]*b.w[0]+(long)a.w[4]*b.w[1]+(long)a.w[3]*b.w[2]+(long)a.w[2]*b.w[3]+(long)a.w[1]*b.w[4]+(long)a.w[0]*b.w[5]+co; c.w[5]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[6]*b.w[0]+(long)a.w[5]*b.w[1]+(long)a.w[4]*b.w[2]+(long)a.w[3]*b.w[3]+(long)a.w[2]*b.w[4]+(long)a.w[1]*b.w[5]+(long)a.w[0]*b.w[6]+co; c.w[6]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[7]*b.w[0]+(long)a.w[6]*b.w[1]+(long)a.w[5]*b.w[2]+(long)a.w[4]*b.w[3]+(long)a.w[3]*b.w[4]+(long)a.w[2]*b.w[5]+(long)a.w[1]*b.w[6]+(long)a.w[0]*b.w[7]+co; c.w[7]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[0]+(long)a.w[7]*b.w[1]+(long)a.w[6]*b.w[2]+(long)a.w[5]*b.w[3]+(long)a.w[4]*b.w[4]+(long)a.w[3]*b.w[5]+(long)a.w[2]*b.w[6]+(long)a.w[1]*b.w[7]+(long)a.w[0]*b.w[8]+co; c.w[8]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;

		t=(long)a.w[8]*b.w[1]+(long)a.w[7]*b.w[2]+(long)a.w[6]*b.w[3]+(long)a.w[5]*b.w[4]+(long)a.w[4]*b.w[5]+(long)a.w[3]*b.w[6]+(long)a.w[2]*b.w[7]+(long)a.w[1]*b.w[8]+co; c.w[9]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[2]+(long)a.w[7]*b.w[3]+(long)a.w[6]*b.w[4]+(long)a.w[5]*b.w[5]+(long)a.w[4]*b.w[6]+(long)a.w[3]*b.w[7]+(long)a.w[2]*b.w[8]+co; c.w[10]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[3]+(long)a.w[7]*b.w[4]+(long)a.w[6]*b.w[5]+(long)a.w[5]*b.w[6]+(long)a.w[4]*b.w[7]+(long)a.w[3]*b.w[8]+co; c.w[11]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[4]+(long)a.w[7]*b.w[5]+(long)a.w[6]*b.w[6]+(long)a.w[5]*b.w[7]+(long)a.w[4]*b.w[8]+co; c.w[12]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[5]+(long)a.w[7]*b.w[6]+(long)a.w[6]*b.w[7]+(long)a.w[5]*b.w[8]+co; c.w[13]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[6]+(long)a.w[7]*b.w[7]+(long)a.w[6]*b.w[8]+co; c.w[14]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*b.w[7]+(long)a.w[7]*b.w[8]+co; c.w[15]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;

		t=(long)a.w[8]*b.w[8]+co; c.w[16]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		c.w[17]=(int)co;

		return c;
	}


/* return a^2 as DBIG */
	public static DBIG sqr(BIG a)
	{
		long t,co;
		DBIG c=new DBIG(0);
		a.norm();

		t=(long)a.w[0]*a.w[0]; c.w[0]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[1]*a.w[0]; t+=t; t+=co; c.w[1]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[2]*a.w[0]; t+=t; t+=(long)a.w[1]*a.w[1]; t+=co; c.w[2]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[3]*a.w[0]+(long)a.w[2]*a.w[1]; t+=t; t+=co; c.w[3]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[4]*a.w[0]+(long)a.w[3]*a.w[1]; t+=t; t+=(long)a.w[2]*a.w[2]; t+=co; c.w[4]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[5]*a.w[0]+(long)a.w[4]*a.w[1]+(long)a.w[3]*a.w[2]; t+=t; t+=co; c.w[5]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[6]*a.w[0]+(long)a.w[5]*a.w[1]+(long)a.w[4]*a.w[2]; t+=t; t+=(long)a.w[3]*a.w[3]; t+=co; c.w[6]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[7]*a.w[0]+(long)a.w[6]*a.w[1]+(long)a.w[5]*a.w[2]+(long)a.w[4]*a.w[3]; t+=t; t+=co; c.w[7]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[0]+(long)a.w[7]*a.w[1]+(long)a.w[6]*a.w[2]+(long)a.w[5]*a.w[3]; t+=t; t+=(long)a.w[4]*a.w[4]; t+=co; c.w[8]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[1]+(long)a.w[7]*a.w[2]+(long)a.w[6]*a.w[3]+(long)a.w[5]*a.w[4]; t+=t; t+=co; c.w[9]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[2]+(long)a.w[7]*a.w[3]+(long)a.w[6]*a.w[4]; t+=t; t+=(long)a.w[5]*a.w[5]; t+=co; c.w[10]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[3]+(long)a.w[7]*a.w[4]+(long)a.w[6]*a.w[5]; t+=t; t+=co; c.w[11]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[4]+(long)a.w[7]*a.w[5]; t+=t; t+=(long)a.w[6]*a.w[6]; t+=co; c.w[12]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[5]+(long)a.w[7]*a.w[6]; t+=t; t+=co; c.w[13]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[6]; t+=t; t+=(long)a.w[7]*a.w[7]; t+=co; c.w[14]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[7]; t+=t; t+=co; c.w[15]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		t=(long)a.w[8]*a.w[8]+co; c.w[16]=(int)t&ROM.MASK; co=t>>ROM.BASEBITS;
		c.w[17]=(int)co;

		return c;
	}

/* reduce a DBIG to a BIG using the appropriate form of the modulus */
	public static BIG mod(DBIG d)
	{
		if (ROM.ROM.fieldDetails.isPseudoMersenne())
		{
			int v,tw;
			BIG t=d.split(ROM.ROM.modCurve.getModBits());
			BIG b=new BIG(d);

			v=t.pmul(ROM.ROM.fieldDetails.getMConst());
			tw=t.w[ROM.NLEN-1];
			t.w[ROM.NLEN-1]&=ROM.TMASK;
			t.inc(ROM.ROM.fieldDetails.getMConst()*((tw>>ROM.TBITS)+(v<<(ROM.BASEBITS-ROM.TBITS))));

			b.add(t);
			b.norm();
			return b;
		}
		if (ROM.ROM.fieldDetails.isMontgomeryFriendly())
		{
			for (int i=0;i<ROM.NLEN;i++)
				d.w[ROM.NLEN+i]+=d.muladd(d.w[i],ROM.ROM.fieldDetails.getMConst()-1,d.w[i],ROM.NLEN+i-1);

			BIG b=new BIG(0);

			for (int i=0;i<ROM.NLEN;i++ )
				b.w[i]=d.w[ROM.NLEN+i];
			b.norm();
			return b;
		}

		if (ROM.ROM.fieldDetails.isNotSpecial())
		{
			BIG md=new BIG(ROM.ROM.fieldDetails.getModulus());
			long sum;
			int sp;
			sum=d.w[0];
			for (int j=0;j<ROM.NLEN;j++)
			{
				for (int i=0;i<j;i++) sum+=(long)d.w[i]*md.w[j-i];
				sp=((int)sum*ROM.ROM.fieldDetails.getMConst())&ROM.MASK;
				d.w[j]=sp; sum+=(long)sp*md.w[0];
				sum=d.w[j+1]+(sum>>ROM.BASEBITS);
			}

			for (int j=ROM.NLEN;j<ROM.DNLEN-2;j++)
			{
				for (int i=j-ROM.NLEN+1;i<ROM.NLEN;i++) sum+=(long)d.w[i]*md.w[j-i];
				d.w[j]=(int)sum&ROM.MASK;
				sum=d.w[j+1]+(sum>>ROM.BASEBITS);
			}

			sum+=(long)d.w[ROM.NLEN-1]*md.w[ROM.NLEN-1];
			d.w[ROM.DNLEN-2]=(int)sum&ROM.MASK;
			sum=d.w[ROM.DNLEN-1]+(sum>>ROM.BASEBITS);
			d.w[ROM.DNLEN-1]=(int)sum&ROM.MASK;

			BIG b=new BIG(0);

			for (int i=0;i<ROM.NLEN;i++ )
				b.w[i]=d.w[ROM.NLEN+i];
			b.norm();
			return b;
		}

		return null;
	}

/* return a*b mod m */
	public static BIG modmul(BIG a,BIG b,BIG m)
	{
		a.mod(m);
		b.mod(m);
		DBIG d=mul(a,b);
		return d.mod(m);
	}

/* return a^2 mod m */
	public static BIG modsqr(BIG a,BIG m)
	{
		a.mod(m);
		DBIG d=sqr(a);
		return d.mod(m);
	}

/* return -a mod m */
	public static BIG modneg(BIG a,BIG m)
	{
		a.mod(m);
		return m.minus(a);
	}

/* return this^e mod m */
	public BIG powmod(BIG e,BIG m)
	{
		int bt;
		norm();
		e.norm();
		BIG a=new BIG(1);
		BIG z=new BIG(e);
		BIG s=new BIG(this);
		while (true)
		{
			bt=z.parity();
			z.fshr(1);
			if (bt==1) a=modmul(a,s,m);
			if (z.iszilch()) break;
			s=modsqr(s,m);
		}
		return a;
	}

/* Jacobi Symbol (this/p). Returns 0, 1 or -1 */
	public int jacobi(BIG p)
	{
		int n8,k,m=0;
		BIG t=new BIG(0);
		BIG x=new BIG(0);
		BIG n=new BIG(0);
		BIG zilch=new BIG(0);
		BIG one=new BIG(1);
		if (p.parity()==0 || comp(this,zilch)==0 || comp(p,one)<=0) return 0;
		norm();
		x.copy(this);
		n.copy(p);
		x.mod(p);

		while (comp(n,one)>0)
		{
			if (comp(x,zilch)==0) return 0;
			n8=n.lastbits(3);
			k=0;
			while (x.parity()==0)
			{
				k++;
				x.shr(1);
			}
			if (k%2==1) m+=(n8*n8-1)/8;
			m+=(n8-1)*(x.lastbits(2)-1)/4;
			t.copy(n);
			t.mod(x);
			n.copy(x);
			x.copy(t);
			m%=2;

		}
		if (m==0) return 1;
		else return -1;
	}

/* this=1/this mod p. Binary method */
	public void invmodp(BIG p)
	{
		mod(p);
		BIG u=new BIG(this);
		BIG v=new BIG(p);
		BIG x1=new BIG(1);
		BIG x2=new BIG(0);
		BIG t=new BIG(0);
		BIG one=new BIG(1);

		while (comp(u,one)!=0 && comp(v,one)!=0)
		{
			while (u.parity()==0)
			{
				u.shr(1);
				if (x1.parity()!=0)
				{
					x1.add(p);
					x1.norm();
				}
				x1.shr(1);
			}
			while (v.parity()==0)
			{
				v.shr(1);
				if (x2.parity()!=0)
				{
					x2.add(p);
					x2.norm();
				}
				x2.shr(1);
			}
			if (comp(u,v)>=0)
			{
				u.sub(v);
				u.norm();
				if (comp(x1,x2)>=0) x1.sub(x2);
				else
				{
					t.copy(p);
					t.sub(x2);
					x1.add(t);
				}
				x1.norm();
			}
			else
			{
				v.sub(u);
				v.norm();
				if (comp(x2,x1)>=0) x2.sub(x1);
				else
				{
					t.copy(p);
					t.sub(x1);
					x2.add(t);
				}
				x2.norm();
			}
		}
		if (comp(u,one)==0) copy(x1);
		else copy(x2);
	}
}
