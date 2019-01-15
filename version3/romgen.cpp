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

/* ECGEN - Helper MIRACL program to generate ROM constants for curves 

A bit rough-and-ready - the output might need minor syntactical corrections

(MINGW build)

g++ -O3 romgen.cpp big.cpp zzn.cpp ecn.cpp zzn2.cpp ecn2.cpp zzn4.cpp ecn4.cpp zzn8.cpp ecn8.cpp miracl.a -o romgen.exe


*/

#include <iostream>
#include "big.h"
#include "ecn.h"
#include "zzn2.h"
#include "ecn2.h"
#include "ecn4.h"
#include "ecn8.h"

using namespace std;

Miracl precision(100,0);

char open,close,term,el=0;

Big output(int chunk,int w,Big t,Big m)
{
	Big last,y=t;

	cout << open;
	for (int i=0;i<w;i++)
	{
		last=y%m;
		cout << "0x" << last;
		y/=m;
		if (i==w-1) break;
		if (el!=0) cout << el << ",";
		else cout << ",";
	}

	if (el!=0) cout << el << close;
	else cout << close;
	return last;
}

#define NOT_SPECIAL 0
#define PSEUDO_MERSENNE 1
#define GENERALISED_MERSENNE 2
#define MONTGOMERY_FRIENDLY 3

#define WEIERSTRASS 0
#define EDWARDS 1
#define MONTGOMERY 2

// set Frobenius constant - depends on embedding degree
void set_frobenius_constant(ZZn2 &X, int ed)
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
	if (ed==12) X=pow(X,(p-1)/6);
	if (ed==24) X=pow(X,(p-7)/12);
	if (ed==48) X=pow(X,(p-19)/24);
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
	T*=(-x);
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

void help()
{
		printf("Elliptic Curves\n");
		printf("1. ED25519\n");
		printf("2. C25519\n");
		printf("3. NIST256\n");
		printf("4. BRAINPOOL\n");
		printf("5. ANSSI\n");
		printf("6. HIFIVE\n");
		printf("7. GOLDILOCKS\n");
		printf("8. NIST384\n");
		printf("9. C41417\n");
		printf("10. NIST521\n");
		printf("11. NUMS256W\n");
		printf("12. NUMS256E\n");
		printf("13. NUMS384W\n");
		printf("14. NUMS384E\n");
		printf("15. NUMS512W\n");
		printf("16. NUMS512E\n");
		printf("17. SECP256K1\n\n");

		printf("18. BN254\n");
		printf("19. BN254CX\n");
		printf("20. BLS383\n");
		printf("21. BLS381\n");
		printf("22. FP256BN\n");
		printf("23. FP512BN\n");
		printf("24. BLS461\n");

		printf("25. BLS24\n");
		printf("26. BLS48\n");

		printf("\nromgen curve wordlength basebits language\n");
		printf("where wordlength is 16, 32 or 64\n");
		printf("basebits is less than wordlength\n\n");
		printf("Use check utility to determine best choice for basebits\n");
		printf("language is c, cpp, java, javascript, go, rust or swift\n\n");
}

char* toupperit(char *s,int lang)
{
	int i;
	static char t[50];
	if (lang!=5) return s;
	for (i=0;;i++)
	{
		t[i]=s[i];
		if (t[i]==0) break;
		t[i]=toupper(t[i]);
	}

	return t;
}

int main(int argc, char **argv)
{
	miracl *mip=&precision;
	Big p,R,B,mc,curve_b,cru,cof,tau[9];
	Big m,x,y,w,t,c,n,r,a,b,gx,gy,r2modp;
	Big np,PP,TT,FF;
	int i,A,curve,bb,chunk,words,mbits,bytes,ip=0;
	int modtype,curvetype,curve_a,curve_b_i,cof_i,lang=0;
	ZZn2 X;
	ECn P;
	ECn2 Q;
	ECn4 QQ;
	ECn8 Q8;
	ZZn4 XA,YA,AA,BB;
	ZZn8 X8,Y8;
	ZZn2 Aa,Ab,Ba,Bb;
	ZZn2 Xa,Ya;
	ZZn zcru;
	char pre0[50],pre1[50],pre2[50],pre3[50],pre4[50],pre5[50],pre6[50];
	char post0[50],post1[50],post2[50],post3[50],post4[50],post5[50],post6[50];
	char pre7[50],post7[50],lg[50];

	char xxx[20],yyy[20],zzz[20];
	
	char curvename[30],fieldname[30];

	argv++; argc--;

	if (argc<4)
	{
		help();
		return 0;
	}
 
	strcpy(curvename,argv[0]);

//	curve=atoi(argv[ip++]);
	for (i=0;;i++)
	{
		if (curvename[i]==0) break;
		curvename[i]=toupper(curvename[i]);
	}

	//cout << "curvename= " << curvename << " " << strlen(curvename) << endl;

	curve=0; ip++;
	chunk=atoi(argv[ip++]);
	bb=atoi(argv[ip++]);

	strcpy(lg,argv[ip]);

	if (chunk !=16 && chunk!=32 && chunk!=64) {help(); return 0;}
	if (bb<0 || bb>=chunk) {help(); return 0;}

// Specify curve constants

	if (strcmp(curvename,"ED25519")==0)
	{ // ED25519
		curve=1;
		printf("Curve= ED25519\n");
		strcpy(fieldname,"25519");
		mbits=255;                 // bits in modulus
		words=(1+((mbits-1)/bb));  // words per Big
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=-1;			     // Curve A parameter
		cof=8;
		p=pow((Big)2,mbits)-19;  // Modulus
		r=pow((Big)2,252)+(char *)"27742317777372353535851937790883648493";   // group order
		mip->IOBASE=16;
		curve_b=(char *)"52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3";  // curve B parameter
		gx=(char *)"216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A";       // generator point
		gy=(char *)"6666666666666666666666666666666666666666666666666666666666666658";
	}

	if (strcmp(curvename,"C25519")==0)
	{
		curve=2;
		printf("Curve= C25519\n");
		strcpy(fieldname,"25519");
		mbits=255;
		words=(1+((mbits-1)/bb));
		curvetype=MONTGOMERY;
		modtype=PSEUDO_MERSENNE;
		curve_a=486662;
		cof=8;
		p=pow((Big)2,mbits)-19;
		r=pow((Big)2,252)+(char *)"27742317777372353535851937790883648493";

		curve_b=0;
		mip->IOBASE=16;
		gx=(char *)"9";
		gy=0;
	}

	if (strcmp(curvename,"NIST256")==0)
	{
		curve=3;
		printf("Curve= NIST256\n");
		strcpy(fieldname,curvename);
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=-3;
		cof=1;
		p=(char *)"115792089210356248762697446949407573530086143415290314195533631308867097853951";
		r=(char *)"115792089210356248762697446949407573529996955224135760342422259061068512044369";
		mip->IOBASE=16;
		curve_b=(char *)"5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b";
		gx=(char *)"6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296";
		gy=(char *)"4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5";
	}

	if (strcmp(curvename,"BRAINPOOL")==0)
	{
		curve=4;
		printf("Curve= BRAINPOOL\n");
		strcpy(fieldname,curvename);
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=-3;
		cof=1;
		p=(char *)"76884956397045344220809746629001649093037950200943055203735601445031516197751";
		mip->IOBASE=16;
		r=(char *)"A9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7";
		mip->IOBASE=10;
		curve_b=(char *)"46214326585032579593829631435610129746736367449296220983687490401182983727876";
		mip->IOBASE=16;
		gx=(char *)"a3e8eb3cc1cfe7b7732213b23a656149afa142c47aafbc2b79a191562e1305f4";
		gy=(char *)"2d996c823439c56d7f7b22e14644417e69bcb6de39d027001dabe8f35b25c9be";
	}

	if (strcmp(curvename,"ANSSI")==0)
	{
		curve=5;
		printf("Curve= ANSSI\n");
		strcpy(fieldname,curvename);
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=-3;
		cof=1;
		p=(char *)"109454571331697278617670725030735128145969349647868738157201323556196022393859";
		mip->IOBASE=16;
		r=(char *)"F1FD178C0B3AD58F10126DE8CE42435B53DC67E140D2BF941FFDD459C6D655E1";
		mip->IOBASE=10;
		curve_b=(char *)"107744541122042688792155207242782455150382764043089114141096634497567301547839";
		mip->IOBASE=16;
		gx=(char *)"b6b3d4c356c139eb31183d4749d423958c27d2dcaf98b70164c97a2dd98f5cff";
		gy=(char *)"6142e0f7c8b204911f9271f0f3ecef8c2701c307e8e4c9e183115a1554062cfb";
	}

	if (strcmp(curvename,"HIFIVE")==0)
	{
		curve=6;
		printf("Curve= HIFIVE\n");
		strcpy(fieldname,curvename);
		mbits=336;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=1;
		cof=8;
		p=pow((Big)2,336)-3;
		mip->IOBASE=16;
		r=(char *)"200000000000000000000000000000000000000000071415FA9850C0BD6B87F93BAA7B2F95973E9FA805";
		mip->IOBASE=10;
		curve_b=(char *)"11111";
		mip->IOBASE=16;
		gx=(char *)"C";
		gy=(char *)"C0DC616B56502E18E1C161D007853D1B14B46C3811C7EF435B6DB5D5650CA0365DB12BEC68505FE8632";
	}

	if (strcmp(curvename,"GOLDILOCKS")==0)
	{
		curve=7;
		printf("Curve= GOLDILOCKS\n");
		strcpy(fieldname,curvename);
		mbits=448;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=GENERALISED_MERSENNE;
		curve_a=1;
		cof=4;
		p=pow((Big)2,448)-pow((Big)2,224)-1;
		r=(p+1-(char *)"28312320572429821613362531907042076847709625476988141958474579766324")/4;
		mip->IOBASE=10;
		curve_b=p-39081;
		mip->IOBASE=16;

		gx=(char *)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa955555555555555555555555555555555555555555555555555555555";
		gy=(char *)"ae05e9634ad7048db359d6205086c2b0036ed7a035884dd7b7e36d728ad8c4b80d6565833a2a3098bbbcb2bed1cda06bdaeafbcdea9386ed";
	}

	if (strcmp(curvename,"NIST384")==0)
	{
		curve=8;
		printf("Curve= NIST384\n");
		strcpy(fieldname,curvename);
		mbits=384;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=-3;
		cof=1;
		p=(char *)"39402006196394479212279040100143613805079739270465446667948293404245721771496870329047266088258938001861606973112319";
		r=p+1-(char *)"1388124618062372383606759648309780106643088307173319169677";

		curve_b=(char *)"27580193559959705877849011840389048093056905856361568521428707301988689241309860865136260764883745107765439761230575";
		mip->IOBASE=16;
		gx=(char *)"aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7";
		gy=(char *)"3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f";
	}

	if (strcmp(curvename,"C41417")==0)
	{
		curve=9;
		printf("Curve= C41417\n");
		strcpy(fieldname,curvename);
		mbits=414;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=1;
		p=pow((Big)2,mbits)-17; 
		cof=8;
		r=pow((Big)2,411)-(char *)"33364140863755142520810177694098385178984727200411208589594759"; // Group Order

		curve_b=(char *)"3617";
		mip->IOBASE=16;
		gx=(char *)"1a334905141443300218c0631c326e5fcd46369f44c03ec7f57ff35498a4ab4d6d6ba111301a73faa8537c64c4fd3812f3cbc595";
		gy=(char *)"22";
	}

	if (strcmp(curvename,"NIST521")==0)
	{
		curve=10;
		printf("Curve= NIST521\n");
		strcpy(fieldname,curvename);
		mbits=521;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=-3;
		cof=1;
		p=pow((Big)2,mbits)-1; 

		mip->IOBASE=16;
		r=(char *)"1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386409"; // Group Order
		curve_b=(char *)"51953EB9618E1C9A1F929A21A0B68540EEA2DA725B99B315F3B8B489918EF109E156193951EC7E937B1652C0BD3BB1BF073573DF883D2C34F1EF451FD46B503F00";

		gx=(char *)"C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBAA14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66";
		gy=(char *)"11839296A789A3BC0045C8A5FB42C7D1BD998F54449579B446817AFBD17273E662C97EE72995EF42640C550B9013FAD0761353C7086A272C24088BE94769FD16650";
	}
	
	if (strcmp(curvename,"NUMS256W")==0)
	{
		curve=11;
		printf("Curve= NUMS256W\n");
		strcpy(fieldname,"256PMW");
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=PSEUDO_MERSENNE;
		curve_a=-3;
		cof=1;
		p=pow((Big)2,mbits)-189; 

		mip->IOBASE=16;
		r=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE43C8275EA265C6020AB20294751A825"; // Group Order
		curve_b=(char *)"25581";
		gx=(char *)"BC9ED6B65AAADB61297A95A04F42CB0983579B0903D4C73ABC52EE1EB21AACB1";
		gy=(char *)"D08FC0F13399B6A673448BF77E04E035C955C3D115310FBB80B5B9CB2184DE9F";
	}

	if (strcmp(curvename,"NUMS256E")==0)
	{
		curve=12;
		printf("Curve= NUMS256E\n");
		strcpy(fieldname,"256PME");
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=1;
		cof=4;
		p=pow((Big)2,mbits)-189; 

		mip->IOBASE=16;
		r=(char *)"4000000000000000000000000000000041955AA52F59439B1A47B190EEDD4AF5"; // Group Order
		curve_b=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC355";
		gx=(char *)"8A7514FB6AEA237DCD1E3D5F69209BD60C398A0EE3083586A0DEC0902EED13DA";
		gy=(char *)"44D53E9FD9D925C7CE9665D9A64B8010715F61D810856ED32FA616E7798A89E6";
	}


	if (strcmp(curvename,"NUMS384W")==0)
	{
		curve=13;
		printf("Curve= NUMS384W\n");
		strcpy(fieldname,"384PM");
		mbits=384;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=PSEUDO_MERSENNE;
		curve_a=-3;
		cof=1;
		p=pow((Big)2,mbits)-317; 

		mip->IOBASE=16;
		r=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFD61EAF1EEB5D6881BEDA9D3D4C37E27A604D81F67B0E61B9"; // Group Order
		curve_b=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF77BB";
		gx=(char *)"757956F0B16F181C4880CA224105F1A60225C1CDFB81F9F4F3BD291B2A6CC742522EED100F61C47BEB9CBA042098152A";
		gy=(char *)"ACDEE368E19B8E38D7E33D300584CF7EB0046977F87F739CB920837D121A837EBCD6B4DBBFF4AD265C74B8EC66180716";
	}

	if (strcmp(curvename,"NUMS384E")==0)
	{
		curve=14;
		printf("Curve= NUMS384E\n");
		strcpy(fieldname,"384PM");
		mbits=384;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=1;
		cof=4;
		p=pow((Big)2,mbits)-317; 

		mip->IOBASE=16;
		r=(char *)"3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE2471A1CB46BE1CF61E4555AAB35C87920B9DCC4E6A3897D"; // Group Order
		curve_b=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFD19F";
		gx=(char *)"61B111FB45A9266CC0B6A2129AE55DB5B30BF446E5BE4C005763FFA8F33163406FF292B16545941350D540E46C206BDE";
		gy=(char *)"82983E67B9A6EEB08738B1A423B10DD716AD8274F1425F56830F98F7F645964B0072B0F946EC48DC9D8D03E1F0729392";
	}

	if (strcmp(curvename,"NUMS512W")==0)
	{
		curve=15;
		printf("Curve= NUMS512W\n");
		strcpy(fieldname,"512PM");
		mbits=512;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=PSEUDO_MERSENNE;
		curve_a=-3;
		cof=1;
		p=pow((Big)2,mbits)-569; 

		mip->IOBASE=16;
		r=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5B3CA4FB94E7831B4FC258ED97D0BDC63B568B36607CD243CE153F390433555D"; // Group Order
		curve_b=(char *)"1D99B";
		gx=(char *)"3AC03447141D0A93DA2B7002A03D3B5298CAD83BB501F6854506E0C25306D9F95021A151076B359E93794286255615831D5D60137D6F5DE2DC8287958CABAE57";
		gy=(char *)"943A54CA29AD56B3CE0EEEDC63EBB1004B97DBDEABBCBB8C8F4B260C7BD14F14A28415DA8B0EEDE9C121A840B25A5602CF2B5C1E4CFD0FE923A08760383527A6";
	}

	if (strcmp(curvename,"NUMS512E")==0)
	{
		curve=16;
		printf("Curve= NUMS512E\n");
		strcpy(fieldname,"512PM");
		mbits=512;
		words=(1+((mbits-1)/bb));
		curvetype=EDWARDS;
		modtype=PSEUDO_MERSENNE;
		curve_a=1;
		cof=4;
		p=pow((Big)2,mbits)-569; 

		mip->IOBASE=16;
		r=(char *)"3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFB4F0636D2FCF91BA9E3FD8C970B686F52A4605786DEFECFF67468CF51BEED46D"; // Group Order
		curve_b=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFECBEF";
		gx=(char *)"DF8E316D128DB69C7A18CB7888D3C5332FD1E79F4DC4A38227A17EBE273B81474621C14EEE46730F78BDC992568904AD0FE525427CC4F015C5B9AB2999EC57FE";
		gy=(char *)"6D09BFF39D49CA7198B0F577A82A256EE476F726D8259D22A92B6B95909E834120CA53F2E9963562601A06862AECC1FD0266D38A9BF1D01F326DDEC0C1E2F5E1";
	}


	if (strcmp(curvename,"SECP256K1")==0)
	{ // SECP256K1
		curve=17;
		printf("Curve= SECP256K1\n");
		strcpy(fieldname,"SECP256K1");
		mbits=256;                 // bits in modulus
		words=(1+((mbits-1)/bb));  // words per Big
		curvetype=WEIERSTRASS;
		/*if (chunk<64)*/ modtype=NOT_SPECIAL;
		//modtype=PSEUDO_MERSENNE;
		curve_a=0;			     // Curve A parameter
		cof=1;
		p=pow((Big)2,mbits)-pow((Big)2,32)-977;  // Modulus
		mip->IOBASE=16;
		r=(char *)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141";   // group order

		curve_b=(char *)"7";  // curve B parameter
		gx=(char *)"79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798";       // generator point
		gy=(char *)"483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8";
	}


	if (strcmp(curvename,"BN254")==0)
	{
		curve=18;
		printf("Curve= BN254\n");
		strcpy(fieldname,curvename);
		mbits=254;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"4080000000000001";    // Fast but not GT_STRONG parameter

		p=36*pow(x,4)-36*pow(x,3)+24*x*x-6*x+1;  // Modulus
		t=6*x*x+1;
		r=p+1-t;                       // Group order
		curve_b=2;
		gx=p-1;                        // generator point in G1
		gy=1;
		cof=1;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_D;        // twist type

		Xa.set((ZZn)0,(ZZn)-1);
		Ya.set((ZZn)1,ZZn(0));
		Q.set(Xa,Ya);

		Q=(p-1+t)*Q;                   // generator point in G2
		cru=(18*pow(x,3)-18*x*x+9*x-2); // cube root of unity for GLV method
	}


	if (strcmp(curvename,"BN254CX")==0)
	{
		curve=19;
		printf("Curve= BN254CX\n");
		strcpy(fieldname,curvename);
		mbits=254;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"4000000003C012B1";

		p=36*pow(x,4)-36*pow(x,3)+24*x*x-6*x+1;
		t=6*x*x+1;
		r=p+1-t;
		curve_b=2;
		gx=p-1;
		gy=1;
		cof=1;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_D;

		Xa.set((ZZn)0,(ZZn)-1);
		Ya.set((ZZn)1,ZZn(0));
		Q.set(Xa,Ya);

		Q=(p-1+t)*Q;                   // generator point in G2
		cru=(18*pow(x,3)-18*x*x+9*x-2);
	}

	if (strcmp(curvename,"BLS383")==0)
	{
		curve=20;
		printf("Curve= BLS383\n");
		strcpy(fieldname,curvename);
		mbits=383;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
	
		mip->IOBASE=16;
	//	x=(char *)"11000001000000040";              
		x=(char *)"10008000001001200";	// SIGN_OF_X is POSITIVE
		p=(pow(x,6)-2*pow(x,5)+2*pow(x,3)+x+1)/3;
		t=x+1;
		r=pow(x,4)-x*x+1;
		cof=(p+1-t)/r;

	//	gx=-2; gy=-1;
	//	curve_b=9;
		gx=1; gy=4;
		curve_b=15;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
	//	mip->TWIST=MR_SEXTIC_D;
		mip->TWIST=MR_SEXTIC_M;

		P.set(gx,gy);
		P*=cof;
		P.get(gx,gy);

		//cout << "gx= " << gx << endl;
		//cout << "gy= " << gy << endl;
		//cout << "y^2= " << (gy*gy)%p << endl;
		//cout << "x^3+b= " << (gx*gx*gx+15)%p << endl;

		//while (!Q.set(randn2())) ; // probably not best way to choose this

		Xa=1;
		while (!Q.set(Xa))
		{
			Xa=Xa+(ZZn2)1;
		}

		TT=t*t-2*p;
		PP=p*p;
		FF=sqrt((4*PP-TT*TT)/3);
		np=PP+1-(-3*FF+TT)/2;  // 2 possibilities...

		Q=(np/r)*Q;

		zcru=pow((ZZn)2,(p-1)/3);
	//	zcru*=zcru;   // right cube root of unity
		cru=(Big)zcru;
	}

	if (strcmp(curvename,"BLS381")==0)
	{
		curve=21;
		printf("Curve= BLS381\n");
		strcpy(fieldname,curvename);
		mbits=381;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"d201000000010000";              // SIGN_OF_X is NEGATIVE
		p=(pow(x,6)+2*pow(x,5)-2*pow(x,3)-x+1)/3;
		t=-x+1;
		r=pow(x,4)-x*x+1;
		cof=(p+1-t)/r;

		curve_b=4;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_M;

		gx=1;
		while (!P.set(gx))
		{
			gx=gx+1;
		}
		P*=cof;
		P.get(gx,gy);

		Xa=1;
		while (!Q.set(Xa))
		{
			Xa=Xa+(ZZn2)1;
		}
		Q.get(Xa,Ya);
		Ya=-Ya;
		Q.set(Xa,Ya);

		TT=t*t-2*p;
		PP=p*p;
		FF=sqrt((4*PP-TT*TT)/3);
		np=PP+1-(-3*FF+TT)/2;  // 2 possibilities...

		Q=(np/r)*Q;

		zcru=pow((ZZn)2,(p-1)/3);
		//zcru*=zcru;   // right cube root of unity ?? if x>0 do this??
		cru=(Big)zcru;
	}


	if (strcmp(curvename,"BLS461")==0)
	{
		curve=24;
		printf("Curve= BLS461\n");
		strcpy(fieldname,curvename);
		mbits=461;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"1FFFFFFBFFFE00000000";              // SIGN_OF_X is NEGATIVE
		p=(pow(x,6)+2*pow(x,5)-2*pow(x,3)-x+1)/3;
		t=-x+1;
		r=pow(x,4)-x*x+1;
		cof=(p+1-t)/r;

		gx=-2; gy=-1;
		curve_b=9;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_M;

		P.set(gx,gy);
		P*=cof;
		P.get(gx,gy);

		while (!Q.set(randn2())) ;  // probably not best way to choose this

		TT=t*t-2*p;
		PP=p*p;
		FF=sqrt((4*PP-TT*TT)/3);
		np=PP+1-(-3*FF+TT)/2;  // 2 possibilities...

		Q=(np/r)*Q;

		zcru=pow((ZZn)2,(p-1)/3);
		//zcru*=zcru;   // right cube root of unity
		cru=(Big)zcru;

	}

	if (strcmp(curvename,"FP256BN")==0)
	{
		curve=22;
		printf("Curve= FP256BN\n");
		strcpy(fieldname,curvename);
		mbits=256;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"6882F5C030B0A801";    // Slow!

		p=36*pow(x,4)-36*pow(x,3)+24*x*x-6*x+1;
		t=6*x*x+1;
		r=p+1-t;
		curve_b=3;
		gx=1;
		gy=2;
		cof=1;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_M;


		Q.set((ZZn2)1);
		//while (!Q.set(randn2())) ;
		Q=(p-1+t)*Q;

		cru=(18*pow(x,3)-18*x*x+9*x-2);
		cout << pre1 << toupperit((char *)"CURVE_Cru",lang) << post1; output(chunk,words,cru,m); cout << term << endl;

	}

	if (strcmp(curvename,"FP512BN")==0)
	{
		curve=23;
		printf("Curve= FP512BN\n");
		strcpy(fieldname,curvename);
		mbits=512;
		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;
		x=(char *)"6882F5C030B0F7F010B306BB5E1BD80F";    // Slow!

		p=36*pow(x,4)+36*pow(x,3)+24*x*x+6*x+1;
		t=6*x*x+1;
		r=p+1-t;
		curve_b=3;
		gx=1;
		gy=2;
		cof=1;
		ecurve((Big)0,curve_b,p,MR_AFFINE);
		mip->TWIST=MR_SEXTIC_M;

		Q.set((ZZn2)1);
		//while (!Q.set(randn2())) ;
		Q=(p-1+t)*Q;

		cru=p-(18*pow(x,3)+18*x*x+9*x+2);
	}


	if (strcmp(curvename,"BLS24")==0)
	{
		curve=25;
		printf("Curve= BLS24\n");
		strcpy(fieldname,curvename);

		mbits=479;

		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;

// Note - this is GT-Strong curve

		x=(char *)"100020011FF80";				// SIGN_OF_X is POSITIVE

		//x=-x;

		p=(1+x+x*x-pow(x,4)+2*pow(x,5)-pow(x,6)+pow(x,8)-2*pow(x,9)+pow(x,10))/3;
		t=x+1;
		r=pow(x,8)-pow(x,4)+1;
		cof=(p+1-t)/r;

		//x=-x;

		gx=5; gy=12;
		curve_b=19;
		ecurve((Big)0,curve_b,p,MR_AFFINE);

		mip->TWIST=MR_SEXTIC_M;

		P.set(gx,gy);
		P*=cof;

		P.get(gx,gy);

		Big x0=0;
	
		forever
		{
			ZZn4 X;
			ZZn2 t;

			x0+=1;
			t.set((ZZn)0,(ZZn)x0);
			X.set(t,(ZZn2)0);
			if (!QQ.set(X)) continue;
			break;
		}

		TT=t*t*t*t-4*p*t*t+2*p*p;
		PP=pow(p,4);
		FF=sqrt((4*PP-TT*TT)/3);
		np=PP+1-(3*FF+TT)/2;

		QQ=(np/r)*QQ;

//cout << "QQ= " << QQ << endl;
//cout << "2*QQ= " << QQ+QQ << endl;
//cout << "3*QQ= " << (QQ+QQ)+QQ << endl;

		zcru=pow((ZZn)2,(p-1)/3);
		//zcru*=zcru;   // right cube root of unity -  not for M-TYPE
		cru=(Big)zcru;
	}


	if (strcmp(curvename,"BLS48")==0)
	{
		curve=26;
		printf("Curve= BLS48\n");
		strcpy(fieldname,curvename);

		mbits=556;

		words=(1+((mbits-1)/bb));
		curvetype=WEIERSTRASS;
		modtype=NOT_SPECIAL;
		curve_a=0;
		mip->IOBASE=16;

// Note - this is GT-Strong curve

		x=(char *)"7DE40020";				// SIGN_OF_X is POSITIVE

		r=pow(x,16)-pow(x,8)+1;
		p=r*(((x-1)*(x-1))/3)+x;
		t=x+1;
		cof=(p+1-t)/r;

// p=19 mod 24

		gx=2; gy=5;
		curve_b=17;
		ecurve((Big)0,curve_b,p,MR_AFFINE);

		mip->TWIST=MR_SEXTIC_M;

		P.set(gx,gy);
		P*=cof;

		P.get(gx,gy);

		Big x0=0;
	
		forever
		{
			ZZn8 XX;
			ZZn4 X;
			ZZn2 t;

			x0+=1;
			t.set((ZZn)0,(ZZn)x0);
			X.set(t,(ZZn2)0);
			XX.set(X,(ZZn4)0);
			if (!Q8.set(X)) continue;
			break;
		}


		tau[0]=2;  // count points on twist over extension p^8
		tau[1]=t;
		for (int jj=1;jj<8;jj++ ) tau[jj+1]=t*tau[jj]-p*tau[jj-1];

		TT=tau[8];

		PP=pow(p,8);
		FF=sqrt((4*PP-TT*TT)/3);
		np=PP+1-(3*FF+TT)/2;  //?

		Q8=(np/r)*Q8;

//cout << "Q8= " << Q8 << endl;
//cout << "r*Q8= " << r*Q8 << endl;
//cout << "2*QQ= " << QQ+QQ << endl;
//cout << "3*QQ= " << (QQ+QQ)+QQ << endl;

		zcru=pow((ZZn)2,(p-1)/3);
		//zcru*=zcru;   // right cube root of unity -  not for M-TYPE
		cru=(Big)zcru;
	}

	if (curve==0) {help(); return 0;}

	bytes=mbits/8;
	if (mbits%8!=0) bytes++;
	bytes*=8;


	lang=0;
	if (strcmp(lg,"c")==0)
	{
		open='{';
		close='}';
		term=';';
		lang=1;
		if (chunk==64) el='L';

		sprintf(xxx,"_%d_%d ",bytes,bb);

		sprintf(pre0,"const int ");
		sprintf(pre1,"const BIG%s",xxx);
		sprintf(pre2,"const chunk ");
		sprintf(pre3,"const BIG%s",xxx);
		sprintf(pre4,"const BIG%s",xxx);
		sprintf(pre5,"const BIG%s",xxx);
		sprintf(pre6,"const BIG%s",xxx);
		sprintf(pre7,"const BIG%s",xxx);

		sprintf(zzz,"_%s",curvename);
		sprintf(yyy,"_%s",fieldname);

		sprintf(post0,"%s= ",zzz);
		sprintf(post1,"%s= ",zzz);
		sprintf(post2,"%s= ",yyy);
		sprintf(post3,"%s[2]= ",zzz);
		sprintf(post4,"%s[2][2]= ",zzz);
		sprintf(post5,"%s[4]= ",zzz);
		sprintf(post6,"%s[4][4]= ",zzz);
		sprintf(post7,"%s= ",yyy);

	}

	if (strcmp(lg,"cpp")==0)
	{
		open='{';
		close='}';
		term=';';
		lang=7;
		if (chunk==64) el='L';

		strcpy(pre0,"const int ");
		strcpy(pre1,"const BIG ");
		strcpy(pre2,"const chunk ");
		strcpy(pre3,"const BIG ");
		strcpy(pre4,"const BIG ");
		strcpy(pre5,"const BIG ");
		strcpy(pre6,"const BIG ");
		strcpy(pre7,"const BIG ");

		strcpy(post0,"= ");
		strcpy(post1,"= ");
		strcpy(post2,"= ");
		strcpy(post3,"[2]= ");
		strcpy(post4,"[2][2]= ");
		strcpy(post5,"[4]= ");
		strcpy(post6,"[4][4]= ");
		strcpy(post7,"= ");

	}

	if (strcmp(lg,"java")==0)
	{
		open='{';
		close='}';
		term=';';
		lang=2;
		if (chunk==64) 
		{	
			el='L';
			strcpy(pre0,"public static final int ");
			strcpy(pre1,"public static final long[] ");
			strcpy(pre2,"public static final long ");
			strcpy(pre3,"public static final long[][] ");
			strcpy(pre4,"public static final long[][][] ");
			strcpy(pre5,"public static final long[][] ");
			strcpy(pre6,"public static final long[][][] ");
			strcpy(pre7,"public static final long[] ");
		}
		else
		{
			strcpy(pre0,"public static final int ");
			strcpy(pre1,"public static final int[] ");
			strcpy(pre2,"public static final int ");
			strcpy(pre3,"public static final int[][] ");
			strcpy(pre4,"public static final int[][][] ");
			strcpy(pre5,"public static final int[][] ");
			strcpy(pre6,"public static final int[][][] ");
			strcpy(pre7,"public static final int[] ");

		}
		strcpy(post0,"= ");
		strcpy(post1,"= ");
		strcpy(post2,"= ");
		strcpy(post3,"= ");
		strcpy(post4,"= ");
		strcpy(post5,"= ");
		strcpy(post6,"= ");
		strcpy(post7,"= ");

	}

	if (strcmp(lg,"javascript")==0)
	{
		open='[';
		close=']';
		term=',';
		lang=3;
		strcpy(pre0,"");
		strcpy(pre1,"");
		strcpy(pre2,"");
		strcpy(pre3,"");
		strcpy(pre4,"");
		strcpy(pre5,"");
		strcpy(pre6,"");
		strcpy(pre7,"");
		strcpy(post0,": ");
		strcpy(post1,": ");
		strcpy(post2,": ");
		strcpy(post3,": ");
		strcpy(post4,": ");
		strcpy(post5,": ");
		strcpy(post6,": ");
		strcpy(post7,": ");
	}

	if (strcmp(lg,"go")==0)
	{
		open='{';
		close='}';
		term=' ';
		lang=4;
		strcpy(pre0,"const ");
		strcpy(pre1,"var ");
		strcpy(pre2,"const ");
		strcpy(pre3,"var ");
		strcpy(pre4,"var ");
		strcpy(pre5,"var ");
		strcpy(pre6,"var ");
		strcpy(pre7,"var ");
		strcpy(post0," int= ");
		strcpy(post1,"= [...]Chunk ");
		strcpy(post2," Chunk=");
		sprintf(post3,"=[2][%d]Chunk ",words);
		sprintf(post4,"=[2][2][%d]Chunk ",words);
		sprintf(post5,"=[4][%d]Chunk ",words);
		sprintf(post6,"=[4][4][%d]Chunk ",words);
		strcpy(post7,"= [...]Chunk ");
	}

	if (strcmp(lg,"rust")==0)
	{
		open='[';
		close=']';
		term=';';
		lang=5;
		strcpy(pre0,"pub const ");
		strcpy(pre1,"pub const ");
		strcpy(pre2,"pub const ");
		strcpy(pre3,"pub const ");
		strcpy(pre4,"pub const ");
		strcpy(pre5,"pub const ");
		strcpy(pre6,"pub const ");
		strcpy(pre7,"pub const ");
		strcpy(post0,":isize = ");
		strcpy(post1,":[Chunk;NLEN]=");
		strcpy(post2,":Chunk=");
		strcpy(post3,":[[Chunk;NLEN];2]=");
		strcpy(post4,":[[[Chunk;NLEN];2];2]=");
		strcpy(post5,":[[Chunk;NLEN];4]=");
		strcpy(post6,":[[[Chunk;NLEN];4];4]=");
		strcpy(post7,":[Chunk;NLEN]=");

	}

	if (strcmp(lg,"swift")==0)
	{
		open='[';
		close=']';
		term=' ';
		lang=6;
		strcpy(pre0,"static let ");
		strcpy(pre1,"static public let ");
		strcpy(pre2,"static let ");
		strcpy(pre3,"static let ");
		strcpy(pre4,"static let ");
		strcpy(pre5,"static let ");
		strcpy(pre6,"static let ");
		strcpy(pre7,"static let ");
		strcpy(post0,":Int = ");
		strcpy(post1,":[Chunk] = ");
		strcpy(post2,":Chunk = ");
		strcpy(post3,":[[Chunk]] = ");
		strcpy(post4,":[[[Chunk]]] = ");
		strcpy(post5,":[[Chunk]] = ");
		strcpy(post6,":[[[Chunk]]] = ");
		strcpy(post7,":[Chunk] = ");
	}


	if (lang==0) {help(); return 0;}

	//cout << "\nMOD8 = " << p%8 << endl;

	m=pow((Big)2,bb);

	cout << "*** rom field parameters*****" << endl;
	cout << "// Base Bits= " << bb << endl; 

	cout << pre7 << toupperit((char *)"Modulus",lang) << post7; mc=output(chunk,words,p,m); cout << term << endl;
	r2modp=pow((Big)2,2*words*bb)%p;


//cout << "\ngx*R2modp= " << r2modp*gx << endl;
//cout << "mod p = " << redc(r2modp*gx) << endl << endl;

	cout << pre7 << toupperit((char *)"R2modp",lang) << post7; output(chunk,words,r2modp,m); cout << term << endl;



	if (modtype==NOT_SPECIAL)
		cout << pre2 << toupperit((char *)"MConst",lang)  << post2 << "0x" << inverse(m-p%m,m);	

	if (modtype==MONTGOMERY_FRIENDLY)
		cout << pre2 << toupperit((char *)"MConst",lang) << post2 << "0x" << mc+1;	

	if (modtype==PSEUDO_MERSENNE)
		cout << pre2 << toupperit((char *)"MConst",lang)  << post2 << "0x" << pow((Big)2,mbits)-p;	
	
	if (modtype==GENERALISED_MERSENNE)
		cout << pre2 << toupperit((char *)"MConst",lang)  << post2 << "0x1";	
	
	if (el!=0) cout << "L;" << endl;
	else cout << term << endl;


	if (curve>17)
	{ // Frobenius constants -  depend on embedding degree
		if (curve<25)
			set_frobenius_constant(X,12);
		else
		{
			if (curve==25) set_frobenius_constant(X,24);
			if (curve==26) set_frobenius_constant(X,48);
		}

		X.get(a,b);
		cout << pre1 << toupperit((char *)"Fra",lang) << post7; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"Frb",lang) << post7; output(chunk,words,b,m); cout << term << endl;

	}

	cout << endl;
	cout << "//*** rom curve parameters *****" << endl;
	cout << "// Base Bits= " << bb << endl; 
	cout << "\n" << pre0 << "CURVE_A"  << post0 << curve_a << term << endl;

	curve_b_i=toint(curve_b);
	if (curve_b_i==MR_TOOBIG) 
	{
		curve_b_i=toint(p-curve_b);
		if (curve_b_i==MR_TOOBIG)
			curve_b_i=0;
		else curve_b_i=-curve_b_i;
	}

	cof_i=toint(cof);
	if (cof_i==MR_TOOBIG) 
		cof_i=0;


	cout << pre0 << toupperit((char *)"CURVE_Cof_I",lang) << post0 << cof_i << term << endl;
	cout << pre1 << toupperit((char *)"CURVE_Cof",lang) << post1; output(chunk,words,cof,m); cout << term << endl;
	cout << pre0 << "CURVE_B_I" << post0 << curve_b_i << term << endl;
	cout << pre1 << "CURVE_B" << post1; output(chunk,words,curve_b,m); cout << term << endl;
	cout << pre1 << toupperit((char *)"CURVE_Order",lang) << post1; output(chunk,words,r,m); cout << term << endl;
	cout << pre1 << toupperit((char *)"CURVE_Gx",lang) << post1; output(chunk,words,gx,m); cout << term << endl;
	cout << pre1 << toupperit((char *)"CURVE_Gy",lang) << post1; output(chunk,words,gy,m); cout << term << endl;
	
// BN curves
	if (curve==18 || curve==19 || curve==22 || curve==23)
	{
		cout << endl;

		cout << pre1 << toupperit((char *)"CURVE_Bnx",lang) << post1; output(chunk,words,x,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Cru",lang) << post1; output(chunk,words,cru,m); cout << term << endl;

		Q.get(Xa,Ya);
		Xa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxb",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Ya.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pya",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pyb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		cout << pre3 << "CURVE_W"  << post3 << open; output(chunk,words,6*x*x-4*x+1,m);cout << ","; output(chunk,words,(2*x-1),m); cout << close << term << endl;
		cout << pre4 << "CURVE_SB" << post4 << open; cout << open; output(chunk,words,6*x*x-2*x,m); cout << ","; output(chunk,words,(2*x-1),m); cout << close;cout << ","; cout << open; output(chunk,words,(2*x-1),m); cout << ","; output(chunk,words,r-(6*x*x-4*x+1),m); cout << close; cout << close << term  << endl;

		cout << pre5 << "CURVE_WB" << post5 << open; output(chunk,words,2*x*x-3*x+1,m); cout << ","; output(chunk,words,12*x*x*x-8*x*x+x,m); 
		cout << ","; output(chunk,words,6*x*x*x-4*x*x+x,m); cout << ","; output(chunk,words,2*x*x-x,m); cout << close << term << endl;
	
		cout << pre6 << "CURVE_BB" << post6 << open; 
		cout << open;
		output(chunk,words,r-x+1,m); 
		cout << ","; output(chunk,words,r-x,m); 
		cout << ","; output(chunk,words,r-x,m); 
		cout << ","; output(chunk,words,2*x,m); 
		cout << close;

		cout << ","; cout << open;output(chunk,words,2*x-1,m); 
		cout << ","; output(chunk,words,r-x,m); 
		cout << ","; output(chunk,words,r-x+1,m); 
		cout << ","; output(chunk,words,r-x,m); 
		cout << close;
		cout << ","; cout << open; output(chunk,words,2*x,m); 
		cout << ","; output(chunk,words,2*x-1,m); 
		cout << ","; output(chunk,words,2*x-1,m); 
		cout << ","; output(chunk,words,2*x-1,m); 
		cout << close;

		cout << ","; cout << open; output(chunk,words,x+1,m); 
		cout << ","; output(chunk,words,4*x-2,m); 
		cout << ","; output(chunk,words,r-2*x-1,m); 
		cout << ","; output(chunk,words,x+1,m); 
		cout << close;
		cout << close << term << endl;

	}

//BLS curves
	if (curve==20 || curve==21 || curve==24)
	{
		cout << endl;

		cout << pre1 << toupperit((char *)"CURVE_Bnx",lang) << post1 ; output(chunk,words,x,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Cru",lang) << post1; output(chunk,words,cru,m); cout << term << endl;

		Q.get(Xa,Ya);
		Xa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxb",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Ya.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pya",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pyb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		Q*=r;
		if (!Q.iszero())
		{
			cout << "**** Failed ****" << endl;
			cout << "\nQ= " << Q << endl << endl;
		}

		cout << pre3 << "CURVE_W" << post3 << open; output(chunk,words,(Big)0,m);cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
		cout << pre4 << "CURVE_SB" << post4 << open; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close;cout << ","; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close; cout << close << term << endl;

		cout << pre5 << "CURVE_WB" << post5 << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
	
		cout << pre6 << "CURVE_BB" << post6 << open; 
		cout << open;
		output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open;output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << close << term << endl;

	}



	if (curve==25)
	{
		cout << endl;

		cout << pre1 << toupperit((char *)"CURVE_Bnx",lang) << post1 ; output(chunk,words,x,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Cru",lang) << post1; output(chunk,words,cru,m); cout << term << endl;

		QQ.get(XA,YA);
		XA.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxbb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		YA.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pyaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pyab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pyba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pybb",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		

		QQ*=r;
		if (!Q.iszero())
		{
			cout << "**** Failed ****" << endl;
			cout << "\nQQ= " << QQ << endl << endl;
		}

		cout << pre3 << "CURVE_W" << post3 << open; output(chunk,words,(Big)0,m);cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
		cout << pre4 << "CURVE_SB" << post4 << open; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close;cout << ","; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close; cout << close << term << endl;

		cout << pre5 << "CURVE_WB" << post5 << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
	
		cout << pre6 << "CURVE_BB" << post6 << open; 
		cout << open;
		output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open;output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << close << term << endl;

	}

	if (curve==26)
	{
		cout << endl;

		cout << pre1 << toupperit((char *)"CURVE_Bnx",lang) << post1 ; output(chunk,words,x,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Cru",lang) << post1; output(chunk,words,cru,m); cout << term << endl;

		Q8.get(X8,Y8);
		X8.get(AA,BB);

		AA.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxaaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxaab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxaba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxabb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		BB.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxbaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxbab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pxbba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pxbbb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		Y8.get(AA,BB);

		AA.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pyaaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pyaab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pyaba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pyabb",lang) << post1; output(chunk,words,b,m); cout << term << endl;

		BB.get(Aa,Bb);
		Aa.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pybaa",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pybab",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		Bb.get(a,b);
		cout << pre1 << toupperit((char *)"CURVE_Pybba",lang) << post1; output(chunk,words,a,m); cout << term << endl;
		cout << pre1 << toupperit((char *)"CURVE_Pybbb",lang) << post1; output(chunk,words,b,m); cout << term << endl;
		

		Q8*=r;
		if (!Q.iszero())
		{
			cout << "**** Failed ****" << endl;
			cout << "\nQQ= " << QQ << endl << endl;
		}

		cout << pre3 << "CURVE_W" << post3 << open; output(chunk,words,(Big)0,m);cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
		cout << pre4 << "CURVE_SB" << post4 << open; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close;cout << ","; cout << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close; cout << close << term << endl;

		cout << pre5 << "CURVE_WB" << post5 << open; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); cout << ","; output(chunk,words,(Big)0,m); cout << close << term << endl;
	
		cout << pre6 << "CURVE_BB" << post6 << open; 
		cout << open;
		output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open;output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;

		cout << ","; cout << open; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << ","; output(chunk,words,(Big)0,m); 
		cout << close;
		cout << close << term << endl;

	}


}
