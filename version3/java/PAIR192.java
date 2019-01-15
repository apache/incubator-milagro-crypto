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

/* AMCL BN Curve Pairing functions */

package org.apache.milagro.amcl.XXX;

public final class PAIR192 {

//	public static final boolean GT_STRONG=false;


/* Line function */
	public static FP24 line(ECP4 A,ECP4 B,FP Qx,FP Qy)
	{
		FP8 a,b,c;                            
		if (A==B)
		{ // Doubling
			FP4 XX=new FP4(A.getx());  //X
			FP4 YY=new FP4(A.gety());  //Y
			FP4 ZZ=new FP4(A.getz());  //Z
			FP4 YZ=new FP4(YY);        //Y 
			YZ.mul(ZZ);                //YZ
			XX.sqr();	               //X^2
			YY.sqr();	               //Y^2
			ZZ.sqr();			       //Z^2
			
			YZ.imul(4);
			YZ.neg(); YZ.norm();       //-2YZ
			YZ.qmul(Qy);               //-2YZ.Ys

			XX.imul(6);                //3X^2
			XX.qmul(Qx);               //3X^2.Xs

			int sb=3*ROM.CURVE_B_I;
			ZZ.imul(sb); 	
			
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.D_TYPE)
			{
				ZZ.div_2i();
			}
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				ZZ.times_i();
				ZZ.add(ZZ);
				YZ.times_i();
				YZ.norm();
			}
			
			ZZ.norm(); // 3b.Z^2 

			YY.add(YY);
			ZZ.sub(YY); ZZ.norm();     // 3b.Z^2-Y^2

			a=new FP8(YZ,ZZ);          // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs 
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.D_TYPE)
			{			
				b=new FP8(XX);             // L(0,1) | L(0,0) | L(1,0)
				c=new FP8(0);
			}
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				b=new FP8(0);
				c=new FP8(XX); c.times_i();
			}
			A.dbl();
		}
		else
		{ // Addition - assume B is affine

			FP4 X1=new FP4(A.getx());    // X1
			FP4 Y1=new FP4(A.gety());    // Y1
			FP4 T1=new FP4(A.getz());    // Z1
			FP4 T2=new FP4(A.getz());    // Z1
			
			T1.mul(B.gety());    // T1=Z1.Y2 
			T2.mul(B.getx());    // T2=Z1.X2

			X1.sub(T2); X1.norm();  // X1=X1-Z1.X2
			Y1.sub(T1); Y1.norm();  // Y1=Y1-Z1.Y2

			T1.copy(X1);            // T1=X1-Z1.X2
			X1.qmul(Qy);            // X1=(X1-Z1.X2).Ys

			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				X1.times_i();
				X1.norm();
			}

			T1.mul(B.gety());       // T1=(X1-Z1.X2).Y2

			T2.copy(Y1);            // T2=Y1-Z1.Y2
			T2.mul(B.getx());       // T2=(Y1-Z1.Y2).X2
			T2.sub(T1); T2.norm();          // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
			Y1.qmul(Qx);  Y1.neg(); Y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

			a=new FP8(X1,T2);       // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.D_TYPE)
			{
				b=new FP8(Y1);
				c=new FP8(0);
			}
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				b=new FP8(0);
				c=new FP8(Y1); c.times_i();
			}
			A.add(B);
		}
		return new FP24(a,b,c);
	}

/* Optimal R-ate pairing */
	public static FP24 ate(ECP4 P1,ECP Q1)
	{
		FP2 f;
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG n=new BIG(x);
		FP24 lv;
		int bt;
		
		ECP4 P=new ECP4(P1);
		ECP Q=new ECP(Q1);

		P.affine();
		Q.affine();

		BIG n3=new BIG(n);
		n3.pmul(3);
		n3.norm();

		FP Qx=new FP(Q.getx());
		FP Qy=new FP(Q.gety());

		ECP4 A=new ECP4();
		FP24 r=new FP24(1);
		A.copy(P);

		ECP4 MP=new ECP4();
		MP.copy(P); MP.neg();

		int nb=n3.nbits();

		for (int i=nb-2;i>=1;i--)
		{
			r.sqr();
			lv=line(A,A,Qx,Qy);
			r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);

			bt=n3.bit(i)-n.bit(i); 
			if (bt==1)
			{
				lv=line(A,P,Qx,Qy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
			}
			if (bt==-1)
			{
				lv=line(A,MP,Qx,Qy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
			}
		}

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX)
		{
			r.conj();
		}

		return r;
	}

/* Optimal R-ate double pairing e(P,Q).e(R,S) */
	public static FP24 ate2(ECP4 P1,ECP Q1,ECP4 R1,ECP S1)
	{
		FP2 f;
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG n=new BIG(x);
		FP24 lv;
		int bt;

		ECP4 P=new ECP4(P1);
		ECP Q=new ECP(Q1);

		P.affine();
		Q.affine();

		ECP4 R=new ECP4(R1);
		ECP S=new ECP(S1);

		R.affine();
		S.affine();


		BIG n3=new BIG(n);
		n3.pmul(3);
		n3.norm();

		FP Qx=new FP(Q.getx());
		FP Qy=new FP(Q.gety());
		FP Sx=new FP(S.getx());
		FP Sy=new FP(S.gety());

		ECP4 A=new ECP4();
		ECP4 B=new ECP4();
		FP24 r=new FP24(1);

		A.copy(P);
		B.copy(R);

		ECP4 MP=new ECP4();
		MP.copy(P); MP.neg();
		ECP4 MR=new ECP4();
		MR.copy(R); MR.neg();


		int nb=n3.nbits();

		for (int i=nb-2;i>=1;i--)
		{
			r.sqr();
			lv=line(A,A,Qx,Qy);
			r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);

			lv=line(B,B,Sx,Sy);
			r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);

			bt=n3.bit(i)-n.bit(i); // bt=n.bit(i);
			if (bt==1)
			{
				lv=line(A,P,Qx,Qy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
				lv=line(B,R,Sx,Sy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
			}
			if (bt==-1)
			{
				lv=line(A,MP,Qx,Qy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
				lv=line(B,MR,Sx,Sy);
				r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);
			}
		}

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX)
		{
			r.conj();
		}

		return r;
	}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
	public static FP24 fexp(FP24 m)
	{
		FP2 f=new FP2(new BIG(ROM.Fra),new BIG(ROM.Frb));
		BIG x=new BIG(ROM.CURVE_Bnx);
		FP24 r=new FP24(m);

/* Easy part of final exp */
		FP24 lv=new FP24(r);
		lv.inverse();
		r.conj();

		r.mul(lv);
		lv.copy(r);
		r.frob(f,4);
		r.mul(lv);

		FP24 t0,t1,t2,t3,t4,t5,t6,t7;
/* Hard part of final exp */	
// Ghamman & Fouotsa Method

		t7=new FP24(r); t7.usqr();
		t1=t7.pow(x);

		x.fshr(1);
		t2=t1.pow(x);
		x.fshl(1);

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}
		t3=new FP24(t1); t3.conj();
		t2.mul(t3);
		t2.mul(r);

		t3=t2.pow(x);
		t4=t3.pow(x);
		t5=t4.pow(x);

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t3.conj(); t5.conj();
		}

		t3.frob(f,6); t4.frob(f,5);
		t3.mul(t4);

		t6=t5.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t6.conj();
		}

		t5.frob(f,4);
		t3.mul(t5);

		t0=new FP24(t2); t0.conj();
		t6.mul(t0);

		t5.copy(t6);
		t5.frob(f,3);

		t3.mul(t5);
		t5=t6.pow(x);
		t6=t5.pow(x);

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t5.conj();
		}

		t0.copy(t5);
		t0.frob(f,2);
		t3.mul(t0);
		t0.copy(t6);
		t0.frob(f,1);

		t3.mul(t0);
		t5=t6.pow(x);

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t5.conj();
		}
		t2.frob(f,7);

		t5.mul(t7);
		t3.mul(t2);
		t3.mul(t5);

		r.mul(t3);

		r.reduce();
		return r;
	}

/* GLV method */
	public static BIG[] glv(BIG e)
	{
		BIG[] u=new BIG[2];
// -(x^4).P = (Beta.x,y)
		BIG q=new BIG(ROM.CURVE_Order);
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG x2=BIG.smul(x,x);
		x=BIG.smul(x2,x2);
		u[0]=new BIG(e);
		u[0].mod(x);
		u[1]=new BIG(e);
		u[1].div(x);
		u[1].rsub(q);

		return u;
	}

/* Galbraith & Scott Method */
	public static BIG[] gs(BIG e)
	{
		BIG[] u=new BIG[8];

		BIG q=new BIG(ROM.CURVE_Order);
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG w=new BIG(e);
		for (int i=0;i<7;i++)
		{
			u[i]=new BIG(w);
			u[i].mod(x);
			w.div(x);
		}
		u[7]=new BIG(w);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX)
		{
			u[1].copy(BIG.modneg(u[1],q));
			u[3].copy(BIG.modneg(u[3],q));
			u[5].copy(BIG.modneg(u[5],q));
			u[7].copy(BIG.modneg(u[7],q));
		}

		return u;
	}	

/* Multiply P by e in group G1 */
	public static ECP G1mul(ECP P,BIG e)
	{
		ECP R;
		if (CONFIG_CURVE.USE_GLV)
		{
			R=new ECP();
			R.copy(P);
			int i,np,nn;
			ECP Q=new ECP();
			Q.copy(P); Q.affine();
			BIG q=new BIG(ROM.CURVE_Order);
			FP cru=new FP(new BIG(ROM.CURVE_Cru));
			BIG t=new BIG(0);
			BIG[] u=glv(e);
			Q.getx().mul(cru);

			np=u[0].nbits();
			t.copy(BIG.modneg(u[0],q));
			nn=t.nbits();
			if (nn<np)
			{
				u[0].copy(t);
				R.neg();
			}

			np=u[1].nbits();
			t.copy(BIG.modneg(u[1],q));
			nn=t.nbits();
			if (nn<np)
			{
				u[1].copy(t);
				Q.neg();
			}
			u[0].norm();
			u[1].norm();
			R=R.mul2(u[0],Q,u[1]);
			
		}
		else
		{
			R=P.mul(e);
		}
		return R;
	}

/* Multiply P by e in group G2 */
	public static ECP4 G2mul(ECP4 P,BIG e)
	{
		ECP4 R;
		if (CONFIG_CURVE.USE_GS_G2)
		{
			ECP4[] Q=new ECP4[8];
			FP2[] F=ECP4.frob_constants();

			BIG q=new BIG(ROM.CURVE_Order);
			BIG[] u=gs(e);

			BIG t=new BIG(0);
			int i,np,nn;

			Q[0]=new ECP4(); Q[0].copy(P);
			for (i=1;i<8;i++)
			{
				Q[i]=new ECP4(); Q[i].copy(Q[i-1]);
				Q[i].frob(F,1);
			}
			for (i=0;i<8;i++)
			{
				np=u[i].nbits();
				t.copy(BIG.modneg(u[i],q));
				nn=t.nbits();
				if (nn<np)
				{
					u[i].copy(t);
					Q[i].neg();
				}
				u[i].norm();
			}

			R=ECP4.mul8(Q,u);
		}
		else
		{
			R=P.mul(e);
		}
		return R;
	}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP8.java */
	public static FP24 GTpow(FP24 d,BIG e)
	{
		FP24 r;
		if (CONFIG_CURVE.USE_GS_GT)
		{
			FP24[] g=new FP24[8];
			FP2 f=new FP2(new BIG(ROM.Fra),new BIG(ROM.Frb));
			BIG q=new BIG(ROM.CURVE_Order);
			BIG t=new BIG(0);
			int i,np,nn;
			BIG[] u=gs(e);

			g[0]=new FP24(d);
			for (i=1;i<8;i++)
			{
				g[i]=new FP24(0); g[i].copy(g[i-1]);
				g[i].frob(f,1);
			}
			for (i=0;i<8;i++)
			{
				np=u[i].nbits();
				t.copy(BIG.modneg(u[i],q));
				nn=t.nbits();
				if (nn<np)
				{
					u[i].copy(t);
					g[i].conj();
				}
				u[i].norm();
			}
			r=FP24.pow8(g,u);
		}
		else
		{
			r=d.pow(e);
		}
		return r;
	}

}

