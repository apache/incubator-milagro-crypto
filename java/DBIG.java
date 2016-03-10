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

/* AMCL double length DBIG number class */

public class DBIG {
	protected int[] w=new int[ROM.DNLEN];

/* Constructors */
	public DBIG(int x)
	{
		w[0]=x;
		for (int i=1;i<ROM.DNLEN;i++)
			w[i]=0;
	}

	public DBIG(DBIG x)
	{
		for (int i=0;i<ROM.DNLEN;i++)
			w[i]=x.w[i];
	}

	public DBIG(BIG x)
	{
		for (int i=0;i<ROM.NLEN-1;i++)
			w[i]=x.get(i);

		w[ROM.NLEN-1]=x.get(ROM.NLEN-1)&ROM.MASK; /* top word normalized */
		w[ROM.NLEN]=x.get(ROM.NLEN-1)>>ROM.BASEBITS;

		for (int i=ROM.NLEN+1;i<ROM.DNLEN;i++) w[i]=0;
	}

/* get and set digits of this */
	public int get(int i)
	{
		return w[i];
	}

	public void set(int i,int x)
	{
		w[i]=x;
	}

/* test this=0? */
	public boolean iszilch() {
		for (int i=0;i<ROM.DNLEN;i++)
			if (w[i]!=0) return false;
		return true;
	}

/* normalise this */
	public void norm() {
		int d,carry=0;
		for (int i=0;i<ROM.DNLEN-1;i++)
		{
			d=w[i]+carry;
			w[i]=d&ROM.MASK;
			carry=d>>ROM.BASEBITS;
		}
		w[ROM.DNLEN-1]=(w[ROM.DNLEN-1]+carry);
	}

/* shift this right by k bits */
	public void shr(int k) {
		int n=k%ROM.BASEBITS;
		int m=k/ROM.BASEBITS;
		for (int i=0;i<ROM.DNLEN-m-1;i++)
			w[i]=(w[m+i]>>n)|((w[m+i+1]<<(ROM.BASEBITS-n))&ROM.MASK);
		w[ROM.DNLEN-m-1]=w[ROM.DNLEN-1]>>n;
		for (int i=ROM.DNLEN-m;i<ROM.DNLEN;i++) w[i]=0;
	}

/* shift this left by k bits */
	public void shl(int k) {
		int n=k%ROM.BASEBITS;
		int m=k/ROM.BASEBITS;

		w[ROM.DNLEN-1]=((w[ROM.DNLEN-1-m]<<n))|(w[ROM.DNLEN-m-2]>>(ROM.BASEBITS-n));
		for (int i=ROM.DNLEN-2;i>m;i--)
			w[i]=((w[i-m]<<n)&ROM.MASK)|(w[i-m-1]>>(ROM.BASEBITS-n));
		w[m]=(w[0]<<n)&ROM.MASK;
		for (int i=0;i<m;i++) w[i]=0;
	}

/* return number of bits in this */
	public int nbits() {
		int bts,k=ROM.DNLEN-1;
		int c;
		norm();
		while (w[k]==0 && k>=0) k--;
		if (k<0) return 0;
		bts=ROM.BASEBITS*k;
		c=w[k];
		while (c!=0) {c/=2; bts++;}
		return bts;
	}

/* convert this to string */
	public String toString() {
		DBIG b;
		String s="";
		int len=nbits();
		if (len%4==0) len>>=2; //len/=4;
		else {len>>=2; len++;}

		for (int i=len-1;i>=0;i--)
		{
			b=new DBIG(this);
			b.shr(i*4);
			s+=Integer.toHexString(b.w[0]&15);
		}
		return s;
	}

/* return this+x */
/*
	public DBIG plus(DBIG x) {
		DBIG s=new DBIG(0);
		for (int i=0;i<ROM.DNLEN;i++)
			s.w[i]=w[i]+x.w[i];
		return s;
	}
*/
/* return this-x */
/*
	public DBIG minus(DBIG x) {
		DBIG d=new DBIG(0);
		for (int i=0;i<ROM.DNLEN;i++)
			d.w[i]=w[i]-x.w[i];
		return d;
	}
*/
/* this+=x */
	public void add(DBIG x) {
		for (int i=0;i<ROM.DNLEN;i++)
			w[i]+=x.w[i];
	}

/* this-=x */
	public void sub(DBIG x) {
		for (int i=0;i<ROM.DNLEN;i++)
			w[i]-=x.w[i];
	}

/* set this[i]+=x*y+c, and return high part */
	public int muladd(int x,int y,int c,int i)
	{
		long prod=(long)x*y+c+w[i];
		w[i]=(int)prod&ROM.MASK;
		return (int)(prod>>ROM.BASEBITS);
	}

/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
	public static int comp(DBIG a,DBIG b)
	{
		for (int i=ROM.DNLEN-1;i>=0;i--)
		{
			if (a.w[i]==b.w[i]) continue;
			if (a.w[i]>b.w[i]) return 1;
			else  return -1;
		}
		return 0;
	}

/* reduces this DBIG mod a BIG, and returns the BIG */
	public BIG mod(BIG c)
	{
		int k=0;
		norm();
		DBIG m=new DBIG(c);

		if (comp(this,m)<0) return new BIG(this);

		do
		{
			m.shl(1);
			k++;
		}
		while (comp(this,m)>=0);

		while (k>0)
		{
			m.shr(1);
			if (comp(this,m)>=0)
			{
				sub(m);
				norm();
			}
			k--;
		}
		return new BIG(this);
	}

/* reduces this DBIG mod a DBIG in place */
/*	public void mod(DBIG m)
	{
		int k=0;
		if (comp(this,m)<0) return;

		do
		{
			m.shl(1);
			k++;
		}
		while (comp(this,m)>=0);

		while (k>0)
		{
			m.shr(1);
			if (comp(this,m)>=0)
			{
				sub(m);
				norm();
			}
			k--;
		}
		return;

	}*/

/* return this/c */
	public BIG div(BIG c)
	{
		int k=0;
		DBIG m=new DBIG(c);
		BIG a=new BIG(0);
		BIG e=new BIG(1);
		norm();

		while (comp(this,m)>=0)
		{
			e.fshl(1);
			m.shl(1);
			k++;
		}

		while (k>0)
		{
			m.shr(1);
			e.shr(1);
			if (comp(this,m)>0)
			{
				a.add(e);
				a.norm();
				sub(m);
				norm();
			}
			k--;
		}
		return a;
	}

/* split DBIG at position n, return higher half, keep lower half */
	public BIG split(int n)
	{
		BIG t=new BIG(0);
		int nw,m=n%ROM.BASEBITS;
		int carry=w[ROM.DNLEN-1]<<(ROM.BASEBITS-m);

		for (int i=ROM.DNLEN-2;i>=ROM.NLEN-1;i--)
		{
			nw=(w[i]>>m)|carry;
			carry=(w[i]<<(ROM.BASEBITS-m))&ROM.MASK;
			t.set(i-ROM.NLEN+1,nw);
		}
		w[ROM.NLEN-1]&=(((int)1<<m)-1);
		return t;
	}
}
