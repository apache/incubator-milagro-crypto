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

public class DBIG
{
	protected internal long[] w = new long[ROM.DNLEN];

/* Constructors */
	public DBIG(int x)
	{
		w[0] = x;
		for (int i = 1;i < ROM.DNLEN;i++)
		{
			w[i] = 0;
		}
	}

	public DBIG(DBIG x)
	{
		for (int i = 0;i < ROM.DNLEN;i++)
		{
			w[i] = x.w[i];
		}
	}

	public DBIG(BIG x)
	{
		for (int i = 0;i < ROM.NLEN - 1;i++)
		{
			w[i] = x.get(i);
		}

		w[ROM.NLEN - 1] = x.get(ROM.NLEN - 1) & ROM.MASK; // top word normalized
		w[ROM.NLEN] = x.get(ROM.NLEN - 1) >> ROM.BASEBITS;

		for (int i = ROM.NLEN + 1;i < ROM.DNLEN;i++)
		{
			w[i] = 0;
		}
	}

/* get and set digits of this */
	public virtual long get(int i)
	{
		return w[i];
	}

	public virtual void set(int i, long x)
	{
		w[i] = x;
	}

	public virtual void inc(int i, long x)
	{
		w[i] += x;
	}

/* test this=0? */
	public virtual bool iszilch()
	{
		for (int i = 0;i < ROM.DNLEN;i++)
		{
			if (w[i] != 0)
			{
				return false;
			}
		}
		return true;
	}

/* normalise this */
	public virtual void norm()
	{
		long d , carry = 0;
		for (int i = 0;i < ROM.DNLEN - 1;i++)
		{
			d = w[i] + carry;
			w[i] = d & ROM.MASK;
			carry = d >> ROM.BASEBITS;
		}
		w[ROM.DNLEN - 1] = (w[ROM.DNLEN - 1] + carry);
	}

/* shift this right by k bits */
	public virtual void shr(int k)
	{
		int n = k % ROM.BASEBITS;
		int m = k / ROM.BASEBITS;
		for (int i = 0;i < ROM.DNLEN - m - 1;i++)
		{
			w[i] = (w[m + i] >> n) | ((w[m + i + 1] << (ROM.BASEBITS - n)) & ROM.MASK);
		}
		w[ROM.DNLEN - m - 1] = w[ROM.DNLEN - 1] >> n;
		for (int i = ROM.DNLEN - m;i < ROM.DNLEN;i++)
		{
			w[i] = 0;
		}
	}

/* shift this left by k bits */
	public virtual void shl(int k)
	{
		int n = k % ROM.BASEBITS;
		int m = k / ROM.BASEBITS;

		w[ROM.DNLEN - 1] = ((w[ROM.DNLEN - 1 - m] << n)) | (w[ROM.DNLEN - m - 2]>>(ROM.BASEBITS - n));
		for (int i = ROM.DNLEN - 2;i > m;i--)
		{
			w[i] = ((w[i - m] << n) & ROM.MASK) | (w[i - m - 1]>>(ROM.BASEBITS - n));
		}
		w[m] = (w[0] << n) & ROM.MASK;
		for (int i = 0;i < m;i++)
		{
			w[i] = 0;
		}
	}

/* return number of bits in this */
	public virtual int nbits()
	{
		int bts , k = ROM.DNLEN - 1;
		long c;
		norm();
		while (w[k] == 0 && k >= 0)
		{
			k--;
		}
		if (k < 0)
		{
			return 0;
		}
		bts = ROM.BASEBITS * k;
		c = w[k];
		while (c != 0)
		{
			c /= 2;
			bts++;
		}
		return bts;
	}

/* convert this to string */
	public override string ToString()
	{
		DBIG b;
		string s = "";
		int len = nbits();
		if (len % 4 == 0)
		{
			len >>= 2; //len/=4;
		}
		else
		{
			len >>= 2;
			len++;
		}

		for (int i = len - 1;i >= 0;i--)
		{
			b = new DBIG(this);
			b.shr(i * 4);
			s += (b.w[0] & 15).ToString("x");
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
	public virtual void add(DBIG x)
	{
		for (int i = 0;i < ROM.DNLEN;i++)
		{
			w[i] += x.w[i];
		}
	}

/* this-=x */
	public virtual void sub(DBIG x)
	{
		for (int i = 0;i < ROM.DNLEN;i++)
		{
			w[i] -= x.w[i];
		}
	}

/* set this[i]+=x*y+c, and return high part */
/* This is time critical */
/* What if you knew the bottom half in advance ?? */
	public virtual long muladd(long a, long b, long c, int i)
	{
		long x0, x1, y0, y1;
		x0 = a & ROM.HMASK;
		x1 = (a >> ROM.HBITS);
		y0 = b & ROM.HMASK;
		y1 = (b >> ROM.HBITS);
		long bot = x0 * y0;
		long top = x1 * y1;
		long mid = x0 * y1 + x1 * y0;
		x0 = mid & ROM.HMASK;
		x1 = (mid >> ROM.HBITS);
		bot += x0 << ROM.HBITS;
		bot += c;
		bot += w[i];
		top += x1;
		long carry = bot >> ROM.BASEBITS;
		bot &= ROM.MASK;
		top += carry;
		w[i] = bot;
		return top;
	}

/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
	public static int comp(DBIG a, DBIG b)
	{
		for (int i = ROM.DNLEN - 1;i >= 0;i--)
		{
			if (a.w[i] == b.w[i])
			{
				continue;
			}
			if (a.w[i] > b.w[i])
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
		return 0;
	}

/* reduces this DBIG mod a BIG, and returns the BIG */
	public virtual BIG mod(BIG c)
	{
		int k = 0;
		norm();
		DBIG m = new DBIG(c);

		if (comp(this,m) < 0)
		{
			return new BIG(this);
		}

		do
		{
			m.shl(1);
			k++;
		} while (comp(this,m) >= 0);

		while (k > 0)
		{
			m.shr(1);
			if (comp(this,m) >= 0)
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
	public virtual BIG div(BIG c)
	{
		int k = 0;
		DBIG m = new DBIG(c);
		BIG a = new BIG(0);
		BIG e = new BIG(1);
		norm();

		while (comp(this,m) >= 0)
		{
			e.fshl(1);
			m.shl(1);
			k++;
		}

		while (k > 0)
		{
			m.shr(1);
			e.shr(1);
			if (comp(this,m) > 0)
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
	public virtual BIG Split(int n)
	{
		BIG t = new BIG(0);
		int m = n % ROM.BASEBITS;
		long nw , carry = w[ROM.DNLEN - 1] << (ROM.BASEBITS - m);

		for (int i = ROM.DNLEN - 2;i >= ROM.NLEN - 1;i--)
		{
			nw = (w[i] >> m) | carry;
			carry = (w[i] << (ROM.BASEBITS - m)) & ROM.MASK;
			t.set(i - ROM.NLEN + 1,nw);
		}
		w[ROM.NLEN - 1] &= (((long)1 << m) - 1);
		return t;
	}
}
