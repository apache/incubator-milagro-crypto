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

/* AMCL BLS Curve pairing functions */

//#define HAS_MAIN

#include "pair256_ZZZ.h"

/* Line function */
static void PAIR_ZZZ_line(FP48_YYY *v,ECP8_ZZZ *A,ECP8_ZZZ *B,FP_YYY *Qx,FP_YYY *Qy)
{
	//FP2_YYY t;

	FP8_YYY X1,Y1,T1,T2;
	FP8_YYY XX,YY,ZZ,YZ;
    FP16_YYY a,b,c;

	if (A==B)
    {
        /* doubling */
 		FP8_YYY_copy(&XX,&(A->x));	//FP8_YYY XX=new FP8_YYY(A.getx());  //X
		FP8_YYY_copy(&YY,&(A->y));	//FP8_YYY YY=new FP8_YYY(A.gety());  //Y
		FP8_YYY_copy(&ZZ,&(A->z));	//FP8_YYY ZZ=new FP8_YYY(A.getz());  //Z


		FP8_YYY_copy(&YZ,&YY);		//FP8_YYY YZ=new FP8_YYY(YY);        //Y 
		FP8_YYY_mul(&YZ,&YZ,&ZZ);		//YZ.mul(ZZ);                //YZ
		FP8_YYY_sqr(&XX,&XX);		//XX.sqr();	               //X^2
		FP8_YYY_sqr(&YY,&YY);		//YY.sqr();	               //Y^2
		FP8_YYY_sqr(&ZZ,&ZZ);		//ZZ.sqr();			       //Z^2
			
		FP8_YYY_imul(&YZ,&YZ,4);	//YZ.imul(4);
		FP8_YYY_neg(&YZ,&YZ);		//YZ.neg(); 
		FP8_YYY_norm(&YZ);			//YZ.norm();       //-4YZ

		FP8_YYY_imul(&XX,&XX,6);					//6X^2
		//FP2_YYY_from_FP(&t,Qx);
		FP8_YYY_tmul(&XX,&XX,Qx);	               //6X^2.Xs

		FP8_YYY_imul(&ZZ,&ZZ,3*CURVE_B_I_ZZZ);	//3Bz^2 
		//FP2_YYY_from_FP(&t,Qy);
		FP8_YYY_tmul(&YZ,&YZ,Qy);	//-4YZ.Ys

#if SEXTIC_TWIST_ZZZ==D_TYPE
		FP8_YYY_div_2i(&ZZ);		//6(b/i)z^2
#endif
#if SEXTIC_TWIST_ZZZ==M_TYPE
		FP8_YYY_times_i(&ZZ);
		FP8_YYY_add(&ZZ,&ZZ,&ZZ);  // 6biz^2
		FP8_YYY_times_i(&YZ);
		FP8_YYY_norm(&YZ);	
#endif
		FP8_YYY_norm(&ZZ);			// 6bi.Z^2 

		FP8_YYY_add(&YY,&YY,&YY);	// 2y^2
		FP8_YYY_sub(&ZZ,&ZZ,&YY);	// 
		FP8_YYY_norm(&ZZ);			// 6b.Z^2-2Y^2

		FP16_YYY_from_FP8s(&a,&YZ,&ZZ); // -4YZ.Ys | 6b.Z^2-2Y^2 | 6X^2.Xs 
#if SEXTIC_TWIST_ZZZ==D_TYPE
		FP16_YYY_from_FP8(&b,&XX);	
		FP16_YYY_zero(&c);
#endif
#if SEXTIC_TWIST_ZZZ==M_TYPE
		FP16_YYY_zero(&b);
		FP16_YYY_from_FP8H(&c,&XX);
#endif

		ECP8_ZZZ_dbl(A);				//A.dbl();
    }
    else
    {
        /* addition */

		FP8_YYY_copy(&X1,&(A->x));		//FP8_YYY X1=new FP8_YYY(A.getx());    // X1
		FP8_YYY_copy(&Y1,&(A->y));		//FP8_YYY Y1=new FP8_YYY(A.gety());    // Y1
		FP8_YYY_copy(&T1,&(A->z));		//FP8_YYY T1=new FP8_YYY(A.getz());    // Z1
			
		FP8_YYY_copy(&T2,&T1);		//FP8_YYY T2=new FP8_YYY(A.getz());    // Z1

		FP8_YYY_mul(&T1,&T1,&(B->y));	//T1.mul(B.gety());    // T1=Z1.Y2 
		FP8_YYY_mul(&T2,&T2,&(B->x));	//T2.mul(B.getx());    // T2=Z1.X2

		FP8_YYY_sub(&X1,&X1,&T2);		//X1.sub(T2); 
		FP8_YYY_norm(&X1);				//X1.norm();  // X1=X1-Z1.X2
		FP8_YYY_sub(&Y1,&Y1,&T1);		//Y1.sub(T1); 
		FP8_YYY_norm(&Y1);				//Y1.norm();  // Y1=Y1-Z1.Y2

		FP8_YYY_copy(&T1,&X1);			//T1.copy(X1);            // T1=X1-Z1.X2
		//FP2_YYY_from_FP(&t,Qy);
		FP8_YYY_tmul(&X1,&X1,Qy);		//X1.pmul(Qy);            // X1=(X1-Z1.X2).Ys
#if SEXTIC_TWIST_ZZZ==M_TYPE
		FP8_YYY_times_i(&X1);
		FP8_YYY_norm(&X1);
#endif

		FP8_YYY_mul(&T1,&T1,&(B->y));	//T1.mul(B.gety());       // T1=(X1-Z1.X2).Y2

		FP8_YYY_copy(&T2,&Y1);			//T2.copy(Y1);            // T2=Y1-Z1.Y2
		FP8_YYY_mul(&T2,&T2,&(B->x));	//T2.mul(B.getx());       // T2=(Y1-Z1.Y2).X2
		FP8_YYY_sub(&T2,&T2,&T1);		//T2.sub(T1); 
		FP8_YYY_norm(&T2);				//T2.norm();          // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
		//FP2_YYY_from_FP(&t,Qx);
		FP8_YYY_tmul(&Y1,&Y1,Qx);		//Y1.pmul(Qx);  
		FP8_YYY_neg(&Y1,&Y1);			//Y1.neg(); 
		FP8_YYY_norm(&Y1);				//Y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

		FP16_YYY_from_FP8s(&a,&X1,&T2);	// (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
#if SEXTIC_TWIST_ZZZ==D_TYPE
		FP16_YYY_from_FP8(&b,&Y1);		//b=new FP4(Y1);
		FP16_YYY_zero(&c);
#endif
#if SEXTIC_TWIST_ZZZ==M_TYPE
		FP16_YYY_zero(&b);
		FP16_YYY_from_FP8H(&c,&Y1);		//b=new FP4(Y1);
#endif
		ECP8_ZZZ_add(A,B);			// A.add(B);
    }

    FP48_YYY_from_FP16s(v,&a,&b,&c);
}

/* Optimal R-ate pairing r=e(P,Q) */
void PAIR_ZZZ_ate(FP48_YYY *r,ECP8_ZZZ *P1,ECP_ZZZ *Q1)
{
    BIG_XXX x,n,n3;
	FP_YYY Qx,Qy;
    int i,j,nb,bt;
    ECP8_ZZZ A,NP,P;
	ECP_ZZZ Q;
    FP48_YYY lv;

    BIG_XXX_rcopy(x,CURVE_Bnx_ZZZ);

    BIG_XXX_copy(n,x);

    //BIG_XXX_norm(n);
	BIG_XXX_pmul(n3,n,3);
	BIG_XXX_norm(n3);

	ECP8_ZZZ_copy(&P,P1);
	ECP_ZZZ_copy(&Q,Q1);

	ECP8_ZZZ_affine(&P);
	ECP_ZZZ_affine(&Q);


    FP_YYY_copy(&Qx,&(Q.x));
    FP_YYY_copy(&Qy,&(Q.y));

    ECP8_ZZZ_copy(&A,&P);
	ECP8_ZZZ_copy(&NP,&P); ECP8_ZZZ_neg(&NP);

    FP48_YYY_one(r);
    nb=BIG_XXX_nbits(n3);  // n3

	j=0;
    /* Main Miller Loop */
    for (i=nb-2; i>=1; i--)
    {
		j++;
		FP48_YYY_sqr(r,r);
        PAIR_ZZZ_line(&lv,&A,&A,&Qx,&Qy);
        FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
//printf("r= "); FP48_YYY_output(r); printf("\n");
//if (j>3) exit(0);
		bt= BIG_XXX_bit(n3,i)-BIG_XXX_bit(n,i);  // BIG_XXX_bit(n,i); 
        if (bt==1)
        {
//printf("bt=1\n");
            PAIR_ZZZ_line(&lv,&A,&P,&Qx,&Qy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
        }
		if (bt==-1)
		{
//printf("bt=-1\n");
			//ECP8_ZZZ_neg(P);
            PAIR_ZZZ_line(&lv,&A,&NP,&Qx,&Qy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
			//ECP8_ZZZ_neg(P);
		}

    }

#if SIGN_OF_X_ZZZ==NEGATIVEX
    FP48_YYY_conj(r,r);
#endif

}

/* Optimal R-ate double pairing e(P,Q).e(R,S) */
void PAIR_ZZZ_double_ate(FP48_YYY *r,ECP8_ZZZ *P1,ECP_ZZZ *Q1,ECP8_ZZZ *R1,ECP_ZZZ *S1)
{
    BIG_XXX x,n,n3;
	FP_YYY Qx,Qy,Sx,Sy;
    int i,nb,bt;
    ECP8_ZZZ A,B,NP,NR,P,R;
	ECP_ZZZ Q,S;
    FP48_YYY lv;

    BIG_XXX_rcopy(x,CURVE_Bnx_ZZZ);
    BIG_XXX_copy(n,x);

    //BIG_XXX_norm(n);
	BIG_XXX_pmul(n3,n,3);
	BIG_XXX_norm(n3);

	ECP8_ZZZ_copy(&P,P1);
	ECP_ZZZ_copy(&Q,Q1);

	ECP8_ZZZ_affine(&P);
	ECP_ZZZ_affine(&Q);

	ECP8_ZZZ_copy(&R,R1);
	ECP_ZZZ_copy(&S,S1);

	ECP8_ZZZ_affine(&R);
	ECP_ZZZ_affine(&S);

    FP_YYY_copy(&Qx,&(Q.x));
    FP_YYY_copy(&Qy,&(Q.y));

    FP_YYY_copy(&Sx,&(S.x));
    FP_YYY_copy(&Sy,&(S.y));

    ECP8_ZZZ_copy(&A,&P);
    ECP8_ZZZ_copy(&B,&R);
	ECP8_ZZZ_copy(&NP,&P); ECP8_ZZZ_neg(&NP);
	ECP8_ZZZ_copy(&NR,&R); ECP8_ZZZ_neg(&NR);


    FP48_YYY_one(r);
    nb=BIG_XXX_nbits(n3);

    /* Main Miller Loop */
    for (i=nb-2; i>=1; i--)
    {
        FP48_YYY_sqr(r,r);
        PAIR_ZZZ_line(&lv,&A,&A,&Qx,&Qy);
        FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);

        PAIR_ZZZ_line(&lv,&B,&B,&Sx,&Sy);
        FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);

		bt=BIG_XXX_bit(n3,i)-BIG_XXX_bit(n,i); // bt=BIG_XXX_bit(n,i);
        if (bt==1)
        {
            PAIR_ZZZ_line(&lv,&A,&P,&Qx,&Qy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);

            PAIR_ZZZ_line(&lv,&B,&R,&Sx,&Sy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
        }
		if (bt==-1)
		{
			//ECP8_ZZZ_neg(P); 
            PAIR_ZZZ_line(&lv,&A,&NP,&Qx,&Qy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
			//ECP8_ZZZ_neg(P); 
			//ECP8_ZZZ_neg(R);
            PAIR_ZZZ_line(&lv,&B,&NR,&Sx,&Sy);
            FP48_YYY_smul(r,&lv,SEXTIC_TWIST_ZZZ);
			//ECP8_ZZZ_neg(R);
		}
	}



#if SIGN_OF_X_ZZZ==NEGATIVEX
    FP48_YYY_conj(r,r);
#endif

}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */

void PAIR_ZZZ_fexp(FP48_YYY *r)
{
    FP2_YYY X;
    BIG_XXX x;
	FP_YYY a,b;
    FP48_YYY t1,t2,t3,t7;  

    BIG_XXX_rcopy(x,CURVE_Bnx_ZZZ);
    FP_YYY_rcopy(&a,Fra_YYY);
    FP_YYY_rcopy(&b,Frb_YYY);
    FP2_YYY_from_FPs(&X,&a,&b);

    /* Easy part of final exp - r^(p^24-1)(p^8+1)*/

    FP48_YYY_inv(&t7,r);
    FP48_YYY_conj(r,r);

    FP48_YYY_mul(r,&t7);
    FP48_YYY_copy(&t7,r);

    FP48_YYY_frob(r,&X,8);

    FP48_YYY_mul(r,&t7);

// Ghamman & Fouotsa Method for hard part of fexp - r^e1 . r^p^e2 . r^p^2^e3 ..

// e0 = u^17 - 2*u^16 + u^15 - u^9 + 2*u^8 - u^7 + 3	// .p^0
// e1 = u^16 - 2*u^15 + u^14 - u^8 + 2*u^7 - u^6		// .p^1
// e2 = u^15 - 2*u^14 + u^13 - u^7 + 2*u^6 - u^5
// e3 = u^14 - 2*u^13 + u^12 - u^6 + 2*u^5 - u^4
// e4 = u^13 - 2*u^12 + u^11 - u^5 + 2*u^4 - u^3
// e5 = u^12 - 2*u^11 + u^10 - u^4 + 2*u^3 - u^2
// e6 = u^11 - 2*u^10 + u^9 - u^3 + 2*u^2 - u
// e7 =  u^10 - 2*u^9 + u^8 - u^2 + 2*u - 1
// e8 =  u^9 - 2*u^8 + u^7
// e9 =  u^8 - 2*u^7 + u^6
// e10 = u^7 - 2*u^6 + u^5
// e11 = u^6 - 2*u^5 + u^4
// e12 = u^5 - 2*u^4 + u^3
// e13 = u^4 - 2*u^3 + u^2
// e14 = u^3 - 2*u^2 + u
// e15 = u^2 - 2*u + 1

// e15 = u^2-2*u+1
// e14 = u.e15
// e13 = u.e14
// e12 = u.e13
// e11 = u.e12
// e10 = u.e11
// e9 =  u.e10
// e8 =  u.e9
// e7 =  u.e8 - e15
// e6 =  u.e7
// e5 =  u.e6
// e4 =  u.e5
// e3 =  u.e4
// e2 =  u.e3
// e1 =  u.e2
// e0 =  u.e1 + 3

// f^e0.f^e1^p.f^e2^p^2.. .. f^e14^p^14.f^e15^p^15

	FP48_YYY_usqr(&t7,r);			// t7=f^2
	FP48_YYY_pow(&t1,&t7,x);		// t1=f^2u

	BIG_XXX_fshr(x,1);
	FP48_YYY_pow(&t2,&t1,x);		// t2=f^2u^(u/2) =  f^u^2
	BIG_XXX_fshl(x,1);				// x must be even

#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif

	FP48_YYY_conj(&t3,&t1);		// t3=f^-2u
	FP48_YYY_mul(&t2,&t3);		// t2=f^u^2.f^-2u
	FP48_YYY_mul(&t2,r);		// t2=f^u^2.f^-2u.f = f^(u^2-2u+1) = f^e15

	FP48_YYY_mul(r,&t7);		// f^3

	FP48_YYY_pow(&t1,&t2,x);	// f^e15^u = f^(u.e15) = f^(u^3-2u^2+u) = f^(e14)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,14);	// f^(u^3-2u^2+u)^p^14
	FP48_YYY_mul(r,&t3);		// f^3.f^(u^3-2u^2+u)^p^14

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e14) = f^(u^4-2u^3+u^2) =  f^(e13)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,13);	// f^(e13)^p^13
	FP48_YYY_mul(r,&t3);		// f^3.f^(u^3-2u^2+u)^p^14.f^(u^4-2u^3+u^2)^p^13

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e13)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,12);	// f^(e12)^p^12
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e12)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,11);	// f^(e11)^p^11
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e11)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,10);	// f^(e10)^p^10
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e10)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,9);	// f^(e9)^p^9
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e9)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,8);	// f^(e8)^p^8
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e8)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_conj(&t3,&t2);
	FP48_YYY_mul(&t1,&t3);  // f^(u.e8).f^-e15
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,7);	// f^(e7)^p^7
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e7)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,6);	// f^(e6)^p^6
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e6)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,5);	// f^(e5)^p^5
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e5)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,4);	// f^(e4)^p^4
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e4)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,3);	// f^(e3)^p^3
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e3)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,2);	// f^(e2)^p^2
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e2)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_copy(&t3,&t1);
	FP48_YYY_frob(&t3,&X,1);	// f^(e1)^p^1
	FP48_YYY_mul(r,&t3);		

	FP48_YYY_pow(&t1,&t1,x);	// f^(u.e1)
#if SIGN_OF_X_ZZZ==NEGATIVEX
	FP48_YYY_conj(&t1,&t1);
#endif
	FP48_YYY_mul(r,&t1);		// r.f^e0		

	FP48_YYY_frob(&t2,&X,15);	// f^(e15.p^15)
	FP48_YYY_mul(r,&t2);


	FP48_YYY_reduce(r);

}

#ifdef USE_GLV_ZZZ
/* GLV method */
static void glv(BIG_XXX u[2],BIG_XXX e)
{

// -(x^8).P = (Beta.x,y)

    BIG_XXX x,x2,q;
    BIG_XXX_rcopy(x,CURVE_Bnx_ZZZ);
    
	BIG_XXX_smul(x2,x,x);
	BIG_XXX_smul(x,x2,x2);
	BIG_XXX_smul(x2,x,x);

    BIG_XXX_copy(u[0],e);
    BIG_XXX_mod(u[0],x2);
    BIG_XXX_copy(u[1],e);
    BIG_XXX_sdiv(u[1],x2);

    BIG_XXX_rcopy(q,CURVE_Order_ZZZ);
    BIG_XXX_sub(u[1],q,u[1]);


    return;
}
#endif // USE_GLV

/* Galbraith & Scott Method */
static void gs(BIG_XXX u[16],BIG_XXX e)
{
    int i;

    BIG_XXX x,w,q;
	BIG_XXX_rcopy(q,CURVE_Order_ZZZ);
    BIG_XXX_rcopy(x,CURVE_Bnx_ZZZ);
    BIG_XXX_copy(w,e);

    for (i=0; i<15; i++)
    {
        BIG_XXX_copy(u[i],w);
        BIG_XXX_mod(u[i],x);
        BIG_XXX_sdiv(w,x);
    }
	BIG_XXX_copy(u[15],w);

/*  */
#if SIGN_OF_X_ZZZ==NEGATIVEX
	BIG_XXX_modneg(u[1],u[1],q);
	BIG_XXX_modneg(u[3],u[3],q);
	BIG_XXX_modneg(u[5],u[5],q);
	BIG_XXX_modneg(u[7],u[7],q);
	BIG_XXX_modneg(u[9],u[9],q);
	BIG_XXX_modneg(u[11],u[11],q);
	BIG_XXX_modneg(u[13],u[13],q);
	BIG_XXX_modneg(u[15],u[15],q);
#endif


    return;
}

/* Multiply P by e in group G1 */
void PAIR_ZZZ_G1mul(ECP_ZZZ *P,BIG_XXX e)
{
#ifdef USE_GLV_ZZZ   /* Note this method is patented */
    int np,nn;
    ECP_ZZZ Q;
	FP_YYY cru;
    BIG_XXX t,q;
    BIG_XXX u[2];

    BIG_XXX_rcopy(q,CURVE_Order_ZZZ);
    glv(u,e);

    //ECP_ZZZ_affine(P);
    ECP_ZZZ_copy(&Q,P); ECP_ZZZ_affine(&Q);
    FP_YYY_rcopy(&cru,CURVE_Cru_ZZZ);
    FP_YYY_mul(&(Q.x),&(Q.x),&cru);

    /* note that -a.B = a.(-B). Use a or -a depending on which is smaller */

    np=BIG_XXX_nbits(u[0]);
    BIG_XXX_modneg(t,u[0],q);
    nn=BIG_XXX_nbits(t);
    if (nn<np)
    {
        BIG_XXX_copy(u[0],t);
        ECP_ZZZ_neg(P);
    }

    np=BIG_XXX_nbits(u[1]);
    BIG_XXX_modneg(t,u[1],q);
    nn=BIG_XXX_nbits(t);
    if (nn<np)
    {
        BIG_XXX_copy(u[1],t);
        ECP_ZZZ_neg(&Q);
    }
    BIG_XXX_norm(u[0]);
    BIG_XXX_norm(u[1]);    
    ECP_ZZZ_mul2(P,&Q,u[0],u[1]);

#else
    ECP_ZZZ_mul(P,e);
#endif
}

/* Multiply P by e in group G2 */
void PAIR_ZZZ_G2mul(ECP8_ZZZ *P,BIG_XXX e)
{
#ifdef USE_GS_G2_ZZZ   /* Well I didn't patent it :) */
    int i,np,nn;
    ECP8_ZZZ Q[16];
    FP2_YYY X[3];
    BIG_XXX x,y,u[16];

	ECP8_ZZZ_frob_constants(X);

    BIG_XXX_rcopy(y,CURVE_Order_ZZZ);
    gs(u,e);

    //ECP8_ZZZ_affine(P);

    ECP8_ZZZ_copy(&Q[0],P);
    for (i=1; i<16; i++)
    {
        ECP8_ZZZ_copy(&Q[i],&Q[i-1]);
        ECP8_ZZZ_frob(&Q[i],X,1);
    }

    for (i=0; i<16; i++)
    {
        np=BIG_XXX_nbits(u[i]);
        BIG_XXX_modneg(x,u[i],y);
        nn=BIG_XXX_nbits(x);
        if (nn<np)
        {
            BIG_XXX_copy(u[i],x);
            ECP8_ZZZ_neg(&Q[i]);
        }
        BIG_XXX_norm(u[i]);  
		//ECP8_ZZZ_affine(&Q[i]);
    }

    ECP8_ZZZ_mul16(P,Q,u);

#else
    ECP8_ZZZ_mul(P,e);
#endif
}

/* f=f^e */
void PAIR_ZZZ_GTpow(FP48_YYY *f,BIG_XXX e)
{
#ifdef USE_GS_GT_ZZZ   /* Note that this option requires a lot of RAM! Maybe better to use compressed XTR method, see FP16.cpp */
    int i,np,nn;
    FP48_YYY g[16];
    FP2_YYY X;
    BIG_XXX t,q;
	FP_YYY fx,fy;
    BIG_XXX u[16];

    FP_YYY_rcopy(&fx,Fra_YYY);
    FP_YYY_rcopy(&fy,Frb_YYY);
    FP2_YYY_from_FPs(&X,&fx,&fy);

    BIG_XXX_rcopy(q,CURVE_Order_ZZZ);
    gs(u,e);

    FP48_YYY_copy(&g[0],f);
    for (i=1; i<16; i++)
    {
        FP48_YYY_copy(&g[i],&g[i-1]);
        FP48_YYY_frob(&g[i],&X,1);
    }

    for (i=0; i<16; i++)
    {
        np=BIG_XXX_nbits(u[i]);
        BIG_XXX_modneg(t,u[i],q);
        nn=BIG_XXX_nbits(t);
        if (nn<np)
        {
            BIG_XXX_copy(u[i],t);
            FP48_YYY_conj(&g[i],&g[i]);
        }
        BIG_XXX_norm(u[i]);
    }
    FP48_YYY_pow16(f,g,u);

#else
    FP48_YYY_pow(f,f,e);
#endif
}

/* test group membership test - no longer needed */
/* with GT-Strong curve, now only check that m!=1, conj(m)*m==1, and m.m^{p^4}=m^{p^2} */

/*
int PAIR_ZZZ_GTmember(FP48_YYY *m)
{
	BIG_XXX a,b;
	FP2_YYY X;
	FP48_YYY r,w;
	if (FP48_YYY_isunity(m)) return 0;
	FP48_YYY_conj(&r,m);
	FP48_YYY_mul(&r,m);
	if (!FP48_YYY_isunity(&r)) return 0;

	BIG_XXX_rcopy(a,CURVE_Fra);
	BIG_XXX_rcopy(b,CURVE_Frb);
	FP2_YYY_from_BIG_XXXs(&X,a,b);


	FP48_YYY_copy(&r,m); FP48_YYY_frob(&r,&X); FP48_YYY_frob(&r,&X);
	FP48_YYY_copy(&w,&r); FP48_YYY_frob(&w,&X); FP48_YYY_frob(&w,&X);
	FP48_YYY_mul(&w,m);


#ifndef GT_STRONG
	if (!FP48_YYY_equals(&w,&r)) return 0;

	BIG_XXX_rcopy(a,CURVE_Bnx);

	FP48_YYY_copy(&r,m); FP48_YYY_pow(&w,&r,a); FP48_YYY_pow(&w,&w,a);
	FP48_YYY_sqr(&r,&w); FP48_YYY_mul(&r,&w); FP48_YYY_sqr(&r,&r);

	FP48_YYY_copy(&w,m); FP48_YYY_frob(&w,&X);
 #endif

	return FP48_YYY_equals(&w,&r);
}

*/


#ifdef HAS_MAIN

using namespace std;
using namespace ZZZ;


// g++ -O2 pair256_BLS48.cpp ecp8_BLS48.cpp fp48_BLS48.cpp fp16_BLS48.cpp fp8_BLS48.cpp fp4_BLS48.cpp fp2_BLS48.cpp ecp_BLS48.cpp fp_BLS48.cpp big_B560_29.cpp rom_curve_BLS48.cpp rom_field_BLS48.cpp rand.cpp hash.cpp oct.cpp -o pair256_BLS48.exe

int main()
{
    int i;
    char byt[32];
    csprng rng;
    BIG xa,xb,ya,yb,w,a,b,t1,q,u[2],v[4],m,r,xx,x2,x4,p;
    ECP8 P,G;
    ECP Q,R;
    FP48 g,gp;
    FP16 t,c,cp,cpm1,cpm2;
	FP8 X,Y;
    FP2 x,y,f,Aa,Bb;
	FP cru;

	for (i=0;i<32;i++)
		byt[i]=i+9;
	RAND_seed(&rng,32,byt);

	BIG_rcopy(r,CURVE_Order);
	BIG_rcopy(p,Modulus);


    BIG_rcopy(xa,CURVE_Gx_ZZZ);
    BIG_rcopy(ya,CURVE_Gy_ZZZ);

    ECP_ZZZ_set(&Q,xa,ya);
    if (Q.inf) printf("Failed to set - point not on curve\n");
    else printf("G1 set success\n");

    printf("Q= ");
    ECP_ZZZ_output(&Q);
    printf("\n");

	ECP8_generator(&P);

    if (P.inf) printf("Failed to set - point not on curve\n");
    else printf("G2 set success\n");

    BIG_rcopy(a,Fra);
    BIG_rcopy(b,Frb);
    FP2_from_BIGs(&f,a,b);


//exit(0);

    PAIR_ZZZ_ate(&g,&P,&Q);

	printf("gb= ");
    FP48_output(&g);
    printf("\n");
    PAIR_ZZZ_fexp(&g);

    printf("g= ");
    FP48_output(&g);
    printf("\n");

	//FP48_pow(&g,&g,r);

   // printf("g^r= ");
    //FP48_output(&g);
    //printf("\n");

	ECP_ZZZ_copy(&R,&Q);
	ECP8_copy(&G,&P);

	ECP8_dbl(&G);
	ECP_dbl(&R);
	ECP_affine(&R);

    PAIR_ZZZ_ate(&g,&G,&Q);
    PAIR_ZZZ_fexp(&g);

    printf("g1= ");
    FP48_output(&g);
    printf("\n");

    PAIR_ZZZ_ate(&g,&P,&R);
    PAIR_ZZZ_fexp(&g);

    printf("g2= ");
    FP48_output(&g);
    printf("\n");


	PAIR_ZZZ_G1mul(&Q,r);
	printf("rQ= ");ECP_output(&Q); printf("\n");

	PAIR_ZZZ_G2mul(&P,r);
	printf("rP= ");ECP8_output(&P); printf("\n");

	//FP48_pow(&g,&g,r);
	PAIR_ZZZ_GTpow(&g,r);
	printf("g^r= ");FP48_output(&g); printf("\n");


	BIG_randomnum(w,r,&rng);

	FP48_copy(&gp,&g);

	PAIR_ZZZ_GTpow(&g,w);

	FP48_trace(&t,&g);

	printf("g^r=  ");FP16_output(&t); printf("\n");

	FP48_compow(&t,&gp,w,r);

	printf("t(g)= "); FP16_output(&t); printf("\n");

//    PAIR_ZZZ_ate(&g,&P,&R);
//    PAIR_ZZZ_fexp(&g);

//    printf("g= ");
//    FP48_output(&g);
//    printf("\n");

//	PAIR_ZZZ_GTpow(&g,xa);
}

#endif
