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

/* AMCL Weierstrass elliptic curve functions over FP2 */

public final class ECP2 {
	private FP2 x;
	private FP2 y;
	private FP2 z;
	private boolean INF;

/* Constructor - set this=O */
	public ECP2() {
		INF=true;
		x=new FP2(0);
		y=new FP2(1);
		z=new FP2(1);
	}

/* Test this=O? */
	public boolean is_infinity() {
		return INF;
	}
/* copy this=P */
	public void copy(ECP2 P)
	{
		x.copy(P.x);
		y.copy(P.y);
		z.copy(P.z);
		INF=P.INF;
	}
/* set this=O */
	public void inf() {
		INF=true;
		x.zero();
		y.zero();
		z.zero();
	}

/* Conditional move of Q to P dependant on d */
	public void cmove(ECP2 Q,int d)
	{
		x.cmove(Q.x,d);
		y.cmove(Q.y,d);
		z.cmove(Q.z,d);

		boolean bd;
		if (d==0) bd=false;
		else bd=true;
		INF^=(INF^Q.INF)&bd;
	}

/* return 1 if b==c, no branching */
	public static int teq(int b,int c)
	{
		int x=b^c;
		x-=1;  // if x=0, x now -1
		return ((x>>31)&1);
	}

/* Constant time select from pre-computed table */
	public void select(ECP2 W[],int b)
	{
		ECP2 MP=new ECP2(); 
		int m=b>>31;
		int babs=(b^m)-m;

		babs=(babs-1)/2;

		cmove(W[0],teq(babs,0));  // conditional move
		cmove(W[1],teq(babs,1));
		cmove(W[2],teq(babs,2));
		cmove(W[3],teq(babs,3));
		cmove(W[4],teq(babs,4));
		cmove(W[5],teq(babs,5));
		cmove(W[6],teq(babs,6));
		cmove(W[7],teq(babs,7));
 
		MP.copy(this);
		MP.neg();
		cmove(MP,(int)(m&1));
	}

/* Test if P == Q */
	public boolean equals(ECP2 Q) {
		if (is_infinity() && Q.is_infinity()) return true;
		if (is_infinity() || Q.is_infinity()) return false;

		FP2 zs2=new FP2(z); zs2.sqr();
		FP2 zo2=new FP2(Q.z); zo2.sqr();
		FP2 zs3=new FP2(zs2); zs3.mul(z);
		FP2 zo3=new FP2(zo2); zo3.mul(Q.z);
		zs2.mul(Q.x);
		zo2.mul(x);
		if (!zs2.equals(zo2)) return false;
		zs3.mul(Q.y);
		zo3.mul(y);
		if (!zs3.equals(zo3)) return false;

		return true;
	}
/* set this=-this */
	public void neg() {
		if (is_infinity()) return;
		y.neg(); y.norm();
		return;
	}
/* set to Affine - (x,y,z) to (x,y) */
	public void affine() {
		if (is_infinity()) return;
		FP2 one=new FP2(1);
		if (z.equals(one)) return;
		z.inverse();

		FP2 z2=new FP2(z);
		z2.sqr();
		x.mul(z2); x.reduce();
		y.mul(z2); 
		y.mul(z);  y.reduce();
		z.copy(one);
	}
/* extract affine x as FP2 */
	public FP2 getX()
	{
		affine();
		return x;
	}
/* extract affine y as FP2 */
	public FP2 getY()
	{
		affine();
		return y;
	}
/* extract projective x */
	public FP2 getx()
	{
		return x;
	}
/* extract projective y */
	public FP2 gety()
	{
		return y;
	}
/* extract projective z */
	public FP2 getz()
	{
		return z;
	}
/* convert to byte array */
	public void toBytes(byte[] b)
	{
		byte[] t=new byte[ROM.MODBYTES];
		affine();
		x.getA().toBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++)
			b[i]=t[i];
		x.getB().toBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++)
			b[i+ROM.MODBYTES]=t[i];

		y.getA().toBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++)
			b[i+2*ROM.MODBYTES]=t[i];
		y.getB().toBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++)
			b[i+3*ROM.MODBYTES]=t[i];
	}
/* convert from byte array to point */
	public static ECP2 fromBytes(byte[] b)
	{
		byte[] t=new byte[ROM.MODBYTES];
		BIG ra;
		BIG rb;

		for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i];
		ra=BIG.fromBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i+ROM.MODBYTES];
		rb=BIG.fromBytes(t);
		FP2 rx=new FP2(ra,rb);

		for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i+2*ROM.MODBYTES];
		ra=BIG.fromBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i+3*ROM.MODBYTES];
		rb=BIG.fromBytes(t);
		FP2 ry=new FP2(ra,rb);

		return new ECP2(rx,ry);
	}
/* convert this to hex string */
	public String toString() {
		if (is_infinity()) return "infinity";
		affine();
		return "("+x.toString()+","+y.toString()+")";
	}

/* Calculate RHS of twisted curve equation x^3+B/i */
	public static FP2 RHS(FP2 x) {
		x.norm();
		FP2 r=new FP2(x);
		r.sqr();
		FP2 b=new FP2(new BIG(ROM.CURVE_B));
		b.div_ip();
		r.mul(x);
		r.add(b);

		r.reduce();
		return r;
	}

/* construct this from (x,y) - but set to O if not on curve */
	public ECP2(FP2 ix,FP2 iy) {
		x=new FP2(ix);
		y=new FP2(iy);
		z=new FP2(1);
		FP2 rhs=RHS(x);
		FP2 y2=new FP2(y);
		y2.sqr();
		if (y2.equals(rhs)) INF=false;
		else {x.zero();INF=true;}
	}

/* construct this from x - but set to O if not on curve */
	public ECP2(FP2 ix) {
		x=new FP2(ix);
		y=new FP2(1);
		z=new FP2(1);
		FP2 rhs=RHS(x);
		if (rhs.sqrt()) 
		{
			y.copy(rhs);
			INF=false;
		}
		else {x.zero();INF=true;}
	}

/* this+=this */
	public int dbl() {
		if (INF) return -1;
		if (y.iszilch())
		{
			inf();
			return -1;
		}

		FP2 w1=new FP2(x);
		FP2 w2=new FP2(0);
		FP2 w3=new FP2(x);
		FP2 w8=new FP2(x);

		w1.sqr();
		w8.copy(w1);
		w8.imul(3);

		w2.copy(y); w2.sqr();
		w3.copy(x); w3.mul(w2);
		w3.imul(4);
		w1.copy(w3); w1.neg();
		w1.norm();

		x.copy(w8); x.sqr();
		x.add(w1);
		x.add(w1);
		x.norm();

		z.mul(y);
		z.add(z);

		w2.add(w2);
		w2.sqr();
		w2.add(w2);
		w3.sub(x);
		y.copy(w8); y.mul(w3);
	//	w2.norm();
		y.sub(w2);

		y.norm();
		z.norm();

		return 1;
	}

/* this+=Q - return 0 for add, 1 for double, -1 for O */
	public int add(ECP2 Q) {
		if (INF)
		{
			copy(Q);
			return -1;
		}
		if (Q.INF) return -1;

		boolean aff=false;

		if (Q.z.isunity()) aff=true;

		FP2 A,C;
		FP2 B=new FP2(z);
		FP2 D=new FP2(z);
		if (!aff)
		{
			A=new FP2(Q.z);
			C=new FP2(Q.z);

			A.sqr(); B.sqr();
			C.mul(A); D.mul(B);

			A.mul(x);
			C.mul(y);
		}
		else
		{
			A=new FP2(x);
			C=new FP2(y);
	
			B.sqr();
			D.mul(B);
		}

		B.mul(Q.x); B.sub(A);
		D.mul(Q.y); D.sub(C);

		if (B.iszilch())
		{
			if (D.iszilch())
			{
				dbl();
				return 1;
			}
			else
			{
				INF=true;
				return -1;
			}
		}

		if (!aff) z.mul(Q.z);
		z.mul(B);

		FP2 e=new FP2(B); e.sqr();
		B.mul(e);
		A.mul(e);

		e.copy(A);
		e.add(A); e.add(B);
		x.copy(D); x.sqr(); x.sub(e);

		A.sub(x);
		y.copy(A); y.mul(D); 
		C.mul(B); y.sub(C);

		x.norm();
		y.norm();
		z.norm();

		return 0;
	}

/* set this-=Q */
	public int sub(ECP2 Q) {
		Q.neg();
		int D=add(Q);
		Q.neg();
		return D;
	}
/* set this*=q, where q is Modulus, using Frobenius */
	public void frob(FP2 X)
	{
		if (INF) return;
		FP2 X2=new FP2(X);
		X2.sqr();
		x.conj();
		y.conj();
		z.conj();
		z.reduce();
		x.mul(X2);
		y.mul(X2);
		y.mul(X);
	}

/* normalises m-array of ECP2 points. Requires work vector of m FP2s */

	public static void multiaffine(int m,ECP2[] P)
	{
		int i;
		FP2 t1=new FP2(0);
		FP2 t2=new FP2(0);

		FP2[] work=new FP2[m];
		work[0]=new FP2(1);
		work[1]=new FP2(P[0].z);
		for (i=2;i<m;i++)
		{
			work[i]=new FP2(work[i-1]);
			work[i].mul(P[i-1].z);
		}

		t1.copy(work[m-1]); t1.mul(P[m-1].z);

		t1.inverse();

		t2.copy(P[m-1].z);
		work[m-1].mul(t1);

		for (i=m-2;;i--)
		{
			if (i==0)
			{
				work[0].copy(t1);
				work[0].mul(t2);
				break;
			}
			work[i].mul(t2);
			work[i].mul(t1);
			t2.mul(P[i].z);
		}
/* now work[] contains inverses of all Z coordinates */

		for (i=0;i<m;i++)
		{
			P[i].z.one();
			t1.copy(work[i]); t1.sqr();
			P[i].x.mul(t1);
			t1.mul(work[i]);
			P[i].y.mul(t1);
		}    
	}

/* P*=e */
	public ECP2 mul(BIG e)
	{
/* fixed size windows */
		int i,b,nb,m,s,ns;
		BIG mt=new BIG();
		BIG t=new BIG();
		ECP2 P=new ECP2();
		ECP2 Q=new ECP2();
		ECP2 C=new ECP2();
		ECP2[] W=new ECP2[8];
		byte[] w=new byte[1+(ROM.NLEN*ROM.BASEBITS+3)/4];

		if (is_infinity()) return new ECP2();

		affine();

/* precompute table */
		Q.copy(this);
		Q.dbl();
		W[0]=new ECP2();
		W[0].copy(this);

		for (i=1;i<8;i++)
		{
			W[i]=new ECP2();
			W[i].copy(W[i-1]);
			W[i].add(Q);
		}

/* convert the table to affine */

		multiaffine(8,W);

/* make exponent odd - add 2P if even, P if odd */
		t.copy(e);
		s=t.parity();
		t.inc(1); t.norm(); ns=t.parity(); mt.copy(t); mt.inc(1); mt.norm();
		t.cmove(mt,s);
		Q.cmove(this,ns);
		C.copy(Q);

		nb=1+(t.nbits()+3)/4;
/* convert exponent to signed 4-bit window */
		for (i=0;i<nb;i++)
		{
			w[i]=(byte)(t.lastbits(5)-16);
			t.dec(w[i]); t.norm();
			t.fshr(4);	
		}
		w[nb]=(byte)t.lastbits(5);
	
		P.copy(W[(w[nb]-1)/2]);  
		for (i=nb-1;i>=0;i--)
		{
			Q.select(W,w[i]);
			P.dbl();
			P.dbl();
			P.dbl();
			P.dbl();
			P.add(Q);
		}
		P.sub(C);
		P.affine();
		return P;
	}

/* P=u0.Q0+u1*Q1+u2*Q2+u3*Q3 */
	public static ECP2 mul4(ECP2[] Q,BIG[] u)
	{
		int i,j,nb;
		int[] a=new int[4];
		ECP2 T=new ECP2();
		ECP2 C=new ECP2();
		ECP2 P=new ECP2();
		ECP2[] W=new ECP2[8];

		BIG mt=new BIG();
		BIG[] t=new BIG[4];

		byte[] w=new byte[ROM.NLEN*ROM.BASEBITS+1];

		for (i=0;i<4;i++)
		{
			t[i]=new BIG(u[i]);
			Q[i].affine();
		}

/* precompute table */

		W[0]=new ECP2(); W[0].copy(Q[0]); W[0].sub(Q[1]);
		W[1]=new ECP2(); W[1].copy(W[0]);
		W[2]=new ECP2(); W[2].copy(W[0]);
		W[3]=new ECP2(); W[3].copy(W[0]);
		W[4]=new ECP2(); W[4].copy(Q[0]); W[4].add(Q[1]);
		W[5]=new ECP2(); W[5].copy(W[4]);
		W[6]=new ECP2(); W[6].copy(W[4]);
		W[7]=new ECP2(); W[7].copy(W[4]);
		T.copy(Q[2]); T.sub(Q[3]);
		W[1].sub(T);
		W[2].add(T);
		W[5].sub(T);
		W[6].add(T);
		T.copy(Q[2]); T.add(Q[3]);
		W[0].sub(T);
		W[3].add(T);
		W[4].sub(T);
		W[7].add(T);

		multiaffine(8,W);

/* if multiplier is even add 1 to multiplier, and add P to correction */
		mt.zero(); C.inf();
		for (i=0;i<4;i++)
		{
			if (t[i].parity()==0)
			{
				t[i].inc(1); t[i].norm();
				C.add(Q[i]);
			}
			mt.add(t[i]); mt.norm();
		}

		nb=1+mt.nbits();

/* convert exponent to signed 1-bit window */
		for (j=0;j<nb;j++)
		{
			for (i=0;i<4;i++)
			{
				a[i]=(byte)(t[i].lastbits(2)-2);
				t[i].dec(a[i]); t[i].norm(); 
				t[i].fshr(1);
			}
			w[j]=(byte)(8*a[0]+4*a[1]+2*a[2]+a[3]);
		}
		w[nb]=(byte)(8*t[0].lastbits(2)+4*t[1].lastbits(2)+2*t[2].lastbits(2)+t[3].lastbits(2));

		P.copy(W[(w[nb]-1)/2]);  
		for (i=nb-1;i>=0;i--)
		{
			T.select(W,w[i]);
			P.dbl();
			P.add(T);
		}
		P.sub(C); /* apply correction */

		P.affine();
		return P;
	}

/*
	public static void main(String[] args) {
		BIG r=new BIG(ROM.Modulus);

		BIG Pxa=new BIG(ROM.CURVE_Pxa);
		BIG Pxb=new BIG(ROM.CURVE_Pxb);
		BIG Pya=new BIG(ROM.CURVE_Pya);
		BIG Pyb=new BIG(ROM.CURVE_Pyb);

		BIG Fra=new BIG(ROM.CURVE_Fra);
		BIG Frb=new BIG(ROM.CURVE_Frb);

		FP2 f=new FP2(Fra,Frb);

		FP2 Px=new FP2(Pxa,Pxb);
		FP2 Py=new FP2(Pya,Pyb);

		ECP2 P=new ECP2(Px,Py);

		System.out.println("P= "+P.toString());

		P=P.mul(r);
		System.out.println("P= "+P.toString());

		ECP2 Q=new ECP2(Px,Py);
		Q.frob(f);
		System.out.println("Q= "+Q.toString());
	} */


}