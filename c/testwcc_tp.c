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


/* Demonstrate WCC with time permits */

/* Build executible after installation:
   gcc -std=c99 -g testwcc_dta.c  -I/opt/amcl/include -L/opt/amcl/lib -lamcl -lwcc -o testwcc_dta */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpin.h"
#include "wcc.h"

#define DEBUG

int main()
{
  int i,rtn;

  /* Master secret shares */
  char ms[PGS];
  octet MS={sizeof(ms),sizeof(ms),ms};

  // Sender key
  char akeyG1[2*PFS+1];
  octet AKeyG1={0,sizeof(akeyG1), akeyG1};

  // Sender time permits
  char aTPG1[2*PFS+1];
  octet ATPG1={sizeof(aTPG1),sizeof(aTPG1), aTPG1};

  // Receiver keys
  char bkeyG2[4*PFS];
  octet BKeyG2={0,sizeof(bkeyG2), bkeyG2};

  // Receiver time permits
  char bTPG2[4*PFS];
  octet BTPG2={sizeof(bTPG2),sizeof(bTPG2), bTPG2};

  char ahv[HASH_BYTES],alice_id[256],bhv[HASH_BYTES],bob_id[256];
  octet AHV={0,sizeof(ahv),ahv};
  octet BHV={0,sizeof(bhv),bhv};

  octet IdA={0,sizeof(alice_id),alice_id};
  octet IdB={0,sizeof(bob_id),bob_id};

  char x[PGS];
  octet X={sizeof(x),sizeof(x),x};
  char y[PGS];
  octet Y={sizeof(y),sizeof(y),y};
  char w[PGS];
  octet W={sizeof(w),sizeof(w),w};
  char pia[PGS];
  octet PIA={sizeof(pia),sizeof(pia),pia};
  char pib[PGS];
  octet PIB={sizeof(pib),sizeof(pib),pib};

  char pgg1[2*PFS+1];
  octet PgG1={0,sizeof(pgg1), pgg1};

  char pag1[2*PFS+1];
  octet PaG1={0,sizeof(pag1), pag1};

  char pbg2[4*PFS];
  octet PbG2={0,sizeof(pbg2), pbg2};

  char seed[32] = {0};
  octet SEED = {0,sizeof(seed),seed};
  csprng RNG;

  char message1[256];
  char message2[256];
  octet MESSAGE1 = {0, sizeof(message1), message1};
  octet MESSAGE2 = {0, sizeof(message2), message2};

  char t1[16];  // Tag
  char t2[16];  // Tag
  char k1[16];  // AES Key
  char k2[16];  // AES Key
  char iv[12]; // IV - Initialisation vector
  char c[100];  // Ciphertext
  char p[100];  // Recovered Plaintext
  octet T1={sizeof(t1),sizeof(t1),t1};
  octet T2={sizeof(t2),sizeof(t2),t2};
  octet K1={0,sizeof(k1),k1};
  octet K2={0,sizeof(k2),k2};
  octet IV={0,sizeof(iv),iv};
  octet C={0,sizeof(c),c};
  octet P={0,sizeof(p),p};

  int date;
  date = WCC_today();
  printf("Date %d \n", date);

  int hashDoneOn = 1;
  int hashDoneOff = 0;

  OCT_jstring(&MESSAGE1,"Hello Bob");
  OCT_jstring(&MESSAGE2,"Hello Alice");

  /* unrandom seed value! */
  SEED.len=32;
  for (i=0;i<32;i++) SEED.val[i]=i+1;

  /* initialise random number generator */
  WCC_CREATE_CSPRNG(&RNG,&SEED);

  /* Generate Client master secret for MIRACL and Customer */
  rtn = WCC_RANDOM_GENERATE(&RNG,&MS);
  if (rtn != 0) {
      printf("TA WCC_RANDOM_GENERATE(&RNG,&MS) Error %d\n", rtn);
      return 1;
  }
  printf("TA MASTER SECRET: ");
  OCT_output(&MS);

  // Alice's ID
  OCT_jstring(&IdA,"alice@miracl.com");

  // TA: Generate Alice's sender key
  WCC_HASH_ID(&IdA,&AHV);
  rtn = WCC_GET_G1_MULTIPLE(hashDoneOn,&MS,&AHV,&AKeyG1);
  if (rtn != 0) {
      printf("TA WCC_GET_G1_MULTIPLE(hashDoneOn,&MS,&AHV,&AKeyG1) Error %d\n", rtn);
      return 1;
  }
  printf("TA Alice's sender key: ");
  OCT_output(&AKeyG1);

  // TA: Generate Alice's G1 time permit
  rtn = WCC_GET_G1_PERMIT(date,&MS,&AHV,&ATPG1);
  if (rtn != 0) {
      printf("TA WCC_GET_G1_PERMIT(date,&MS,&AHV,&ATPG1) Error %d\n", rtn);
      return 1;
  }
  printf("TA Alice's sender time permit: ");
  OCT_output(&ATPG1);

  // Bob's ID
  OCT_jstring(&IdB,"bob@miracl.com");

  // TA: Generate Bob's receiver key
  WCC_HASH_ID(&IdB,&BHV);
  rtn = WCC_GET_G2_MULTIPLE(hashDoneOn,&MS,&BHV,&BKeyG2);
  if (rtn != 0) {
      printf("TA WCC_GET_G2_MULTIPLE(hashDoneOn,&MS,&BHV,&BKeyG2) Error %d\n", rtn);
      return 1;
  }
  printf("TA Bob's receiver key: ");
  OCT_output(&BKeyG2);

  // TA: Generate Bob's receiver time permit
  rtn = WCC_GET_G2_PERMIT(date,&MS,&BHV,&BTPG2);
  if (rtn != 0)
    {
      printf("TA WCC_GET_G2_PERMIT(date,&MS,&BHV,&BTPG2) Error %d\n", rtn);
      return 1;
    }
  printf("TA Bob's receiver time permit: ");
  OCT_output(&BTPG2);
  printf("\n");

  printf("Alice\n");

  rtn = WCC_RANDOM_GENERATE(&RNG,&X);
  if (rtn != 0) {
      printf("Alice WCC_RANDOM_GENERATE(&RNG,&X) Error %d\n", rtn);
      return 1;
  }
#ifdef DEBUG
  printf("Alice X: ");
  OCT_output(&X);
  printf("\n");
#endif

  rtn = WCC_GET_G1_TPMULT(date,&X,&IdA,&PaG1);
  if (rtn != 0) {
      printf("Alice WCC_GET_G1_TPMULT(date,&X,&IdA,&PaG1) Error %d\n", rtn);
      return 1;
  }

  printf("Alice sends IdA and PaG1 to Bob\n\n");
  printf("Alice IdA: "); 
  OCT_output_string(&IdA); 
  printf("\n");
  printf("Alice PaG1: ");
  OCT_output(&PaG1);
  printf("\n");

  printf("Bob\n");

  rtn = WCC_RANDOM_GENERATE(&RNG,&W);
  if (rtn != 0) {
      printf("Bob WCC_RANDOM_GENERATE(&RNG,&W) Error %d\n", rtn);
      return 1;
  }
#ifdef DEBUG
  printf("Bob W: ");
  OCT_output(&W);
  printf("\n");
#endif
  rtn = WCC_GET_G1_TPMULT(date,&W,&IdA,&PgG1);
  if (rtn != 0) {
      printf("Bob WCC_GET_G1_TPMULT(date,&W,&IdA,&PgG1) Error %d\n", rtn);
      return 1;
  }
#ifdef DEBUG
  printf("PgG1: ");
  OCT_output(&PgG1);
  printf("\n");
#endif

  rtn = WCC_RANDOM_GENERATE(&RNG,&Y);
  if (rtn != 0) {
      printf("Bob WCC_RANDOM_GENERATE(&RNG,&Y) Error %d\n", rtn);
      return 1;
  }
#ifdef DEBUG
  printf("Bob Y: ");
  OCT_output(&Y);
  printf("\n");
#endif
  rtn = WCC_GET_G2_TPMULT(date,&Y,&IdB,&PbG2);
  if (rtn != 0) {
      printf("Bob WCC_GET_G1_TPMULT(date,&Y,&IdB,&PbG2) Error %d\n", rtn);
      return 1;
  }
#ifdef DEBUG
  printf("Bob PbG2: ");
  OCT_output(&PbG2);
  printf("\n");
#endif

  // pia = Hq(PaG1,PbG2,PgG1,IdB)
  WCC_Hq(&PaG1,&PbG2,&PgG1,&IdB,&PIA);

  // pib = Hq(PbG2,PaG1,PgG1,IdA)
  WCC_Hq(&PbG2,&PaG1,&PgG1,&IdA,&PIB);

#ifdef DEBUG
  printf("Bob PIA: ");
  OCT_output(&PIA);
  printf("\n");
  printf("Bob PIB: ");
  OCT_output(&PIB);
  printf("\n");
#endif

  // Bob calculates AES Key
  WCC_RECEIVER_KEY(date, &Y, &W,  &PIA, &PIB,  &PaG1, &PgG1, &BKeyG2, &BTPG2, &IdA, &K2);
  if (rtn != 0) {
      printf("Bob WCC_RECEIVER_KEY() Error %d\n", rtn);
      return 1;
  }
  printf("Bob AES Key: ");
  OCT_output(&K2);

  printf("Bob sends IdB, PbG2 and PgG1 to Alice\n\n");
  printf("Bob IdB: "); 
  OCT_output_string(&IdB); 
  printf("\n");
  printf("Bob PbG2: ");
  OCT_output(&PbG2);
  printf("\n");
  printf("Bob PgG1: ");
  OCT_output(&PgG1);
  printf("\n");

  printf("Alice\n");

  // pia = Hq(PaG1,PbG2,PgG1,IdB)
  WCC_Hq(&PaG1,&PbG2,&PgG1,&IdB,&PIA);

  // pib = Hq(PbG2,PaG1,PgG1,IdA)
  WCC_Hq(&PbG2,&PaG1,&PgG1,&IdA,&PIB);

#ifdef DEBUG
  printf("Alice PIA: ");
  OCT_output(&PIA);
  printf("\n");
  printf("Alice PIB: ");
  OCT_output(&PIB);
  printf("\n");
#endif

  // Alice calculates AES Key
  rtn = WCC_SENDER_KEY(date, &X, &PIA, &PIB, &PbG2, &PgG1, &AKeyG1, &ATPG1, &IdB, &K1);
  if (rtn != 0) {
      printf("Alice WCC_SENDER_KEY() Error %d\n", rtn);
      return 1;
  }
  printf("Alice AES Key: ");
  OCT_output(&K1);

  // Send message
  IV.len=12;
  for (i=0;i<IV.len;i++)
    IV.val[i]=i+1;
  printf("Alice: IV ");
  OCT_output(&IV);

  printf("Alice: Message to encrypt for Bob: ");
  OCT_output_string(&MESSAGE1);
  printf("\n");

  WCC_AES_GCM_ENCRYPT(&K1, &IV, &IdA, &MESSAGE1, &C, &T1);

  printf("Alice: Ciphertext: ");
  OCT_output(&C);

  printf("Alice: Encryption Tag: ");
  OCT_output(&T1);
  printf("\n");

  WCC_AES_GCM_DECRYPT(&K2, &IV, &IdA, &C, &P, &T2);

  printf("Bob: Decrypted message received from Alice: ");
  OCT_output_string(&P);
  printf("\n");

  printf("Bob: Decryption Tag: ");
  OCT_output(&T2);
  printf("\n");

  if (!OCT_comp(&MESSAGE1,&P)) {
      printf("FAILURE Decryption\n");
      return 1;
  }

  if (!OCT_comp(&T1,&T2)) {
      printf("FAILURE TAG mismatch\n");
      return 1;
  }

  WCC_KILL_CSPRNG(&RNG);

  return 0;
}
