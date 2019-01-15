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

/* ECDH/ECIES/ECDSA Functions - see main program below */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ecdh.h"

#define ROUNDUP(a,b) ((a)-1)/(b)+1

/* general purpose hash function w=hash(p|n|x|y) */
/* pad or truncate ouput to length pad if pad!=0 */
static void hashit(int sha,octet *p,int n,octet *x,octet *w,int pad)
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

    for (i=0; i<p->len; i++)
    {
        switch(sha)
        {
        case SHA256:
            HASH256_process(&sha256,p->val[i]);
            break;
        case SHA384:
            HASH384_process(&sha512,p->val[i]);
            break;
        case SHA512:
            HASH512_process(&sha512,p->val[i]);
            break;
        }
    }
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
    if (!pad)
        OCT_jbytes(w,hh,hlen);
    else
    {
        if (pad<=hlen)
            OCT_jbytes(w,hh,pad);
        else
        {
            OCT_jbytes(w,hh,hlen);
            OCT_jbyte(w,0,pad-hlen);
        }
    }
    return;
}

/* Hash octet p to octet w */
void HASH(int sha,octet *p,octet *w)
{
    hashit(sha,p,-1,NULL,w,0);
}

/* Calculate HMAC of m using key k. HMAC is tag of length olen */
int HMAC(int sha,octet *m,octet *k,int olen,octet *tag)
{
    /* Input is from an octet m        *
     * olen is requested output length in bytes. k is the key  *
     * The output is the calculated tag */
    int hlen,b;
    char h[128],k0[128];
    octet H= {0,sizeof(h),h};
    octet K0= {0,sizeof(k0),k0};

    hlen=sha;
    if (hlen>32) b=128;
    else b=64;

    if (olen<4 /*|| olen>hlen*/) return 0;

    if (k->len > b) hashit(sha,k,-1,NULL,&K0,0);
    else            OCT_copy(&K0,k);

    OCT_jbyte(&K0,0,b-K0.len);

    OCT_xorbyte(&K0,0x36);

    hashit(sha,&K0,-1,m,&H,0);

    OCT_xorbyte(&K0,0x6a);   /* 0x6a = 0x36 ^ 0x5c */
    hashit(sha,&K0,-1,&H,&H,olen);

    OCT_empty(tag);

    OCT_jbytes(tag,H.val,olen);

    return 1;
}

/* Key Derivation Functions */
/* Input octet z */
/* Output key of length olen */
/*
void KDF1(octet *z,int olen,octet *key)
{
    char h[32];
	octet H={0,sizeof(h),h};
    int counter,cthreshold;
    int hlen=32;

    OCT_empty(key);

    cthreshold=ROUNDUP(olen,hlen);

    for (counter=0;counter<cthreshold;counter++)
    {
        hashit(z,counter,NULL,NULL,&H);
        if (key->len+hlen>olen) OCT_jbytes(key,H.val,olen%hlen);
        else                    OCT_joctet(key,&H);
    }
}
*/
void KDF2(int sha,octet *z,octet *p,int olen,octet *key)
{
    /* NOTE: the parameter olen is the length of the output k in bytes */
    char h[64];
    octet H= {0,sizeof(h),h};
    int counter,cthreshold;
    int hlen=sha;

    OCT_empty(key);

    cthreshold=ROUNDUP(olen,hlen);

    for (counter=1; counter<=cthreshold; counter++)
    {
        hashit(sha,z,counter,p,&H,0);
        if (key->len+hlen>olen)  OCT_jbytes(key,H.val,olen%hlen);
        else                     OCT_joctet(key,&H);
    }

}

/* Password based Key Derivation Function */
/* Input password p, salt s, and repeat count */
/* Output key of length olen */
void PBKDF2(int sha,octet *p,octet *s,int rep,int olen,octet *key)
{
    int i,j,len,d=ROUNDUP(olen,sha);
    char f[64],u[64];   /*****/
    octet F= {0,sizeof(f),f};
    octet U= {0,sizeof(u),u};
    OCT_empty(key);

    for (i=1; i<=d; i++)
    {
        len=s->len;
        OCT_jint(s,i,4);

        HMAC(sha,s,p,sha,&F);  /* sha not EFS */

        s->len=len;
        OCT_copy(&U,&F);
        for (j=2; j<=rep; j++)
        {
            HMAC(sha,&U,p,sha,&U); /* sha not EFS */
            OCT_xor(&F,&U);
        }

        OCT_joctet(key,&F);
    }

    OCT_chop(key,NULL,olen);
}

/* AES encryption/decryption. Encrypt byte array M using key K and returns ciphertext */
void AES_CBC_IV0_ENCRYPT(octet *k,octet *m,octet *c)
{
    /* AES CBC encryption, with Null IV and key k */
    /* Input is from an octet string m, output is to an octet string c */
    /* Input is padded as necessary to make up a full final block */
    amcl_aes a;
    int fin;
    int i,j,ipt,opt;
    char buff[16];
    int padlen;

    OCT_clear(c);
    if (m->len==0) return;
    AES_init(&a,CBC,k->len,k->val,NULL);

    ipt=opt=0;
    fin=0;
    for(;;)
    {
        for (i=0; i<16; i++)
        {
            if (ipt<m->len) buff[i]=m->val[ipt++];
            else
            {
                fin=1;
                break;
            }
        }
        if (fin) break;
        AES_encrypt(&a,buff);
        for (i=0; i<16; i++)
            if (opt<c->max) c->val[opt++]=buff[i];
    }

    /* last block, filled up to i-th index */

    padlen=16-i;
    for (j=i; j<16; j++) buff[j]=padlen;
    AES_encrypt(&a,buff);
    for (i=0; i<16; i++)
        if (opt<c->max) c->val[opt++]=buff[i];
    AES_end(&a);
    c->len=opt;
}

/* decrypts and returns TRUE if all consistent, else returns FALSE */
int AES_CBC_IV0_DECRYPT(octet *k,octet *c,octet *m)
{
    /* padding is removed */
    amcl_aes a;
    int i,ipt,opt,ch;
    char buff[16];
    int fin,bad;
    int padlen;
    ipt=opt=0;

    OCT_clear(m);
    if (c->len==0) return 1;
    ch=c->val[ipt++];

    AES_init(&a,CBC,k->len,k->val,NULL);
    fin=0;

    for(;;)
    {
        for (i=0; i<16; i++)
        {
            buff[i]=ch;
            if (ipt>=c->len)
            {
                fin=1;
                break;
            }
            else ch=c->val[ipt++];
        }
        AES_decrypt(&a,buff);
        if (fin) break;
        for (i=0; i<16; i++)
            if (opt<m->max) m->val[opt++]=buff[i];
    }
    AES_end(&a);
    bad=0;
    padlen=buff[15];
    if (i!=15 || padlen<1 || padlen>16) bad=1;
    if (padlen>=2 && padlen<=16)
        for (i=16-padlen; i<16; i++) if (buff[i]!=padlen) bad=1;

    if (!bad) for (i=0; i<16-padlen; i++)
            if (opt<m->max) m->val[opt++]=buff[i];

    m->len=opt;
    if (bad) return 0;
    return 1;
}

/* Calculate a public/private EC GF(p) key pair. W=S.G mod EC(p),
 * where S is the secret key and W is the public key
 * and G is fixed generator.
 * If RNG is NULL then the private key is provided externally in S
 * otherwise it is generated randomly internally */
int ECP_KEY_PAIR_GENERATE(csprng *RNG,octet* S,octet *W)
{
    BIG r,gx,s;
    ECP G;
    int res=0;
    BIG_rcopy(gx,CURVE_Gx);

#if CURVETYPE!=MONTGOMERY
    BIG gy;
    BIG_rcopy(gy,CURVE_Gy);
    ECP_set(&G,gx,gy);
#else
    ECP_set(&G,gx);
#endif

    BIG_rcopy(r,CURVE_Order);
    if (RNG!=NULL)
    {
        BIG_randomnum(s,r,RNG);
    }
    else
    {
        BIG_fromBytes(s,S->val);
        BIG_mod(s,r);
    }

#ifdef AES_S
    BIG_mod2m(s,2*AES_S);
//	BIG_toBytes(S->val,s);
#endif

    ECP_mul(&G,s);
#if CURVETYPE!=MONTGOMERY
    ECP_get(gx,gy,&G);
#else
    ECP_get(gx,&G);
    /*
    	ECP_rhs(gy,gx);
    	FP_sqrt(gy,gy);
    	FP_neg(gy,gy);
    	FP_inv(gy,gy);
    	FP_mul(r,gx,gy);
    	FP_reduce(r);

        BIG_zero(gy);
    	BIG_inc(gy,486664);
    	FP_neg(gy,gy);
    	FP_sqrt(gy,gy);
    	FP_reduce(gy);
    	FP_mul(r,r,gy);
    	FP_reduce(r);

    	printf("x= "); BIG_output(r); printf("\n");

    	BIG_copy(r,gx);
    	BIG_dec(r,1);
    	BIG_copy(gy,gx);
    	BIG_inc(gy,1);
    	FP_inv(gy,gy);
    	FP_mul(r,r,gy);
    	FP_reduce(r);

    	printf("y= "); BIG_output(r); printf("\n");

    	BIG_zero(r);
    	BIG_inc(r,121665);
    	BIG_zero(gy);
    	BIG_inc(gy,121666);
    	FP_inv(gy,gy);
    	FP_mul(r,r,gy);
    	FP_neg(r,r);
    	FP_reduce(r);

    	printf("d= "); BIG_output(r); printf("\n");
    */

#endif

    S->len=EGS;
    BIG_toBytes(S->val,s);

#if CURVETYPE!=MONTGOMERY
    W->len=2*EFS+1;
    W->val[0]=4;
    BIG_toBytes(&(W->val[1]),gx);
    BIG_toBytes(&(W->val[EFS+1]),gy);
#else
    W->len=EFS+1;
    W->val[0]=2;
    BIG_toBytes(&(W->val[1]),gx);
#endif

    return res;
}

/* validate public key. Set full=true for fuller check */
int ECP_PUBLIC_KEY_VALIDATE(int full,octet *W)
{
    BIG q,r,wx;
    ECP WP;
    int valid;
    int res=0;

    BIG_rcopy(q,Modulus);
    BIG_rcopy(r,CURVE_Order);

    BIG_fromBytes(wx,&(W->val[1]));
    if (BIG_comp(wx,q)>=0) res=ECDH_INVALID_PUBLIC_KEY;
#if CURVETYPE!=MONTGOMERY
    BIG wy;
    BIG_fromBytes(wy,&(W->val[EFS+1]));
    if (BIG_comp(wy,q)>=0) res=ECDH_INVALID_PUBLIC_KEY;
#endif
    if (res==0)
    {

#if CURVETYPE!=MONTGOMERY
        valid=ECP_set(&WP,wx,wy);
#else
        valid=ECP_set(&WP,wx);
#endif
        if (!valid || ECP_isinf(&WP)) res=ECDH_INVALID_PUBLIC_KEY;
        if (res==0 && full)
        {

            ECP_mul(&WP,r);
            if (!ECP_isinf(&WP)) res=ECDH_INVALID_PUBLIC_KEY;
        }
    }

    return res;
}

/* IEEE-1363 Diffie-Hellman online calculation Z=S.WD */
int ECPSVDP_DH(octet *S,octet *WD,octet *Z)
{
    BIG r,s,wx;
    int valid;
    ECP W;
    int res=0;

    BIG_fromBytes(s,S->val);

    BIG_fromBytes(wx,&(WD->val[1]));
#if CURVETYPE!=MONTGOMERY
    BIG wy;
    BIG_fromBytes(wy,&(WD->val[EFS+1]));
    valid=ECP_set(&W,wx,wy);
#else
    valid=ECP_set(&W,wx);
#endif
    if (!valid) res=ECDH_ERROR;
    if (res==0)
    {
        BIG_rcopy(r,CURVE_Order);
        BIG_mod(s,r);

        ECP_mul(&W,s);
        if (ECP_isinf(&W)) res=ECDH_ERROR;
        else
        {
#if CURVETYPE!=MONTGOMERY
            ECP_get(wx,wx,&W);
#else
            ECP_get(wx,&W);
#endif
            Z->len=MODBYTES;
            BIG_toBytes(Z->val,wx);
        }
    }
    return res;
}

#if CURVETYPE!=MONTGOMERY

/* IEEE ECDSA Signature, C and D are signature on F using private key S */
int ECPSP_DSA(int sha,csprng *RNG,octet *K,octet *S,octet *F,octet *C,octet *D)
{
    char h[128];
    octet H= {0,sizeof(h),h};

    BIG gx,gy,r,s,f,c,d,u,vx,w;
    ECP G,V;

    hashit(sha,F,-1,NULL,&H,sha);
    BIG_rcopy(gx,CURVE_Gx);
    BIG_rcopy(gy,CURVE_Gy);
    BIG_rcopy(r,CURVE_Order);

    BIG_fromBytes(s,S->val);

    int hlen=H.len;
    if (H.len>MODBYTES) hlen=MODBYTES;
    BIG_fromBytesLen(f,H.val,hlen);

    ECP_set(&G,gx,gy);

    do
    {
        if (RNG!=NULL)
        {
            BIG_randomnum(u,r,RNG);
            BIG_randomnum(w,r,RNG); /* randomize calculation */
        }
        else
        {
            BIG_fromBytes(u,K->val);
            BIG_mod(u,r);
        }

#ifdef AES_S
        BIG_mod2m(u,2*AES_S);
#endif
        ECP_copy(&V,&G);
        ECP_mul(&V,u);

        ECP_get(vx,vx,&V);

        BIG_copy(c,vx);
        BIG_mod(c,r);
        if (BIG_iszilch(c)) continue;
        if (RNG!=NULL)
        {
            BIG_modmul(u,u,w,r);
        }

        BIG_invmodp(u,u,r);
        BIG_modmul(d,s,c,r);

        BIG_add(d,f,d);
        if (RNG!=NULL)
        {
            BIG_modmul(d,d,w,r);
        }

        BIG_modmul(d,u,d,r);

    }
    while (BIG_iszilch(d));

    C->len=D->len=EGS;

    BIG_toBytes(C->val,c);
    BIG_toBytes(D->val,d);

    return 0;
}

/* IEEE1363 ECDSA Signature Verification. Signature C and D on F is verified using public key W */
int ECPVP_DSA(int sha,octet *W,octet *F, octet *C,octet *D)
{
    char h[128];
    octet H= {0,sizeof(h),h};

    BIG r,gx,gy,wx,wy,f,c,d,h2;
    int res=0;
    ECP G,WP;
    int valid;

    hashit(sha,F,-1,NULL,&H,sha);
    BIG_rcopy(gx,CURVE_Gx);
    BIG_rcopy(gy,CURVE_Gy);
    BIG_rcopy(r,CURVE_Order);

    OCT_shl(C,C->len-MODBYTES);
    OCT_shl(D,D->len-MODBYTES);

    BIG_fromBytes(c,C->val);
    BIG_fromBytes(d,D->val);

    int hlen=H.len;
    if (hlen>MODBYTES) hlen=MODBYTES;

    BIG_fromBytesLen(f,H.val,hlen);

    //BIG_fromBytes(f,H.val);

    if (BIG_iszilch(c) || BIG_comp(c,r)>=0 || BIG_iszilch(d) || BIG_comp(d,r)>=0)
        res=ECDH_INVALID;

    if (res==0)
    {
        BIG_invmodp(d,d,r);
        BIG_modmul(f,f,d,r);
        BIG_modmul(h2,c,d,r);

        ECP_set(&G,gx,gy);

        BIG_fromBytes(wx,&(W->val[1]));
        BIG_fromBytes(wy,&(W->val[EFS+1]));

        valid=ECP_set(&WP,wx,wy);

        if (!valid) res=ECDH_ERROR;
        else
        {
            ECP_mul2(&WP,&G,h2,f);

            if (ECP_isinf(&WP)) res=ECDH_INVALID;
            else
            {
                ECP_get(d,d,&WP);
                BIG_mod(d,r);
                if (BIG_comp(d,c)!=0) res=ECDH_INVALID;
            }
        }
    }

    return res;
}

/* IEEE1363 ECIES encryption. Encryption of plaintext M uses public key W and produces ciphertext V,C,T */
void ECP_ECIES_ENCRYPT(int sha,octet *P1,octet *P2,csprng *RNG,octet *W,octet *M,int tlen,octet *V,octet *C,octet *T)
{

    int i,len;
    char z[EFS],vz[3*EFS+1],k[2*EAS],k1[EAS],k2[EAS],l2[8],u[EFS];
    octet Z= {0,sizeof(z),z};
    octet VZ= {0,sizeof(vz),vz};
    octet K= {0,sizeof(k),k};
    octet K1= {0,sizeof(k1),k1};
    octet K2= {0,sizeof(k2),k2};
    octet L2= {0,sizeof(l2),l2};
    octet U= {0,sizeof(u),u};

    if (ECP_KEY_PAIR_GENERATE(RNG,&U,V)!=0) return;
    if (ECPSVDP_DH(&U,W,&Z)!=0) return;

    OCT_copy(&VZ,V);
    OCT_joctet(&VZ,&Z);

    KDF2(sha,&VZ,P1,2*EAS,&K);

    K1.len=K2.len=EAS;
    for (i=0; i<EAS; i++)
    {
        K1.val[i]=K.val[i];
        K2.val[i]=K.val[EAS+i];
    }

    AES_CBC_IV0_ENCRYPT(&K1,M,C);

    OCT_jint(&L2,P2->len,8);

    len=C->len;
    OCT_joctet(C,P2);
    OCT_joctet(C,&L2);
    HMAC(sha,C,&K2,tlen,T);
    C->len=len;
}

/* IEEE1363 ECIES decryption. Decryption of ciphertext V,C,T using private key U outputs plaintext M */
int ECP_ECIES_DECRYPT(int sha,octet *P1,octet *P2,octet *V,octet *C,octet *T,octet *U,octet *M)
{

    int i,len;
    char z[EFS],vz[3*EFS+1],k[2*EAS],k1[EAS],k2[EAS],l2[8],tag[32];
    octet Z= {0,sizeof(z),z};
    octet VZ= {0,sizeof(vz),vz};
    octet K= {0,sizeof(k),k};
    octet K1= {0,sizeof(k1),k1};
    octet K2= {0,sizeof(k2),k2};
    octet L2= {0,sizeof(l2),l2};
    octet TAG= {0,sizeof(tag),tag};

    if (ECPSVDP_DH(U,V,&Z)!=0) return 0;

    OCT_copy(&VZ,V);
    OCT_joctet(&VZ,&Z);

    KDF2(sha,&VZ,P1,EFS,&K);

    K1.len=K2.len=EAS;
    for (i=0; i<EAS; i++)
    {
        K1.val[i]=K.val[i];
        K2.val[i]=K.val[EAS+i];
    }

    if (!AES_CBC_IV0_DECRYPT(&K1,C,M)) return 0;

    OCT_jint(&L2,P2->len,8);

    len=C->len;
    OCT_joctet(C,P2);
    OCT_joctet(C,&L2);
    HMAC(sha,C,&K2,T->len,&TAG);
    C->len=len;

    if (!OCT_comp(T,&TAG)) return 0;

    return 1;

}

#endif
