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

/* MPIN Functions */

/* Version 3.0 - supports Time Permits */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpin.h"

#define ROUNDUP(a,b) ((a)-1)/(b)+1

/* Special mpin hashing */
static void mpin_hash(int sha,FP4 *f, ECP *P,octet *w)
{
    int i;
    BIG x,y;
    char h[64];
    hash256 sha256;
    hash512 sha512;
    char t[6*MODBYTES];  // to hold 6 BIGs
    int hlen=sha;
    BIG_copy(x,f->a.a);
    FP_redc(x);
    BIG_toBytes(&t[0],x);
    BIG_copy(x,f->a.b);
    FP_redc(x);
    BIG_toBytes(&t[MODBYTES],x);
    BIG_copy(x,f->b.a);
    FP_redc(x);
    BIG_toBytes(&t[2*MODBYTES],x);
    BIG_copy(x,f->b.b);
    FP_redc(x);
    BIG_toBytes(&t[3*MODBYTES],x);
    ECP_get(x,y,P);
    BIG_toBytes(&t[4*MODBYTES],x);
    BIG_toBytes(&t[5*MODBYTES],y);

    OCT_empty(w);
    switch (sha)
    {
    case SHA256:
        HASH256_init(&sha256);
        for (i=0; i<6*MODBYTES; i++) HASH256_process(&sha256,t[i]);
        HASH256_hash(&sha256,h);
        break;
    case SHA384:
        HASH384_init(&sha512);
        for (i=0; i<6*MODBYTES; i++) HASH384_process(&sha512,t[i]);
        HASH384_hash(&sha512,h);
        break;
    case SHA512:
        HASH512_init(&sha512);
        for (i=0; i<6*MODBYTES; i++) HASH512_process(&sha512,t[i]);
        HASH512_hash(&sha512,h);
        break;
    }

    OCT_jbytes(w,h,PAS);
    for (i=0; i<hlen; i++) h[i]=0;
}

/* these next two functions help to implement elligator squared - http://eprint.iacr.org/2014/043 */
/* maps a random u to a point on the curve */
static void map(ECP *P,BIG u,int cb)
{
    BIG x,q;

    BIG_rcopy(q,Modulus);
    BIG_copy(x,u);
    BIG_mod(x,q);

    while (!ECP_setx(P,x,cb))
        BIG_inc(x,1);
}

/* returns u derived from P. Random value in range 1 to return value should then be added to u */
static int unmap(BIG u,int *cb,ECP *P)
{
    int s,r=0;
    BIG x;

    s=ECP_get(x,x,P);
    BIG_copy(u,x);
    do
    {
        BIG_dec(u,1);
        r++;
    }
    while (!ECP_setx(P,u,s));
    ECP_setx(P,x,s);

    *cb=s;

    return r;
}

/* map octet string containing hash to point on curve of correct order */
static void mapit(octet *h,ECP *P)
{
    BIG q,x,c;
    BIG_fromBytes(x,h->val);
    BIG_rcopy(q,Modulus);
    BIG_mod(x,q);

    while (!ECP_setx(P,x,0))
        BIG_inc(x,1);

    BIG_rcopy(c,CURVE_Cof);
    ECP_mul(P,c);
}

/* needed for SOK */
/* static void mapit2(octet *h,ECP2 *Q) */
/* { */
/* 	BIG q,one,Fx,Fy,x,hv; */
/* 	FP2 X; */
/* 	ECP2 T,K; */
/* 	BIG_fromBytes(hv,h->val); */
/* 	BIG_rcopy(q,Modulus); */
/* 	BIG_one(one); */
/* 	BIG_mod(hv,q); */

/* 	for (;;) */
/* 	{ */
/* 		FP2_from_BIGs(&X,one,hv); */
/* 		if (ECP2_setx(Q,&X)) break; */
/* 		BIG_inc(hv,1);  */
/* 	} */

/* /\* Fast Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez *\/ */
/* 	BIG_rcopy(Fx,CURVE_Fra); */
/* 	BIG_rcopy(Fy,CURVE_Frb); */
/* 	FP2_from_BIGs(&X,Fx,Fy); */
/* 	BIG_rcopy(x,CURVE_Bnx); */

/* 	ECP2_copy(&T,Q); */
/* 	ECP2_mul(&T,x); */
/* 	ECP2_neg(&T);  /\* our x is negative *\/ */
/* 	ECP2_copy(&K,&T); */
/* 	ECP2_dbl(&K); */
/* 	ECP2_add(&K,&T); */
/* 	ECP2_affine(&K); */

/* 	ECP2_frob(&K,&X); */
/* 	ECP2_frob(Q,&X); ECP2_frob(Q,&X); ECP2_frob(Q,&X);  */
/* 	ECP2_add(Q,&T); */
/* 	ECP2_add(Q,&K); */
/* 	ECP2_frob(&T,&X); ECP2_frob(&T,&X); */
/* 	ECP2_add(Q,&T); */
/* 	ECP2_affine(Q); */
/* } */



/* general purpose hash function w=hash(p|n|x|y) */
static void hashit(int sha,int n,octet *x,octet *w)
{
    int i,c[4],hlen;
    hash256 sha256;
    hash512 sha512;
    char hh[64];

    switch (sha)
    {
    case SHA256:
        HASH256_init(&sha256);
        break;
    case SHA384:
        HASH384_init(&sha512);
        break;
    case SHA512:
        HASH512_init(&sha512);
        break;
    }

    hlen=sha;

    if (n>0)
    {
        c[0]=(n>>24)&0xff;
        c[1]=(n>>16)&0xff;
        c[2]=(n>>8)&0xff;
        c[3]=(n)&0xff;
        for (i=0; i<4; i++)
        {
            switch(sha)
            {
            case SHA256:
                HASH256_process(&sha256,c[i]);
                break;
            case SHA384:
                HASH384_process(&sha512,c[i]);
                break;
            case SHA512:
                HASH512_process(&sha512,c[i]);
                break;
            }
        }
    }
    if (x!=NULL) for (i=0; i<x->len; i++)
        {
            switch(sha)
            {
            case SHA256:
                HASH256_process(&sha256,x->val[i]);
                break;
            case SHA384:
                HASH384_process(&sha512,x->val[i]);
                break;
            case SHA512:
                HASH512_process(&sha512,x->val[i]);
                break;
            }
        }

    for (i=0; i<hlen; i++) hh[i]=0;
    switch (sha)
    {
    case SHA256:
        HASH256_hash(&sha256,hh);
        break;
    case SHA384:
        HASH384_hash(&sha512,hh);
        break;
    case SHA512:
        HASH512_hash(&sha512,hh);
        break;
    }

    OCT_empty(w);

    if (hlen>=MODBYTES)
        OCT_jbytes(w,hh,MODBYTES);
    else
    {
        OCT_jbytes(w,hh,hlen);
        OCT_jbyte(w,0,MODBYTES-hlen);
    }
}

unsign32 MPIN_today(void)
{
    /* return time in slots since epoch */
    unsign32 ti=(unsign32)time(NULL);
    return (uint32_t)(ti/(60*TIME_SLOT_MINUTES));
}

/* Hash the M-Pin transcript - new */

void MPIN_HASH_ALL(int sha,octet *HID,octet *xID,octet *xCID,octet *SEC,octet *Y,octet *R,octet *W,octet *H)
{
    char t[10*MODBYTES+4];
    octet T= {0,sizeof(t),t};

    OCT_joctet(&T,HID);
    if (xCID!=NULL) OCT_joctet(&T,xCID);
    else OCT_joctet(&T,xID);
    OCT_joctet(&T,SEC);
    OCT_joctet(&T,Y);
    OCT_joctet(&T,R);
    OCT_joctet(&T,W);

    hashit(sha,0,&T,H);
}

void MPIN_HASH_ID(int sha,octet *ID,octet *HID)
{
    hashit(sha,0,ID,HID);
}

/* these next two functions implement elligator squared - http://eprint.iacr.org/2014/043 */
/* Elliptic curve point E in format (0x04,x,y} is converted to form {0x0-,u,v} */
/* Note that u and v are indistinguisible from random strings */
int MPIN_ENCODING(csprng *RNG,octet *E)
{
    int rn,m,su,sv,res=0;

    BIG q,u,v;
    ECP P,W;

    if (!ECP_fromOctet(&P,E)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        BIG_rcopy(q,Modulus);

        BIG_randomnum(u,q,RNG);

        su=RAND_byte(RNG);
        if (su<0) su=-su;
        su%=2;
        map(&W,u,su);
        ECP_sub(&P,&W);

        rn=unmap(v,&sv,&P);
        m=RAND_byte(RNG);
        if (m<0) m=-m;
        m%=rn;
        BIG_inc(v,m+1);
        E->val[0]=su+2*sv;
        BIG_toBytes(&(E->val[1]),u);
        BIG_toBytes(&(E->val[PFS+1]),v);
    }

    return res;
}

int MPIN_DECODING(octet *D)
{
    int su,sv;
    BIG u,v;
    ECP P,W;
    int res=0;

    if ((D->val[0]&0x04)!=0) res=MPIN_INVALID_POINT;
    if (res==0)
    {

        BIG_fromBytes(u,&(D->val[1]));
        BIG_fromBytes(v,&(D->val[PFS+1]));

        su=D->val[0]&1;
        sv=(D->val[0]>>1)&1;

        map(&W,u,su);
        map(&P,v,sv);

        ECP_add(&P,&W);
        ECP_toOctet(D,&P);
    }
    return res;
}

/* R=R1+R2 in group G1 */
int MPIN_RECOMBINE_G1(octet *R1,octet *R2,octet *R)
{
    ECP P,T;
    int res=0;
    if (res==0)
    {
        if (!ECP_fromOctet(&P,R1)) res=MPIN_INVALID_POINT;
        if (!ECP_fromOctet(&T,R2)) res=MPIN_INVALID_POINT;
    }
    if (res==0)
    {
        ECP_add(&P,&T);
        ECP_toOctet(R,&P);
    }
    return res;
}

/* W=W1+W2 in group G2 */
int MPIN_RECOMBINE_G2(octet *W1,octet *W2,octet *W)
{
    ECP2 Q,T;
    int res=0;
    if (!ECP2_fromOctet(&Q,W1)) res=MPIN_INVALID_POINT;
    if (!ECP2_fromOctet(&T,W2)) res=MPIN_INVALID_POINT;
    if (res==0)
    {
        ECP2_add(&Q,&T);
        ECP2_toOctet(W,&Q);
    }
    return res;
}

/* create random secret S */
int MPIN_RANDOM_GENERATE(csprng *RNG,octet* S)
{
    BIG r,s;

    BIG_rcopy(r,CURVE_Order);
    BIG_randomnum(s,r,RNG);
#ifdef AES_S
    BIG_mod2m(s,2*AES_S);
#endif
    BIG_toBytes(S->val,s);
    S->len=MODBYTES;
    return 0;
}

/* Extract PIN from TOKEN for identity CID */
int MPIN_EXTRACT_PIN(int sha,octet *CID,int pin,octet *TOKEN)
{
    ECP P,R;
    int res=0;
    char h[MODBYTES];
    octet H= {0,sizeof(h),h};

    if (!ECP_fromOctet(&P,TOKEN))  res=MPIN_INVALID_POINT;
    if (res==0)
    {
        hashit(sha,-1,CID,&H);
        mapit(&H,&R);

        pin%=MAXPIN;

        ECP_pinmul(&R,pin,PBLEN);
        ECP_sub(&P,&R);

        ECP_toOctet(TOKEN,&P);
    }
    return res;
}

/* Implement step 2 on client side of MPin protocol - SEC=-(x+y)*SEC */
int MPIN_CLIENT_2(octet *X,octet *Y,octet *SEC)
{
    BIG px,py,r;
    ECP P;
    int res=0;
    BIG_rcopy(r,CURVE_Order);
    if (!ECP_fromOctet(&P,SEC)) res=MPIN_INVALID_POINT;
    if (res==0)
    {
        BIG_fromBytes(px,X->val);
        BIG_fromBytes(py,Y->val);
        BIG_add(px,px,py);
        BIG_mod(px,r);
        //	BIG_sub(px,r,px);
        PAIR_G1mul(&P,px);
        ECP_neg(&P);
        ECP_toOctet(SEC,&P);
    }
    return res;
}

/*
 W=x*H(G);
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 if type=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
*/

int MPIN_GET_G1_MULTIPLE(csprng *RNG,int type,octet *X,octet *G,octet *W)
{
    ECP P;
    BIG r,x;
    int res=0;
    if (RNG!=NULL)
    {
        BIG_rcopy(r,CURVE_Order);
        BIG_randomnum(x,r,RNG);
#ifdef AES_S
        BIG_mod2m(x,2*AES_S);
#endif
        X->len=MODBYTES;
        BIG_toBytes(X->val,x);
    }
    else
        BIG_fromBytes(x,X->val);

    if (type==0)
    {
        if (!ECP_fromOctet(&P,G)) res=MPIN_INVALID_POINT;
    }
    else mapit(G,&P);

    if (res==0)
    {
        PAIR_G1mul(&P,x);
        ECP_toOctet(W,&P);
    }
    return res;
}

/*
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 W=x*G where G is point on the curve
 if type==1 W=(x^-1)G
*/

int MPIN_GET_G2_MULTIPLE(csprng *RNG,int type,octet *X,octet *G,octet *W)
{
    ECP2 P;
    BIG r,x;
    int res=0;
    BIG_rcopy(r,CURVE_Order);
    if (RNG!=NULL)
    {
        BIG_randomnum(x,r,RNG);
#ifdef AES_S
        BIG_mod2m(x,2*AES_S);
#endif
        X->len=MODBYTES;
        BIG_toBytes(X->val,x);
    }
    else
    {
        BIG_fromBytes(x,X->val);
        if (type==1) BIG_invmodp(x,x,r);
    }

    if (!ECP2_fromOctet(&P,G)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        PAIR_G2mul(&P,x);
        ECP2_toOctet(W,&P);
    }
    return res;
}



/* Client secret CST=s*H(CID) where CID is client ID and s is master secret */
/* CID is hashed externally */
int MPIN_GET_CLIENT_SECRET(octet *S,octet *CID,octet *CST)
{
    return MPIN_GET_G1_MULTIPLE(NULL,1,S,CID,CST);
}

/* Implement step 1 on client side of MPin protocol */
int MPIN_CLIENT_1(int sha,int date,octet *CLIENT_ID,csprng *RNG,octet *X,int pin,octet *TOKEN,octet *SEC,octet *xID,octet *xCID,octet *PERMIT)
{
    BIG r,x;
    ECP P,T,W;
    int res=0;
    char h[MODBYTES];
    octet H= {0,sizeof(h),h};

    BIG_rcopy(r,CURVE_Order);
    if (RNG!=NULL)
    {
        BIG_randomnum(x,r,RNG);
#ifdef AES_S
        BIG_mod2m(x,2*AES_S);
#endif
        X->len=MODBYTES;
        BIG_toBytes(X->val,x);
    }
    else
        BIG_fromBytes(x,X->val);

    hashit(sha,-1,CLIENT_ID,&H);
    mapit(&H,&P);

    if (!ECP_fromOctet(&T,TOKEN)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        pin%=MAXPIN;

        ECP_copy(&W,&P);				// W=H(ID)
        ECP_pinmul(&W,pin,PBLEN);			// W=alpha.H(ID)
        ECP_add(&T,&W);					// T=Token+alpha.H(ID) = s.H(ID)

        if (date)
        {
            if (PERMIT!=NULL)
            {
                if (!ECP_fromOctet(&W,PERMIT)) res=MPIN_INVALID_POINT;
                ECP_add(&T,&W);					// SEC=s.H(ID)+s.H(T|ID)
            }
            hashit(sha,date,&H,&H);
            mapit(&H,&W);
            if (xID!=NULL)
            {
                PAIR_G1mul(&P,x);				// P=x.H(ID)
                ECP_toOctet(xID,&P);  // xID
                PAIR_G1mul(&W,x);               // W=x.H(T|ID)
                ECP_add(&P,&W);
            }
            else
            {
                ECP_add(&P,&W);
                PAIR_G1mul(&P,x);
            }
            if (xCID!=NULL) ECP_toOctet(xCID,&P);  // U
        }
        else
        {
            if (xID!=NULL)
            {
                PAIR_G1mul(&P,x);				// P=x.H(ID)
                ECP_toOctet(xID,&P);  // xID
            }
        }
    }

    if (res==0)
        ECP_toOctet(SEC,&T);  // V

    return res;
}

/* Extract Server Secret SST=S*Q where Q is fixed generator in G2 and S is master secret */
int MPIN_GET_SERVER_SECRET(octet *S,octet *SST)
{
    BIG r,s;
    FP2 qx,qy;
    ECP2 Q;
    int res=0;

    BIG_rcopy(r,CURVE_Order);
    BIG_rcopy(qx.a,CURVE_Pxa);
    FP_nres(qx.a);
    BIG_rcopy(qx.b,CURVE_Pxb);
    FP_nres(qx.b);
    BIG_rcopy(qy.a,CURVE_Pya);
    FP_nres(qy.a);
    BIG_rcopy(qy.b,CURVE_Pyb);
    FP_nres(qy.b);
    ECP2_set(&Q,&qx,&qy);

    if (res==0)
    {
        BIG_fromBytes(s,S->val);
        PAIR_G2mul(&Q,s);
        ECP2_toOctet(SST,&Q);
    }

    return res;
}


/* Time Permit CTT=s*H(date|H(CID)) where s is master secret */
int MPIN_GET_CLIENT_PERMIT(int sha,int date,octet *S,octet *CID,octet *CTT)
{
    BIG s;
    ECP P;
    char h[MODBYTES];
    octet H= {0,sizeof(h),h};

    hashit(sha,date,CID,&H);

    mapit(&H,&P);
    BIG_fromBytes(s,S->val);
    PAIR_G1mul(&P,s);

    ECP_toOctet(CTT,&P);
    return 0;
}

// if date=0 only use HID, set HCID=NULL
// if date and PE, use HID and HCID

/* Outputs H(CID) and H(CID)+H(T|H(CID)) for time permits. If no time permits set HTID=NULL */
void MPIN_SERVER_1(int sha,int date,octet *CID,octet *HID,octet *HTID)
{
    char h[MODBYTES];
    octet H= {0,sizeof(h),h};
    ECP P,R;

#ifdef USE_ANONYMOUS
    mapit(CID,&P);
#else
    hashit(sha,-1,CID,&H);
    mapit(&H,&P);
#endif

    ECP_toOctet(HID,&P);  // new

    if (date)
    {
        //	if (HID!=NULL) ECP_toOctet(HID,&P);
#ifdef USE_ANONYMOUS
        hashit(sha,date,CID,&H);
#else
        hashit(sha,date,&H,&H);
#endif
        mapit(&H,&R);
        ECP_add(&P,&R);
        ECP_toOctet(HTID,&P);
    }
    //else ECP_toOctet(HID,&P);

}

/* Implement M-Pin on server side */
int MPIN_SERVER_2(int date,octet *HID,octet *HTID,octet *Y,octet *SST,octet *xID,octet *xCID,octet *mSEC,octet *E,octet *F)
{
    BIG px,py,y;
    FP2 qx,qy;
    FP12 g;
    ECP2 Q,sQ;
    ECP P,R;
    int res=0;

    BIG_rcopy(qx.a,CURVE_Pxa);
    FP_nres(qx.a);
    BIG_rcopy(qx.b,CURVE_Pxb);
    FP_nres(qx.b);
    BIG_rcopy(qy.a,CURVE_Pya);
    FP_nres(qy.a);
    BIG_rcopy(qy.b,CURVE_Pyb);
    FP_nres(qy.b);

    if (!ECP2_set(&Q,&qx,&qy)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        if (!ECP2_fromOctet(&sQ,SST)) res=MPIN_INVALID_POINT;
    }

    if (res==0)
    {
        if (date)
        {
            BIG_fromBytes(px,&(xCID->val[1]));
            BIG_fromBytes(py,&(xCID->val[PFS+1]));
        }
        else
        {
            BIG_fromBytes(px,&(xID->val[1]));
            BIG_fromBytes(py,&(xID->val[PFS+1]));
        }
        if (!ECP_set(&R,px,py)) res=MPIN_INVALID_POINT; // x(A+AT)
    }
    if (res==0)
    {
        BIG_fromBytes(y,Y->val);
        if (date)
        {
            if (!ECP_fromOctet(&P,HTID))  res=MPIN_INVALID_POINT;
        }
        else
        {
            if (!ECP_fromOctet(&P,HID))  res=MPIN_INVALID_POINT;
        }
    }
    if (res==0)
    {
        PAIR_G1mul(&P,y);  // y(A+AT)
        ECP_add(&P,&R); // x(A+AT)+y(A+T)
        if (!ECP_fromOctet(&R,mSEC))  res=MPIN_INVALID_POINT; // V
    }
    if (res==0)
    {
        PAIR_double_ate(&g,&Q,&R,&sQ,&P);
        PAIR_fexp(&g);

        if (!FP12_isunity(&g))
        {
            if (HID!=NULL && xID!=NULL && E!=NULL && F !=NULL)
            {
                /* xID is set to NULL if there is no way to calculate PIN error */
                FP12_toOctet(E,&g);

                /* Note error is in the PIN, not in the time permit! Hence the need to exclude Time Permit from this check */

                if (date)
                {
                    if (!ECP_fromOctet(&P,HID)) res=MPIN_INVALID_POINT;
                    if (!ECP_fromOctet(&R,xID)) res=MPIN_INVALID_POINT; // U

                    if (res==0)
                    {
                        PAIR_G1mul(&P,y);  // yA
                        ECP_add(&P,&R); // yA+xA
                    }
                }
                if (res==0)
                {
                    PAIR_ate(&g,&Q,&P);
                    PAIR_fexp(&g);
                    FP12_toOctet(F,&g);
                }
            }
            res=MPIN_BAD_PIN;
        }
    }

    return res;
}

#if MAXPIN==10000
#define MR_TS 10  /* 2^10/10 approx = sqrt(MAXPIN) */
#define TRAP 200  /* 2*sqrt(MAXPIN) */
#endif

#if MAXPIN==1000000
#define MR_TS 14
#define TRAP 2000
#endif

/* Pollards kangaroos used to return PIN error */
int MPIN_KANGAROO(octet *E,octet *F)
{
    int i,j,m,s,dn,dm,steps;
    int distance[MR_TS];
    FP12 ge,gf,t,table[MR_TS];
    int res=0;
    // BIG w;

    FP12_fromOctet(&ge,E);
    FP12_fromOctet(&gf,F);

    FP12_copy(&t,&gf);

    for (s=1,m=0; m<MR_TS; m++)
    {
        distance[m]=s;
        FP12_copy(&table[m],&t);
        s*=2;
        FP12_usqr(&t,&t);
        FP12_reduce(&t);
    }

    FP12_one(&t);

    for (dn=0,j=0; j<TRAP; j++)
    {

        //BIG_copy(w,t.a.a.a);
        //FP_redc(w);
        //i=BIG_lastbits(w,20)%MR_TS;

        i=t.a.a.a[0]%MR_TS;

        FP12_mul(&t,&table[i]);
        FP12_reduce(&t);
        dn+=distance[i];
    }

    FP12_conj(&gf,&t);
    steps=0;
    dm=0;
    while (dm-dn<MAXPIN)
    {
        steps++;
        if (steps>4*TRAP) break;

        //BIG_copy(w,ge.a.a.a);
        //FP_redc(w);
        //i=BIG_lastbits(w,20)%MR_TS;

        i=ge.a.a.a[0]%MR_TS;

        FP12_mul(&ge,&table[i]);
        FP12_reduce(&ge);
        dm+=distance[i];
        if (FP12_equals(&ge,&t))
        {
            res=dm-dn;
            break;
        }
        if (FP12_equals(&ge,&gf))
        {
            res=dn-dm;
            break;
        }
    }
    if (steps>4*TRAP || dm-dn>=MAXPIN)
    {
        res=0;    /* Trap Failed  - probable invalid token */
    }

    return res;
}

/* Functions to support M-Pin Full */

int MPIN_PRECOMPUTE(octet *TOKEN,octet *CID,octet *CP,octet *G1,octet *G2)
{
    ECP P,T;
    ECP2 Q;
    FP2 qx,qy;
    FP12 g;
    int res=0;

    if (!ECP_fromOctet(&T,TOKEN)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        mapit(CID,&P);
        if (CP!=NULL)
        {
            if (!ECP2_fromOctet(&Q,CP)) res=MPIN_INVALID_POINT;
        }
        else
        {
            BIG_rcopy(qx.a,CURVE_Pxa);
            FP_nres(qx.a);
            BIG_rcopy(qx.b,CURVE_Pxb);
            FP_nres(qx.b);
            BIG_rcopy(qy.a,CURVE_Pya);
            FP_nres(qy.a);
            BIG_rcopy(qy.b,CURVE_Pyb);
            FP_nres(qy.b);
            if (!ECP2_set(&Q,&qx,&qy)) res=MPIN_INVALID_POINT;
        }
    }
    if (res==0)
    {
        PAIR_ate(&g,&Q,&T);
        PAIR_fexp(&g);

        FP12_toOctet(G1,&g);
        if (G2!=NULL)
        {
            PAIR_ate(&g,&Q,&P);
            PAIR_fexp(&g);
            FP12_toOctet(G2,&g);
        }
    }
    return res;
}

/* calculate common key on client side */
/* wCID = w.(A+AT) */
int MPIN_CLIENT_KEY(int sha,octet *G1,octet *G2,int pin,octet *R,octet *X,octet *H,octet *wCID,octet *CK)
{
    FP12 g1,g2;
    FP4 c,cp,cpm1,cpm2;
    FP2 f;
    ECP W;
    int res=0;
    BIG r,z,x,q,m,a,b,h;

    FP12_fromOctet(&g1,G1);
    FP12_fromOctet(&g2,G2);
    BIG_fromBytes(z,R->val);
    BIG_fromBytes(x,X->val);
    BIG_fromBytes(h,H->val);

    if (!ECP_fromOctet(&W,wCID)) res=MPIN_INVALID_POINT;

    if (res==0)
    {
        BIG_rcopy(r,CURVE_Order);
        BIG_add(z,z,h);    // new
        BIG_mod(z,r);

        PAIR_G1mul(&W,x);

        BIG_rcopy(a,CURVE_Fra);
        BIG_rcopy(b,CURVE_Frb);
        FP2_from_BIGs(&f,a,b);

        BIG_rcopy(q,Modulus);
        BIG_copy(m,q);
        BIG_mod(m,r);

        BIG_copy(a,z);
        BIG_mod(a,m);

        BIG_copy(b,z);
        BIG_sdiv(b,m);

        FP12_pinpow(&g2,pin,PBLEN);
        FP12_mul(&g1,&g2);

        FP12_trace(&c,&g1);

        FP12_copy(&g2,&g1);
        FP12_frob(&g2,&f);
        FP12_trace(&cp,&g2);

        FP12_conj(&g1,&g1);
        FP12_mul(&g2,&g1);
        FP12_trace(&cpm1,&g2);
        FP12_mul(&g2,&g1);
        FP12_trace(&cpm2,&g2);

        FP4_xtr_pow2(&c,&cp,&c,&cpm1,&cpm2,a,b);
        mpin_hash(sha,&c,&W,CK);

    }
    return res;
}

/* calculate common key on server side */
/* Z=r.A - no time permits involved */

int MPIN_SERVER_KEY(int sha,octet *Z,octet *SST,octet *W,octet *H,octet *HID,octet *xID,octet *xCID,octet *SK)
{
    int res=0;
    FP12 g;
    FP4 c;
    ECP R,U,A;
    ECP2 sQ;
    BIG w,h;

    if (!ECP2_fromOctet(&sQ,SST)) res=MPIN_INVALID_POINT;
    if (!ECP_fromOctet(&R,Z)) res=MPIN_INVALID_POINT;


    if (!ECP_fromOctet(&A,HID)) res=MPIN_INVALID_POINT;

    // new
    if (xCID!=NULL)
    {
        if (!ECP_fromOctet(&U,xCID)) res=MPIN_INVALID_POINT;
    }
    else
    {
        if (!ECP_fromOctet(&U,xID)) res=MPIN_INVALID_POINT;
    }
    BIG_fromBytes(w,W->val);
    BIG_fromBytes(h,H->val);


    PAIR_ate(&g,&sQ,&A);
    PAIR_fexp(&g);

    if (res==0)
    {
        PAIR_G1mul(&A,h);
        ECP_add(&R,&A);  // new
        PAIR_ate(&g,&sQ,&R);
        PAIR_fexp(&g);
        PAIR_G1mul(&U,w);
        FP12_trace(&c,&g);
        mpin_hash(sha,&c,&U,SK);
    }
    return res;
}

unsign32 MPIN_GET_TIME(void)
{
    return (unsign32)time(NULL);
}

/* Generate Y = H(TimeValue, xCID/xID) */
void MPIN_GET_Y(int sha,int TimeValue,octet *xCID,octet *Y)
{
    BIG q,y;
    char h[MODBYTES];
    octet H= {0,sizeof(h),h};

    hashit(sha,TimeValue,xCID,&H);
    BIG_fromBytes(y,H.val);
    BIG_rcopy(q,CURVE_Order);
    BIG_mod(y,q);
    BIG_toBytes(Y->val,y);
    Y->len=PGS;
}

/* One pass MPIN Client */
int MPIN_CLIENT(int sha,int date,octet *ID,csprng *RNG,octet *X,int pin,octet *TOKEN,octet *V,octet *U,octet *UT,octet *TP,octet *MESSAGE,int TimeValue,octet *Y)
{
    int rtn=0;
    char m[M_SIZE];
    octet M= {0,sizeof(m),m};

    octet *pID;
    if (date == 0)
        pID = U;
    else
        pID = UT;

    rtn = MPIN_CLIENT_1(sha,date,ID,RNG,X,pin,TOKEN,V,U,UT,TP);
    if (rtn != 0)
        return rtn;

    OCT_joctet(&M,pID);
    if (MESSAGE!=NULL)
    {
        OCT_joctet(&M,MESSAGE);
    }

    MPIN_GET_Y(sha,TimeValue,&M,Y);

    rtn = MPIN_CLIENT_2(X,Y,V);
    if (rtn != 0)
        return rtn;

    return 0;
}

/* One pass MPIN Server */
int MPIN_SERVER(int sha,int date,octet *HID,octet *HTID,octet *Y,octet *sQ,octet *U,octet *UT,octet *V,octet *E,octet *F,octet *ID,octet *MESSAGE,int TimeValue)
{
    int rtn=0;
    char m[M_SIZE];
    octet M= {0,sizeof(m),m};

    octet *pU;
    if (date == 0)
        pU = U;
    else
        pU = UT;

    MPIN_SERVER_1(sha,date,ID,HID,HTID);

    OCT_joctet(&M,pU);
    if (MESSAGE!=NULL)
    {
        OCT_joctet(&M,MESSAGE);
    }

    MPIN_GET_Y(sha,TimeValue,&M,Y);

    rtn = MPIN_SERVER_2(date,HID,HTID,Y,sQ,U,UT,V,E,F);
    if (rtn != 0)
        return rtn;

    return 0;
}

/* AES-GCM Encryption of octets, K is key, H is header,
   P is plaintext, C is ciphertext, T is authentication tag */
void MPIN_AES_GCM_ENCRYPT(octet *K,octet *IV,octet *H,octet *P,octet *C,octet *T)
{
    gcm g;
    GCM_init(&g,K->len,K->val,IV->len,IV->val);
    GCM_add_header(&g,H->val,H->len);
    GCM_add_plain(&g,C->val,P->val,P->len);
    C->len=P->len;
    GCM_finish(&g,T->val);
    T->len=16;
}

/* AES-GCM Decryption of octets, K is key, H is header,
   P is plaintext, C is ciphertext, T is authentication tag */
void MPIN_AES_GCM_DECRYPT(octet *K,octet *IV,octet *H,octet *C,octet *P,octet *T)
{
    gcm g;
    GCM_init(&g,K->len,K->val,IV->len,IV->val);
    GCM_add_header(&g,H->val,H->len);
    GCM_add_cipher(&g,P->val,C->val,C->len);
    P->len=C->len;
    GCM_finish(&g,T->val);
    T->len=16;
}

/* Return the Field size */
int MPIN_FS()
{
    return PFS;
}

/* Return the Group size */
int MPIN_GS()
{
    return PGS;
}

/*
int MPIN_TEST_PAIRING(octet *CID,octet *R)
{
    BIG b,px;
	FP2 qx,qy;
	FP12 g;
    ECP2 Q;
	ECP P;
    int res=0;

	hashit(-1,CID,&P);
	BIG_rcopy(qx.a,CURVE_Pxa); FP_nres(qx.a);
	BIG_rcopy(qx.b,CURVE_Pxb); FP_nres(qx.b);
	BIG_rcopy(qy.a,CURVE_Pya); FP_nres(qy.a);
	BIG_rcopy(qy.b,CURVE_Pyb); FP_nres(qy.b);

	if (!ECP2_set(&Q,&qx,&qy))  res=MPIN_INVALID_POINT;

	if (res==0)
	{
		PAIR_ate(&g,&Q,&P);
        PAIR_fexp(&g);
		FP12_trace(&(g.a),&g);

		BIG_copy(b,g.a.a.a); FP_redc(b); printf("trace pairing= "); BIG_output(b); printf("\n");
		BIG_copy(b,g.a.a.b); FP_redc(b); printf("trace pairing= "); BIG_output(b); printf("\n");
		BIG_copy(b,g.a.b.a); FP_redc(b); printf("trace pairing= "); BIG_output(b); printf("\n");
		BIG_copy(b,g.a.b.b); FP_redc(b); printf("trace pairing= "); BIG_output(b); printf("\n");

	}

    return res;
}
*/

/*
int main()
{
	ECP2 X;
	FP2 x,y,rhs;
	BIG r;
	char hcid[HASH_BYTES],client_id[100];
	octet HCID={0,sizeof(hcid),hcid};
	octet CLIENT_ID={0,sizeof(client_id),client_id};

	OCT_jstring(&CLIENT_ID,"testUser@certivox.com");
	MPIN_HASH_ID(&CLIENT_ID,&HCID);

	printf("Client ID= "); OCT_output_string(&CLIENT_ID); printf("\n");

	mapit2(&HCID,&X);

	ECP2_output(&X);

	BIG_rcopy(r,CURVE_Order);

	ECP2_mul(&X,r);

	ECP2_output(&X);

}
*/
