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

/* test driver and function exerciser for MPIN Functions */
/* Version 3.0 - supports Time Permits */

/* Build executible after installation:

  gcc -std=c99 -g ./testmpin.c -I/opt/amcl/include -L/opt/amcl/lib -lamcl -lmpin -o testmpin

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpin.h"
#include "randapi.h"

#define PERMITS  /* for time permits ON or OFF */
#define PINERROR /* For PIN ERROR detection ON or OFF */
#define FULL     /* for M-Pin Full or M-Pin regular */
//#define SINGLE_PASS /* SINGLE PASS M-Pin */

int mpin(csprng *RNG)
{
    int pin,rtn,err;
#ifdef PERMITS
    int date=MPIN_today();
#else
    int date=0;
#endif
    char x[PGS],s[PGS],y[PGS],client_id[100],sst[4*PFS],token[2*PFS+1],sec[2*PFS+1],permit[2*PFS+1],xcid[2*PFS+1],xid[2*PFS+1],e[12*PFS],f[12*PFS];
    char hcid[PFS],hsid[PFS],hid[2*PFS+1],htid[2*PFS+1],h[PGS];
#ifdef FULL
    char r[PGS],z[2*PFS+1],w[PGS],t[2*PFS+1];
    char g1[12*PFS],g2[12*PFS];
    char ck[PAS],sk[PAS];
#endif
    octet S= {0,sizeof(s),s};
    octet X= {0,sizeof(x),x};
    octet Y= {0,sizeof(y),y};
    octet H= {0,sizeof(h),h};
    octet CLIENT_ID= {0,sizeof(client_id),client_id};
    octet SST= {0,sizeof(sst),sst};
    octet TOKEN= {0,sizeof(token),token};
    octet SEC= {0,sizeof(sec),sec};
    octet PERMIT= {0,sizeof(permit),permit};
    octet xCID= {0,sizeof(xcid),xcid};
    octet xID= {0,sizeof(xid),xid};
    octet HCID= {0,sizeof(hcid),hcid};
    octet HSID= {0,sizeof(hsid),hsid};
    octet HID= {0,sizeof(hid),hid};
    octet HTID= {0,sizeof(htid),htid};
    octet E= {0,sizeof(e),e};
    octet F= {0,sizeof(f),f};
#ifdef FULL
    octet R= {0,sizeof(r),r};
    octet Z= {0,sizeof(z),z};
    octet W= {0,sizeof(w),w};
    octet T= {0,sizeof(t),t};
    octet G1= {0,sizeof(g1),g1};
    octet G2= {0,sizeof(g2),g2};
    octet SK= {0,sizeof(sk),sk};
    octet CK= {0,sizeof(ck),ck};
#endif
    octet *pxID,*pxCID,*pHID,*pHTID,*pE,*pF,*pPERMIT,*prHID;
    char idhex[100];

    /* Trusted Authority set-up */
    MPIN_RANDOM_GENERATE(RNG,&S);
    printf("Master Secret= ");
    OCT_output(&S);

    /* Create Client Identity */
    OCT_jstring(&CLIENT_ID,"testUser@miracl.com");
    MPIN_HASH_ID(HASH_TYPE_MPIN,&CLIENT_ID,&HCID);  /* Either Client or TA calculates Hash(ID) - you decide! */

    printf("Client ID Hash= ");
    OCT_output(&HCID);
    printf("\n");

    OCT_toHex(&CLIENT_ID,idhex);
    printf("Client ID= %s\n",idhex);// OCT_toHex(&CLIENT_ID); printf("\n");

    /* Client and Server are issued secrets by DTA */
    MPIN_GET_SERVER_SECRET(&S,&SST);
    printf("Server Secret= ");
    OCT_output(&SST);

    MPIN_GET_CLIENT_SECRET(&S,&HCID,&TOKEN);
    printf("Client Secret= ");
    OCT_output(&TOKEN);

    /* Client extracts PIN from secret to create Token */
    pin=1234;
    printf("Client extracts PIN= %d\n",pin);
    MPIN_EXTRACT_PIN(HASH_TYPE_MPIN,&CLIENT_ID,pin,&TOKEN);
    printf("Client Token= ");
    OCT_output(&TOKEN);

#ifdef FULL
    MPIN_PRECOMPUTE(&TOKEN,&HCID,NULL,&G1,&G2);
#endif

#ifdef PERMITS
    /* Client gets "Time Permit" from DTA */
    printf("Client gets Time Permit\n");

    MPIN_GET_CLIENT_PERMIT(HASH_TYPE_MPIN,date,&S,&HCID,&PERMIT);
    printf("Time Permit= ");
    OCT_output(&PERMIT);

    /* This encoding makes Time permit look random */
    if (MPIN_ENCODING(RNG,&PERMIT)!=0) printf("Encoding error\n");
    /* printf("Encoded Time Permit= "); OCT_output(&PERMIT); */
    if (MPIN_DECODING(&PERMIT)!=0) printf("Decoding error\n");
    /* printf("Decoded Time Permit= "); OCT_output(&PERMIT); */
#endif

    /* MPin Protocol */

    /* Client enters PIN */
    printf("\nPIN= ");
    if(scanf("%d",&pin)) {};
    /* to avoid silly compile error */
    getchar();

    /* Set date=0 and PERMIT=NULL if time permits not in use

    Client First pass: Inputs CLIENT_ID, optional RNG, pin, TOKEN and PERMIT. Output xID = x.H(CLIENT_ID) and re-combined secret SEC
    If PERMITS are is use, then date!=0 and PERMIT is added to secret and xCID = x.(H(CLIENT_ID)+H(date|H(CLIENT_ID)))
    Random value x is supplied externally if RNG=NULL, otherwise generated and passed out by RNG

    HSID - hashed client ID as calculated by the server
    HCID - hashed client ID as calculated by the client

    IMPORTANT: To save space and time..
    If Time Permits OFF set xCID = NULL, HTID=NULL and use xID and HID only
    If Time permits are ON, AND pin error detection is required then all of xID, xCID, HID and HTID are required
    If Time permits are ON, AND pin error detection is NOT required, set xID=NULL, HID=NULL and use xCID and HTID only.

    */

    pxID=&xID;
    pxCID=&xCID;
    pHID=&HID;
    pHTID=&HTID;
    pE=&E;
    pF=&F;
    pPERMIT=&PERMIT;

#ifdef PERMITS
    prHID=pHTID;
#ifndef PINERROR
    pxID=NULL;
//   pHID=NULL;  //new
#endif
#else
    prHID=pHID;
    pPERMIT=NULL;
    pxCID=NULL;
    pHTID=NULL;
#endif
#ifndef PINERROR
    pE=NULL;
    pF=NULL;
#endif

    /* When set only send hashed IDs to server */
    octet *pID;
#ifdef USE_ANONYMOUS
    pID = &HCID;
#else
    pID = &CLIENT_ID;
#endif

#ifdef SINGLE_PASS
    int timeValue;
    printf("MPIN Single Pass\n");
    timeValue = MPIN_GET_TIME();

    rtn=MPIN_CLIENT(HASH_TYPE_MPIN,date,&CLIENT_ID,RNG,&X,pin,&TOKEN,&SEC,pxID,pxCID,pPERMIT,NULL,timeValue,&Y);

    if (rtn != 0)
    {
        printf("MPIN_CLIENT ERROR %d\n", rtn);
        return 1;
    }

#ifdef FULL
    MPIN_GET_G1_MULTIPLE(RNG,1,&R,&HCID,&Z);  /* Also Send Z=r.ID to Server, remember random r */
#endif


    rtn=MPIN_SERVER(HASH_TYPE_MPIN,date,pHID,pHTID,&Y,&SST,pxID,pxCID,&SEC,pE,pF,pID,NULL,timeValue);

#ifdef FULL
    MPIN_HASH_ID(HASH_TYPE_MPIN,&CLIENT_ID,&HSID);  // new
    MPIN_GET_G1_MULTIPLE(RNG,0,&W,prHID,&T);  /* Also send T=w.ID to client, remember random w  */
#endif

#else // SINGLE_PASS
    printf("MPIN Multi Pass\n");
    if (MPIN_CLIENT_1(HASH_TYPE_MPIN,date,&CLIENT_ID,RNG,&X,pin,&TOKEN,&SEC,pxID,pxCID,pPERMIT)!=0)
    {
        printf("Error from Client side - First Pass\n");
        return 0;
    }

    /* Send U=x.ID to server, and recreate secret from token and pin */

#ifdef FULL
    MPIN_HASH_ID(HASH_TYPE_MPIN,&CLIENT_ID,&HCID);
    MPIN_GET_G1_MULTIPLE(RNG,1,&R,&HCID,&Z);  /* Also Send Z=r.ID to Server, remember random r, DH component */
#endif

    /* Server calculates H(ID) and H(ID)+H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
    MPIN_SERVER_1(HASH_TYPE_MPIN,date,pID,pHID,pHTID);

    /* Server generates Random number Y and sends it to Client */
    MPIN_RANDOM_GENERATE(RNG,&Y);

#ifdef FULL
    MPIN_HASH_ID(HASH_TYPE_MPIN,&CLIENT_ID,&HSID); //new
    MPIN_GET_G1_MULTIPLE(RNG,0,&W,prHID,&T);  /* Also send T=w.ID to client, remember random w, DH component  */
#endif

    /* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
    if (MPIN_CLIENT_2(&X,&Y,&SEC)!=0)
    {
        printf("Error from Client side - Second Pass\n");
        return 1;
    }

    /* Server Second phase. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
    /* If PIN error not required, set E and F = NULL */
    rtn=MPIN_SERVER_2(date,pHID,pHTID,&Y,&SST,pxID,pxCID,&SEC,pE,pF);
#endif // SINGLE_PASS

    if (rtn!=0)
    {
        printf("Server says - Bad Pin.\n");
#ifdef PINERROR

        err=MPIN_KANGAROO(&E,&F);
        if (err) printf("(Client PIN is out by %d)\n",err);

#endif
        return 1;
    }
    else
    {
        printf("Server says - PIN is good! You really are ");
        OCT_output_string(&CLIENT_ID);
        printf(".\n");
    }

#ifdef FULL
    MPIN_HASH_ALL(HASH_TYPE_MPIN,&HCID,pxID,pxCID,&SEC,&Y,&Z,&T,&H);  // new
    MPIN_CLIENT_KEY(HASH_TYPE_MPIN,&G1,&G2,pin,&R,&X,&H,&T,&CK);      // new H
    printf("Client Key = ");
    OCT_output(&CK);

    MPIN_HASH_ALL(HASH_TYPE_MPIN,&HSID,pxID,pxCID,&SEC,&Y,&Z,&T,&H);
    MPIN_SERVER_KEY(HASH_TYPE_MPIN,&Z,&SST,&W,&H,pHID,pxID,pxCID,&SK); // new H,pHID
    printf("Server Key = ");
    OCT_output(&SK);
#endif
    return 0;
}

int main()
{
  int i;
    unsigned long ran;

	char raw[100];
    octet RAW= {0,sizeof(raw),raw};
    csprng RNG;                /* Crypto Strong RNG */

    time((time_t *)&ran);

    RAW.len=100;				/* fake random seed source */
    RAW.val[0]=ran;
    RAW.val[1]=ran>>8;
    RAW.val[2]=ran>>16;
    RAW.val[3]=ran>>24;
    for (i=0; i<100; i++) RAW.val[i]=i+1;

    CREATE_CSPRNG(&RNG,&RAW);   /* initialise strong RNG */

	mpin(&RNG);

	KILL_CSPRNG(&RNG);
}

