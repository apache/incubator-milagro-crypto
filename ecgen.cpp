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

/* BNGEN - Helper MIRACL program to generate constants for EC curves

(MINGW build)

g++ -O3 ecgen.cpp big.cpp miracl.a -o ecgen.exe


*/

#include <iostream>
#include "big.h"
#include "zzn2.h"
#include "ecn2.h"

using namespace std;

Miracl precision(20,0);

Big output(int d,int w,Big t,Big m)
{
	Big last,y=t;

	if (d!=2) cout << "{";
	else cout << "[";
	for (int i=0;i<w;i++)
	{
		last=y%m;
		cout << "0x" << last;
		y/=m;
		if ((y==0 && d==0) || i==w-1) break;
		if (d==3) cout << "L,";
		else cout << ",";
	}
	if (d!=2)
	{
		if (d==3) cout << "L}";
		else cout << "}";
	}
	else cout << "]";
	return last;
}

#define NOT_SPECIAL 0
#define PSEUDO_MERSENNE 1
#define MONTGOMERY_FRIENDLY 3

#define WEIERSTRASS 0
#define EDWARDS 1
#define MONTGOMERY 2

/*** Set Modulus and Curve Type Here ***/

#define MODTYPE  PSEUDO_MERSENNE
#define CURVETYPE EDWARDS

int main()
{
	miracl *mip=&precision;
	Big p,q,R,B;
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,D,C,MC;
	int i,A;
	int CHUNK[4]={16,32,64,32};
	int WORDS[4]={20,9,5,11};//{20,9,5};
	int BITS[4]={13,29,56,24};//{13,29,56};

// ***  Set prime Modulus, curve order, B parameter of curve and generator point
/*
// This is for ED25519
	p=pow((Big)2,255)-19;  // or whatever
	r=pow((Big)2,252)+"27742317777372353535851937790883648493";  // or whatever
	B=p-moddiv((Big)121665,(Big)121666,p);  // or whatever
	gy=moddiv((Big)4,(Big)5,p);   // Set generator point (x,y)
	gx=moddiv((gy*gy-1)%p,(B*gy*gy+1)%p,p);
	gx=p-sqrt(gx,p);
	mip->IOBASE=16;
*/

// This is for NIST256 curve
//	mip->IOBASE=10;
//	p="115792089210356248762697446949407573530086143415290314195533631308867097853951";
//	r="115792089210356248762697446949407573529996955224135760342422259061068512044369";
//	mip->IOBASE=16;
//	B="5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b";
//	gx="6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296";
//	gy="4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5";
/*

// This is for w-254-mont Curve

	p=pow((Big)2,240)*(pow((Big)2,14)-127)-1;
	mip->IOBASE=16;
	r=p+1-"147E7415F25C8A3F905BE63B507207C1";
	B=p-12146;
	gx="2";
	gy="140E3FD33B2E56014AE15A75BD778AEBDFB738E3F8511931AD65DF37F90D4EBC";

// ed-254-mont Curve
	p=pow((Big)2,240)*(pow((Big)2,14)-127)-1;
	mip->IOBASE=16;
	r=(p+1-"51AB3E4DD0A7413C5430B004EE459CE4")/4;
	B=13947;
	gx="1";
	gy="19F0E690D6A4C335951D00D502363F4E36329A840E3212187C52D0FDAF2701E5";

// w-255-mers Curve
	p=pow((Big)2,255)-765;
	mip->IOBASE=16;
	r=p+1-"79B5C7D7C52D4C2054705367C3A6B219";
	B=p-20925;
	gx="1";
	gy="6F7A6AC0EDBA7833921EBFF9B2FF7D177DB6C78CDDFDA60D1733FF6769CB44BA";

// ed-255-mers Curve
	p=pow((Big)2,255)-765;
	mip->IOBASE=16;
	r=(p+1-"8C3961E84965F3454ED8B84BEF244F30")/4;
	B=60055;
	gx="4";
	gy="26CB78534A7BB545EC254CDD8E0C47E552914B8AED445A45BA2A255BD08736A0";

// Brainpool 256-bit curve
	mip->IOBASE=16;
	p="A9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377";
	r="A9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7";
	B="662C61C430D84EA4FE66A7733D0B76B7BF93EBC4AF2F49256AE58101FEE92B04";
	gx="A3E8EB3CC1CFE7B7732213B23A656149AFA142C47AAFBC2B79A191562E1305F4";
	gy="2D996C823439C56D7F7B22E14644417E69BCB6DE39D027001DABE8F35B25C9BE";

// ANSSI (French) curve
	mip->IOBASE=16;
	p="f1fd178c0b3ad58f10126de8ce42435b3961adbcabc8ca6de8fcf353d86e9c03";
	r="F1FD178C0B3AD58F10126DE8CE42435B53DC67E140D2BF941FFDD459C6D655E1";
	B="EE353FCA5428A9300D4ABA754A44C00FDFEC0C9AE4B1A1803075ED967B7BB73F";
	gx="B6B3D4C356C139EB31183D4749D423958C27D2DCAF98B70164C97A2DD98F5CFF";
	gy="6142E0F7C8B204911F9271F0F3ECEF8C2701C307E8E4C9E183115A1554062CFB";
*/
	p=pow((Big)2,255)-765;
	mip->IOBASE=16;
	r=(p+1-"8C3961E84965F3454ED8B84BEF244F30")/4;
	B=60055;
	gx="4";
	gy="26CB78534A7BB545EC254CDD8E0C47E552914B8AED445A45BA2A255BD08736A0";


#if MODTYPE==PSEUDO_MERSENNE
	C=765;              // p=2^n - C, where C is very small
#endif
#if CURVETYPE==WEIERSTRASS
	A=-3;   // or 0
#endif
#if CURVETYPE==EDWARDS
	A=-1;  // or +1
#endif
#if CURVETYPE==MONTGOMERY
	A=-55790;
#endif

	cout << "/* AMCL - ROM  C file for EC curves */" << endl << endl;

	cout << "#define MBITS " << bits(p) << endl;
	cout << "#define MOD8 " << p%8 << endl;
	cout << endl;
	cout << "const int CURVE_A=" << A << ";" << endl;
	for (i=0;i<3;i++)
	{
		cout << "#if CHUNK==" << CHUNK[i] << endl << endl;
		m=pow((Big)2,BITS[i]);

		cout << "const BIG Modulus="; MC=output(0,WORDS[i],p,m); cout << ";" << endl;

#if MODTYPE==NOT_SPECIAL
		cout << "const chunk MConst=0x" << inverse(m-p%m,m) << ";" << endl;
#endif
#if MODTYPE==MONTGOMERY_FRIENDLY
		cout << "const chunk MConst=0x" << MC+1 << ";" << endl;
#endif
#if MODTYPE==PSEUDO_MERSENNE
		cout << "const chunk MConst=0x" << C << ";" << endl;
#endif

#if MODTYPE!=PSEUDO_MERSENNE
		R=pow((Big)2,WORDS[i]*BITS[i]);
//		cout << "const BIG Monty=";output(0,WORDS[i],inverse(R,p),m); cout << ";" << endl;
#endif

		cout << "const BIG CURVE_Order="; output(0,WORDS[i],r,m); cout << ";" << endl;
		cout << "const BIG CURVE_B="; output(0,WORDS[i],B,m); cout << ";" <<  endl;

		cout << "const BIG CURVE_Gx="; output(0,WORDS[i],gx,m); cout << ";" << endl;
		cout << "const BIG CURVE_Gy="; output(0,WORDS[i],gy,m); cout << ";" << endl;

		cout << "#endif" << endl << endl;

	}

	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Java file for 32-bit VM for EC curve */" << endl << endl;

	cout << "public static final int MODBITS= " << bits(p) << ";" << endl;
	cout << "public static final int MOD8= " << p%8 << ";" << endl;
	cout << endl;
	cout << "public static final int MODTYPE= " << MODTYPE << ";" << endl;
	m=pow((Big)2,BITS[1]);


	cout << "public static final int[] Modulus= "; MC=output(1,WORDS[1],p,m); cout << ";" << endl;
	R=pow((Big)2,WORDS[1]*BITS[1]);


#if MODTYPE==NOT_SPECIAL
	cout << "public static final int MConst=0x" << inverse(m-p%m,m) << ";" <<  endl;
#endif
#if MODTYPE==MONTGOMERY_FRIENDLY
	cout << "public static final int MConst=0x" << MC+1 << ";" << endl;
#endif
#if MODTYPE==PSEUDO_MERSENNE
	cout << "public static final int MConst=0x" << C << ";" << endl;
#endif

#if MODTYPE!=PSEUDO_MERSENNE
//	cout << "public static final int[] Monty=";output(1,WORDS[1],inverse(R,p),m); cout << ";" << endl;
#endif

	cout << endl;
	cout << "public static final int CURVETYPE= " << CURVETYPE << ";" << endl;


	cout << "public static final int CURVE_A = " << A << ";" << endl;
	cout << "public static final int[] CURVE_B = "; output(1,WORDS[1],B,m); cout << ";" << endl;

	cout << "public static final int[] CURVE_Order="; output(1,WORDS[1],r,m); cout << ";" << endl;

	cout << "public static final int[] CURVE_Gx ="; output(1,WORDS[1],gx,m); cout << ";" << endl;
	cout << "public static final int[] CURVE_Gy ="; output(1,WORDS[1],gy,m); cout << ";" << endl;



	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Java file for 64-bit VM for EC curve */" << endl << endl;

	cout << "public static final int MODBITS= " << bits(p) << ";" << endl;
	cout << "public static final int MOD8= " << p%8 << ";" << endl;
	cout << endl;
	cout << "public static final int MODTYPE= " << MODTYPE << ";" << endl;
	m=pow((Big)2,BITS[2]);


	cout << "public static final long[] Modulus= "; MC=output(3,WORDS[2],p,m); cout << ";" << endl;
	R=pow((Big)2,WORDS[2]*BITS[2]);


#if MODTYPE==NOT_SPECIAL
	cout << "public static final long MConst=0x" << inverse(m-p%m,m) << "L;" <<  endl;
#endif
#if MODTYPE==MONTGOMERY_FRIENDLY
	cout << "public static final long MConst=0x" << MC+1 << "L;" << endl;
#endif
#if MODTYPE==PSEUDO_MERSENNE
	cout << "public static final long MConst=0x" << C << "L;" << endl;
#endif

#if MODTYPE!=PSEUDO_MERSENNE
//	cout << "public static final long[] Monty=";output(3,WORDS[2],inverse(R,p),m); cout << ";" << endl;
#endif

	cout << endl;
	cout << "public static final int CURVETYPE= " << CURVETYPE << ";" << endl;


	cout << "public static final int CURVE_A = " << A << ";" << endl;
	cout << "public static final long[] CURVE_B = "; output(3,WORDS[2],B,m); cout << ";" << endl;

	cout << "public static final long[] CURVE_Order="; output(3,WORDS[2],r,m); cout << ";" << endl;

	cout << "public static final long[] CURVE_Gx ="; output(3,WORDS[2],gx,m); cout << ";" << endl;
	cout << "public static final long[] CURVE_Gy ="; output(3,WORDS[2],gy,m); cout << ";" << endl;




	cout << endl;
	cout << "Cut here -----------------------------------------------------------" << endl;
	cout << "/* AMCL - ROM  Javascript file for EC curve - Weierstrass Only */" << endl << endl;

	cout << "MODBITS: " << bits(p) << "," << endl;
	cout << "MOD8: " << p%8 << "," << endl;
	cout << endl;
	cout << "MODTYPE:" << MODTYPE << "," << endl;
	m=pow((Big)2,BITS[3]);

	cout << "Modulus: "; MC=output(2,WORDS[3],p,m); cout << "," << endl;

#if MODTYPE==NOT_SPECIAL
	cout << "MConst:0x" << inverse(m-p%m,m) << "," <<  endl;
#endif
#if MODTYPE==MONTGOMERY_FRIENDLY
	cout << "MConst:0x" << MC+1 << "," << endl;
#endif
#if MODTYPE==PSEUDO_MERSENNE
	cout << "MConst:0x" << C << "," << endl;
#endif

	R=pow((Big)2,WORDS[3]*BITS[3]);
#if MODTYPE!=PSEUDO_MERSENNE
//	cout << "Monty:";output(2,WORDS[3],inverse(R,p),m); cout << "," << endl;
#endif

	cout << endl;
	cout << "CURVETYPE:" << CURVETYPE << "," << endl;

	cout << "CURVE_A : " << A << "," << endl;
	cout << "CURVE_B : "; output(2,WORDS[3],B,m); cout << "," << endl;

	cout << "CURVE_Order:"; output(2,WORDS[3],r,m); cout << "," << endl;

	cout << "CURVE_Gx :"; output(2,WORDS[3],gx,m); cout << "," << endl;
	cout << "CURVE_Gy :"; output(2,WORDS[3],gy,m); cout << "," << endl;

}
