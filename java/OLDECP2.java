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

package org.apache.milagro.amcl.XXX;

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
		z=new FP2(0);
	}

/* Test this=O? */
	public boolean is_infinity() {
		if (INF) return true;                    //******
		return (x.iszilch() && z.iszilch());
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
		y.one();
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


		FP2 a=new FP2(x);                            // *****
		FP2 b=new FP2(Q.x);
		a.mul(Q.z); 
		b.mul(z); 
		if (!a.equals(b)) return false;

		a.copy(y); a.mul(Q.z); 
		b.copy(Q.y); b.mul(z); 
		if (!a.equals(b)) return false;

		return true;
	}
/* set this=-this */
	public void neg() {
//		if (is_infinity()) return;
		y.norm();
		y.neg(); y.norm();
		return;
	}
/* set to Affine - (x,y,z) to (x,y) */
	public void affine() {
		if (is_infinity()) return;
		FP2 one=new FP2(1);
		if (z.equals(one))
		{
			x.reduce();
			y.reduce();
			return;
		}
		z.inverse();

		x.mul(z); x.reduce();               // *****
		y.mul(z); y.reduce();
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
		byte[] t=new byte[BIG.MODBYTES];
		affine();
		x.getA().toBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++)
			b[i]=t[i];
		x.getB().toBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++)
			b[i+BIG.MODBYTES]=t[i];

		y.getA().toBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++)
			b[i+2*BIG.MODBYTES]=t[i];
		y.getB().toBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++)
			b[i+3*BIG.MODBYTES]=t[i];
	}
/* convert from byte array to point */
	public static ECP2 fromBytes(byte[] b)
	{
		byte[] t=new byte[BIG.MODBYTES];
		BIG ra;
		BIG rb;

		for (int i=0;i<BIG.MODBYTES;i++) t[i]=b[i];
		ra=BIG.fromBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++) t[i]=b[i+BIG.MODBYTES];
		rb=BIG.fromBytes(t);
		FP2 rx=new FP2(ra,rb);

		for (int i=0;i<BIG.MODBYTES;i++) t[i]=b[i+2*BIG.MODBYTES];
		ra=BIG.fromBytes(t);
		for (int i=0;i<BIG.MODBYTES;i++) t[i]=b[i+3*BIG.MODBYTES];
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

		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{
			b.div_ip();
		}
		if (ECP.SEXTIC_TWIST==ECP.M_TYPE)
		{
			b.norm();
			b.mul_ip();
			b.norm();
		}


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
//System.out.println("Into dbl");
		FP2 iy=new FP2(y);
		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{
			iy.mul_ip(); iy.norm();
		}
		FP2 t0=new FP2(y);                  //***** Change 
		t0.sqr();            
		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{		
			t0.mul_ip();
		}
		FP2 t1=new FP2(iy);  
		t1.mul(z);
		FP2 t2=new FP2(z);
		t2.sqr();

		z.copy(t0);
		z.add(t0); z.norm(); 
		z.add(z); 
		z.add(z); 
		z.norm();  

		t2.imul(3*ROM.CURVE_B_I); 
		if (ECP.SEXTIC_TWIST==ECP.M_TYPE)
		{
			t2.mul_ip();
			t2.norm();
		}

		FP2 x3=new FP2(t2);
		x3.mul(z); 

		FP2 y3=new FP2(t0);   

		y3.add(t2); y3.norm();
		z.mul(t1);
		t1.copy(t2); t1.add(t2); t2.add(t1); t2.norm();  
		t0.sub(t2); t0.norm();                           //y^2-9bz^2
		y3.mul(t0); y3.add(x3);                          //(y^2+3z*2)(y^2-9z^2)+3b.z^2.8y^2
		t1.copy(x); t1.mul(iy);						//
		x.copy(t0); x.norm(); x.mul(t1); x.add(x);       //(y^2-9bz^2)xy2

		x.norm(); 
		y.copy(y3); y.norm();
//System.out.println("Out of dbl");
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
//System.out.println("Into add");
		int b=3*ROM.CURVE_B_I;
		FP2 t0=new FP2(x);
		t0.mul(Q.x);         // x.Q.x
		FP2 t1=new FP2(y);
		t1.mul(Q.y);		 // y.Q.y

		FP2 t2=new FP2(z);
		t2.mul(Q.z);
		FP2 t3=new FP2(x);
		t3.add(y); t3.norm();          //t3=X1+Y1
		FP2 t4=new FP2(Q.x);            
		t4.add(Q.y); t4.norm();			//t4=X2+Y2
		t3.mul(t4);						//t3=(X1+Y1)(X2+Y2)
		t4.copy(t0); t4.add(t1);		//t4=X1.X2+Y1.Y2

		t3.sub(t4); t3.norm(); 
		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{		
			t3.mul_ip();  t3.norm();         //t3=(X1+Y1)(X2+Y2)-(X1.X2+Y1.Y2) = X1.Y2+X2.Y1
		}
		t4.copy(y);                    
		t4.add(z); t4.norm();			//t4=Y1+Z1
		FP2 x3=new FP2(Q.y);
		x3.add(Q.z); x3.norm();			//x3=Y2+Z2

		t4.mul(x3);						//t4=(Y1+Z1)(Y2+Z2)
		x3.copy(t1);					//
		x3.add(t2);						//X3=Y1.Y2+Z1.Z2
	
		t4.sub(x3); t4.norm(); 
		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{	
			t4.mul_ip(); t4.norm();          //t4=(Y1+Z1)(Y2+Z2) - (Y1.Y2+Z1.Z2) = Y1.Z2+Y2.Z1
		}
		x3.copy(x); x3.add(z); x3.norm();	// x3=X1+Z1
		FP2 y3=new FP2(Q.x);				
		y3.add(Q.z); y3.norm();				// y3=X2+Z2
		x3.mul(y3);							// x3=(X1+Z1)(X2+Z2)
		y3.copy(t0);
		y3.add(t2);							// y3=X1.X2+Z1+Z2
		y3.rsub(x3); y3.norm();				// y3=(X1+Z1)(X2+Z2) - (X1.X2+Z1.Z2) = X1.Z2+X2.Z1

		if (ECP.SEXTIC_TWIST==ECP.D_TYPE)
		{
			t0.mul_ip(); t0.norm(); // x.Q.x
			t1.mul_ip(); t1.norm(); // y.Q.y
		}
		x3.copy(t0); x3.add(t0); 
		t0.add(x3); t0.norm();
		t2.imul(b); 	
		if (ECP.SEXTIC_TWIST==ECP.M_TYPE)
		{
			t2.mul_ip();
		}
		FP2 z3=new FP2(t1); z3.add(t2); z3.norm();
		t1.sub(t2); t1.norm(); 
		y3.imul(b); 
		if (ECP.SEXTIC_TWIST==ECP.M_TYPE)
		{
			y3.mul_ip(); 
			y3.norm();
		}
		x3.copy(y3); x3.mul(t4); t2.copy(t3); t2.mul(t1); x3.rsub(t2);
		y3.mul(t0); t1.mul(z3); y3.add(t1);
		t0.mul(t3); z3.mul(t4); z3.add(t0);

		x.copy(x3); x.norm(); 
		y.copy(y3); y.norm();
		z.copy(z3); z.norm();
//System.out.println("Out of add");
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
		byte[] w=new byte[1+(BIG.NLEN*BIG.BASEBITS+3)/4];

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
// Bos & Costello https://eprint.iacr.org/2013/458.pdf
// Faz-Hernandez & Longa & Sanchez  https://eprint.iacr.org/2013/158.pdf
// Side channel attack secure 

	public static ECP2 mul4(ECP2[] Q,BIG[] u)
	{
		int i,j,nb,pb;
		ECP2 W=new ECP2();
		ECP2 P=new ECP2();
		ECP2[] T=new ECP2[8];

		BIG mt=new BIG();
		BIG[] t=new BIG[4];

		byte[] w=new byte[BIG.NLEN*BIG.BASEBITS+1];
		byte[] s=new byte[BIG.NLEN*BIG.BASEBITS+1];

		for (i=0;i<4;i++)
		{
			t[i]=new BIG(u[i]);
			Q[i].affine();
		}

        T[0] = new ECP2(); T[0].copy(Q[0]);  // Q[0]
        T[1] = new ECP2(); T[1].copy(T[0]); T[1].add(Q[1]);  // Q[0]+Q[1]
        T[2] = new ECP2(); T[2].copy(T[0]); T[2].add(Q[2]);  // Q[0]+Q[2]
        T[3] = new ECP2(); T[3].copy(T[1]); T[3].add(Q[2]);  // Q[0]+Q[1]+Q[2]
        T[4] = new ECP2(); T[4].copy(T[0]); T[4].add(Q[3]);  // Q[0]+Q[3]
        T[5] = new ECP2(); T[5].copy(T[1]); T[5].add(Q[3]);  // Q[0]+Q[1]+Q[3]
        T[6] = new ECP2(); T[6].copy(T[2]); T[6].add(Q[3]);  // Q[0]+Q[2]+Q[3]
        T[7] = new ECP2(); T[7].copy(T[3]); T[7].add(Q[3]);  // Q[0]+Q[1]+Q[2]+Q[3]

    // Make it odd
        pb=1-t[0].parity();
        t[0].inc(pb);
        t[0].norm();

    // Number of bits
        mt.zero();
        for (i=0;i<4;i++) {
            mt.add(t[i]); mt.norm();
        }
        nb=1+mt.nbits();

    // Sign pivot 
        s[nb-1]=1;
        for (i=0;i<nb-1;i++) {
            t[0].fshr(1);
            s[i]=(byte)(2*t[0].parity()-1);
        }

    // Recoded exponent
        for (i=0; i<nb; i++) {
            w[i]=0;
            int k=1;
            for (j=1; j<4; j++) {
                byte bt=(byte)(s[i]*t[j].parity());
                t[j].fshr(1);
                t[j].dec((int)(bt)>>1);
                t[j].norm();
                w[i]+=bt*(byte)k;
                k*=2;
            }
        } 

    // Main loop
        P.select(T,(int)(2*w[nb-1]+1));  
        for (i=nb-2;i>=0;i--) {
            P.dbl();
            W.select(T,(int)(2*w[i]+s[i]));
            P.add(W);
        }

    // apply correction
        W.copy(P);   
        W.sub(Q[0]);
        P.cmove(W,pb);   
		P.affine();
		return P;
	}        


/* P=u0.Q0+u1*Q1+u2*Q2+u3*Q3 */
/*
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

		byte[] w=new byte[BIG.NLEN*BIG.BASEBITS+1];

		for (i=0;i<4;i++)
		{
			t[i]=new BIG(u[i]);
			Q[i].affine();
		}

// precompute table 

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

// if multiplier is even add 1 to multiplier, and add P to correction 
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

// convert exponent to signed 1-bit window 
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
		P.sub(C); // apply correction 

		P.affine();
		return P;
	}
*/

/* needed for SOK */
	public static ECP2 mapit(byte[] h)
	{
		BIG q=new BIG(ROM.Modulus);
		BIG x=BIG.fromBytes(h);
		BIG one=new BIG(1);
		FP2 X;
		ECP2 Q;
		x.mod(q);
		while (true)
		{
			X=new FP2(one,x);
			Q=new ECP2(X);
			if (!Q.is_infinity()) break;
			x.inc(1); x.norm();
		}

		BIG Fra=new BIG(ROM.Fra);
		BIG Frb=new BIG(ROM.Frb);
		X=new FP2(Fra,Frb);

		if (ECP.SEXTIC_TWIST==ECP.M_TYPE)
		{
			X.inverse();
			X.norm();
		}

		x=new BIG(ROM.CURVE_Bnx);

/* Fast Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez */

		if (ECP.CURVE_PAIRING_TYPE==ECP.BN)
		{
			ECP2 T,K;

			T=new ECP2(); T.copy(Q);
			T=T.mul(x); 
			
			if (ECP.SIGN_OF_X==ECP.NEGATIVEX)
			{
				T.neg();
			}	
			K=new ECP2(); K.copy(T);
			K.dbl(); K.add(T); //K.affine();

			K.frob(X);
			Q.frob(X); Q.frob(X); Q.frob(X);
			Q.add(T); Q.add(K);
			T.frob(X); T.frob(X);
			Q.add(T);

		}

/* Efficient hash maps to G2 on BLS curves - Budroni, Pintore */
/* Q -> x2Q -xQ -Q +F(xQ -Q) +F(F(2Q)) */

		if (ECP.CURVE_PAIRING_TYPE==ECP.BLS)
		{
			ECP2 xQ,x2Q;
			xQ=new ECP2();
			x2Q=new ECP2();

			xQ=Q.mul(x);
			x2Q=xQ.mul(x);

			if (ECP.SIGN_OF_X==ECP.NEGATIVEX)
			{
				xQ.neg();
			}	

			x2Q.sub(xQ);
			x2Q.sub(Q);

			xQ.sub(Q);
			xQ.frob(X);

			Q.dbl();
			Q.frob(X);
			Q.frob(X);

			Q.add(x2Q);
			Q.add(xQ);
		}
		Q.affine();
		return Q;
	}

	public static ECP2 generator()
	{
		return new ECP2(new FP2(new BIG(ROM.CURVE_Pxa),new BIG(ROM.CURVE_Pxb)),new FP2(new BIG(ROM.CURVE_Pya),new BIG(ROM.CURVE_Pyb)));
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