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

/* Test good token and correct PIN with D-TA for
   nTimePermitTests days in the future */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "mpin.h"

int main()
{
  int i,PIN1,PIN2,rtn,err,iter;

  char id[256];
  octet ID = {0,sizeof(id),id};

  char x[PGS],y[PGS];
  octet X={sizeof(x), sizeof(x),x};
  octet Y={sizeof(y),sizeof(y),y};

  /* Master secret shares */
  char ms1[PGS], ms2[PGS];
  octet MS1={sizeof(ms1),sizeof(ms1),ms1};
  octet MS2={sizeof(ms2),sizeof(ms2),ms2};

  /* Hash values of ID */
  char hcid[32];
  octet HCID={sizeof(hcid),sizeof(hcid), hcid};

  /* Client secret and shares */
  char cs1[2*PFS+1], cs2[2*PFS+1], sec[2*PFS+1];
  octet SEC={sizeof(sec),sizeof(sec),sec};
  octet CS1={sizeof(cs1),sizeof(cs1), cs1};
  octet CS2={sizeof(cs2),sizeof(cs2), cs2};

  /* Server secret and shares */
  char ss1[4*PFS], ss2[4*PFS], serverSecret[4*PFS];
  octet ServerSecret={sizeof(serverSecret),sizeof(serverSecret),serverSecret};
  octet SS1={sizeof(ss1),sizeof(ss1),ss1};
  octet SS2={sizeof(ss2),sizeof(ss2),ss2};

  /* Time Permit and shares */
  char tp1[2*PFS+1], tp2[2*PFS+1], tp[2*PFS+1];
  octet TP={sizeof(tp),sizeof(tp),tp};
  octet TP1={sizeof(tp1),sizeof(tp1),tp1};
  octet TP2={sizeof(tp2),sizeof(tp2),tp2};

  /* Token stored on computer */
  char token[2*PFS+1];
  octet TOKEN={sizeof(token),sizeof(token),token};

  char ut[2*PFS+1],u[2*PFS+1];
  octet UT={sizeof(ut),sizeof(ut),ut};
  octet U={sizeof(u),sizeof(u),u};

  char hid[2*PFS+1],htid[2*PFS+1];
  octet HID={0,sizeof(hid),hid};
  octet HTID={0,sizeof(htid),htid};

  char e[12*PFS], f[12*PFS];
  octet E={sizeof(e),sizeof(e),e};
  octet F={sizeof(f),sizeof(f),f};

  octet *pID;

  PIN1 = 1234;
  PIN2 = 1234;

  /* Assign the End-User an ID */
  char* user = "testuser@miracl.com";
  OCT_jstring(&ID,user);
  printf("CLIENT: ID %s\n", user);

  int date = 0;
  char seed[100] = {0};
  octet SEED = {0,sizeof(seed),seed};
  csprng RNG;

  /* unrandom seed value! */
  SEED.len=100;
  for (i=0;i<100;i++) SEED.val[i]=i+1;

  /* initialise random number generator */
  MPIN_CREATE_CSPRNG(&RNG,&SEED);

  /* Hash ID */
  MPIN_HASH_ID(&ID,&HCID);
  OCT_output(&HCID);

  /* When set only send hashed IDs to server */
#ifdef USE_ANONYMOUS
  pID = &HCID;
#else
  pID = &ID;
#endif

  /* Generate Client master secret for MIRACL and Customer */
  rtn = MPIN_RANDOM_GENERATE(&RNG,&MS1);
  if (rtn != 0) {
      printf("MPIN_RANDOM_GENERATE(&RNG,&MS1) Error %d\n", rtn);
      return 1;
  }
  rtn = MPIN_RANDOM_GENERATE(&RNG,&MS2);
  if (rtn != 0) {
      printf("MPIN_RANDOM_GENERATE(&RNG,&MS2) Error %d\n", rtn);
      return 1;
  }
  printf("MASTER SECRET MIRACL:= 0x");
  OCT_output(&MS1);
  printf("MASTER SECRET CUSTOMER:= 0x");
  OCT_output(&MS2);

  /* Generate server secret shares */
  rtn = MPIN_GET_SERVER_SECRET(&MS1,&SS1);
  if (rtn != 0) {
      printf("MPIN_GET_SERVER_SECRET(&MS1,&SS1) Error %d\n", rtn);
      return 1;
  }
  rtn = MPIN_GET_SERVER_SECRET(&MS2,&SS2);
  if (rtn != 0) {
      printf("MPIN_GET_SERVER_SECRET(&MS2,&SS2) Error %d\n", rtn);
      return 1;
  }
  printf("SS1 = 0x");
  OCT_output(&SS1);
  printf("SS2 = 0x");
  OCT_output(&SS2);

  /* Combine server secret share */
  rtn = MPIN_RECOMBINE_G2(&SS1, &SS2, &ServerSecret);
  if (rtn != 0) {
      printf("MPIN_RECOMBINE_G2(&SS1, &SS2, &ServerSecret) Error %d\n", rtn);
      return 1;
  }
  printf("ServerSecret = 0x");
  OCT_output(&ServerSecret);

  /* Generate client secret shares */
  rtn = MPIN_GET_CLIENT_SECRET(&MS1,&HCID,&CS1);
  if (rtn != 0) {
      printf("MPIN_GET_CLIENT_SECRET(&MS1,&HCID,&CS1) Error %d\n", rtn);
      return 1;
  }
  rtn = MPIN_GET_CLIENT_SECRET(&MS2,&HCID,&CS2);
  if (rtn != 0) {
      printf("MPIN_GET_CLIENT_SECRET(&MS2,&HCID,&CS2) Error %d\n", rtn);
      return 1;
  }
  printf("CS1 = 0x");
  OCT_output(&CS1);
  printf("CS2 = 0x");
  OCT_output(&CS2);

  /* Combine client secret shares : TOKEN is the full client secret */
  rtn = MPIN_RECOMBINE_G1(&CS1, &CS2, &TOKEN);
  if (rtn != 0) {
      printf("MPIN_RECOMBINE_G1(&CS1, &CS2, &TOKEN) Error %d\n", rtn);
      return 1;
  }
  printf("Client Secret = 0x");
  OCT_output(&TOKEN);

  /* Client extracts PIN1 from secret to create Token */
  rtn = MPIN_EXTRACT_PIN(&ID, PIN1, &TOKEN);
  if (rtn != 0) {
      printf("MPIN_EXTRACT_PIN( &ID, PIN, &TOKEN) Error %d\n", rtn);
      return 1;
  }
  printf("Token = 0x");
  OCT_output(&TOKEN);

  /* Generate Time Permit shares */
  date = MPIN_today();
  for(iter=1; iter<nTimePermitTests+1; iter++)
    {
      printf("Date %d \n", date);
      rtn = MPIN_GET_CLIENT_PERMIT(date,&MS1,&HCID,&TP1);
      if (rtn != 0) {
          printf("MPIN_GET_CLIENT_PERMIT(date,&MS1,&HCID,&TP1) Error %d\n", rtn);
          return 1;
      }
      rtn = MPIN_GET_CLIENT_PERMIT(date,&MS2,&HCID,&TP2);
      if (rtn != 0) {
          printf("MPIN_GET_CLIENT_PERMIT(date,&MS2,&HCID,&TP2) Error %d\n", rtn);
          return 1;
      }
      printf("TP1 = 0x");
      OCT_output(&TP1);
      printf("TP2 = 0x");
      OCT_output(&TP2);

      /* Combine Time Permit shares */
      rtn = MPIN_RECOMBINE_G1(&TP1, &TP2, &TP);
      if (rtn != 0) {
          printf("MPIN_RECOMBINE_G1(&TP1, &TP2, &TP) Error %d\n", rtn);
          return 1;
      }
      printf("Time Permit = 0x");
      OCT_output(&TP);

      /* Client first pass */
      rtn = MPIN_CLIENT_1(date,&ID,&RNG,&X,PIN2,&TOKEN,&SEC,&U,&UT,&TP);
      if (rtn != 0) {
          printf("MPIN_CLIENT_1 ERROR %d\n", rtn);
          return 1;
      }

      /* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
      MPIN_SERVER_1(date,pID,&HID,&HTID);

      /* Server generates Random number Y and sends it to Client */
      rtn = MPIN_RANDOM_GENERATE(&RNG,&Y);
      if (rtn != 0) {
          printf("MPIN_RANDOM_GENERATE(&RNG,&Y) Error %d\n", rtn);
          return 1;
      }
      printf("Y = 0x");
      OCT_output(&Y);

      /* Client second pass */
      rtn = MPIN_CLIENT_2(&X,&Y,&SEC);
      if (rtn != 0) {
          printf("MPIN_CLIENT_2(&X,&Y,&SEC) Error %d\n", rtn);
          return 1;
      }
      printf("V = 0x");
      OCT_output(&SEC);

      /* Server second pass */
      rtn = MPIN_SERVER_2(date,&HID,&HTID,&Y,&ServerSecret,&U,&UT,&SEC,&E,&F);
      if (rtn != 0) {
          err=MPIN_KANGAROO(&E,&F);
          if (err==0) {
              printf("Iteration %d FAILURE Invalid Token Error Code %d\n", iter, rtn);
              break;
	  } else {
              printf("Iteration %d FAILURE PIN Error %d, Error Code %d\n", iter, err, rtn);
              break;
	   }
      } else {
          printf("Iteration %d SUCCESS Error Code %d\n", iter, rtn);
      }
      date++;
    }
  return 0;
}
