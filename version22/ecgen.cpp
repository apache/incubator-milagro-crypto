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

/* ECGEN - Helper MIRACL program to generate constants for EC curves 

(MINGW build)

g++ -O3 ecgen.cpp ecn.cpp big.cpp miracl.a -o ecgen.exe


*/

#include <iostream>
#include "big.h"
#include "zzn2.h"
#include "ecn2.h"

using namespace std;

Miracl precision(20,0);

Big output(int chunk,int w,Big t,Big m)
{
	Big last,y=t;

	cout << "{";
	for (int i=0;i<w;i++)
	{
		last=y%m;
		cout << "0x" << last;
		y/=m;
		if (i==w-1) break;
		if (chunk==64) cout << "L,";
		else cout << ",";
	}

	if (chunk==64) cout << "L}";
	else cout << "}";
	return last;
}

#define NOT_SPECIAL 0
#define PSEUDO_MERSENNE 1
#define GENERALISED_MERSENNE 2
#define MONTGOMERY_FRIENDLY 3

#define WEIERSTRASS 0
#define EDWARDS 1
#define MONTGOMERY 2

/*** Set Modulus and Curve Type Here ***/ 

/* Fill in this bit.... */

#define CHUNK 64   /* processor word size */
#define MBITS 336  /* Modulus size in bits */

/* This next from output of check.cpp program */
#define BASEBITS 60

#define WORDS (1+((MBITS-1)/BASEBITS))
#define MODTYPE  PSEUDO_MERSENNE
#define CURVETYPE EDWARDS
#define CURVE_A 1  // like A parameter in CURVE: y^2=x^3+Ax+B

/* .....to here */


int main()
{
	miracl *mip=&precision;
	Big p,q,R,B;
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,D,C,MC;
	int i,A;


/* Fill in this bit... */

	p=pow((Big)2,MBITS)-3;   // Modulus
	mip->IOBASE=16;
	r=(char *)"200000000000000000000000000000000000000000071415FA9850C0BD6B87F93BAA7B2F95973E9FA805"; // Group Order
	B=11111;    // B parameter of elliptic curve
	gx=(char *)"C";  // generator point
	gy=(char *)"C0DC616B56502E18E1C161D007853D1B14B46C3811C7EF435B6DB5D5650CA0365DB12BEC68505FE8632";

/* .....to here */
	
	cout << "MOD8 = " << p%8 << endl;

	m=pow((Big)2,BASEBITS);

	cout << "Modulus="; MC=output(CHUNK,WORDS,p,m); cout << ";" << endl;

#if MODTYPE==NOT_SPECIAL
		cout << "MConst=0x" << inverse(m-p%m,m) << ";" << endl;	
#endif
#if MODTYPE==MONTGOMERY_FRIENDLY
		cout << "MConst=0x" << MC+1 << ";" << endl;	
#endif
#if MODTYPE==PSEUDO_MERSENNE
		cout << "MConst=0x" << pow((Big)2,MBITS)-p << ";" << endl;			
#endif

	cout << "Order="; output(CHUNK,WORDS,r,m); cout << ";" << endl;
	cout << "CURVE_B="; output(CHUNK,WORDS,B,m); cout << ";" <<  endl;
	cout << "CURVE_Gx="; output(CHUNK,WORDS,gx,m); cout << ";" << endl;
	cout << "CURVE_Gy="; output(CHUNK,WORDS,gy,m); cout << ";" << endl;

}
