/*
Copyright 2015 CertiVox UK Ltd

This file is part of The CertiVox MIRACL IOT Crypto SDK (MiotCL)

MiotCL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MiotCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MiotCL.  If not, see <http://www.gnu.org/licenses/>.

You can be released from the requirements of the license by purchasing 
a commercial license.
*/

/* BNGEN - Helper MIRACL program to generate constants for BN curve 

(MINGW build)

g++ -O3 bngen.cpp big.cpp zzn.cpp zzn2.cpp ecn2.cpp miracl.a -o bngen.exe

This ONLY works for D-type curves of the form y^2=x^3+2, with a negative x parameter, and x=3 mod 4

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

void q_power_frobenius(ECn2 &A,ZZn2 &F)
{ 
// Fast multiplication of A by q (for Trace-Zero group members only)
    ZZn2 x,y,z,w,r;

    A.get(x,y);

	w=F*F;
	r=F;

	if (get_mip()->TWIST==MR_SEXTIC_M) r=inverse(F);  // could be precalculated
	if (get_mip()->TWIST==MR_SEXTIC_D) r=F;

	w=r*r;
	x=w*conj(x);
	y=r*w*conj(y);

    A.set(x,y);
}

//
// Faster Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez
//

void cofactor(ECn2& S,ZZn2 &F,Big& x)
{
	ECn2 T,K;
	T=S;
	T*=-x;
	T.norm();
	K=(T+T)+T;
	K.norm();
	q_power_frobenius(K,F);
	q_power_frobenius(S,F); q_power_frobenius(S,F); q_power_frobenius(S,F); 
	S+=T; S+=K;
	q_power_frobenius(T,F); q_power_frobenius(T,F);
	S+=T;
	S.norm();
}

void set_frobenius_constant(ZZn2 &X)
{
    Big p=get_modulus();
    switch (get_mip()->pmod8)
    {
    case 5:
         X.set((Big)0,(Big)1); // = (sqrt(-2)^(p-1)/2     
         break;
    case 3:                    // = (1+sqrt(-1))^(p-1)/2                                
         X.set((Big)1,(Big)1);      
         break;
   case 7: 
         X.set((Big)2,(Big)1); // = (2+sqrt(-1))^(p-1)/2
    default: break;
    }
    X=pow(X,(p-1)/6);
}

/* Fill in this bit yourself.... */

#define CHUNK 64   /* processor word size */
#define MBITS 454  /* Modulus size in bits */

/* This next from output of check.cpp program */
#define BASEBITS 60

#define MODTYPE  NOT_SPECIAL
#define CURVETYPE WEIERSTRASS
#define CURVE_A 0  // like A parameter in CURVE: y^2=x^3+Ax+B

/* .....to here */

#define WORDS (1+((MBITS-1)/BASEBITS))

int main()
{
	miracl *mip=&precision;
	Big p,q,R,cru;
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,B,xa,xb,ya,yb,cof;
	ZZn2 X;
	ECn2 Q;
	ZZn2 Xa,Ya;
	int i;

	mip->IOBASE=16;

/* Set BN value x which determines curve - note that x is assumed to be negative */

//  x=(char *)"6000000000101041";    // for full 256-bit GT_STRONG parameter
//	x=(char *)"4080000000000001";    // Fast but not not GT_STRONG parameter

//	x=(char *)"4000020100608205"; // G2 and GT-Strong parameter
//	x=(char *)"4000000003C012B1";    // CertiVox's GT_STRONG parameter
//	x=(char *)"10000000000000000000004000000000000001001";
//	x=(char *)"4000806000004081";    // Best GT_STRONG parameter

/* Fill in this bit yourself... */

//	x=(char *)"4080000000000001";    // Nogami's fast parameter
	x=(char *)"10000010000000000000100000001";
//	x=(char *)"10000000000000000000004000000000000001001";

/* ... to here */

	p=36*pow(x,4)-36*pow(x,3)+24*x*x-6*x+1;
    ecurve((Big)0,(Big)2,p,MR_AFFINE);
    mip->TWIST=MR_SEXTIC_D;
	t=6*x*x+1;
	q=p+1-t;
	cof=1;
	B=2;
	gx=p-1;
	gy=1;

	cout << "MOD8 " << p%8 << endl;

	m=pow((Big)2,BASEBITS);
		
	cout << "MConst=0x" << inverse(m-p%m,m) << ";" << endl;	

	cout << "Modulus="; output(CHUNK,WORDS,p,m); cout << ";" << endl;
	
	cout << "CURVE_Order="; output(CHUNK,WORDS,q,m); cout << ";" << endl;
	cout << "CURVE_Cof="; output(CHUNK,WORDS,cof,m); cout << ";" << endl;
	cout << "CURVE_B= "; output(CHUNK,WORDS,B,m); cout << ";" << endl;
	cout << "CURVE_Gx="; output(CHUNK,WORDS,gx,m); cout << ";" << endl;
	cout << "CURVE_Gy="; output(CHUNK,WORDS,gy,m); cout << ";" << endl;
	cout << endl;
	cout << "CURVE_Bnx="; output(CHUNK,WORDS,x,m); cout << ";" << endl;

	cru=(18*pow(x,3)-18*x*x+9*x-2);
	cout << "CURVE_Cru="; output(CHUNK,WORDS,cru,m); cout << ";" << endl;

	set_frobenius_constant(X);
	X.get(a,b);
	cout << "CURVE_Fra="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Frb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;

	Xa.set((ZZn)0,(ZZn)-1);
	Ya.set((ZZn)1,ZZn(0));
	Q.set(Xa,Ya);

//		cofactor(Q,X,x);

	Q=(p-1+t)*Q;

	Q.get(Xa,Ya);
	Xa.get(a,b);
	cout << "CURVE_Pxa="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Pxb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;
	Ya.get(a,b);
	cout << "CURVE_Pya="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Pyb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;

//		Q*=q;
//		cout << "Q= " << Q << endl;


	cout << "CURVE_W[2]={"; output(CHUNK,WORDS,6*x*x-4*x+1,m);cout << ","; output(CHUNK,WORDS,(2*x-1),m); cout << "};" << endl;
	cout << "CURVE_SB[2][2]={"; cout << "{"; output(CHUNK,WORDS,6*x*x-2*x,m); cout << ","; output(CHUNK,WORDS,(2*x-1),m); cout << "}";cout << ","; cout << "{"; output(CHUNK,WORDS,(2*x-1),m); cout << ","; output(CHUNK,WORDS,q-(6*x*x-4*x+1),m); cout << "}"; cout << "};" << endl;

	cout << "CURVE_WB[4]={"; output(CHUNK,WORDS,2*x*x-3*x+1,m); cout << ","; output(CHUNK,WORDS,12*x*x*x-8*x*x+x,m); 
	cout << ","; output(CHUNK,WORDS,6*x*x*x-4*x*x+x,m); cout << ","; output(CHUNK,WORDS,2*x*x-x,m); cout << "};" << endl;
	
	cout << "CURVE_BB[4][4]={"; 
	cout << "{";
	output(CHUNK,WORDS,q-x+1,m); 
	cout << ","; output(CHUNK,WORDS,q-x,m); 
	cout << ","; output(CHUNK,WORDS,q-x,m); 
	cout << ","; output(CHUNK,WORDS,2*x,m); 
	cout << "}";

	cout << ","; cout << "{";output(CHUNK,WORDS,2*x-1,m); 
	cout << ","; output(CHUNK,WORDS,q-x,m); 
	cout << ","; output(CHUNK,WORDS,q-x+1,m); 
	cout << ","; output(CHUNK,WORDS,q-x,m); 
	cout << "}";
	cout << ","; cout << "{"; output(CHUNK,WORDS,2*x,m); 
	cout << ","; output(CHUNK,WORDS,2*x-1,m); 
	cout << ","; output(CHUNK,WORDS,2*x-1,m); 
	cout << ","; output(CHUNK,WORDS,2*x-1,m); 
	cout << "}";

	cout << ","; cout << "{"; output(CHUNK,WORDS,x+1,m); 
	cout << ","; output(CHUNK,WORDS,4*x-2,m); 
	cout << ","; output(CHUNK,WORDS,q-2*x-1,m); 
	cout << ","; output(CHUNK,WORDS,x+1,m); 
	cout << "}";
	cout << "};" << endl;
}
