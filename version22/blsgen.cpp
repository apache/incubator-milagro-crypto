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

/* BLSGEN - Helper MIRACL program to generate constants for BlS curves 

(MINGW build)

g++ -O3 blsgen.cpp big.cpp zzn.cpp ecn.cpp zzn2.cpp ecn2.cpp miracl.a -o blsgen.exe

This ONLY works for D-type curves of the form y^2=x^3+1, with a positive x parameter

*/

#include <iostream>
#include "big.h"
#include "ecn.h"
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
#define MBITS 455  /* Modulus size in bits */

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
	Big p,q,R,Beta;
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,B,xa,xb,ya,yb,cof;
	Big np,PP,TT,FF;
	ZZn cru;
	ZZn2 X;
	ECn P;
	ECn2 Q;
	ZZn2 Xa,Ya;
	int i,j;

	mip->IOBASE=16;

/* Set BLS value x which determines curve  */

	x= (char *)"10002000002000010007";   
	B=1;
	x= (char *)"10000000000004100100";
	B=7;
	x= (char *)"10000020000080000800";
	B=10;
/* ... to here */

	p=(pow(x,6)-2*pow(x,5)+2*pow(x,3)+x+1)/3;
    ecurve((Big)0,B,p,MR_AFFINE);
    mip->TWIST=MR_SEXTIC_D;
	t=x+1;
    q=pow(x,4)-x*x+1;
	cof=(p+1-t)/q;

//	cout << "cof= " << (p+1-t)/q << endl;

	gx=-1; gy=3;
	if (!P.set(gx,gy))
	{
		cout << "Failed - try another x " << endl;
		return 0;
	}

//	while (!P.set(gx) || (cof*P).iszero()) gx=gx+1;

	P*=cof;
	P.get(gx,gy);

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

	modulo(p);

	cru=pow((ZZn)2,(p-1)/3);
	cru*=cru;   // right cube root of unity

	cout << "CURVE_Cru="; output(CHUNK,WORDS,(Big)cru,m); cout << ";" << endl;

	set_frobenius_constant(X);
	X.get(a,b);
	cout << "CURVE_Fra="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Frb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;

	while (!Q.set(randn2())) ;

	TT=t*t-2*p;
	PP=p*p;
	FF=sqrt((4*PP-TT*TT)/3);
	np=PP+1-(-3*FF+TT)/2;  // 2 possibilities...

	Q=(np/q)*Q;

	Q.get(Xa,Ya);
	Xa.get(a,b);
	cout << "CURVE_Pxa="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Pxb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;
	Ya.get(a,b);
	cout << "CURVE_Pya="; output(CHUNK,WORDS,a,m); cout << ";" << endl;
	cout << "CURVE_Pyb="; output(CHUNK,WORDS,b,m); cout << ";" << endl;

	Q*=q;
	if (!Q.iszero())
	{
		cout << "**** Failed ****" << endl;
		cout << "\nQ= " << Q << endl << endl;
	}
}
