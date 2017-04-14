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

/* Finite Field arithmetic  Fp^2 functions */

/* FP2 elements are of the form a+ib, where i is sqrt(-1) */

public final class FP2 {
	private final FP a;
	private final FP b;

/* reduce components mod Modulus */
	public void reduce()
	{
		a.reduce();
		b.reduce();
	}

/* normalise components of w */
	public void norm()
	{
		a.norm();
		b.norm();
	}

/* test this=0 ? */
	public boolean iszilch() {
		reduce();
		return (a.iszilch() && b.iszilch());
	}

	public void cmove(FP2 g,int d)
	{
		a.cmove(g.a,d);
		b.cmove(g.b,d);
	}

/* test this=1 ? */
	public boolean isunity() {
		FP one=new FP(1);
		return (a.equals(one) && b.iszilch());
	}

/* test this=x */
	public boolean equals(FP2 x) {
		return (a.equals(x.a) && b.equals(x.b));
	}

/* Constructors */
	public FP2(int c)
	{
		a=new FP(c);
		b=new FP(0);
	}

	public FP2(FP2 x)
	{
		a=new FP(x.a);
		b=new FP(x.b);
	}

	public FP2(FP c,FP d)
	{
		a=new FP(c);
		b=new FP(d);
	}

	public FP2(BIG c,BIG d)
	{
		a=new FP(c);
		b=new FP(d);
	}

	public FP2(FP c)
	{
		a=new FP(c);
		b=new FP(0);
	}

	public FP2(BIG c)
	{
		a=new FP(c);
		b=new FP(0);
	}

/* extract a */
	public BIG getA()
	{
		return a.redc();
	}

/* extract b */
	public BIG getB()
	{
		return b.redc();
	}

/* copy this=x */
	public void copy(FP2 x)
	{
		a.copy(x.a);
		b.copy(x.b);
	}

/* set this=0 */
	public void zero()
	{
		a.zero();
		b.zero();
	}

/* set this=1 */
	public void one()
	{
		a.one();
		b.zero();
	}

/* negate this mod Modulus */
	public void neg()
	{
		norm();
		FP m=new FP(a);
		FP t=new FP(0);

		m.add(b);
		m.neg();
		m.norm();
		t.copy(m); t.add(b);
		b.copy(m);
		b.add(a);
		a.copy(t);
	}

/* set to a-ib */
	public void conj()
	{
		b.neg();
	}

/* this+=a */
	public void add(FP2 x)
	{
		a.add(x.a);
		b.add(x.b);
	}

/* this-=a */
	public void sub(FP2 x)
	{
		FP2 m=new FP2(x);
		m.neg();
		add(m);
	}

/* this*=s, where s is an FP */
	public void pmul(FP s)
	{
		a.mul(s);
		b.mul(s);
	}

/* this*=i, where i is an int */
	public void imul(int c)
	{
		a.imul(c);
		b.imul(c);
	}

/* this*=this */
	public void sqr()
	{
		norm();

		FP w1=new FP(a);
		FP w3=new FP(a);
		FP mb=new FP(b);
		w3.mul(b);
		w1.add(b);
		mb.neg();
		a.add(mb);
		a.mul(w1);
		b.copy(w3); b.add(w3);
		norm();
	}

/* this*=y */
	public void mul(FP2 y)
	{
		norm();  /* This is needed here as {a,b} is not normed before additions */

		FP w1=new FP(a);
		FP w2=new FP(b);
		FP w5=new FP(a);
		FP mw=new FP(0);

		w1.mul(y.a);  // w1=a*y.a  - this norms w1 and y.a, NOT a
		w2.mul(y.b);  // w2=b*y.b  - this norms w2 and y.b, NOT b
		w5.add(b);    // w5=a+b
		b.copy(y.a); b.add(y.b); // b=y.a+y.b

		b.mul(w5);
		mw.copy(w1); mw.add(w2); mw.neg();

		b.add(mw); mw.add(w1);
		a.copy(w1);	a.add(mw);

		norm();

	}

/* sqrt(a+ib) = sqrt(a+sqrt(a*a-n*b*b)/2)+ib/(2*sqrt(a+sqrt(a*a-n*b*b)/2)) */
/* returns true if this is QR */
	public boolean sqrt()
	{
		if (iszilch()) return true;
		FP w1=new FP(b);
		FP w2=new FP(a);
		w1.sqr(); w2.sqr(); w1.add(w2);
		if (w1.jacobi()!=1) { zero(); return false; }
		w1=w1.sqrt();
		w2.copy(a); w2.add(w1); w2.div2();
		if (w2.jacobi()!=1)
		{
			w2.copy(a); w2.sub(w1); w2.div2();
			if (w2.jacobi()!=1) { zero(); return false; }
		}
		w2=w2.sqrt();
		a.copy(w2);
		w2.add(w2);
		w2.inverse();
		b.mul(w2);
		return true;
	}

/* output to hex string */
	public String toString()
	{
		return ("["+a.toString()+","+b.toString()+"]");
	}

	public String toRawString()
	{
		return ("["+a.toRawString()+","+b.toRawString()+"]");
	}

/* this=1/this */
	public void inverse()
	{
		norm();
		FP w1=new FP(a);
		FP w2=new FP(b);

		w1.sqr();
		w2.sqr();
		w1.add(w2);
		w1.inverse();
		a.mul(w1);
		w1.neg();
		b.mul(w1);
	}

/* this/=2 */
	public void div2()
	{
		a.div2();
		b.div2();
	}

/* this*=sqrt(-1) */
	public void times_i()
	{
	//	a.norm();
		FP z=new FP(a);
		a.copy(b); a.neg();
		b.copy(z);
	}

/* w*=(1+sqrt(-1)) */
/* where X*2-(1+sqrt(-1)) is irreducible for FP4, assumes p=3 mod 8 */
	public void mul_ip()
	{
		norm();
		FP2 t=new FP2(this);
		FP z=new FP(a);
		a.copy(b);
		a.neg();
		b.copy(z);
		add(t);
		norm();
	}

/* w/=(1+sqrt(-1)) */
	public void div_ip()
	{
		FP2 t=new FP2(0);
		norm();
		t.a.copy(a); t.a.add(b);
		t.b.copy(b); t.b.sub(a);
		copy(t);
		div2();
	}
/*
	public FP2 pow(BIG e)
	{
		int bt;
		FP2 r=new FP2(1);
		e.norm();
		norm();
		while (true)
		{
			bt=e.parity();
			e.fshr(1);
			if (bt==1) r.mul(this);
			if (e.iszilch()) break;
			sqr();
		}

		r.reduce();
		return r;
	}

	public static void main(String[] args) {
		BIG m=new BIG(ROM.Modulus);
		BIG x=new BIG(3);
		BIG e=new BIG(27);
		BIG pp1=new BIG(m);
		BIG pm1=new BIG(m);
		BIG a=new BIG(1);
		BIG b=new BIG(1);
		FP2 w=new FP2(a,b);
		FP2 z=new FP2(w);

		byte[] RAW=new byte[100];

		RAND rng=new RAND();
		for (int i=0;i<100;i++) RAW[i]=(byte)(i);

		rng.seed(100,RAW);

	//	for (int i=0;i<100;i++)
	//	{
			a.randomnum(rng);
			b.randomnum(rng);

			w=new FP2(a,b);
			System.out.println("w="+w.toString());

			z=new FP2(w);
			z.inverse();
			System.out.println("z="+z.toString());

			z.inverse();
			if (!z.equals(w)) System.out.println("Error");
	//	}

//		System.out.println("m="+m.toString());
//		w.sqr();
//		w.mul(z);

		System.out.println("w="+w.toString());


		pp1.inc(1); pp1.norm();
		pm1.dec(1); pm1.norm();
		System.out.println("p+1="+pp1.toString());
		System.out.println("p-1="+pm1.toString());
		w=w.pow(pp1);
		w=w.pow(pm1);
		System.out.println("w="+w.toString());
	}
*/
}
