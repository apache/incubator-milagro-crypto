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

/* Boneh-Lynn-Shacham signature 256-bit API */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bls256_ZZZ.h"

/* hash a message to an ECP point, using SHA3 */

static void BLS_HASHIT(ECP_ZZZ *P,char *m)
{
	int i;
    sha3 hs;
	char h[MODBYTES_XXX];
    octet HM= {0,sizeof(h),h};
	SHA3_init(&hs,SHAKE256);
    for (i=0;m[i]!=0;i++) SHA3_process(&hs,m[i]);
    SHA3_shake(&hs,HM.val,MODBYTES_XXX);
	HM.len=MODBYTES_XXX;
	ECP_ZZZ_mapit(P,&HM);
}

/* generate key pair, private key S, public key W */

int BLS_ZZZ_KEY_PAIR_GENERATE(csprng *RNG,octet* S,octet *W)
{
	ECP8_ZZZ G;
	BIG_XXX s,q;
    BIG_XXX_rcopy(q,CURVE_Order_ZZZ);
	ECP8_ZZZ_generator(&G);
	BIG_XXX_randomnum(s,q,RNG);
    BIG_XXX_toBytes(S->val,s);
    S->len=MODBYTES_XXX;
    PAIR_ZZZ_G2mul(&G,s);
	ECP8_ZZZ_toOctet(W,&G);
	return BLS_OK;
}

/* Sign message m using private key S to produce signature SIG */

int BLS_ZZZ_SIGN(octet *SIG,char *m,octet *S)
{
	BIG_XXX s;
	ECP_ZZZ D;
	BLS_HASHIT(&D,m);
	BIG_XXX_fromBytes(s,S->val);
	PAIR_ZZZ_G1mul(&D,s);
	ECP_ZZZ_toOctet(SIG,&D,true); /* compress output */
	return BLS_OK;
}

/* Verify signature given message m, the signature SIG, and the public key W */

int BLS_ZZZ_VERIFY(octet *SIG,char *m,octet *W)
{
	FP48_YYY v;
	ECP8_ZZZ G,PK;
	ECP_ZZZ D,HM;
	BLS_HASHIT(&HM,m);
	ECP_ZZZ_fromOctet(&D,SIG);
	ECP8_ZZZ_generator(&G);
	ECP8_ZZZ_fromOctet(&PK,W);
	ECP_ZZZ_neg(&D);
    PAIR_ZZZ_double_ate(&v,&G,&D,&PK,&HM);
    PAIR_ZZZ_fexp(&v);
    if (FP48_YYY_isunity(&v)) return BLS_OK;
	return BLS_FAIL;
}

