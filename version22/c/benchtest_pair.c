/* Test and benchmark pairing functions
	First build amcl.a from build_pair batch file
	gcc -O3 benchtest_pair.c amcl.a -o benchtest_pair.exe
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "amcl.h" /* Make sure and select a pairing-friendly curve in here! */

#define MIN_TIME 10.0
#define MIN_ITERS 10 

int main()
{
    csprng RNG;
	BIG q,s,r,x,y,a,b,m;
	ECP P,G;
	FP2 wx,wy,f; 
	FP4 c,cp,cpm1,cpm2,cr;
    ECP2 Q,W;
	FP12 g,w;
	unsigned long ran;

    int i,iterations;
    clock_t start;
    double elapsed;
	char pr[10];

#if CHOICE==BN254
	printf("BN254 Curve\n");
#endif
#if CHOICE==BN454
	printf("BN454 Curve\n");	
#endif
#if CHOICE==BN646
	printf("BN646 Curve\n");	
#endif

#if CHOICE==BN254_CX 
	printf("BN254_CX Curve\n");	
#endif
#if CHOICE==BN254_T
	printf("BN254_T Curve\n");	
#endif	
#if CHOICE==BN254_T2 
	printf("BN254_T2 Curve\n");	
#endif
#if CHOICE==BLS455 
	printf("BLS455 Curve\n");	
#endif
#if CHOICE==BLS383 
	printf("BLS383 Curve\n");	
#endif

#if CHUNK==16
	printf("16-bit Build\n");
#endif
#if CHUNK==32
	printf("32-bit Build\n");
#endif
#if CHUNK==64
	printf("64-bit Build\n");
#endif

	time((time_t *)&ran);
	pr[0]=ran;
	pr[1]=ran>>8;
	pr[2]=ran>>16;
	pr[3]=ran>>24;
	for (i=4;i<10;i++) pr[i]=i;

    RAND_seed(&RNG,10,pr);

	BIG_rcopy(x,CURVE_Gx);

	BIG_rcopy(y,CURVE_Gy);
    ECP_set(&G,x,y);

	
	BIG_rcopy(r,CURVE_Order);
	BIG_randomnum(s,r,&RNG);
	ECP_copy(&P,&G);
    PAIR_G1mul(&P,r);

	if (!ECP_isinf(&P))
	{
		printf("FAILURE - rG!=O\n");
		return 0;
	}
	
	iterations=0;
    start=clock();
    do {
		ECP_copy(&P,&G);
		PAIR_G1mul(&P,s);

		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("G1 mul              - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

    
    BIG_rcopy(wx.a,CURVE_Pxa); FP_nres(wx.a);
    BIG_rcopy(wx.b,CURVE_Pxb); FP_nres(wx.b);
    BIG_rcopy(wy.a,CURVE_Pya); FP_nres(wy.a);
    BIG_rcopy(wy.b,CURVE_Pyb); FP_nres(wy.b);    
	ECP2_set(&W,&wx,&wy);

	ECP2_copy(&Q,&W);
    ECP2_mul(&Q,r);

	if (!ECP2_isinf(&Q))
	{
		printf("FAILURE - rQ!=O\n");
		return 0;
	}

	iterations=0;
    start=clock();
    do {
		ECP2_copy(&Q,&W);
		PAIR_G2mul(&Q,s);

		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("G2 mul              - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

	PAIR_ate(&w,&Q,&P);
	PAIR_fexp(&w);

	FP12_copy(&g,&w);

	PAIR_GTpow(&g,r);

	if (!FP12_isunity(&g))
	{
		printf("FAILURE - g^r!=1\n");
		return 0;
	}

	iterations=0;
    start=clock();
    do {
		FP12_copy(&g,&w);
		PAIR_GTpow(&g,s);

		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("GT pow              - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

	BIG_rcopy(a,CURVE_Fra);
	BIG_rcopy(b,CURVE_Frb);
	FP2_from_BIGs(&f,a,b);

	BIG_rcopy(q,Modulus);

	BIG_copy(m,q);
	BIG_mod(m,r);

	BIG_copy(a,s);
	BIG_mod(a,m);

	BIG_copy(b,s);
	BIG_sdiv(b,m);

	FP12_copy(&g,&w);
	FP12_trace(&c,&g);

	FP12_frob(&g,&f);
	FP12_trace(&cp,&g);

	FP12_conj(&w,&w);
	FP12_mul(&g,&w);

	FP12_trace(&cpm1,&g);
	FP12_mul(&g,&w);
	FP12_trace(&cpm2,&g);

	iterations=0;
    start=clock();
    do {
		FP4_xtr_pow2(&cr,&cp,&c,&cpm1,&cpm2,a,b);
		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("GT pow (compressed) - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

	iterations=0;
    start=clock();
    do {
		PAIR_ate(&w,&Q,&P);
		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("PAIRing ATE         - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

	iterations=0;
    start=clock();
    do {
		FP12_copy(&g,&w);
		PAIR_fexp(&g);
		iterations++;
		elapsed=(clock()-start)/(double)CLOCKS_PER_SEC;
    } while (elapsed<MIN_TIME || iterations<MIN_ITERS);
    elapsed=1000.0*elapsed/iterations;
    printf("PAIRing FEXP        - %8d iterations  ",iterations);
    printf(" %8.2lf ms per iteration\n",elapsed);

	ECP_copy(&P,&G);	
	ECP2_copy(&Q,&W);

	PAIR_G1mul(&P,s);
	PAIR_ate(&g,&Q,&P);
	PAIR_fexp(&g);

	ECP_copy(&P,&G);

	PAIR_G2mul(&Q,s);
	PAIR_ate(&w,&Q,&P);
	PAIR_fexp(&w);

	if (!FP12_equals(&g,&w))
	{
		printf("FAILURE - e(sQ,p)!=e(Q,sP) \n");
		return 0;
	}

	ECP2_copy(&Q,&W);
	PAIR_ate(&g,&Q,&P);
	PAIR_fexp(&g);

	PAIR_GTpow(&g,s);

	if (!FP12_equals(&g,&w))
	{
		printf("FAILURE - e(sQ,p)!=e(Q,P)^s \n");
		return 0;
	}

	printf("All tests pass\n");

	return 0;
}
