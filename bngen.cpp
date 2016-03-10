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


/* set d=0 for c, d=1 to include leading zeros, d=2 for JS-type square brackets, d=3 for L (for long) appended */
void output(int d,int w,Big t,Big m)
{
	Big y=t;

	if (d!=2) cout << "{";
	else cout << "[";
	for (int i=0;i<w;i++)
	{
		cout << "0x" << y%m;
		y/=m;
		if ((y==0 && (d==0 || d==2)) || i==w-1) break;
		if (d==3) cout << "L,";
		else cout << ",";
	}
	if (d!=2)
	{
		if (d==3) cout << "L}";
		else cout << "}";
	}
	else cout << "]";
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

int main()
{
	miracl *mip=&precision;
	Big p,q,R,cru;
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,D,xa,xb,ya,yb;
	ZZn2 X;
	ECn2 Q;
	ZZn2 Xa,Ya;
	int i;
	int CHUNK[4]={16,32,64,32};
	int WORDS[4]={20,9,5,11};//{20,9,5};
	int BITS[4]={13,29,56,24};//{13,29,56};

	mip->IOBASE=16;

/* Set BN value x which determines curve - note that x is assumed to be negative */

//  x=(char *)"6000000000101041";    // for full 256-bit GT_STRONG parameter
//	x=(char *)"4080000000000001";    // Fast but not not GT_STRONG parameter

	x=(char *)"4000020100608205"; // G2 and GT-Strong parameter
	x=(char *)"4000000003C012B1";    // MIRACL's GT_STRONG parameter
	x=(char *)"4080000000000001";    // Nogami's fast parameter
	x=(char *)"4000806000004081";    // Best GT_STRONG parameter

	p=36*pow(x,4)-36*pow(x,3)+24*x*x-6*x+1;
    ecurve((Big)0,(Big)2,p,MR_AFFINE);
    mip->TWIST=MR_SEXTIC_D;

	cout << "/* AMCL - ROM  C file for BN curve - Weierstrass Only */" << endl << endl;

	cout << "#define MBITS " << bits(p) << endl;
	cout << "#define MOD8 " << p%8 << endl;
	cout << endl;
	cout << "const int CURVE_A=0;" << endl;
	for (i=0;i<3;i++)
	{
		cout << "#if CHUNK==" << CHUNK[i] << endl << endl;
		m=pow((Big)2,BITS[i]);


		cout << "const chunk MConst=0x" << inverse(m-p%m,m) << ";" << endl;

		cout << "const BIG Modulus="; output(0,WORDS[i],p,m); cout << ";" << endl;
		t=6*x*x+1;
		q=p+1-t;
		cout << "const BIG CURVE_Order="; output(0,WORDS[i],q,m); cout << ";" << endl;
		cout << "const BIG CURVE_B={0x2};" << endl;

		R=pow((Big)2,WORDS[i]*BITS[i]);
	//	cout << "const BIG Monty=";output(0,WORDS[i],inverse(R,p),m); cout << ";" << endl;

		cout << "const BIG CURVE_Bnx="; output(0,WORDS[i],x,m); cout << ";" << endl;

		cru=(18*pow(x,3)-18*x*x+9*x-2);
		cout << "const BIG CURVE_Cru="; output(0,WORDS[i],cru,m); cout << ";" << endl;

		set_frobenius_constant(X);

		X.get(a,b);
		cout << "const BIG CURVE_Fra="; output(0,WORDS[i],a,m); cout << ";" << endl;
		cout << "const BIG CURVE_Frb="; output(0,WORDS[i],b,m); cout << ";" << endl;

		Xa.set((ZZn)0,(ZZn)-1);
		Ya.set((ZZn)1,ZZn(0));
		Q.set(Xa,Ya);

		cofactor(Q,X,x);

		Q.get(Xa,Ya);
		Xa.get(a,b);
		cout << "const BIG CURVE_Pxa="; output(0,WORDS[i],a,m); cout << ";" << endl;
		cout << "const BIG CURVE_Pxb="; output(0,WORDS[i],b,m); cout << ";" << endl;
		Ya.get(a,b);
		cout << "const BIG CURVE_Pya="; output(0,WORDS[i],a,m); cout << ";" << endl;
		cout << "const BIG CURVE_Pyb="; output(0,WORDS[i],b,m); cout << ";" << endl;

//		Q*=q;
//		cout << "Q= " << Q << endl;

		cout << "const BIG CURVE_Gx="; output(0,WORDS[i],p-1,m); cout << ";" << endl;
		cout << "const BIG CURVE_Gy="; output(0,WORDS[i],(Big)1,m); cout << ";" << endl;

		cout << "const BIG CURVE_W[2]={"; output(0,WORDS[i],6*x*x-4*x+1,m);cout << ","; output(0,WORDS[i],(2*x-1),m); cout << "};" << endl;
		cout << "const BIG CURVE_SB[2][2]={"; cout << "{"; output(0,WORDS[i],6*x*x-2*x,m); cout << ","; output(0,WORDS[i],(2*x-1),m); cout << "}";cout << ","; cout << "{"; output(0,WORDS[i],(2*x-1),m); cout << ","; output(0,WORDS[i],q-(6*x*x-4*x+1),m); cout << "}"; cout << "};" << endl;

		cout << "const BIG CURVE_WB[4]={"; output(0,WORDS[i],2*x*x-3*x+1,m); cout << ","; output(0,WORDS[i],12*x*x*x-8*x*x+x,m);
		cout << ","; output(0,WORDS[i],6*x*x*x-4*x*x+x,m); cout << ","; output(0,WORDS[i],2*x*x-x,m); cout << "};" << endl;

		cout << "const BIG CURVE_BB[4][4]={";
		cout << "{";
		output(0,WORDS[i],q-x+1,m);
		cout << ","; output(0,WORDS[i],q-x,m);
		cout << ","; output(0,WORDS[i],q-x,m);
		cout << ","; output(0,WORDS[i],2*x,m);
		cout << "}";

		cout << ","; cout << "{";output(0,WORDS[i],2*x-1,m);
		cout << ","; output(0,WORDS[i],q-x,m);
		cout << ","; output(0,WORDS[i],q-x+1,m);
		cout << ","; output(0,WORDS[i],q-x,m);
		cout << "}";
		cout << ","; cout << "{"; output(0,WORDS[i],2*x,m);
		cout << ","; output(0,WORDS[i],2*x-1,m);
		cout << ","; output(0,WORDS[i],2*x-1,m);
		cout << ","; output(0,WORDS[i],2*x-1,m);
		cout << "}";

		cout << ","; cout << "{"; output(0,WORDS[i],x+1,m);
		cout << ","; output(0,WORDS[i],4*x-2,m);
		cout << ","; output(0,WORDS[i],q-2*x-1,m);
		cout << ","; output(0,WORDS[i],x+1,m);
		cout << "}";
		cout << "};" << endl;

		cout << "#endif" << endl << endl;

	}

	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Java file for 32-bit VM and BN curve - Weierstrass Only */" << endl << endl;

	cout << "public static final int MODBITS= " << bits(p) << ";" << endl;
	cout << "public static final int MOD8= " << p%8 << ";" << endl;
	cout << endl;
	cout << "public static final int MODTYPE=NOT_SPECIAL;" << endl;
	m=pow((Big)2,BITS[1]);


	cout << "public static final int[] Modulus= "; output(1,WORDS[1],p,m); cout << ";" << endl;
	R=pow((Big)2,WORDS[1]*BITS[1]);
//	cout << "public static final int[] Monty=";output(1,WORDS[1],inverse(R,p),m); cout << ";" << endl;
	cout << "public static final int MConst=0x" << inverse(m-p%m,m) << ";" <<  endl;
	cout << endl;
	cout << "public static final int CURVETYPE=WEIERSTRASS;" << endl;


	cout << "public static final int CURVE_A = 0;" << endl;
	cout << "public static final int[] CURVE_B = {0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};" << endl;
	t=6*x*x+1;
	q=p+1-t;
	cout << "public static final int[] CURVE_Order="; output(1,WORDS[1],q,m); cout << ";" << endl;


	cout << "public static final int[] CURVE_Bnx="; output(1,WORDS[1],x,m); cout << ";" << endl;

	cru=(18*pow(x,3)-18*x*x+9*x-2);
	cout << "public static final int[] CURVE_Cru="; output(1,WORDS[1],cru,m); cout << ";" << endl;

	set_frobenius_constant(X);

	X.get(a,b);
	cout << "public static final int[] CURVE_Fra="; output(1,WORDS[1],a,m); cout << ";" << endl;
	cout << "public static final int[] CURVE_Frb="; output(1,WORDS[1],b,m); cout << ";" << endl;

	Xa.set((ZZn)0,(ZZn)-1);
	Ya.set((ZZn)1,ZZn(0));
	Q.set(Xa,Ya);

	cofactor(Q,X,x);

	Q.get(Xa,Ya);
	Xa.get(a,b);
	cout << "public static final int[] CURVE_Pxa="; output(1,WORDS[1],a,m); cout << ";" << endl;
	cout << "public static final int[] CURVE_Pxb="; output(1,WORDS[1],b,m); cout << ";" << endl;
	Ya.get(a,b);
	cout << "public static final int[] CURVE_Pya="; output(1,WORDS[1],a,m); cout << ";" << endl;
	cout << "public static final int[] CURVE_Pyb="; output(1,WORDS[1],b,m); cout << ";" << endl;

//		Q*=q;
//		cout << "Q= " << Q << endl;

	cout << "public static final int[] CURVE_Gx ="; output(1,WORDS[1],p-1,m); cout << ";" << endl;
	cout << "public static final int[] CURVE_Gy ="; output(1,WORDS[1],(Big)1,m); cout << ";" << endl;

	cout << "public static final int[][] CURVE_W={"; output(1,WORDS[1],6*x*x-4*x+1,m);cout << ","; output(1,WORDS[1],(2*x-1),m); cout << "};" << endl;
	cout << "public static final int[][][] CURVE_SB={"; cout << "{"; output(1,WORDS[1],6*x*x-2*x,m); cout << ","; output(1,WORDS[1],(2*x-1),m); cout << "}";cout << ","; cout << "{"; output(1,WORDS[1],(2*x-1),m); cout << ","; output(1,WORDS[1],q-(6*x*x-4*x+1),m); cout << "}"; cout << "};" << endl;

	cout << "public static final int[][] CURVE_WB={"; output(1,WORDS[1],2*x*x-3*x+1,m); cout << ","; output(1,WORDS[1],12*x*x*x-8*x*x+x,m);
	cout << ","; output(1,WORDS[1],6*x*x*x-4*x*x+x,m); cout << ","; output(1,WORDS[1],2*x*x-x,m); cout << "};" << endl;

	cout << "public static final int[][][] CURVE_BB={";
	cout << "{";
	output(1,WORDS[1],q-x+1,m);
	cout << ","; output(1,WORDS[1],q-x,m);
	cout << ","; output(1,WORDS[1],q-x,m);
	cout << ","; output(1,WORDS[1],2*x,m);
	cout << "}";

	cout << ","; cout << "{";output(1,WORDS[1],2*x-1,m);
	cout << ","; output(1,WORDS[1],q-x,m);
	cout << ","; output(1,WORDS[1],q-x+1,m);
	cout << ","; output(1,WORDS[1],q-x,m);
	cout << "}";
	cout << ","; cout << "{"; output(1,WORDS[1],2*x,m);
	cout << ","; output(1,WORDS[1],2*x-1,m);
	cout << ","; output(1,WORDS[1],2*x-1,m);
	cout << ","; output(1,WORDS[1],2*x-1,m);
	cout << "}";

	cout << ","; cout << "{"; output(1,WORDS[1],x+1,m);
	cout << ","; output(1,WORDS[1],4*x-2,m);
	cout << ","; output(1,WORDS[1],q-2*x-1,m);
	cout << ","; output(1,WORDS[1],x+1,m);
	cout << "}";
	cout << "};" << endl;


	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Java file for 64-bit VM and BN curve - Weierstrass Only */" << endl << endl;

	cout << "public static final int MODBITS= " << bits(p) << ";" << endl;
	cout << "public static final int MOD8= " << p%8 << ";" << endl;
	cout << endl;
	cout << "public static final int MODTYPE=NOT_SPECIAL;" << endl;
	m=pow((Big)2,BITS[2]);


	cout << "public static final long[] Modulus= "; output(3,WORDS[2],p,m); cout << ";" << endl;
	R=pow((Big)2,WORDS[2]*BITS[2]);
//	cout << "public static final long[] Monty=";output(3,WORDS[2],inverse(R,p),m); cout << ";" << endl;
	cout << "public static final long MConst=0x" << inverse(m-p%m,m) << "L;" <<  endl;
	cout << endl;
	cout << "public static final int CURVETYPE=WEIERSTRASS;" << endl;


	cout << "public static final int CURVE_A = 0;" << endl;
	cout << "public static final long[] CURVE_B = {0x2L,0x0L,0x0L,0x0L,0x0L};" << endl;
	t=6*x*x+1;
	q=p+1-t;
	cout << "public static final long[] CURVE_Order="; output(3,WORDS[2],q,m); cout << ";" << endl;


	cout << "public static final long[] CURVE_Bnx="; output(3,WORDS[2],x,m); cout << ";" << endl;

	cru=(18*pow(x,3)-18*x*x+9*x-2);
	cout << "public static final long[] CURVE_Cru="; output(3,WORDS[2],cru,m); cout << ";" << endl;

	set_frobenius_constant(X);

	X.get(a,b);
	cout << "public static final long[] CURVE_Fra="; output(3,WORDS[2],a,m); cout << ";" << endl;
	cout << "public static final long[] CURVE_Frb="; output(3,WORDS[2],b,m); cout << ";" << endl;

	Xa.set((ZZn)0,(ZZn)-1);
	Ya.set((ZZn)1,ZZn(0));
	Q.set(Xa,Ya);

	cofactor(Q,X,x);

	Q.get(Xa,Ya);
	Xa.get(a,b);
	cout << "public static final long[] CURVE_Pxa="; output(3,WORDS[2],a,m); cout << ";" << endl;
	cout << "public static final long[] CURVE_Pxb="; output(3,WORDS[2],b,m); cout << ";" << endl;
	Ya.get(a,b);
	cout << "public static final long[] CURVE_Pya="; output(3,WORDS[2],a,m); cout << ";" << endl;
	cout << "public static final long[] CURVE_Pyb="; output(3,WORDS[2],b,m); cout << ";" << endl;

//		Q*=q;
//		cout << "Q= " << Q << endl;

	cout << "public static final long[] CURVE_Gx ="; output(3,WORDS[2],p-1,m); cout << ";" << endl;
	cout << "public static final long[] CURVE_Gy ="; output(3,WORDS[2],(Big)1,m); cout << ";" << endl;

	cout << "public static final long[][] CURVE_W={"; output(3,WORDS[2],6*x*x-4*x+1,m);cout << ","; output(3,WORDS[2],(2*x-1),m); cout << "};" << endl;
	cout << "public static final long[][][] CURVE_SB={"; cout << "{"; output(3,WORDS[2],6*x*x-2*x,m); cout << ","; output(3,WORDS[2],(2*x-1),m); cout << "}";cout << ","; cout << "{"; output(3,WORDS[2],(2*x-1),m); cout << ","; output(3,WORDS[2],q-(6*x*x-4*x+1),m); cout << "}"; cout << "};" << endl;

	cout << "public static final long[][] CURVE_WB={"; output(3,WORDS[2],2*x*x-3*x+1,m); cout << ","; output(3,WORDS[2],12*x*x*x-8*x*x+x,m);
	cout << ","; output(3,WORDS[2],6*x*x*x-4*x*x+x,m); cout << ","; output(3,WORDS[2],2*x*x-x,m); cout << "};" << endl;

	cout << "public static final long[][][] CURVE_BB={";
	cout << "{";
	output(3,WORDS[2],q-x+1,m);
	cout << ","; output(3,WORDS[2],q-x,m);
	cout << ","; output(3,WORDS[2],q-x,m);
	cout << ","; output(3,WORDS[2],2*x,m);
	cout << "}";

	cout << ","; cout << "{";output(3,WORDS[2],2*x-1,m);
	cout << ","; output(3,WORDS[2],q-x,m);
	cout << ","; output(3,WORDS[2],q-x+1,m);
	cout << ","; output(3,WORDS[2],q-x,m);
	cout << "}";
	cout << ","; cout << "{"; output(3,WORDS[2],2*x,m);
	cout << ","; output(3,WORDS[2],2*x-1,m);
	cout << ","; output(3,WORDS[2],2*x-1,m);
	cout << ","; output(3,WORDS[2],2*x-1,m);
	cout << "}";

	cout << ","; cout << "{"; output(3,WORDS[2],x+1,m);
	cout << ","; output(3,WORDS[2],4*x-2,m);
	cout << ","; output(3,WORDS[2],q-2*x-1,m);
	cout << ","; output(3,WORDS[2],x+1,m);
	cout << "}";
	cout << "};" << endl;



	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Javascript file for BN curve - Weierstrass Only */" << endl << endl;

	cout << "MODBITS: " << bits(p) << "," << endl;
	cout << "MOD8: " << p%8 << "," << endl;
	cout << endl;
	cout << "MODTYPE:0," << endl;
	m=pow((Big)2,BITS[3]);


	cout << "Modulus: "; output(2,WORDS[3],p,m); cout << "," << endl;
	R=pow((Big)2,WORDS[3]*BITS[3]);
//	cout << "Monty:";output(2,WORDS[3],inverse(R,p),m); cout << "," << endl;
	cout << "MConst:0x" << inverse(m-p%m,m) << "," <<  endl;
	cout << endl;
	cout << "CURVETYPE:0," << endl;


	cout << "CURVE_A : 0," << endl;
	cout << "CURVE_B : [0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0]," << endl;
	t=6*x*x+1;
	q=p+1-t;
	cout << "CURVE_Order:"; output(2,WORDS[3],q,m); cout << "," << endl;


	cout << "CURVE_Bnx:"; output(2,WORDS[3],x,m); cout << "," << endl;

	cru=(18*pow(x,3)-18*x*x+9*x-2);
	cout << "CURVE_Cru:"; output(2,WORDS[3],cru,m); cout << "," << endl;

	set_frobenius_constant(X);

	X.get(a,b);
	cout << "CURVE_Fra:"; output(2,WORDS[3],a,m); cout << "," << endl;
	cout << "CURVE_Frb:"; output(2,WORDS[3],b,m); cout << "," << endl;

	Xa.set((ZZn)0,(ZZn)-1);
	Ya.set((ZZn)1,ZZn(0));
	Q.set(Xa,Ya);

	cofactor(Q,X,x);

	Q.get(Xa,Ya);
	Xa.get(a,b);
	cout << "CURVE_Pxa:"; output(2,WORDS[3],a,m); cout << "," << endl;
	cout << "CURVE_Pxb:"; output(2,WORDS[3],b,m); cout << "," << endl;
	Ya.get(a,b);
	cout << "CURVE_Pya:"; output(2,WORDS[3],a,m); cout << "," << endl;
	cout << "CURVE_Pyb:"; output(2,WORDS[3],b,m); cout << "," << endl;

//		Q*=q;
//		cout << "Q= " << Q << endl;

	cout << "CURVE_Gx :"; output(2,WORDS[3],p-1,m); cout << "," << endl;
	cout << "CURVE_Gy :"; output(2,WORDS[3],(Big)1,m); cout << "," << endl;

	cout << "CURVE_W:["; output(2,WORDS[3],6*x*x-4*x+1,m);cout << ","; output(2,WORDS[3],(2*x-1),m); cout << "]," << endl;
	cout << "CURVE_SB:["; cout << "["; output(2,WORDS[3],6*x*x-2*x,m); cout << ","; output(2,WORDS[3],(2*x-1),m); cout << "]";cout << ","; cout << "["; output(2,WORDS[3],(2*x-1),m); cout << ","; output(2,WORDS[3],q-(6*x*x-4*x+1),m); cout << "]"; cout << "]," << endl;

	cout << "CURVE_WB:["; output(2,WORDS[3],2*x*x-3*x+1,m); cout << ","; output(2,WORDS[3],12*x*x*x-8*x*x+x,m);
	cout << ","; output(2,WORDS[3],6*x*x*x-4*x*x+x,m); cout << ","; output(2,WORDS[3],2*x*x-x,m); cout << "]," << endl;

	cout << "CURVE_BB:[";
	cout << "[";
	output(2,WORDS[3],q-x+1,m);
	cout << ","; output(2,WORDS[3],q-x,m);
	cout << ","; output(2,WORDS[3],q-x,m);
	cout << ","; output(2,WORDS[3],2*x,m);
	cout << "]";

	cout << ","; cout << "[";output(2,WORDS[3],2*x-1,m);
	cout << ","; output(2,WORDS[3],q-x,m);
	cout << ","; output(2,WORDS[3],q-x+1,m);
	cout << ","; output(2,WORDS[3],q-x,m);
	cout << "]";
	cout << ","; cout << "["; output(2,WORDS[3],2*x,m);
	cout << ","; output(2,WORDS[3],2*x-1,m);
	cout << ","; output(2,WORDS[3],2*x-1,m);
	cout << ","; output(2,WORDS[3],2*x-1,m);
	cout << "]";

	cout << ","; cout << "["; output(2,WORDS[3],x+1,m);
	cout << ","; output(2,WORDS[3],4*x-2,m);
	cout << ","; output(2,WORDS[3],q-2*x-1,m);
	cout << ","; output(2,WORDS[3],x+1,m);
	cout << "]";
	cout << "]," << endl;
}
