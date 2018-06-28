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

/* AMCL Fp^12 functions */
/* SU=m, m is Stack Usage (no lazy )*/
/* FP48 elements are of the form a+i.b+i^2.c */

#include "fp48_YYY.h"

using namespace XXX;

/* return 1 if b==c, no branching */
static int teq(sign32 b,sign32 c)
{
    sign32 x=b^c;
    x-=1;  // if x=0, x now -1
    return (int)((x>>31)&1);
}


/* Constant time select from pre-computed table */
static void FP48_select(YYY::FP48 *f,YYY::FP48 g[],sign32 b)
{
    YYY::FP48 invf;
    sign32 m=b>>31;
    sign32 babs=(b^m)-m;

    babs=(babs-1)/2;

    FP48_cmove(f,&g[0],teq(babs,0));  // conditional move
    FP48_cmove(f,&g[1],teq(babs,1));
    FP48_cmove(f,&g[2],teq(babs,2));
    FP48_cmove(f,&g[3],teq(babs,3));
    FP48_cmove(f,&g[4],teq(babs,4));
    FP48_cmove(f,&g[5],teq(babs,5));
    FP48_cmove(f,&g[6],teq(babs,6));
    FP48_cmove(f,&g[7],teq(babs,7));

    FP48_copy(&invf,f);
    FP48_conj(&invf,&invf);  // 1/f
    FP48_cmove(f,&invf,(int)(m&1));
}


/* test x==0 ? */
/* SU= 8 */
int YYY::FP48_iszilch(FP48 *x)
{
    if (FP16_iszilch(&(x->a)) && FP16_iszilch(&(x->b)) && FP16_iszilch(&(x->c))) return 1;
    return 0;
}

/* test x==1 ? */
/* SU= 8 */
int YYY::FP48_isunity(FP48 *x)
{
    if (FP16_isunity(&(x->a)) && FP16_iszilch(&(x->b)) && FP16_iszilch(&(x->c))) return 1;
    return 0;
}

/* FP48 copy w=x */
/* SU= 16 */
void YYY::FP48_copy(FP48 *w,FP48 *x)
{
    if (x==w) return;
    FP16_copy(&(w->a),&(x->a));
    FP16_copy(&(w->b),&(x->b));
    FP16_copy(&(w->c),&(x->c));
}

/* FP48 w=1 */
/* SU= 8 */
void YYY::FP48_one(FP48 *w)
{
    FP16_one(&(w->a));
    FP16_zero(&(w->b));
    FP16_zero(&(w->c));
}

/* return 1 if x==y, else 0 */
/* SU= 16 */
int YYY::FP48_equals(FP48 *x,FP48 *y)
{
    if (FP16_equals(&(x->a),&(y->a)) && FP16_equals(&(x->b),&(y->b)) && FP16_equals(&(x->b),&(y->b)))
        return 1;
    return 0;
}

/* Set w=conj(x) */
/* SU= 8 */
void YYY::FP48_conj(FP48 *w,FP48 *x)
{
    FP48_copy(w,x);
    FP16_conj(&(w->a),&(w->a));
    FP16_nconj(&(w->b),&(w->b));
    FP16_conj(&(w->c),&(w->c));
}

/* Create FP48 from FP16 */
/* SU= 8 */
void YYY::FP48_from_FP16(FP48 *w,FP16 *a)
{
    FP16_copy(&(w->a),a);
    FP16_zero(&(w->b));
    FP16_zero(&(w->c));
}

/* Create FP48 from 3 FP16's */
/* SU= 16 */
void YYY::FP48_from_FP16s(FP48 *w,FP16 *a,FP16 *b,FP16 *c)
{
    FP16_copy(&(w->a),a);
    FP16_copy(&(w->b),b);
    FP16_copy(&(w->c),c);
}

/* Granger-Scott Unitary Squaring. This does not benefit from lazy reduction */
/* SU= 600 */
void YYY::FP48_usqr(FP48 *w,FP48 *x)
{
    FP16 A,B,C,D;

    FP16_copy(&A,&(x->a));

    FP16_sqr(&(w->a),&(x->a));
    FP16_add(&D,&(w->a),&(w->a));
    FP16_add(&(w->a),&D,&(w->a));

    FP16_norm(&(w->a));
    FP16_nconj(&A,&A);

    FP16_add(&A,&A,&A);
    FP16_add(&(w->a),&(w->a),&A);
    FP16_sqr(&B,&(x->c));
    FP16_times_i(&B);

    FP16_add(&D,&B,&B);
    FP16_add(&B,&B,&D);
    FP16_norm(&B);

    FP16_sqr(&C,&(x->b));

    FP16_add(&D,&C,&C);
    FP16_add(&C,&C,&D);

    FP16_norm(&C);
    FP16_conj(&(w->b),&(x->b));
    FP16_add(&(w->b),&(w->b),&(w->b));
    FP16_nconj(&(w->c),&(x->c));

    FP16_add(&(w->c),&(w->c),&(w->c));
    FP16_add(&(w->b),&B,&(w->b));
    FP16_add(&(w->c),&C,&(w->c));

    FP48_reduce(w);	    /* reduce here as in pow function repeated squarings would trigger multiple reductions */
}

/* FP48 squaring w=x^2 */
/* SU= 600 */
void YYY::FP48_sqr(FP48 *w,FP48 *x)
{
    /* Use Chung-Hasan SQR2 method from http://cacr.uwaterloo.ca/techreports/2006/cacr2006-24.pdf */

    FP16 A,B,C,D;

    FP16_sqr(&A,&(x->a));
    FP16_mul(&B,&(x->b),&(x->c));
    FP16_add(&B,&B,&B);
FP16_norm(&B);
    FP16_sqr(&C,&(x->c));

    FP16_mul(&D,&(x->a),&(x->b));
    FP16_add(&D,&D,&D);

    FP16_add(&(w->c),&(x->a),&(x->c));
    FP16_add(&(w->c),&(x->b),&(w->c));
FP16_norm(&(w->c));	

    FP16_sqr(&(w->c),&(w->c));

    FP16_copy(&(w->a),&A);
    FP16_add(&A,&A,&B);

    FP16_norm(&A);

    FP16_add(&A,&A,&C);
    FP16_add(&A,&A,&D);

    FP16_norm(&A);

    FP16_neg(&A,&A);
    FP16_times_i(&B);
    FP16_times_i(&C);

    FP16_add(&(w->a),&(w->a),&B);
    FP16_add(&(w->b),&C,&D);
    FP16_add(&(w->c),&(w->c),&A);

    FP48_norm(w);
}

/* FP48 full multiplication w=w*y */


/* SU= 896 */
/* FP48 full multiplication w=w*y */
void YYY::FP48_mul(FP48 *w,FP48 *y)
{
    FP16 z0,z1,z2,z3,t0,t1;

    FP16_mul(&z0,&(w->a),&(y->a));
    FP16_mul(&z2,&(w->b),&(y->b));  //

    FP16_add(&t0,&(w->a),&(w->b));
    FP16_add(&t1,&(y->a),&(y->b));  //

FP16_norm(&t0);
FP16_norm(&t1);

    FP16_mul(&z1,&t0,&t1);
    FP16_add(&t0,&(w->b),&(w->c));
    FP16_add(&t1,&(y->b),&(y->c));  //

FP16_norm(&t0);
FP16_norm(&t1);

    FP16_mul(&z3,&t0,&t1);

    FP16_neg(&t0,&z0);
    FP16_neg(&t1,&z2);

    FP16_add(&z1,&z1,&t0);   // z1=z1-z0
//    FP16_norm(&z1);
    FP16_add(&(w->b),&z1,&t1);
// z1=z1-z2
    FP16_add(&z3,&z3,&t1);        // z3=z3-z2
    FP16_add(&z2,&z2,&t0);        // z2=z2-z0

    FP16_add(&t0,&(w->a),&(w->c));
    FP16_add(&t1,&(y->a),&(y->c));

FP16_norm(&t0);
FP16_norm(&t1);

    FP16_mul(&t0,&t1,&t0);
    FP16_add(&z2,&z2,&t0);

    FP16_mul(&t0,&(w->c),&(y->c));
    FP16_neg(&t1,&t0);

    FP16_add(&(w->c),&z2,&t1);
    FP16_add(&z3,&z3,&t1);
    FP16_times_i(&t0);
    FP16_add(&(w->b),&(w->b),&t0);
FP16_norm(&z3);
    FP16_times_i(&z3);
    FP16_add(&(w->a),&z0,&z3);

    FP48_norm(w);
}

/* FP48 multiplication w=w*y */
/* SU= 744 */
/* catering for special case that arises from special form of ATE pairing line function */
void YYY::FP48_smul(FP48 *w,FP48 *y,int type)
{
    FP16 z0,z1,z2,z3,t0,t1;

	if (type==D_TYPE)
	{ // y->c is 0

		FP16_copy(&z3,&(w->b));
		FP16_mul(&z0,&(w->a),&(y->a));

		FP16_pmul(&z2,&(w->b),&(y->b).a);
		FP16_add(&(w->b),&(w->a),&(w->b));
		FP16_copy(&t1,&(y->a));
		FP8_add(&t1.a,&t1.a,&(y->b).a);

		FP16_norm(&t1);
		FP16_norm(&(w->b));

		FP16_mul(&(w->b),&(w->b),&t1);
		FP16_add(&z3,&z3,&(w->c));
		FP16_norm(&z3);
		FP16_pmul(&z3,&z3,&(y->b).a);
		FP16_neg(&t0,&z0);
		FP16_neg(&t1,&z2);

		FP16_add(&(w->b),&(w->b),&t0);   // z1=z1-z0
//    FP16_norm(&(w->b));
		FP16_add(&(w->b),&(w->b),&t1);   // z1=z1-z2

		FP16_add(&z3,&z3,&t1);        // z3=z3-z2
		FP16_add(&z2,&z2,&t0);        // z2=z2-z0

		FP16_add(&t0,&(w->a),&(w->c));

		FP16_norm(&t0);
		FP16_norm(&z3);

		FP16_mul(&t0,&(y->a),&t0);
		FP16_add(&(w->c),&z2,&t0);

		FP16_times_i(&z3);
		FP16_add(&(w->a),&z0,&z3);
	}

	if (type==M_TYPE)
	{ // y->b is zero
		FP16_mul(&z0,&(w->a),&(y->a));
		FP16_add(&t0,&(w->a),&(w->b));
		FP16_norm(&t0);

		FP16_mul(&z1,&t0,&(y->a));
		FP16_add(&t0,&(w->b),&(w->c));
		FP16_norm(&t0);

		FP16_pmul(&z3,&t0,&(y->c).b);
		FP16_times_i(&z3);

		FP16_neg(&t0,&z0);
		FP16_add(&z1,&z1,&t0);   // z1=z1-z0

		FP16_copy(&(w->b),&z1);

		FP16_copy(&z2,&t0);

		FP16_add(&t0,&(w->a),&(w->c));
		FP16_add(&t1,&(y->a),&(y->c));

		FP16_norm(&t0);
		FP16_norm(&t1);

		FP16_mul(&t0,&t1,&t0);
		FP16_add(&z2,&z2,&t0);

		FP16_pmul(&t0,&(w->c),&(y->c).b);
		FP16_times_i(&t0);
		FP16_neg(&t1,&t0);
		FP16_times_i(&t0);

		FP16_add(&(w->c),&z2,&t1);
		FP16_add(&z3,&z3,&t1);

		FP16_add(&(w->b),&(w->b),&t0);
		FP16_norm(&z3);
		FP16_times_i(&z3);
		FP16_add(&(w->a),&z0,&z3);
	}
    FP48_norm(w);
}

/* Set w=1/x */
/* SU= 600 */
void YYY::FP48_inv(FP48 *w,FP48 *x)
{
    FP16 f0,f1,f2,f3;
//    FP48_norm(x);

    FP16_sqr(&f0,&(x->a));
    FP16_mul(&f1,&(x->b),&(x->c));
    FP16_times_i(&f1);
    FP16_sub(&f0,&f0,&f1);  /* y.a */
	FP16_norm(&f0); 		

    FP16_sqr(&f1,&(x->c));
    FP16_times_i(&f1);
    FP16_mul(&f2,&(x->a),&(x->b));
    FP16_sub(&f1,&f1,&f2);  /* y.b */
	FP16_norm(&f1); 

    FP16_sqr(&f2,&(x->b));
    FP16_mul(&f3,&(x->a),&(x->c));
    FP16_sub(&f2,&f2,&f3);  /* y.c */
	FP16_norm(&f2); 

    FP16_mul(&f3,&(x->b),&f2);
    FP16_times_i(&f3);
    FP16_mul(&(w->a),&f0,&(x->a));
    FP16_add(&f3,&(w->a),&f3);
    FP16_mul(&(w->c),&f1,&(x->c));
    FP16_times_i(&(w->c));



    FP16_add(&f3,&(w->c),&f3);
	FP16_norm(&f3);
	
    FP16_inv(&f3,&f3);
    FP16_mul(&(w->a),&f0,&f3);
    FP16_mul(&(w->b),&f1,&f3);
    FP16_mul(&(w->c),&f2,&f3);

}

/* constant time powering by small integer of max length bts */

void YYY::FP48_pinpow(FP48 *r,int e,int bts)
{
    int i,b;
    FP48 R[2];

    FP48_one(&R[0]);
    FP48_copy(&R[1],r);

    for (i=bts-1; i>=0; i--)
    {
        b=(e>>i)&1;
        FP48_mul(&R[1-b],&R[b]);
        FP48_usqr(&R[b],&R[b]);
    }
    FP48_copy(r,&R[0]);
}

/* Compressed powering of unitary elements y=x^(e mod r) */

void YYY::FP48_compow(FP16 *c,FP48 *x,BIG e,BIG r)
{
    FP48 g1,g2;
	FP16 cp,cpm1,cpm2;
    FP2 f;
	BIG q,a,b,m;

    BIG_rcopy(a,Fra);
    BIG_rcopy(b,Frb);
    FP2_from_BIGs(&f,a,b);

    BIG_rcopy(q,Modulus);

    FP48_copy(&g1,x);
	FP48_copy(&g2,x);

    BIG_copy(m,q);
    BIG_mod(m,r);

    BIG_copy(a,e);
    BIG_mod(a,m);

    BIG_copy(b,e);
    BIG_sdiv(b,m);

    FP48_trace(c,&g1);

	if (BIG_iszilch(b))
	{
		FP16_xtr_pow(c,c,e);
		return;
	}

    FP48_frob(&g2,&f,1);
    FP48_trace(&cp,&g2);
    FP48_conj(&g1,&g1);
    FP48_mul(&g2,&g1);
    FP48_trace(&cpm1,&g2);
    FP48_mul(&g2,&g1);

    FP48_trace(&cpm2,&g2);

    FP16_xtr_pow2(c,&cp,c,&cpm1,&cpm2,a,b);

}

/* Note this is simple square and multiply, so not side-channel safe */

void YYY::FP48_pow(FP48 *r,FP48 *a,BIG b)
{
    FP48 w;
    BIG b3;
    int i,nb,bt;
    BIG_norm(b);
	BIG_pmul(b3,b,3);
	BIG_norm(b3);

    FP48_copy(&w,a);

	nb=BIG_nbits(b3);
	for (i=nb-2;i>=1;i--)
	{
		FP48_usqr(&w,&w);
		bt=BIG_bit(b3,i)-BIG_bit(b,i);
		if (bt==1)
			FP48_mul(&w,a);
		if (bt==-1)
		{
			FP48_conj(a,a);
			FP48_mul(&w,a);
			FP48_conj(a,a);
		}
	}

	FP48_copy(r,&w);
	FP48_reduce(r);
}


/* SU= 528 */
/* set r=a^b */
/* Note this is simple square and multiply, so not side-channel safe 

void YYY::FP48_ppow(FP48 *r,FP48 *a,BIG b)
{
    FP48 w;
    BIG z,zilch;
    int bt;
    BIG_zero(zilch);
    BIG_norm(b);
    BIG_copy(z,b);
    FP48_copy(&w,a);
    FP48_one(r);

    while(1)
    {
        bt=BIG_parity(z);
        BIG_shr(z,1);
        if (bt)
		{
			//printf("In mul\n");
            FP48_mul(r,&w);
			//printf("Out of mul\n");
		}
        if (BIG_comp(z,zilch)==0) break;
		//printf("In sqr\n");
        FP48_sqr(&w,&w);
		//printf("Out of sqr\n");
    }

    FP48_reduce(r);
}  */

/* p=q0^u0.q1^u1.q2^u2.q3^u3... */
/* Side channel attack secure */
// Bos & Costello https://eprint.iacr.org/2013/458.pdf
// Faz-Hernandez & Longa & Sanchez  https://eprint.iacr.org/2013/158.pdf

void YYY::FP48_pow16(FP48 *p,FP48 *q,BIG u[16])
{
    int i,j,k,nb,pb1,pb2,pb3,pb4,bt;
	FP48 g1[8],g2[8],g3[8],g4[8],r;
	BIG t[16],mt;
    sign8 w1[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s1[NLEN_XXX*BASEBITS_XXX+1];
    sign8 w2[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s2[NLEN_XXX*BASEBITS_XXX+1];
    sign8 w3[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s3[NLEN_XXX*BASEBITS_XXX+1];
    sign8 w4[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s4[NLEN_XXX*BASEBITS_XXX+1];
    FP fx,fy;
	FP2 X;

    FP_rcopy(&fx,Fra);
    FP_rcopy(&fy,Frb);
    FP2_from_FPs(&X,&fx,&fy);

    for (i=0; i<16; i++)
        BIG_copy(t[i],u[i]);

// Precomputed table
    FP48_copy(&g1[0],&q[0]); // q[0]
    FP48_copy(&g1[1],&g1[0]);
	FP48_mul(&g1[1],&q[1]);	// q[0].q[1]
    FP48_copy(&g1[2],&g1[0]);
	FP48_mul(&g1[2],&q[2]);	// q[0].q[2]
	FP48_copy(&g1[3],&g1[1]);
	FP48_mul(&g1[3],&q[2]);	// q[0].q[1].q[2]
	FP48_copy(&g1[4],&g1[0]);
	FP48_mul(&g1[4],&q[3]);  // q[0].q[3]
	FP48_copy(&g1[5],&g1[1]);
	FP48_mul(&g1[5],&q[3]);	// q[0].q[1].q[3]
	FP48_copy(&g1[6],&g1[2]);
	FP48_mul(&g1[6],&q[3]);	// q[0].q[2].q[3]
	FP48_copy(&g1[7],&g1[3]);
	FP48_mul(&g1[7],&q[3]);	// q[0].q[1].q[2].q[3]

// Use Frobenius

	for (i=0;i<8;i++)
	{
		FP48_copy(&g2[i],&g1[i]);
		FP48_frob(&g2[i],&X,4);

		FP48_copy(&g3[i],&g2[i]);
		FP48_frob(&g3[i],&X,4);

		FP48_copy(&g4[i],&g3[i]);
		FP48_frob(&g4[i],&X,4);
	}

// Make them odd
	pb1=1-BIG_parity(t[0]);
	BIG_inc(t[0],pb1);
	BIG_norm(t[0]);

	pb2=1-BIG_parity(t[4]);
	BIG_inc(t[4],pb2);
	BIG_norm(t[4]);

	pb3=1-BIG_parity(t[8]);
	BIG_inc(t[8],pb3);
	BIG_norm(t[8]);

	pb4=1-BIG_parity(t[12]);
	BIG_inc(t[12],pb4);
	BIG_norm(t[12]);

// Number of bits
    BIG_zero(mt);
    for (i=0; i<16; i++)
    {
        BIG_or(mt,mt,t[i]);
    }
    nb=1+BIG_nbits(mt);

// Sign pivot 
	s1[nb-1]=1;
	s2[nb-1]=1;
	s3[nb-1]=1;
	s4[nb-1]=1;
	for (i=0;i<nb-1;i++)
	{
        BIG_fshr(t[0],1);
		s1[i]=2*BIG_parity(t[0])-1;
        BIG_fshr(t[4],1);
		s2[i]=2*BIG_parity(t[4])-1;
        BIG_fshr(t[8],1);
		s3[i]=2*BIG_parity(t[8])-1;
        BIG_fshr(t[12],1);
		s4[i]=2*BIG_parity(t[12])-1;
	}

// Recoded exponents
    for (i=0; i<nb; i++)
    {
		w1[i]=0;
		k=1;
		for (j=1; j<4; j++)
		{
			bt=s1[i]*BIG_parity(t[j]);
			BIG_fshr(t[j],1);

			BIG_dec(t[j],(bt>>1));
			BIG_norm(t[j]);
			w1[i]+=bt*k;
			k*=2;
        }

		w2[i]=0;
		k=1;
		for (j=5; j<8; j++)
		{
			bt=s2[i]*BIG_parity(t[j]);
			BIG_fshr(t[j],1);

			BIG_dec(t[j],(bt>>1));
			BIG_norm(t[j]);
			w2[i]+=bt*k;
			k*=2;
        }

		w3[i]=0;
		k=1;
		for (j=9; j<12; j++)
		{
			bt=s3[i]*BIG_parity(t[j]);
			BIG_fshr(t[j],1);

			BIG_dec(t[j],(bt>>1));
			BIG_norm(t[j]);
			w3[i]+=bt*k;
			k*=2;
        }

		w4[i]=0;
		k=1;
		for (j=13; j<16; j++)
		{
			bt=s4[i]*BIG_parity(t[j]);
			BIG_fshr(t[j],1);

			BIG_dec(t[j],(bt>>1));
			BIG_norm(t[j]);
			w4[i]+=bt*k;
			k*=2;
        }
    }	

// Main loop
	FP48_select(p,g1,2*w1[nb-1]+1);
	FP48_select(&r,g2,2*w2[nb-1]+1);
	FP48_mul(p,&r);
	FP48_select(&r,g3,2*w3[nb-1]+1);
	FP48_mul(p,&r);
	FP48_select(&r,g4,2*w4[nb-1]+1);
	FP48_mul(p,&r);
    for (i=nb-2; i>=0; i--)
    {
		FP48_usqr(p,p);
        FP48_select(&r,g1,2*w1[i]+s1[i]);
        FP48_mul(p,&r);
        FP48_select(&r,g2,2*w2[i]+s2[i]);
        FP48_mul(p,&r);
        FP48_select(&r,g3,2*w3[i]+s3[i]);
        FP48_mul(p,&r);
        FP48_select(&r,g4,2*w4[i]+s4[i]);
        FP48_mul(p,&r);
    }

// apply correction
	FP48_conj(&r,&q[0]);   
	FP48_mul(&r,p);
	FP48_cmove(p,&r,pb1);
	FP48_conj(&r,&q[4]);   
	FP48_mul(&r,p);
	FP48_cmove(p,&r,pb2);

	FP48_conj(&r,&q[8]);   
	FP48_mul(&r,p);
	FP48_cmove(p,&r,pb3);
	FP48_conj(&r,&q[12]);   
	FP48_mul(&r,p);
	FP48_cmove(p,&r,pb4);

	FP48_reduce(p);
}

/*
void YYY::FP48_pow16(FP48 *p,FP48 *q,BIG u[16])
{
    int i,j,a[4],nb,m;
    FP48 g[8],f[8],gg[8],ff[8],c,s[2];
    BIG t[16],mt;
    sign8 w[NLEN_XXX*BASEBITS_XXX+1];
    sign8 z[NLEN_XXX*BASEBITS_XXX+1];
    sign8 ww[NLEN_XXX*BASEBITS_XXX+1];
    sign8 zz[NLEN_XXX*BASEBITS_XXX+1];

    FP fx,fy;
	FP2 X;

    FP_rcopy(&fx,Fra);
    FP_rcopy(&fy,Frb);
    FP2_from_FPs(&X,&fx,&fy);

    for (i=0; i<16; i++)
        BIG_copy(t[i],u[i]);

    FP48_copy(&g[0],&q[0]);
    FP48_conj(&s[0],&q[1]);
    FP48_mul(&g[0],&s[0]);  // P/Q 
    FP48_copy(&g[1],&g[0]);
    FP48_copy(&g[2],&g[0]);
    FP48_copy(&g[3],&g[0]);
    FP48_copy(&g[4],&q[0]);
    FP48_mul(&g[4],&q[1]);  // P*Q 
    FP48_copy(&g[5],&g[4]);
    FP48_copy(&g[6],&g[4]);
    FP48_copy(&g[7],&g[4]);

    FP48_copy(&s[1],&q[2]);
    FP48_conj(&s[0],&q[3]);
    FP48_mul(&s[1],&s[0]);       // R/S 
    FP48_conj(&s[0],&s[1]);
    FP48_mul(&g[1],&s[0]);
    FP48_mul(&g[2],&s[1]);
    FP48_mul(&g[5],&s[0]);
    FP48_mul(&g[6],&s[1]);
    FP48_copy(&s[1],&q[2]);
    FP48_mul(&s[1],&q[3]);      // R*S 
    FP48_conj(&s[0],&s[1]);
    FP48_mul(&g[0],&s[0]);
    FP48_mul(&g[3],&s[1]);
    FP48_mul(&g[4],&s[0]);
    FP48_mul(&g[7],&s[1]);

// Use Frobenius

	for (i=0;i<8;i++)
	{
		FP48_copy(&f[i],&g[i]);
		FP48_frob(&f[i],&X,4);
	}

	for (i=0;i<8;i++)
	{
		FP48_copy(&gg[i],&f[i]);
		FP48_frob(&gg[i],&X,4);
	}

	for (i=0;i<8;i++)
	{
		FP48_copy(&ff[i],&gg[i]);
		FP48_frob(&ff[i],&X,4);
	}


    // if power is even add 1 to power, and add q to correction 
    FP48_one(&c);

    BIG_zero(mt);
    for (i=0; i<16; i++)
    {
        if (BIG_parity(t[i])==0)
        {
            BIG_inc(t[i],1);
            BIG_norm(t[i]);
            FP48_mul(&c,&q[i]);
        }
        BIG_add(mt,mt,t[i]);
        BIG_norm(mt);
    }

    FP48_conj(&c,&c);
    nb=1+BIG_nbits(mt);

    // convert exponents to signed 1-bit windows 
    for (j=0; j<nb; j++)
    {
        for (i=0; i<4; i++)
        {
            a[i]=BIG_lastbits(t[i],2)-2;
            BIG_dec(t[i],a[i]);
            BIG_norm(t[i]);
            BIG_fshr(t[i],1);
        }
        w[j]=8*a[0]+4*a[1]+2*a[2]+a[3];
    }
    w[nb]=8*BIG_lastbits(t[0],2)+4*BIG_lastbits(t[1],2)+2*BIG_lastbits(t[2],2)+BIG_lastbits(t[3],2);


    for (j=0; j<nb; j++)
    {
        for (i=0; i<4; i++)
        {
            a[i]=BIG_lastbits(t[i+4],2)-2;
            BIG_dec(t[i+4],a[i]);
            BIG_norm(t[i+4]);
            BIG_fshr(t[i+4],1);
        }
        z[j]=8*a[0]+4*a[1]+2*a[2]+a[3];
    }
    z[nb]=8*BIG_lastbits(t[4],2)+4*BIG_lastbits(t[5],2)+2*BIG_lastbits(t[6],2)+BIG_lastbits(t[7],2);

    for (j=0; j<nb; j++)
    {
        for (i=0; i<4; i++)
        {
            a[i]=BIG_lastbits(t[i+8],2)-2;
            BIG_dec(t[i+8],a[i]);
            BIG_norm(t[i+8]);
            BIG_fshr(t[i+8],1);
        }
        ww[j]=8*a[0]+4*a[1]+2*a[2]+a[3];
    }
    ww[nb]=8*BIG_lastbits(t[8],2)+4*BIG_lastbits(t[9],2)+2*BIG_lastbits(t[10],2)+BIG_lastbits(t[11],2);

    for (j=0; j<nb; j++)
    {
        for (i=0; i<4; i++)
        {
            a[i]=BIG_lastbits(t[i+12],2)-2;
            BIG_dec(t[i+12],a[i]);
            BIG_norm(t[i+12]);
            BIG_fshr(t[i+12],1);
        }
        zz[j]=8*a[0]+4*a[1]+2*a[2]+a[3];
    }
    zz[nb]=8*BIG_lastbits(t[12],2)+4*BIG_lastbits(t[13],2)+2*BIG_lastbits(t[14],2)+BIG_lastbits(t[15],2);

    FP48_copy(p,&g[(w[nb]-1)/2]);
    FP48_mul(p,&f[(z[nb]-1)/2]);
    FP48_mul(p,&gg[(ww[nb]-1)/2]);
    FP48_mul(p,&ff[(zz[nb]-1)/2]);

    for (i=nb-1; i>=0; i--)
    {
		FP48_usqr(p,p);

        m=w[i]>>7;
        j=(w[i]^m)-m;  // j=abs(w[i]) 
        j=(j-1)/2;
        FP48_copy(&s[0],&g[j]);
        FP48_conj(&s[1],&g[j]);
        FP48_mul(p,&s[m&1]);

        m=z[i]>>7;
        j=(z[i]^m)-m;  // j=abs(w[i]) 
        j=(j-1)/2;
        FP48_copy(&s[0],&f[j]);
        FP48_conj(&s[1],&f[j]);
        FP48_mul(p,&s[m&1]);

        m=ww[i]>>7;
        j=(ww[i]^m)-m;  // j=abs(w[i]) 
        j=(j-1)/2;
        FP48_copy(&s[0],&gg[j]);
        FP48_conj(&s[1],&gg[j]);
        FP48_mul(p,&s[m&1]);

        m=zz[i]>>7;
        j=(zz[i]^m)-m;  // j=abs(w[i]) 
        j=(j-1)/2;
        FP48_copy(&s[0],&ff[j]);
        FP48_conj(&s[1],&ff[j]);
        FP48_mul(p,&s[m&1]);

    }
    FP48_mul(p,&c); // apply correction 
    FP48_reduce(p);
}
*/

/* Set w=w^p using Frobenius */
/* SU= 160 */
void YYY::FP48_frob(FP48 *w,FP2 *f,int n)
{
	int i;
	FP8 X2,X4;
	FP4 F;
    FP2 f3,f2;				// f=(1+i)^(p-19)/24
    FP2_sqr(&f2,f);     // 
    FP2_mul(&f3,&f2,f); // f3=f^3=(1+i)^(p-19)/8

	FP2_mul_ip(&f3);
	FP2_norm(&f3);
	FP2_mul_ip(&f3);    // f3 = (1+i)^16/8.(1+i)^(p-19)/8 = (1+i)^(p-3)/8 
	FP2_norm(&f3);

	for (i=0;i<n;i++)
	{
		FP16_frob(&(w->a),&f3);   // a=a^p
		FP16_frob(&(w->b),&f3);   // b=b^p
		FP16_frob(&(w->c),&f3);   // c=c^p
  
		FP16_qmul(&(w->b),&(w->b),f); FP16_times_i4(&(w->b)); FP16_times_i2(&(w->b)); 
		FP16_qmul(&(w->c),&(w->c),&f2); FP16_times_i4(&(w->c)); FP16_times_i4(&(w->c)); FP16_times_i4(&(w->c)); 

	}
}

/* SU= 8 */
/* normalise all components of w */
void YYY::FP48_norm(FP48 *w)
{
    FP16_norm(&(w->a));
    FP16_norm(&(w->b));
    FP16_norm(&(w->c));
}

/* SU= 8 */
/* reduce all components of w */
void YYY::FP48_reduce(FP48 *w)
{
    FP16_reduce(&(w->a));
    FP16_reduce(&(w->b));
    FP16_reduce(&(w->c));
}

/* trace function w=trace(x) */
/* SU= 8 */
void YYY::FP48_trace(FP16 *w,FP48 *x)
{
    FP16_imul(w,&(x->a),3);
    FP16_reduce(w);
}

/* SU= 8 */
/* Output w in hex */
void YYY::FP48_output(FP48 *w)
{
    printf("[");
    FP16_output(&(w->a));
    printf(",");
    FP16_output(&(w->b));
    printf(",");
    FP16_output(&(w->c));
    printf("]");
}

/* Convert g to octet string w */
void YYY::FP48_toOctet(octet *W,FP48 *g)
{
    BIG a;
    W->len=48*MODBYTES_XXX;

    FP_redc(a,&(g->a.a.a.a.a));
    BIG_toBytes(&(W->val[0]),a);
    FP_redc(a,&(g->a.a.a.a.b));
    BIG_toBytes(&(W->val[MODBYTES_XXX]),a);
    
	FP_redc(a,&(g->a.a.a.b.a));
    BIG_toBytes(&(W->val[2*MODBYTES_XXX]),a);
	FP_redc(a,&(g->a.a.a.b.b));
    BIG_toBytes(&(W->val[3*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.a.b.a.a));
    BIG_toBytes(&(W->val[4*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.a.b.a.b));
    BIG_toBytes(&(W->val[5*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.a.b.b.a));
    BIG_toBytes(&(W->val[6*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.a.b.b.b));
    BIG_toBytes(&(W->val[7*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.b.a.a.a));
    BIG_toBytes(&(W->val[8*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.b.a.a.b));
    BIG_toBytes(&(W->val[9*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.b.a.b.a));
    BIG_toBytes(&(W->val[10*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.b.a.b.b));
    BIG_toBytes(&(W->val[11*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.b.b.a.a));
    BIG_toBytes(&(W->val[12*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.b.b.a.b));
    BIG_toBytes(&(W->val[13*MODBYTES_XXX]),a);

    FP_redc(a,&(g->a.b.b.b.a));
    BIG_toBytes(&(W->val[14*MODBYTES_XXX]),a);
    FP_redc(a,&(g->a.b.b.b.b));
    BIG_toBytes(&(W->val[15*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.a.a.a.a));
    BIG_toBytes(&(W->val[16*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.a.a.a.b));
    BIG_toBytes(&(W->val[17*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.a.a.b.a));
    BIG_toBytes(&(W->val[18*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.a.a.b.b));
    BIG_toBytes(&(W->val[19*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.a.b.a.a));
    BIG_toBytes(&(W->val[20*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.a.b.a.b));
    BIG_toBytes(&(W->val[21*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.a.b.b.a));
    BIG_toBytes(&(W->val[22*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.a.b.b.b));
    BIG_toBytes(&(W->val[23*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.b.a.a.a));
    BIG_toBytes(&(W->val[24*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.b.a.a.b));
    BIG_toBytes(&(W->val[25*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.b.a.b.a));
    BIG_toBytes(&(W->val[26*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.b.a.b.b));
    BIG_toBytes(&(W->val[27*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.b.b.a.a));
    BIG_toBytes(&(W->val[28*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.b.b.a.b));
    BIG_toBytes(&(W->val[29*MODBYTES_XXX]),a);

    FP_redc(a,&(g->b.b.b.b.a));
    BIG_toBytes(&(W->val[30*MODBYTES_XXX]),a);
    FP_redc(a,&(g->b.b.b.b.b));
    BIG_toBytes(&(W->val[31*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.a.a.a.a));
    BIG_toBytes(&(W->val[32*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.a.a.a.b));
    BIG_toBytes(&(W->val[33*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.a.a.b.a));
    BIG_toBytes(&(W->val[34*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.a.a.b.b));
    BIG_toBytes(&(W->val[35*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.a.b.a.a));
    BIG_toBytes(&(W->val[36*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.a.b.a.b));
    BIG_toBytes(&(W->val[37*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.a.b.b.a));
    BIG_toBytes(&(W->val[38*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.a.b.b.b));
    BIG_toBytes(&(W->val[39*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.b.a.a.a));
    BIG_toBytes(&(W->val[40*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.b.a.a.b));
    BIG_toBytes(&(W->val[41*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.b.a.b.a));
    BIG_toBytes(&(W->val[42*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.b.a.b.b));
    BIG_toBytes(&(W->val[43*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.b.b.a.a));
    BIG_toBytes(&(W->val[44*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.b.b.a.b));
    BIG_toBytes(&(W->val[45*MODBYTES_XXX]),a);

    FP_redc(a,&(g->c.b.b.b.a));
    BIG_toBytes(&(W->val[46*MODBYTES_XXX]),a);
    FP_redc(a,&(g->c.b.b.b.b));
    BIG_toBytes(&(W->val[47*MODBYTES_XXX]),a);

}

/* Restore g from octet string w */
void YYY::FP48_fromOctet(FP48 *g,octet *W)
{
	BIG b;

    BIG_fromBytes(b,&W->val[0]);
    FP_nres(&(g->a.a.a.a.a),b);
    BIG_fromBytes(b,&W->val[MODBYTES_XXX]);
    FP_nres(&(g->a.a.a.a.b),b);

    BIG_fromBytes(b,&W->val[2*MODBYTES_XXX]);
    FP_nres(&(g->a.a.a.b.a),b);
    BIG_fromBytes(b,&W->val[3*MODBYTES_XXX]);
    FP_nres(&(g->a.a.a.b.b),b);

    BIG_fromBytes(b,&W->val[4*MODBYTES_XXX]);
    FP_nres(&(g->a.a.b.a.a),b);
    BIG_fromBytes(b,&W->val[5*MODBYTES_XXX]);
    FP_nres(&(g->a.a.b.a.b),b);

    BIG_fromBytes(b,&W->val[6*MODBYTES_XXX]);
    FP_nres(&(g->a.a.b.b.a),b);
    BIG_fromBytes(b,&W->val[7*MODBYTES_XXX]);
    FP_nres(&(g->a.a.b.b.b),b);

    BIG_fromBytes(b,&W->val[8*MODBYTES_XXX]);
    FP_nres(&(g->a.b.a.a.a),b);
    BIG_fromBytes(b,&W->val[9*MODBYTES_XXX]);
    FP_nres(&(g->a.b.a.a.b),b);

    BIG_fromBytes(b,&W->val[10*MODBYTES_XXX]);
    FP_nres(&(g->a.b.a.b.a),b);
    BIG_fromBytes(b,&W->val[11*MODBYTES_XXX]);
    FP_nres(&(g->a.b.a.b.b),b);

    BIG_fromBytes(b,&W->val[12*MODBYTES_XXX]);
    FP_nres(&(g->a.b.b.a.a),b);
    BIG_fromBytes(b,&W->val[13*MODBYTES_XXX]);
    FP_nres(&(g->a.b.b.a.b),b);

    BIG_fromBytes(b,&W->val[14*MODBYTES_XXX]);
    FP_nres(&(g->a.b.b.b.a),b);
    BIG_fromBytes(b,&W->val[15*MODBYTES_XXX]);
    FP_nres(&(g->a.b.b.b.b),b);

    BIG_fromBytes(b,&W->val[16*MODBYTES_XXX]);
    FP_nres(&(g->b.a.a.a.a),b);
    BIG_fromBytes(b,&W->val[17*MODBYTES_XXX]);
    FP_nres(&(g->b.a.a.a.b),b);

    BIG_fromBytes(b,&W->val[18*MODBYTES_XXX]);
    FP_nres(&(g->b.a.a.b.a),b);
    BIG_fromBytes(b,&W->val[19*MODBYTES_XXX]);
    FP_nres(&(g->b.a.a.b.b),b);

    BIG_fromBytes(b,&W->val[20*MODBYTES_XXX]);
    FP_nres(&(g->b.a.b.a.a),b);
    BIG_fromBytes(b,&W->val[21*MODBYTES_XXX]);
    FP_nres(&(g->b.a.b.a.b),b);

    BIG_fromBytes(b,&W->val[22*MODBYTES_XXX]);
    FP_nres(&(g->b.a.b.b.a),b);
    BIG_fromBytes(b,&W->val[23*MODBYTES_XXX]);
    FP_nres(&(g->b.a.b.b.b),b);

    BIG_fromBytes(b,&W->val[24*MODBYTES_XXX]);
    FP_nres(&(g->b.b.a.a.a),b);
    BIG_fromBytes(b,&W->val[25*MODBYTES_XXX]);
    FP_nres(&(g->b.b.a.a.b),b);

    BIG_fromBytes(b,&W->val[26*MODBYTES_XXX]);
    FP_nres(&(g->b.b.a.b.a),b);
    BIG_fromBytes(b,&W->val[27*MODBYTES_XXX]);
    FP_nres(&(g->b.b.a.b.b),b);

    BIG_fromBytes(b,&W->val[28*MODBYTES_XXX]);
    FP_nres(&(g->b.b.b.a.a),b);
    BIG_fromBytes(b,&W->val[29*MODBYTES_XXX]);
    FP_nres(&(g->b.b.b.a.b),b);

    BIG_fromBytes(b,&W->val[30*MODBYTES_XXX]);
    FP_nres(&(g->b.b.b.b.a),b);
    BIG_fromBytes(b,&W->val[31*MODBYTES_XXX]);
    FP_nres(&(g->b.b.b.b.b),b);

    BIG_fromBytes(b,&W->val[32*MODBYTES_XXX]);
    FP_nres(&(g->c.a.a.a.a),b);
    BIG_fromBytes(b,&W->val[33*MODBYTES_XXX]);
    FP_nres(&(g->c.a.a.a.b),b);

    BIG_fromBytes(b,&W->val[34*MODBYTES_XXX]);
    FP_nres(&(g->c.a.a.b.a),b);
    BIG_fromBytes(b,&W->val[35*MODBYTES_XXX]);
    FP_nres(&(g->c.a.a.b.b),b);

    BIG_fromBytes(b,&W->val[36*MODBYTES_XXX]);
    FP_nres(&(g->c.a.b.a.a),b);
    BIG_fromBytes(b,&W->val[37*MODBYTES_XXX]);
    FP_nres(&(g->c.a.b.a.b),b);

    BIG_fromBytes(b,&W->val[38*MODBYTES_XXX]);
    FP_nres(&(g->c.a.b.b.a),b);
    BIG_fromBytes(b,&W->val[39*MODBYTES_XXX]);
    FP_nres(&(g->c.a.b.b.b),b);

    BIG_fromBytes(b,&W->val[40*MODBYTES_XXX]);
    FP_nres(&(g->c.b.a.a.a),b);
    BIG_fromBytes(b,&W->val[41*MODBYTES_XXX]);
    FP_nres(&(g->c.b.a.a.b),b);

    BIG_fromBytes(b,&W->val[42*MODBYTES_XXX]);
    FP_nres(&(g->c.b.a.b.a),b);
    BIG_fromBytes(b,&W->val[43*MODBYTES_XXX]);
    FP_nres(&(g->c.b.a.b.b),b);

    BIG_fromBytes(b,&W->val[44*MODBYTES_XXX]);
    FP_nres(&(g->c.b.b.a.a),b);
    BIG_fromBytes(b,&W->val[45*MODBYTES_XXX]);
    FP_nres(&(g->c.b.b.a.b),b);

    BIG_fromBytes(b,&W->val[46*MODBYTES_XXX]);
    FP_nres(&(g->c.b.b.b.a),b);
    BIG_fromBytes(b,&W->val[47*MODBYTES_XXX]);
    FP_nres(&(g->c.b.b.b.b),b);

}

/* Move b to a if d=1 */
void YYY::FP48_cmove(FP48 *f,FP48 *g,int d)
{
    FP16_cmove(&(f->a),&(g->a),d);
    FP16_cmove(&(f->b),&(g->b),d);
    FP16_cmove(&(f->c),&(g->c),d);
}

/*
using namespace YYY;

int main() {
	int i;
	FP2 f,w0,w1,X;
	FP4 f0,f1;
	FP16 t0,t1,t2;
	FP48 w,t,lv;
	BIG a,b;
	BIG p;


	char raw[100];
	csprng RNG;                // Crypto Strong RNG 

	for (i=0; i<100; i++) raw[i]=i;

	BIG_rcopy(a,Fra);
    BIG_rcopy(b,Frb);
	FP2_from_BIGs(&X,a,b);



    RAND_seed(&RNG,100,raw);   // initialise strong RNG 

	BIG_rcopy(p,Modulus);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f0,&w0,&w1);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f1,&w0,&w1);
	FP16_from_FP4s(&t0,&f0,&f1);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f0,&w0,&w1);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f1,&w0,&w1);
	FP16_from_FP4s(&t1,&f0,&f1);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f0,&w0,&w1);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w0,a,b);

	BIG_randomnum(a,p,&RNG);
	BIG_randomnum(b,p,&RNG);
	FP2_from_BIGs(&w1,a,b);

	FP4_from_FP2s(&f1,&w0,&w1);
	FP16_from_FP4s(&t2,&f0,&f1);

	FP48_from_FP16s(&w,&t0,&t1,&t2);


	FP48_copy(&t,&w);

	printf("w= ");
	FP48_output(&w);
	printf("\n");

	FP48_norm(&w);

	printf("w^p= ");
	FP48_frob(&w,&X);
	FP48_output(&w);
	printf("\n");	

//	printf("p.w= ");
//	FP48_ppow(&t,&t,p);
//	FP48_output(&t);
//	printf("\n");	

	printf("1/w= ");
	FP48_inv(&t,&w);
	FP48_output(&t);
	printf("\n");	

	printf("w= ");
	FP48_inv(&w,&t);
	FP48_output(&w);
	printf("\n");	

	return 0;
}

*/
