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

public final class PAIR256 {

//	public static final boolean GT_STRONG=false;

/* Line function */
	public static FP48 line(ECP8 A,ECP8 B,FP Qx,FP Qy)
	{
		FP16 a,b,c;                            
		if (A==B)
		{ // Doubling
			FP8 XX=new FP8(A.getx());  //X
			FP8 YY=new FP8(A.gety());  //Y
			FP8 ZZ=new FP8(A.getz());  //Z
			FP8 YZ=new FP8(YY);        //Y 
			YZ.mul(ZZ);                //YZ
			XX.sqr();	               //X^2
			YY.sqr();	               //Y^2
			ZZ.sqr();			       //Z^2
			
			YZ.imul(4);
			YZ.neg(); YZ.norm();       //-2YZ
			YZ.tmul(Qy);               //-2YZ.Ys

			XX.imul(6);                //3X^2
			XX.tmul(Qx);               //3X^2.Xs

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

			a=new FP16(YZ,ZZ);          // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs 
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.D_TYPE)
			{			
				b=new FP16(XX);             // L(0,1) | L(0,0) | L(1,0)
				c=new FP16(0);
			}
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				b=new FP16(0);
				c=new FP16(XX); c.times_i();
			}
			A.dbl();
		}
		else
		{ // Addition - assume B is affine

			FP8 X1=new FP8(A.getx());    // X1
			FP8 Y1=new FP8(A.gety());    // Y1
			FP8 T1=new FP8(A.getz());    // Z1
			FP8 T2=new FP8(A.getz());    // Z1
			
			T1.mul(B.gety());    // T1=Z1.Y2 
			T2.mul(B.getx());    // T2=Z1.X2

			X1.sub(T2); X1.norm();  // X1=X1-Z1.X2
			Y1.sub(T1); Y1.norm();  // Y1=Y1-Z1.Y2

			T1.copy(X1);            // T1=X1-Z1.X2
			X1.tmul(Qy);            // X1=(X1-Z1.X2).Ys

			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				X1.times_i();
				X1.norm();
			}

			T1.mul(B.gety());       // T1=(X1-Z1.X2).Y2

			T2.copy(Y1);            // T2=Y1-Z1.Y2
			T2.mul(B.getx());       // T2=(Y1-Z1.Y2).X2
			T2.sub(T1); T2.norm();          // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
			Y1.tmul(Qx);  Y1.neg(); Y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

			a=new FP16(X1,T2);       // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.D_TYPE)
			{
				b=new FP16(Y1);
				c=new FP16(0);
			}
			if (CONFIG_CURVE.SEXTIC_TWIST==CONFIG_CURVE.M_TYPE)
			{
				b=new FP16(0);
				c=new FP16(Y1); c.times_i();
			}
			A.add(B);
		}
		return new FP48(a,b,c);
	}

/* Optimal R-ate pairing */
	public static FP48 ate(ECP8 P1,ECP Q1)
	{
		FP2 f;
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG n=new BIG(x);
		FP48 lv;
		int bt;
		
		ECP8 P=new ECP8(P1);
		ECP Q=new ECP(Q1);

		P.affine();
		Q.affine();


		BIG n3=new BIG(n);
		n3.pmul(3);
		n3.norm();

		FP Qx=new FP(Q.getx());
		FP Qy=new FP(Q.gety());

		ECP8 A=new ECP8();
		FP48 r=new FP48(1);
		A.copy(P);

		ECP8 MP=new ECP8();
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
	public static FP48 ate2(ECP8 P1,ECP Q1,ECP8 R1,ECP S1)
	{
		FP2 f;
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG n=new BIG(x);
		FP48 lv;
		int bt;

		ECP8 P=new ECP8(P1);
		ECP Q=new ECP(Q1);

		P.affine();
		Q.affine();

		ECP8 R=new ECP8(R1);
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

		ECP8 A=new ECP8();
		ECP8 B=new ECP8();
		FP48 r=new FP48(1);

		A.copy(P);
		B.copy(R);

		ECP8 MP=new ECP8();
		MP.copy(P); MP.neg();
		ECP8 MR=new ECP8();
		MR.copy(R); MR.neg();


		int nb=n3.nbits();

		for (int i=nb-2;i>=1;i--)
		{
			r.sqr();
			lv=line(A,A,Qx,Qy);
			r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);

			lv=line(B,B,Sx,Sy);
			r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST);

			bt=n3.bit(i)-n.bit(i);
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
	public static FP48 fexp(FP48 m)
	{
		FP2 f=new FP2(new BIG(ROM.Fra),new BIG(ROM.Frb));
		BIG x=new BIG(ROM.CURVE_Bnx);
		FP48 r=new FP48(m);

/* Easy part of final exp */
		FP48 lv=new FP48(r);
		lv.inverse();
		r.conj();

		r.mul(lv);
		lv.copy(r);
		r.frob(f,8);
		r.mul(lv);

		FP48 t0,t1,t2,t3,t4,t5,t6,t7;
/* Hard part of final exp */	
// Ghamman & Fouotsa Method

		t7=new FP48(r); t7.usqr();
		t1=t7.pow(x);

		x.fshr(1);
		t2=t1.pow(x);
		x.fshl(1);

		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3=new FP48(t1); t3.conj();
		t2.mul(t3);
		t2.mul(r);

		r.mul(t7);

		t1=t2.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}
		t3.copy(t1);
		t3.frob(f,14);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,13);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,12);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,11);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,10);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,9);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,8);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t2); t3.conj();
		t1.mul(t3);
		t3.copy(t1);
		t3.frob(f,7);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,6);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,5);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,4);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,3);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,2);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}

		t3.copy(t1);
		t3.frob(f,1);
		r.mul(t3);
		t1=t1.pow(x);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX) {
			t1.conj();
		}
	
		r.mul(t1);
		t2.frob(f,15);
		r.mul(t2);

		r.reduce();
		return r;
	}

/* GLV method */
	public static BIG[] glv(BIG e)
	{
		BIG[] u=new BIG[2];
// -(x^8).P = (Beta.x,y)
		BIG q=new BIG(ROM.CURVE_Order);
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG x2=BIG.smul(x,x);
		x=BIG.smul(x2,x2);
		x2=BIG.smul(x,x);
		u[0]=new BIG(e);
		u[0].mod(x2);
		u[1]=new BIG(e);
		u[1].div(x2);
		u[1].rsub(q);

		return u;
	}

/* Galbraith & Scott Method */
	public static BIG[] gs(BIG e)
	{
		BIG[] u=new BIG[16];

		BIG q=new BIG(ROM.CURVE_Order);
		BIG x=new BIG(ROM.CURVE_Bnx);
		BIG w=new BIG(e);
		for (int i=0;i<15;i++)
		{
			u[i]=new BIG(w);
			u[i].mod(x);
			w.div(x);
		}
		u[15]=new BIG(w);
		if (CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX)
		{
			u[1].copy(BIG.modneg(u[1],q));
			u[3].copy(BIG.modneg(u[3],q));
			u[5].copy(BIG.modneg(u[5],q));
			u[7].copy(BIG.modneg(u[7],q));
			u[9].copy(BIG.modneg(u[9],q));
			u[11].copy(BIG.modneg(u[11],q));
			u[13].copy(BIG.modneg(u[13],q));
			u[15].copy(BIG.modneg(u[15],q));
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
	public static ECP8 G2mul(ECP8 P,BIG e)
	{
		ECP8 R;
		if (CONFIG_CURVE.USE_GS_G2)
		{
			ECP8[] Q=new ECP8[16];
			FP2[] F=ECP8.frob_constants();

			BIG q=new BIG(ROM.CURVE_Order);
			BIG[] u=gs(e);

			BIG t=new BIG(0);
			int i,np,nn;

			Q[0]=new ECP8(); Q[0].copy(P);
			for (i=1;i<16;i++)
			{
				Q[i]=new ECP8(); Q[i].copy(Q[i-1]);
				Q[i].frob(F,1);
			}
			for (i=0;i<16;i++)
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

			R=ECP8.mul16(Q,u);
		}
		else
		{
			R=P.mul(e);
		}
		return R;
	}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP16.java */
	public static FP48 GTpow(FP48 d,BIG e)
	{
		FP48 r;
		if (CONFIG_CURVE.USE_GS_GT)
		{
			FP48[] g=new FP48[16];
			FP2 f=new FP2(new BIG(ROM.Fra),new BIG(ROM.Frb));
			BIG q=new BIG(ROM.CURVE_Order);
			BIG t=new BIG(0);
			int i,np,nn;
			BIG[] u=gs(e);

			g[0]=new FP48(d);
			for (i=1;i<16;i++)
			{
				g[i]=new FP48(0); g[i].copy(g[i-1]);
				g[i].frob(f,1);
			}
			for (i=0;i<16;i++)
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
			r=FP48.pow16(g,u);
		}
		else
		{
			r=d.pow(e);
		}
		return r;
	}
}

