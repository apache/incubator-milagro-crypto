
#include <stdio.h>
#include "amcl.h"

#ifdef COMBA

int main()
{
	int i,j,k,N;

	N=NLEN;


	printf("Insert this code in BIG_mul() in file big.c between #define UNWOUND and #else \n\n");

	for (i=0;i<N;i++)
		printf("\td[%d]=(dchunk)a[%d]*b[%d];\n",i,i,i);

	printf("\n\ts=d[0];\n\tt = s; c[0]=(chunk)t&BMASK; co=t>>BASEBITS;\n");

	for (k=1;k<N;k++)
	{
		printf("\ts+=d[%d]; t=co+s ",k);
		for (i=k;i>=1+k/2;i--)
			printf("+(dchunk)(a[%d]-a[%d])*(b[%d]-b[%d])",i,k-i,k-i,i);
		printf("; c[%d]=(chunk)t&BMASK; co=t>>BASEBITS; \n",k);
	}
	printf("\n");
	for (k=N;k<2*N-1;k++)
	{
		printf("\ts-=d[%d]; t=co+s ",k-N);
		for (i=N-1;i>=1+k/2;i--)
			printf("+(dchunk)(a[%d]-a[%d])*(b[%d]-b[%d])",i,k-i,k-i,i);
		printf("; c[%d]=(chunk)t&BMASK; co=t>>BASEBITS; \n",k);
	}
	printf("\tc[%d]=(chunk)co;\n",2*N-1);



	printf("\nInsert this code in BIG_sqr() in file big.c between #define UNWOUND and #else \n\n");

	printf("\n\tt=(dchunk)a[0]*a[0]; c[0]=(chunk)t&BMASK; co=t>>BASEBITS;\n");

	for (k=1;k<N;k++)
	{
		printf("\tt= ",k);
		for (i=k;i>=1+k/2;i--)
			printf("+(dchunk)a[%d]*a[%d]",i,k-i);
		printf("; t+=t; t+=co;");
		if (k%2==0) printf(" t+=(dchunk)a[%d]*a[%d];",k/2,k/2);
		printf(" c[%d]=(chunk)t&BMASK; co=t>>BASEBITS; \n", k);
	}
	printf("\n");

	for (k=N;k<2*N-2;k++)
	{
		printf("\tt= ",k-N);
		for (i=N-1;i>=1+k/2;i--)
			printf("+(dchunk)a[%d]*a[%d]",i,k-i);
		printf("; t+=t; t+=co;");
		if (k%2==0) printf(" t+=(dchunk)a[%d]*a[%d];",k/2,k/2);
		printf(" c[%d]=(chunk)t&BMASK; co=t>>BASEBITS; \n", k);
	}
	printf("\tt=co; t+=(dchunk)a[%d]*a[%d]; c[%d]=(chunk)t&BMASK; co=t>>BASEBITS; \n ",N-1,N-1,2*N-2);

	printf("\tc[%d]=(chunk)co;\n",2*N-1);


#if MODTYPE == NOT_SPECIAL

	printf("\nInsert this code in BIG_monty() in file big.c between #define UNWOUND and #else \n\n");

	printf("\tt=d[0]; v[0]=((chunk)t*MC)&BMASK; t+=(dchunk)v[0]*md[0];  s=0; c=(t>>BASEBITS);\n\n");

	for (k=1;k<N;k++)
	{
		printf("\tt=d[%d]+c+s+(dchunk)v[0]*md[%d]",k,k);
		for (i=k-1;i>k/2;i--) printf("+(dchunk)(v[%d]-v[%d])*(md[%d]-md[%d])",k-i,i,i,k-i);
		printf("; v[%d]=((chunk)t*MC)&BMASK; t+=(dchunk)v[%d]*md[0]; ",k,k);
		printf(" dd[%d]=(dchunk)v[%d]*md[%d]; s+=dd[%d]; c=(t>>BASEBITS); \n",k,k,k,k);
	}
	printf("\n");
	for (k=N;k<2*N-1;k++)
	{
		printf("\tt=d[%d]+c+s",k);
		for (i=N-1;i>=1+k/2;i--) printf("+(dchunk)(v[%d]-v[%d])*(md[%d]-md[%d])",k-i,i,i,k-i);
		printf("; a[%d]=(chunk)t&BMASK;  s-=dd[%d]; c=(t>>BASEBITS); \n",k-N,k-N+1);
	}
	printf("\ta[%d]=d[%d]+(chunk)c&BMASK;\n",N-1,2*N-1);	


#endif

}

#endif


