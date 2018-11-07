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

//#include <iostream>
#include "ecp4_ZZZ.h"

using namespace std;
using namespace XXX;
using namespace YYY;

int ZZZ::ECP4_isinf(ECP4 *P)
{
//	if (P->inf) return 1;
	return (FP4_iszilch(&(P->x)) & FP4_iszilch(&(P->z)));
}

/* Set P=Q */
void ZZZ::ECP4_copy(ECP4 *P,ECP4 *Q)
{
//    P->inf=Q->inf;
    FP4_copy(&(P->x),&(Q->x));
    FP4_copy(&(P->y),&(Q->y));
    FP4_copy(&(P->z),&(Q->z));
}

/* set P to Infinity */
void ZZZ::ECP4_inf(ECP4 *P)
{
//    P->inf=1;
    FP4_zero(&(P->x));
    FP4_one(&(P->y));
    FP4_zero(&(P->z));
}

/* Conditional move Q to P dependant on d */
static void ECP4_cmove(ZZZ::ECP4 *P,ZZZ::ECP4 *Q,int d)
{
    FP4_cmove(&(P->x),&(Q->x),d);
    FP4_cmove(&(P->y),&(Q->y),d);
    FP4_cmove(&(P->z),&(Q->z),d);
//    d=~(d-1);
//    P->inf^=(P->inf^Q->inf)&d;
}

/* return 1 if b==c, no branching */
static int teq(sign32 b,sign32 c)
{
    sign32 x=b^c;
    x-=1;  // if x=0, x now -1
    return (int)((x>>31)&1);
}

/* Constant time select from pre-computed table */
static void ECP4_select(ZZZ::ECP4 *P,ZZZ::ECP4 W[],sign32 b)
{
    ZZZ::ECP4 MP;
    sign32 m=b>>31;
    sign32 babs=(b^m)-m;

    babs=(babs-1)/2;

    ECP4_cmove(P,&W[0],teq(babs,0));  // conditional move
    ECP4_cmove(P,&W[1],teq(babs,1));
    ECP4_cmove(P,&W[2],teq(babs,2));
    ECP4_cmove(P,&W[3],teq(babs,3));
    ECP4_cmove(P,&W[4],teq(babs,4));
    ECP4_cmove(P,&W[5],teq(babs,5));
    ECP4_cmove(P,&W[6],teq(babs,6));
    ECP4_cmove(P,&W[7],teq(babs,7));

    ECP4_copy(&MP,P);
    ECP4_neg(&MP);  // minus P
    ECP4_cmove(P,&MP,(int)(m&1));
}

/* Make P affine (so z=1) */
void ZZZ::ECP4_affine(ECP4 *P)
{
    FP4 one,iz;
    if (ECP4_isinf(P)) return;

    FP4_one(&one);
    if (FP4_isunity(&(P->z)))
    {
        FP4_reduce(&(P->x));
        FP4_reduce(&(P->y));
        return;
    }

    FP4_inv(&iz,&(P->z));
    FP4_mul(&(P->x),&(P->x),&iz);
    FP4_mul(&(P->y),&(P->y),&iz);

    FP4_reduce(&(P->x));
    FP4_reduce(&(P->y));
    FP4_copy(&(P->z),&one);
}

/* return 1 if P==Q, else 0 */
/* SU= 312 */
int ZZZ::ECP4_equals(ECP4 *P,ECP4 *Q)
{
    FP4 a,b;
 //   if (ECP4_isinf(P) && ECP4_isinf(Q)) return 1;
 //   if (ECP4_isinf(P) || ECP4_isinf(Q)) return 0;

    FP4_mul(&a,&(P->x),&(Q->z));
    FP4_mul(&b,&(Q->x),&(P->z));
    if (!FP4_equals(&a,&b)) return 0;

    FP4_mul(&a,&(P->y),&(Q->z));
    FP4_mul(&b,&(Q->y),&(P->z));
    if (!FP4_equals(&a,&b)) return 0;
    return 1;

}

/* extract x, y from point P */
int ZZZ::ECP4_get(FP4 *x,FP4 *y,ECP4 *P)
{
	ECP4 W;
	ECP4_copy(&W,P);
	ECP4_affine(&W);
    if (ECP4_isinf(&W)) return -1;
	//ECP4_affine(P);
    FP4_copy(y,&(W.y));
    FP4_copy(x,&(W.x));
    return 0;
}

/* Output point P */
void ZZZ::ECP4_output(ECP4 *P)
{
    FP4 x,y;
    if (ECP4_isinf(P))
    {
        printf("Infinity\n");
        return;
    }
    ECP4_get(&x,&y,P);
    printf("(");
    FP4_output(&x);
    printf(",");
    FP4_output(&y);
    printf(")\n");
}

/* Convert Q to octet string */
void ZZZ::ECP4_toOctet(octet *W,ECP4 *Q)
{
	BIG b;
	FP4 qx,qy;
	FP2 pa,pb;

    ECP4_get(&qx,&qy,Q);

	FP2_copy(&pa,&(qx.a));
	FP2_copy(&pb,&(qx.b));

	FP_redc(b,&(pa.a));
    BIG_toBytes(&(W->val[0]),b);
    FP_redc(b,&(pa.b));
    BIG_toBytes(&(W->val[MODBYTES_XXX]),b);
    FP_redc(b,&(pb.a));
    BIG_toBytes(&(W->val[2*MODBYTES_XXX]),b);
    FP_redc(b,&(pb.b));
    BIG_toBytes(&(W->val[3*MODBYTES_XXX]),b);

	FP2_copy(&pa,&(qy.a));
	FP2_copy(&pb,&(qy.b));
	
	FP_redc(b,&(pa.a));
    BIG_toBytes(&(W->val[4*MODBYTES_XXX]),b);
    FP_redc(b,&(pa.b));
    BIG_toBytes(&(W->val[5*MODBYTES_XXX]),b);
    FP_redc(b,&(pb.a));
    BIG_toBytes(&(W->val[6*MODBYTES_XXX]),b);
    FP_redc(b,&(pb.b));
    BIG_toBytes(&(W->val[7*MODBYTES_XXX]),b);

    W->len=8*MODBYTES_XXX;
}

/* restore Q from octet string */
int ZZZ::ECP4_fromOctet(ECP4 *Q,octet *W)
{
	BIG b;
    FP4 qx,qy;
	FP2 pa,pb;

    BIG_fromBytes(b,&(W->val[0]));
	FP_nres(&(pa.a),b);
    BIG_fromBytes(b,&(W->val[MODBYTES_XXX]));
    FP_nres(&(pa.b),b);
    BIG_fromBytes(b,&(W->val[2*MODBYTES_XXX]));
    FP_nres(&(pb.a),b);
    BIG_fromBytes(b,&(W->val[3*MODBYTES_XXX]));
    FP_nres(&(pb.b),b);

	FP2_copy(&(qx.a),&pa);
	FP2_copy(&(qx.b),&pb);

    BIG_fromBytes(b,&(W->val[4*MODBYTES_XXX]));
	FP_nres(&(pa.a),b);
    BIG_fromBytes(b,&(W->val[5*MODBYTES_XXX]));
    FP_nres(&(pa.b),b);
    BIG_fromBytes(b,&(W->val[6*MODBYTES_XXX]));
    FP_nres(&(pb.a),b);
    BIG_fromBytes(b,&(W->val[7*MODBYTES_XXX]));
    FP_nres(&(pb.b),b);

	FP2_copy(&(qy.a),&pa);
	FP2_copy(&(qy.b),&pb);


    if (ECP4_set(Q,&qx,&qy)) return 1;
    return 0;
}

/* Calculate RHS of twisted curve equation x^3+B/i or x^3+Bi*/
void ZZZ::ECP4_rhs(FP4 *rhs,FP4 *x)
{
    /* calculate RHS of elliptic curve equation */
    FP4 t;
	FP2 t2;
    BIG b;
    FP4_sqr(&t,x);

    FP4_mul(rhs,&t,x);

    /* Assuming CURVE_A=0 */

    BIG_rcopy(b,CURVE_B);

    FP2_from_BIG(&t2,b);
	FP4_from_FP2(&t,&t2);

#if SEXTIC_TWIST_ZZZ == D_TYPE	
    FP4_div_i(&t);   /* IMPORTANT - here we use the correct SEXTIC twist of the curve */
#endif

#if SEXTIC_TWIST_ZZZ == M_TYPE	
    FP4_times_i(&t);   /* IMPORTANT - here we use the correct SEXTIC twist of the curve */
#endif

    FP4_add(rhs,&t,rhs);
    FP4_reduce(rhs);
}

/* Set P=(x,y). Return 1 if (x,y) is on the curve, else return 0*/
/* SU= 232 */
int ZZZ::ECP4_set(ECP4 *P,FP4 *x,FP4 *y)
{
    FP4 rhs,y2;

    FP4_sqr(&y2,y);
    ECP4_rhs(&rhs,x);

//cout << "y2= ";
//FP4_output(&y2);
//cout << endl;
//cout << "rhs= ";
//FP4_output(&rhs);
//cout << endl;

    if (!FP4_equals(&y2,&rhs))
    {
		ECP4_inf(P);
        //P->inf=1;
        return 0;
    }

    //P->inf=0;
    FP4_copy(&(P->x),x);
    FP4_copy(&(P->y),y);

    FP4_one(&(P->z));
    return 1;
}

/* Set P=(x,y). Return 1 if (x,.) is on the curve, else return 0 */
/* SU= 232 */
int ZZZ::ECP4_setx(ECP4 *P,FP4 *x)
{
    FP4 y;
    ECP4_rhs(&y,x);

    if (!FP4_sqrt(&y,&y))
    {
		ECP4_inf(P);
   //     P->inf=1;
        return 0;
    }

    //P->inf=0;
    FP4_copy(&(P->x),x);
    FP4_copy(&(P->y),&y);
    FP4_one(&(P->z));
    return 1;
}

/* Set P=-P */
/* SU= 8 */
void ZZZ::ECP4_neg(ECP4 *P)
{
//	if (ECP4_isinf(P)) return;
	FP4_norm(&(P->y));
    FP4_neg(&(P->y),&(P->y));
    FP4_norm(&(P->y));
}

/* R+=R */
/* return -1 for Infinity, 0 for addition, 1 for doubling */
int ZZZ::ECP4_dbl(ECP4 *P)
{
    FP4 t0,t1,t2,t3,iy,x3,y3;
//    if (P->inf) return -1;

	FP4_copy(&iy,&(P->y));		//FP4 iy=new FP4(y);
#if SEXTIC_TWIST_ZZZ==D_TYPE
	FP4_times_i(&iy);			//iy.mul_ip(); 
	//FP4_norm(&iy);				//iy.norm();
#endif

	FP4_sqr(&t0,&(P->y));			//t0.sqr();   
#if SEXTIC_TWIST_ZZZ==D_TYPE
	FP4_times_i(&t0);			//t0.mul_ip(); 
#endif

	FP4_mul(&t1,&iy,&(P->z));	//t1.mul(z);
	FP4_sqr(&t2,&(P->z));				//t2.sqr();

	FP4_add(&(P->z),&t0,&t0);	//z.add(t0); 
	FP4_norm(&(P->z));				//z.norm(); 
	FP4_add(&(P->z),&(P->z),&(P->z));	//z.add(z); 
	FP4_add(&(P->z),&(P->z),&(P->z));	//z.add(z); 
	FP4_norm(&(P->z));			//z.norm();  

	FP4_imul(&t2,&t2,3*CURVE_B_I);	//t2.imul(3*ROM.CURVE_B_I); 
#if SEXTIC_TWIST_ZZZ==M_TYPE
	FP4_times_i(&t2);
	//FP4_norm(&t2);
#endif

	FP4_mul(&x3,&t2,&(P->z));	//x3.mul(z); 

	FP4_add(&y3,&t0,&t2);		//y3.add(t2); 
	FP4_norm(&y3);				//y3.norm();
	FP4_mul(&(P->z),&(P->z),&t1);	//z.mul(t1);

	FP4_add(&t1,&t2,&t2);		//t1.add(t2); 
	FP4_add(&t2,&t2,&t1);		//t2.add(t1); 
	FP4_norm(&t2);				//t2.norm();  
	FP4_sub(&t0,&t0,&t2);		//t0.sub(t2); 
	FP4_norm(&t0);				//t0.norm();                           //y^2-9bz^2
	FP4_mul(&y3,&y3,&t0);		//y3.mul(t0); 
	FP4_add(&(P->y),&y3,&x3);		//y3.add(x3);                          //(y^2+3z*2)(y^2-9z^2)+3b.z^2.8y^2

	FP4_mul(&t1,&(P->x),&iy);		//t1.mul(iy);						//

	FP4_norm(&t0);			//x.norm(); 
	FP4_mul(&(P->x),&t0,&t1);	//x.mul(t1); 
	FP4_add(&(P->x),&(P->x),&(P->x));	//x.add(x);       //(y^2-9bz^2)xy2

	FP4_norm(&(P->x));			//x.norm(); 

	FP4_norm(&(P->y));			//y.norm();

    return 1;
}

/* Set P+=Q */

int ZZZ::ECP4_add(ECP4 *P,ECP4 *Q)
{
    FP4 t0,t1,t2,t3,t4,x3,y3,z3;
	int b3=3*CURVE_B_I;
/*    if (Q->inf) return 0;
    if (P->inf)
    {
        ECP4_copy(P,Q);
        return 0;
    }
*/
	FP4_mul(&t0,&(P->x),&(Q->x));	//t0.mul(Q.x);         // x.Q.x
	FP4_mul(&t1,&(P->y),&(Q->y));	//t1.mul(Q.y);		 // y.Q.y

	FP4_mul(&t2,&(P->z),&(Q->z));	//t2.mul(Q.z);
	FP4_add(&t3,&(P->x),&(P->y));	//t3.add(y); 
	FP4_norm(&t3);				//t3.norm();          //t3=X1+Y1         
	FP4_add(&t4,&(Q->x),&(Q->y));	//t4.add(Q.y); 
	FP4_norm(&t4);				//t4.norm();			//t4=X2+Y2
	FP4_mul(&t3,&t3,&t4);		//t3.mul(t4);						//t3=(X1+Y1)(X2+Y2)
	FP4_add(&t4,&t0,&t1);		//t4.add(t1);		//t4=X1.X2+Y1.Y2

	FP4_sub(&t3,&t3,&t4);		//t3.sub(t4); 
	FP4_norm(&t3);				//t3.norm(); 
#if SEXTIC_TWIST_ZZZ==D_TYPE
	FP4_times_i(&t3);			//t3.mul_ip();  
	//FP4_norm(&t3);				//t3.norm();         //t3=(X1+Y1)(X2+Y2)-(X1.X2+Y1.Y2) = X1.Y2+X2.Y1
#endif
                   
	FP4_add(&t4,&(P->y),&(P->z));	//t4.add(z); 
	FP4_norm(&t4);				//t4.norm();			//t4=Y1+Z1

	FP4_add(&x3,&(Q->y),&(Q->z));	//x3.add(Q.z); 
	FP4_norm(&x3);				//x3.norm();			//x3=Y2+Z2

	FP4_mul(&t4,&t4,&x3);		//t4.mul(x3);						//t4=(Y1+Z1)(Y2+Z2)

	FP4_add(&x3,&t1,&t2);		//x3.add(t2);						//X3=Y1.Y2+Z1.Z2
	
	FP4_sub(&t4,&t4,&x3);		//t4.sub(x3); 
	FP4_norm(&t4);				//t4.norm(); 
#if SEXTIC_TWIST_ZZZ==D_TYPE
	FP4_times_i(&t4);			//t4.mul_ip(); 
	//FP4_norm(&t4);				//t4.norm();          //t4=(Y1+Z1)(Y2+Z2) - (Y1.Y2+Z1.Z2) = Y1.Z2+Y2.Z1
#endif

	FP4_add(&x3,&(P->x),&(P->z));	//x3.add(z); 
	FP4_norm(&x3);				//x3.norm();	// x3=X1+Z1
		
	FP4_add(&y3,&(Q->x),&(Q->z));	//y3.add(Q.z); 
	FP4_norm(&y3);				//y3.norm();				// y3=X2+Z2
	FP4_mul(&x3,&x3,&y3);		//x3.mul(y3);							// x3=(X1+Z1)(X2+Z2)

	FP4_add(&y3,&t0,&t2);		//y3.add(t2);							// y3=X1.X2+Z1+Z2
	FP4_sub(&y3,&x3,&y3);		//y3.rsub(x3); 
	FP4_norm(&y3);				//y3.norm();				// y3=(X1+Z1)(X2+Z2) - (X1.X2+Z1.Z2) = X1.Z2+X2.Z1
#if SEXTIC_TWIST_ZZZ==D_TYPE
	FP4_times_i(&t0);			//t0.mul_ip(); 
	//FP4_norm(&t0);				//t0.norm(); // x.Q.x
	FP4_times_i(&t1);			//t1.mul_ip(); 
	//FP4_norm(&t1);				//t1.norm(); // y.Q.y
#endif

	FP4_add(&x3,&t0,&t0);		//x3.add(t0); 
	FP4_add(&t0,&t0,&x3);		//t0.add(x3); 
	FP4_norm(&t0);				//t0.norm();
	FP4_imul(&t2,&t2,b3);		//t2.imul(b); 	
#if SEXTIC_TWIST_ZZZ==M_TYPE
	FP4_times_i(&t2);
#endif

	FP4_add(&z3,&t1,&t2);		//z3.add(t2); 
	FP4_norm(&z3);				//z3.norm();
	FP4_sub(&t1,&t1,&t2);		//t1.sub(t2); 
	FP4_norm(&t1);				//t1.norm(); 
	FP4_imul(&y3,&y3,b3);		//y3.imul(b); 
#if SEXTIC_TWIST_ZZZ==M_TYPE
	FP4_times_i(&y3);
	//FP4_norm(&y3);
#endif

	FP4_mul(&x3,&y3,&t4);		//x3.mul(t4); 

	FP4_mul(&t2,&t3,&t1);		//t2.mul(t1); 
	FP4_sub(&(P->x),&t2,&x3);		//x3.rsub(t2);
	FP4_mul(&y3,&y3,&t0);		//y3.mul(t0); 
	FP4_mul(&t1,&t1,&z3);		//t1.mul(z3); 
	FP4_add(&(P->y),&y3,&t1);		//y3.add(t1);
	FP4_mul(&t0,&t0,&t3);		//t0.mul(t3); 
	FP4_mul(&z3,&z3,&t4);		//z3.mul(t4); 
	FP4_add(&(P->z),&z3,&t0);		//z3.add(t0);


	FP4_norm(&(P->x));			//x.norm(); 
	FP4_norm(&(P->y));			//y.norm();
	FP4_norm(&(P->z));			//z.norm();

    return 0;
}

/* Set P-=Q */
/* SU= 16 */
void ZZZ::ECP4_sub(ECP4 *P,ECP4 *Q)
{
	ECP4 NQ;
	ECP4_copy(&NQ,Q);
	ECP4_neg(&NQ);
    //ECP4_neg(Q);
    ECP4_add(P,&NQ);
    //ECP4_neg(Q);
}


void ZZZ::ECP4_reduce(ECP4 *P)
{
	FP4_reduce(&(P->x));
	FP4_reduce(&(P->y));
	FP4_reduce(&(P->z));
}

/* P*=e */
/* SU= 280 */
void ZZZ::ECP4_mul(ECP4 *P,BIG e)
{
    /* fixed size windows */
    int i,nb,s,ns;
    BIG mt,t;
    ECP4 Q,W[8],C;
    sign8 w[1+(NLEN_XXX*BASEBITS_XXX+3)/4];

    if (ECP4_isinf(P)) return;
    //ECP4_affine(P);

    /* precompute table */

    ECP4_copy(&Q,P);
    ECP4_dbl(&Q);
    ECP4_copy(&W[0],P);

    for (i=1; i<8; i++)
    {
        ECP4_copy(&W[i],&W[i-1]);
        ECP4_add(&W[i],&Q);
    }

    /* make exponent odd - add 2P if even, P if odd */
    BIG_copy(t,e);
    s=BIG_parity(t);
    BIG_inc(t,1);
    BIG_norm(t);
    ns=BIG_parity(t);
    BIG_copy(mt,t);
    BIG_inc(mt,1);
    BIG_norm(mt);
    BIG_cmove(t,mt,s);
    ECP4_cmove(&Q,P,ns);
    ECP4_copy(&C,&Q);

    nb=1+(BIG_nbits(t)+3)/4;

    /* convert exponent to signed 4-bit window */
    for (i=0; i<nb; i++)
    {
        w[i]=BIG_lastbits(t,5)-16;
        BIG_dec(t,w[i]);
        BIG_norm(t);
        BIG_fshr(t,4);
    }
    w[nb]=BIG_lastbits(t,5);

    ECP4_copy(P,&W[(w[nb]-1)/2]);
    for (i=nb-1; i>=0; i--)
    {
        ECP4_select(&Q,W,w[i]);
        ECP4_dbl(P);
        ECP4_dbl(P);
        ECP4_dbl(P);
        ECP4_dbl(P);
        ECP4_add(P,&Q);
    }
    ECP4_sub(P,&C); /* apply correction */
	ECP4_affine(P);
}

// calculate frobenius constants 
void ZZZ::ECP4_frob_constants(FP2 F[3])
{
    FP fx,fy;
	FP2 X;

    FP_rcopy(&fx,Fra);
    FP_rcopy(&fy,Frb);
    FP2_from_FPs(&X,&fx,&fy);

	FP2_sqr(&F[0],&X);		// FF=F^2=(1+i)^(p-7)/6
	FP2_copy(&F[2],&F[0]);
	FP2_mul_ip(&F[2]);		// W=(1+i)^6/6.(1+i)^(p-7)/6 = (1+i)^(p-1)/6
	FP2_norm(&F[2]);
	FP2_sqr(&F[1],&F[2]);
	FP2_mul(&F[2],&F[2],&F[1]);  // W=(1+i)^(p-1)/2

	FP2_copy(&F[1],&X);

#if SEXTIC_TWIST_ZZZ == M_TYPE	
	FP2_mul_ip(&F[1]);		// (1+i)^12/12.(1+i)^(p-7)/12 = (1+i)^(p+5)/12
	FP2_inv(&F[1],&F[1]);		// (1+i)^-(p+5)/12
	FP2_sqr(&F[0],&F[1]);		// (1+i)^-(p+5)/6
#endif

	FP2_mul_ip(&F[0]);		// FF=(1+i)^(p-7)/6.(1+i) = (1+i)^(p-1)/6					// (1+i)^6/6.(1+i)^-(p+5)/6 = (1+i)^-(p-1)/6
	FP2_norm(&F[0]);
	FP2_mul(&F[1],&F[1],&F[0]);  // FFF = (1+i)^(p-7)/12 . (1+i)^(p-1)/6 = (1+i)^(p-3)/4	// (1+i)^-(p+5)/12 . (1+i)^-(p-1)/6 = (1+i)^-(p+1)/4

}

/* Calculates q^n.P using Frobenius constants */
void ZZZ::ECP4_frob(ECP4 *P,FP2 F[3],int n)
{
	int i;
	FP4 X,Y,Z;
//    if (P->inf) return;

	//ECP4_get(&X,&Y,P);		// F=(1+i)^(p-7)/12

	FP4_copy(&X,&(P->x));
	FP4_copy(&Y,&(P->y));
	FP4_copy(&Z,&(P->z));

	for (i=0;i<n;i++)
	{
		FP4_frob(&X,&F[2]);		// X^p
		FP4_pmul(&X,&X,&F[0]);	// X^p.(1+i)^(p-1)/6									// X^p.(1+i)^-(p-1)/6
		
		FP4_frob(&Y,&F[2]);		// Y^p
		FP4_pmul(&Y,&Y,&F[1]);
		FP4_times_i(&Y);		// Y.p.(1+i)^(p-3)/4.(1+i)^(2/4) = Y^p.(1+i)^(p-1)/4	// (1+i)^-(p+1)/4 .(1+i)^2/4 = Y^p.(1+i)^-(p-1)/4

		FP4_frob(&Z,&F[2]);
	}

	FP4_copy(&(P->x),&X);
	FP4_copy(&(P->y),&Y);
	FP4_copy(&(P->z),&Z);
	

	//ECP4_set(P,&X,&Y);
}

/* Side channel attack secure */
// Bos & Costello https://eprint.iacr.org/2013/458.pdf
// Faz-Hernandez & Longa & Sanchez  https://eprint.iacr.org/2013/158.pdf

void ZZZ::ECP4_mul8(ECP4 *P,ECP4 Q[8],BIG u[8])
{
    int i,j,k,nb,pb1,pb2,bt;
	ECP4 T1[8],T2[8],W;
    BIG mt,t[8];
    sign8 w1[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s1[NLEN_XXX*BASEBITS_XXX+1];
    sign8 w2[NLEN_XXX*BASEBITS_XXX+1];
    sign8 s2[NLEN_XXX*BASEBITS_XXX+1];	
	FP2 X[3];

	ECP4_frob_constants(X);

    for (i=0; i<8; i++)
	{
        //ECP4_affine(&Q[i]);
        BIG_copy(t[i],u[i]);
	}
// Precomputed table
    ECP4_copy(&T1[0],&Q[0]); // Q[0]
    ECP4_copy(&T1[1],&T1[0]);
	ECP4_add(&T1[1],&Q[1]);	// Q[0]+Q[1]
    ECP4_copy(&T1[2],&T1[0]);
	ECP4_add(&T1[2],&Q[2]);	// Q[0]+Q[2]
	ECP4_copy(&T1[3],&T1[1]);
	ECP4_add(&T1[3],&Q[2]);	// Q[0]+Q[1]+Q[2]
	ECP4_copy(&T1[4],&T1[0]);
	ECP4_add(&T1[4],&Q[3]);  // Q[0]+Q[3]
	ECP4_copy(&T1[5],&T1[1]);
	ECP4_add(&T1[5],&Q[3]);	// Q[0]+Q[1]+Q[3]
	ECP4_copy(&T1[6],&T1[2]);
	ECP4_add(&T1[6],&Q[3]);	// Q[0]+Q[2]+Q[3]
	ECP4_copy(&T1[7],&T1[3]);
	ECP4_add(&T1[7],&Q[3]);	// Q[0]+Q[1]+Q[2]+Q[3]

//  Use Frobenius 

	for (i=0;i<8;i++)
	{
		ECP4_copy(&T2[i],&T1[i]);
		ECP4_frob(&T2[i],X,4);
	}

// Make them odd
	pb1=1-BIG_parity(t[0]);
	BIG_inc(t[0],pb1);
	BIG_norm(t[0]);

	pb2=1-BIG_parity(t[4]);
	BIG_inc(t[4],pb2);
	BIG_norm(t[4]);

// Number of bits
    BIG_zero(mt);
    for (i=0; i<8; i++)
    {
        BIG_or(mt,mt,t[i]);
    }
    nb=1+BIG_nbits(mt);

// Sign pivot 
	s1[nb-1]=1;
	s2[nb-1]=1;
	for (i=0;i<nb-1;i++)
	{
        BIG_fshr(t[0],1);
		s1[i]=2*BIG_parity(t[0])-1;
        BIG_fshr(t[4],1);
		s2[i]=2*BIG_parity(t[4])-1;
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
    }	

// Main loop
	ECP4_select(P,T1,2*w1[nb-1]+1);
	ECP4_select(&W,T2,2*w2[nb-1]+1);
	ECP4_add(P,&W);
    for (i=nb-2; i>=0; i--)
    {
        ECP4_dbl(P);
        ECP4_select(&W,T1,2*w1[i]+s1[i]);
        ECP4_add(P,&W);
        ECP4_select(&W,T2,2*w2[i]+s2[i]);
        ECP4_add(P,&W);
    }

// apply corrections
	ECP4_copy(&W,P);   
	ECP4_sub(&W,&Q[0]);
	ECP4_cmove(P,&W,pb1);
	ECP4_copy(&W,P);   
	ECP4_sub(&W,&Q[4]);
	ECP4_cmove(P,&W,pb2);

	ECP4_affine(P);
}
/*
void ZZZ::ECP4_mul8(ECP4 *P,ECP4 Q[8],BIG u[8])
{
    int i,j,a[4],nb,pb;
    ECP4 W[8],Z[8],T,C;
    BIG mt,t[8];
    sign8 w[NLEN_XXX*BASEBITS_XXX+1];
    sign8 z[NLEN_XXX*BASEBITS_XXX+1];
    FP fx,fy;
	FP2 X;

    FP_rcopy(&fx,Fra);
    FP_rcopy(&fy,Frb);
    FP2_from_FPs(&X,&fx,&fy);

    for (i=0; i<8; i++)
        BIG_copy(t[i],u[i]);

    // precompute tables 

// 12 add/subs 

    ECP4_copy(&W[0],&Q[0]);
    ECP4_sub(&W[0],&Q[1]);  // P-Q 
    ECP4_copy(&W[1],&W[0]);
    ECP4_copy(&W[2],&W[0]);
    ECP4_copy(&W[3],&W[0]);
    ECP4_copy(&W[4],&Q[0]);
    ECP4_add(&W[4],&Q[1]);  // P+Q 
    ECP4_copy(&W[5],&W[4]);
    ECP4_copy(&W[6],&W[4]);
    ECP4_copy(&W[7],&W[4]);

    ECP4_copy(&T,&Q[2]);
    ECP4_sub(&T,&Q[3]);       // R-S 
    ECP4_sub(&W[1],&T);
    ECP4_add(&W[2],&T);
    ECP4_sub(&W[5],&T);
    ECP4_add(&W[6],&T);
    ECP4_copy(&T,&Q[2]);
    ECP4_add(&T,&Q[3]);      // R+S 
    ECP4_sub(&W[0],&T);
    ECP4_add(&W[3],&T);
    ECP4_sub(&W[4],&T);
    ECP4_add(&W[7],&T);


// Use Frobenius 

	for (i=0;i<8;i++)
	{
		ECP4_copy(&Z[i],&W[i]);
		ECP4_frob(&Z[i],&X,4);
	}

    // if multiplier is even add 1 to multiplier, and add P to correction 
    ECP4_inf(&C);

    BIG_zero(mt);
    for (i=0; i<8; i++)
    {
		pb=BIG_parity(t[i]);
		BIG_inc(t[i],1-pb);
		BIG_norm(t[i]);
		ECP4_copy(&T,&C);
		ECP4_add(&T,&Q[i]);
		ECP4_cmove(&C,&T,1-pb);


        BIG_add(mt,mt,t[i]);
        BIG_norm(mt);
    }

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


    ECP4_copy(P,&W[(w[nb]-1)/2]);
	ECP4_add(P,&Z[(z[nb]-1)/2]);
    for (i=nb-1; i>=0; i--)
    {
		ECP4_dbl(P);
        ECP4_select(&T,W,w[i]);
        ECP4_add(P,&T);
        ECP4_select(&T,Z,z[i]);
        ECP4_add(P,&T);
    }
    ECP4_sub(P,&C); // apply correction 
	ECP4_reduce(P);
}
*/
/* Map to hash value to point on G2 from random BIG */

void ZZZ::ECP4_mapit(ECP4 *Q,octet *W)
{
    BIG q,one,x,hv;
    FP2 X[3],T;
	FP4 X4,Y4;

    ECP4 xQ, x2Q, x3Q, x4Q;

	BIG_fromBytes(hv,W->val);
    BIG_rcopy(q,Modulus);
    BIG_one(one);
    BIG_mod(hv,q);

    for (;;)
    {
        FP2_from_BIGs(&T,one,hv);  /*******/
		FP4_from_FP2(&X4,&T);
        if (ECP4_setx(Q,&X4)) break;
        BIG_inc(hv,1);
    }

	ECP4_frob_constants(X);

    BIG_rcopy(x,CURVE_Bnx);

    // Efficient hash maps to G2 on BLS24 curves - Budroni, Pintore 
	// Q -> x4Q -x3Q -Q + F(x3Q-x2Q) + F(F(x2Q-xQ)) + F(F(F(xQ-Q))) +F(F(F(F(2Q))))

	ECP4_copy(&xQ,Q);
	ECP4_mul(&xQ,x);
	ECP4_copy(&x2Q,&xQ);
	ECP4_mul(&x2Q,x);
	ECP4_copy(&x3Q,&x2Q);
	ECP4_mul(&x3Q,x);
	ECP4_copy(&x4Q,&x3Q);
	ECP4_mul(&x4Q,x);

#if SIGN_OF_X_ZZZ==NEGATIVEX
	ECP4_neg(&xQ);
	ECP4_neg(&x3Q);
#endif

	ECP4_sub(&x4Q,&x3Q);
	ECP4_sub(&x4Q,Q);

	ECP4_sub(&x3Q,&x2Q);
	ECP4_frob(&x3Q,X,1);

	ECP4_sub(&x2Q,&xQ);
	ECP4_frob(&x2Q,X,2);

	ECP4_sub(&xQ,Q);
	ECP4_frob(&xQ,X,3);

	ECP4_dbl(Q);
	ECP4_frob(Q,X,4);

	ECP4_add(Q,&x4Q);
	ECP4_add(Q,&x3Q);
	ECP4_add(Q,&x2Q);
	ECP4_add(Q,&xQ);

	ECP4_affine(Q);

}

// ECP$ Get Group Generator

void ZZZ::ECP4_generator(ECP4 *G)
{
	BIG a,b;
	FP2 Aa,Bb;
	FP4 X,Y;

	BIG_rcopy(a,CURVE_Pxaa);
	BIG_rcopy(b,CURVE_Pxab);
	FP2_from_BIGs(&Aa,a,b);

	BIG_rcopy(a,CURVE_Pxba);
	BIG_rcopy(b,CURVE_Pxbb);
	FP2_from_BIGs(&Bb,a,b);

	FP4_from_FP2s(&X,&Aa,&Bb);

	BIG_rcopy(a,CURVE_Pyaa);
	BIG_rcopy(b,CURVE_Pyab);
	FP2_from_BIGs(&Aa,a,b);

	BIG_rcopy(a,CURVE_Pyba);
	BIG_rcopy(b,CURVE_Pybb);
	FP2_from_BIGs(&Bb,a,b);

	FP4_from_FP2s(&Y,&Aa,&Bb);

	ECP4_set(G,&X,&Y);
}


// g++ -O2 ecp4_BLS24.cpp fp4_BLS24.cpp fp2_BLS24.cpp fp_BLS24.cpp big_XXX.cpp rand.cpp hash.cpp rom_field_BLS24.cpp rom_curve_BLS24.cpp oct.cpp -o ecp4_BLS24.exe
/*
int main()
{
	int i;
	ECP4 G,P;
	FP2 Aa,Bb,f;
	FP4 X,Y;
	BIG a,b,r,p;
	char w[100];
	octet W= {0,sizeof(w),w};

	ECP4_ggg(&G);

	BIG_rcopy(a,Fra);
    BIG_rcopy(b,Frb);
	FP2_from_BIGs(&f,a,b);

	
	if (G.inf) cout << "Failed to set - point not on curve" << endl;
	else cout << "set success" << endl;

	ECP4_output(&G);
	ECP4_copy(&P,&G);

	//ECP4_reduce(&G);
	//ECP4_reduce(&P);

	BIG_rcopy(r,CURVE_Order);
	BIG_rcopy(p,Modulus);

	BIG_output(r);
	cout << endl;

	ECP4_mul(&G,r);
	ECP4_output(&G);
	cout << endl;

	ECP4_copy(&G,&P);

	cout << "pG = ";
	ECP4_mul(&G,p);
	ECP4_output(&G);
	cout << endl;

	cout << "G^p= ";
	ECP4_frob(&P,&f,1);
	ECP4_output(&P);
	cout << endl;


	for (i=0;i<MODBYTES_XXX;i++)
	{
		W.val[i]=i+11;
	}
	W.len=MODBYTES_XXX;

	printf("W= ");
	OCT_output(&W);
	printf("\n");

	ECP4_mapit(&P,&W);

	cout << "Hash to P= ";
	ECP4_output(&P);
	cout << endl;

	ECP4_mul(&P,r);

	cout << "rP= ";
	ECP4_output(&P);
	cout << endl;



//	ECP4_dbl(&G);
//	ECP4_output(&G);
//	ECP4_reduce(&G);
//	cout << endl;
//	ECP4_add(&G,&P);
//	ECP4_output(&G);
//	cout << endl;


	return 0;
}
*/

