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

/* Elliptic Curve Point class */

public final class ECP {
	private FP x;
	private FP y;
	private FP z;
	private boolean INF;

/* Constructor - set to O */
	public ECP() {
		INF=true;
		x=new FP(0);
		y=new FP(1);
		z=new FP(1);
	}
/* test for O point-at-infinity */
	public boolean is_infinity() {
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			x.reduce(); y.reduce(); z.reduce();
			return (x.iszilch() && y.equals(z));
		}
		else return INF;
	}
/* Conditional swap of P and Q dependant on d */
	private void cswap(ECP Q,int d)
	{
		x.cswap(Q.x,d);
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) y.cswap(Q.y,d);
		z.cswap(Q.z,d);
		if (ROM.CURVETYPE!=ROM.EDWARDS)
		{
			boolean bd;
			if (d==0) bd=false;
			else bd=true;
			bd=bd&(INF^Q.INF);
			INF^=bd;
			Q.INF^=bd;
		}
	}

/* Conditional move of Q to P dependant on d */
	private void cmove(ECP Q,int d)
	{
		x.cmove(Q.x,d);
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) y.cmove(Q.y,d);
		z.cmove(Q.z,d);
		if (ROM.CURVETYPE!=ROM.EDWARDS)
		{
			boolean bd;
			if (d==0) bd=false;
			else bd=true;
			INF^=(INF^Q.INF)&bd;
		}
	}

/* return 1 if b==c, no branching */
	private static int teq(int b,int c)
	{
		int x=b^c;
		x-=1;  // if x=0, x now -1
		return ((x>>31)&1);
	}

/* Constant time select from pre-computed table */
	private void select(ECP W[],int b)
	{
		ECP MP=new ECP(); 
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

/* Test P == Q */
	public boolean equals(ECP Q) {
		if (is_infinity() && Q.is_infinity()) return true;
		if (is_infinity() || Q.is_infinity()) return false;
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			FP zs2=new FP(z); zs2.sqr();
			FP zo2=new FP(Q.z); zo2.sqr();
			FP zs3=new FP(zs2); zs3.mul(z);
			FP zo3=new FP(zo2); zo3.mul(Q.z);
			zs2.mul(Q.x);
			zo2.mul(x);
			if (!zs2.equals(zo2)) return false;
			zs3.mul(Q.y);
			zo3.mul(y);
			if (!zs3.equals(zo3)) return false;
		}
		else
		{
			FP a=new FP(0);
			FP b=new FP(0);
			a.copy(x); a.mul(Q.z); a.reduce();
			b.copy(Q.x); b.mul(z); b.reduce();
			if (!a.equals(b)) return false;
			if (ROM.CURVETYPE==ROM.EDWARDS)
			{
				a.copy(y); a.mul(Q.z); a.reduce();
				b.copy(Q.y); b.mul(z); b.reduce();
				if (!a.equals(b)) return false;
			}
		}
		return true;
	}

/* this=P */
	public void copy(ECP P)
	{
		x.copy(P.x);
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) y.copy(P.y);
		z.copy(P.z);
		INF=P.INF;
	}
/* this=-this */
	public void neg() {
		if (is_infinity()) return;
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			y.neg(); y.norm();
		}
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			x.neg(); x.norm();
		}
		return;
	}
/* set this=O */
	public void inf() {
		INF=true;
		x.zero();
		y.one();
		z.one();
	//	y=new FP(1);
	//	z=new FP(1);
	}

/* Calculate RHS of curve equation */
	public static FP RHS(FP x) {
		x.norm();
		FP r=new FP(x);
		r.sqr();

		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{ // x^3+Ax+B
			FP b=new FP(new BIG(ROM.CURVE_B));
			r.mul(x);
			if (ROM.CURVE_A==-3)
			{
				FP cx=new FP(x);
				cx.imul(3);
				cx.neg(); cx.norm();
				r.add(cx);
			}
			r.add(b);
		}
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{ // (Ax^2-1)/(Bx^2-1) 
			FP b=new FP(new BIG(ROM.CURVE_B));

			FP one=new FP(1);
			b.mul(r);
			b.sub(one);
			if (ROM.CURVE_A==-1) r.neg();
			r.sub(one);

			b.inverse();

			r.mul(b);
		}
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{ // x^3+Ax^2+x
			FP x3=new FP(0);
			x3.copy(r);
			x3.mul(x);
			r.imul(ROM.CURVE_A);
			r.add(x3);
			r.add(x);
		}
		r.reduce();
		return r;
	}

/* set (x,y) from two BIGs */
	public ECP(BIG ix,BIG iy) {
		x=new FP(ix);
		y=new FP(iy);
		z=new FP(1);
		FP rhs=RHS(x);

		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{
			if (rhs.jacobi()==1) INF=false;
			else inf();
		}
		else
		{
			FP y2=new FP(y);
			y2.sqr();
			if (y2.equals(rhs)) INF=false;
			else inf();
		}
	}
/* set (x,y) from BIG and a bit */
	public ECP(BIG ix,int s) {
		x=new FP(ix);
		FP rhs=RHS(x);
		y=new FP(0);
		z=new FP(1);
		if (rhs.jacobi()==1)
		{
			FP ny=rhs.sqrt();
			if (ny.redc().parity()!=s) ny.neg();
			y.copy(ny);
			INF=false;
		}
		else inf();
	}

/* set from x - calculate y from curve equation */
	public ECP(BIG ix) {
		x=new FP(ix);
		FP rhs=RHS(x);
		y=new FP(0);
		z=new FP(1);
		if (rhs.jacobi()==1)
		{
			if (ROM.CURVETYPE!=ROM.MONTGOMERY) y.copy(rhs.sqrt());
			INF=false;
		}
		else INF=true;
	}

/* set to affine - from (x,y,z) to (x,y) */
	public void affine() {
		if (is_infinity()) return;
		FP one=new FP(1);
		if (z.equals(one)) return;
		z.inverse();
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			FP z2=new FP(z);
			z2.sqr();
			x.mul(z2); x.reduce();
			y.mul(z2); 
			y.mul(z);  y.reduce();
		}
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			x.mul(z); x.reduce();
			y.mul(z); y.reduce();
		}
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{
			x.mul(z); x.reduce();
		}
		z.copy(one);
	}
/* extract x as a BIG */
	public BIG getX()
	{
		affine();
		return x.redc();
	}
/* extract y as a BIG */
	public BIG getY()
	{
		affine();
		return y.redc();
	}

/* get sign of Y */
	public int getS()
	{
		affine();
		BIG y=getY();
		return y.parity();
	}
/* extract x as an FP */
	public FP getx()
	{
		return x;
	}
/* extract y as an FP */
	public FP gety()
	{
		return y;
	}
/* extract z as an FP */
	public FP getz()
	{
		return z;
	}
/* convert to byte array */
	public void toBytes(byte[] b)
	{
		byte[] t=new byte[ROM.MODBYTES];
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) b[0]=0x04;
		else b[0]=0x02;
	
		affine();
		x.redc().toBytes(t);
		for (int i=0;i<ROM.MODBYTES;i++) b[i+1]=t[i];
		if (ROM.CURVETYPE!=ROM.MONTGOMERY)
		{
			y.redc().toBytes(t);
			for (int i=0;i<ROM.MODBYTES;i++) b[i+ROM.MODBYTES+1]=t[i];
		}
	}
/* convert from byte array to point */
	public static ECP fromBytes(byte[] b)
	{
		byte[] t=new byte[ROM.MODBYTES];
		BIG p=new BIG(ROM.Modulus);

		for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i+1];
		BIG px=BIG.fromBytes(t);
		if (BIG.comp(px,p)>=0) return new ECP();

		if (b[0]==0x04)
		{
			for (int i=0;i<ROM.MODBYTES;i++) t[i]=b[i+ROM.MODBYTES+1];
			BIG py=BIG.fromBytes(t);
			if (BIG.comp(py,p)>=0) return new ECP();
			return new ECP(px,py);
		}
		else return new ECP(px);
	}
/* convert to hex string */
	public String toString() {
		if (is_infinity()) return "infinity";
		affine();
		if (ROM.CURVETYPE==ROM.MONTGOMERY) return "("+x.redc().toString()+")";
		else return "("+x.redc().toString()+","+y.redc().toString()+")";
	}
/* this*=2 */
	public void dbl() {
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			if (INF) return;
			if (y.iszilch())
			{
				inf();
				return;
			}

			FP w1=new FP(x);
			FP w6=new FP(z);
			FP w2=new FP(0);
			FP w3=new FP(x);
			FP w8=new FP(x);

			if (ROM.CURVE_A==-3)
			{
				w6.sqr();
				w1.copy(w6);
				w1.neg();
				w3.add(w1);
				w8.add(w6);
				w3.mul(w8);
				w8.copy(w3);
				w8.imul(3);
			}
			else
			{
				w1.sqr();
				w8.copy(w1);
				w8.imul(3);
			}

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
			y.sub(w2);
			y.norm();
			z.norm();
		}
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			FP C=new FP(x);
			FP D=new FP(y);
			FP H=new FP(z);
			FP J=new FP(0);
	
			x.mul(y); x.add(x);
			C.sqr();
			D.sqr();
			if (ROM.CURVE_A==-1) C.neg();	
			y.copy(C); y.add(D);
			y.norm();
			H.sqr(); H.add(H);
			z.copy(y);
			J.copy(y); J.sub(H);
			x.mul(J);
			C.sub(D);
			y.mul(C);
			z.mul(J);

			x.norm();
			y.norm();
			z.norm();
		}
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{
			FP A=new FP(x);
			FP B=new FP(x);		
			FP AA=new FP(0);
			FP BB=new FP(0);
			FP C=new FP(0);
	
			if (INF) return;

			A.add(z);
			AA.copy(A); AA.sqr();
			B.sub(z);
			BB.copy(B); BB.sqr();
			C.copy(AA); C.sub(BB);

			x.copy(AA); x.mul(BB);

			A.copy(C); A.imul((ROM.CURVE_A+2)/4);

			BB.add(A);
			z.copy(BB); z.mul(C);
			x.norm();
			z.norm();
		}
		return;
	}

/* this+=Q */
	public void add(ECP Q) {
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			if (INF)
			{
				copy(Q);
				return;
			}
			if (Q.INF) return;

			boolean aff=false;

			FP one=new FP(1);
			if (Q.z.equals(one)) aff=true;

			FP A,C;
			FP B=new FP(z);
			FP D=new FP(z);
			if (!aff)
			{
				A=new FP(Q.z);
				C=new FP(Q.z);

				A.sqr(); B.sqr();
				C.mul(A); D.mul(B);

				A.mul(x);
				C.mul(y);
			}
			else
			{
				A=new FP(x);
				C=new FP(y);
	
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
					return;
				}
				else
				{
					INF=true;
					return;
				}
			}

			if (!aff) z.mul(Q.z);
			z.mul(B);

			FP e=new FP(B); e.sqr();
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
		}
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			FP b=new FP(new BIG(ROM.CURVE_B));
			FP A=new FP(z);
			FP B=new FP(0);
			FP C=new FP(x);
			FP D=new FP(y);
			FP E=new FP(0);
			FP F=new FP(0);
			FP G=new FP(0);
		//	FP H=new FP(0);
		//	FP I=new FP(0);
	
			A.mul(Q.z);
			B.copy(A); B.sqr();
			C.mul(Q.x);
			D.mul(Q.y);

			E.copy(C); E.mul(D); E.mul(b);
			F.copy(B); F.sub(E); 
			G.copy(B); G.add(E); 

			if (ROM.CURVE_A==1)
			{
				E.copy(D); E.sub(C);
			}
			C.add(D);

			B.copy(x); B.add(y);
			D.copy(Q.x); D.add(Q.y); 
			B.mul(D);
			B.sub(C);
			B.mul(F);
			x.copy(A); x.mul(B);

			if (ROM.CURVE_A==1)
			{
				C.copy(E); C.mul(G);
			}
			if (ROM.CURVE_A==-1)
			{
				C.mul(G);
			}
			y.copy(A); y.mul(C);
			z.copy(F); z.mul(G);
			x.norm(); y.norm(); z.norm();
		}
		return;
	}

/* Differential Add for Montgomery curves. this+=Q where W is this-Q and is affine. */
	public void dadd(ECP Q,ECP W) {
			FP A=new FP(x);
			FP B=new FP(x);
			FP C=new FP(Q.x);
			FP D=new FP(Q.x);
			FP DA=new FP(0);
			FP CB=new FP(0);	
			
			A.add(z); 
			B.sub(z); 

			C.add(Q.z);
			D.sub(Q.z);

			DA.copy(D); DA.mul(A);
			CB.copy(C); CB.mul(B);

			A.copy(DA); A.add(CB); A.sqr();
			B.copy(DA); B.sub(CB); B.sqr();

			x.copy(A);
			z.copy(W.x); z.mul(B);

			if (z.iszilch()) inf();
			else INF=false;

			x.norm();
	}
/* this-=Q */
	public void sub(ECP Q) {
		Q.neg();
		add(Q);
		Q.neg();
	}

	public static void multiaffine(int m,ECP[] P)
	{
		int i;
		FP t1=new FP(0);
		FP t2=new FP(0);

		FP[] work=new FP[m];

		for (i=0;i<m;i++)
			work[i]=new FP(0);
	
		work[0].one();
		work[1].copy(P[0].z);

		for (i=2;i<m;i++)
		{
			work[i].copy(work[i-1]);
			work[i].mul(P[i-1].z);
		}

		t1.copy(work[m-1]);
		t1.mul(P[m-1].z);
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
			t1.copy(work[i]);
			t1.sqr();
			P[i].x.mul(t1);
			t1.mul(work[i]);
			P[i].y.mul(t1);
		}    
	}

/* constant time multiply by small integer of length bts - use ladder */
	public ECP pinmul(int e,int bts) {	
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
			return this.mul(new BIG(e));
		else
		{
			int nb,i,b;
			ECP P=new ECP();
			ECP R0=new ECP();
			ECP R1=new ECP(); R1.copy(this);

			for (i=bts-1;i>=0;i--)
			{
				b=(e>>i)&1;
				P.copy(R1);
				P.add(R0);
				R0.cswap(R1,b);
				R1.copy(P);
				R0.dbl();
				R0.cswap(R1,b);
			}
			P.copy(R0);
			P.affine();
			return P;
		}
	}

/* return e.this */

	public ECP mul(BIG e) {
		if (e.iszilch() || is_infinity()) return new ECP();
		ECP P=new ECP();
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{
/* use Ladder */
			int nb,i,b;
			ECP D=new ECP();
			ECP R0=new ECP(); R0.copy(this);
			ECP R1=new ECP(); R1.copy(this);
			R1.dbl();
			D.copy(this); D.affine();
			nb=e.nbits();
			for (i=nb-2;i>=0;i--)
			{
				b=e.bit(i);
				P.copy(R1);
				P.dadd(R0,D);
				R0.cswap(R1,b);
				R1.copy(P);
				R0.dbl();
				R0.cswap(R1,b);
			}
			P.copy(R0);
		}
		else
		{
// fixed size windows 
			int i,b,nb,m,s,ns;
			BIG mt=new BIG();
			BIG t=new BIG();
			ECP Q=new ECP();
			ECP C=new ECP();
			ECP[] W=new ECP[8];
			byte[] w=new byte[1+(ROM.NLEN*ROM.BASEBITS+3)/4];

			affine();

// precompute table 
			Q.copy(this);
			Q.dbl();
			W[0]=new ECP();
			W[0].copy(this);

			for (i=1;i<8;i++)
			{
				W[i]=new ECP();
				W[i].copy(W[i-1]);
				W[i].add(Q);
			}

// convert the table to affine 
			if (ROM.CURVETYPE==ROM.WEIERSTRASS) 
				multiaffine(8,W);

// make exponent odd - add 2P if even, P if odd 
			t.copy(e);
			s=t.parity();
			t.inc(1); t.norm(); ns=t.parity(); mt.copy(t); mt.inc(1); mt.norm();
			t.cmove(mt,s);
			Q.cmove(this,ns);
			C.copy(Q);

			nb=1+(t.nbits()+3)/4;

// convert exponent to signed 4-bit window 
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
			P.sub(C); /* apply correction */
		}
		P.affine();
		return P;
	}

/* Return e.this+f.Q */

	public ECP mul2(BIG e,ECP Q,BIG f) {
		BIG te=new BIG();
		BIG tf=new BIG();
		BIG mt=new BIG();
		ECP S=new ECP();
		ECP T=new ECP();
		ECP C=new ECP();
		ECP[] W=new ECP[8];
		byte[] w=new byte[1+(ROM.NLEN*ROM.BASEBITS+1)/2];		
		int i,s,ns,nb;
		byte a,b;

		affine();
		Q.affine();

		te.copy(e);
		tf.copy(f);

// precompute table 
		W[1]=new ECP(); W[1].copy(this); W[1].sub(Q);
		W[2]=new ECP(); W[2].copy(this); W[2].add(Q);
		S.copy(Q); S.dbl();
		W[0]=new ECP(); W[0].copy(W[1]); W[0].sub(S);
		W[3]=new ECP(); W[3].copy(W[2]); W[3].add(S);
		T.copy(this); T.dbl();
		W[5]=new ECP(); W[5].copy(W[1]); W[5].add(T);
		W[6]=new ECP(); W[6].copy(W[2]); W[6].add(T);
		W[4]=new ECP(); W[4].copy(W[5]); W[4].sub(S);
		W[7]=new ECP(); W[7].copy(W[6]); W[7].add(S);

// convert the table to affine 
		if (ROM.CURVETYPE==ROM.WEIERSTRASS) 
			multiaffine(8,W);

// if multiplier is odd, add 2, else add 1 to multiplier, and add 2P or P to correction 

		s=te.parity();
		te.inc(1); te.norm(); ns=te.parity(); mt.copy(te); mt.inc(1); mt.norm();
		te.cmove(mt,s);
		T.cmove(this,ns);
		C.copy(T);

		s=tf.parity();
		tf.inc(1); tf.norm(); ns=tf.parity(); mt.copy(tf); mt.inc(1); mt.norm();
		tf.cmove(mt,s);
		S.cmove(Q,ns);
		C.add(S);

		mt.copy(te); mt.add(tf); mt.norm();
		nb=1+(mt.nbits()+1)/2;

// convert exponent to signed 2-bit window 
		for (i=0;i<nb;i++)
		{
			a=(byte)(te.lastbits(3)-4);
			te.dec(a); te.norm(); 
			te.fshr(2);
			b=(byte)(tf.lastbits(3)-4);
			tf.dec(b); tf.norm(); 
			tf.fshr(2);
			w[i]=(byte)(4*a+b);
		}
		w[nb]=(byte)(4*te.lastbits(3)+tf.lastbits(3));
		S.copy(W[(w[nb]-1)/2]);  

		for (i=nb-1;i>=0;i--)
		{
			T.select(W,w[i]);
			S.dbl();
			S.dbl();
			S.add(T);
		}
		S.sub(C); /* apply correction */
		S.affine();
		return S;
	}

/*
	public static void main(String[] args) {

		BIG Gx=new BIG(ROM.CURVE_Gx);
		BIG Gy;
		ECP P;
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) Gy=new BIG(ROM.CURVE_Gy);
		BIG r=new BIG(ROM.CURVE_Order);

		//r.dec(7);
	
		System.out.println("Gx= "+Gx.toString());		
		if (ROM.CURVETYPE!=ROM.MONTGOMERY) System.out.println("Gy= "+Gy.toString());	

		if (ROM.CURVETYPE!=ROM.MONTGOMERY) P=new ECP(Gx,Gy);
		else  P=new ECP(Gx);

		System.out.println("P= "+P.toString());		

		ECP R=P.mul(r);
		//for (int i=0;i<10000;i++)
		//	R=P.mul(r);
	
		System.out.println("R= "+R.toString());
    } */
}

