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

/* Test and benchmark elliptic curve and RSA functions */

public class BenchtestEC
{
/* generate an RSA key pair */
	public static final int MIN_TIME=10; /* seconds */
	public static final int MIN_ITERS=10; 

	public static void main(String[] args) 
	{
		int i,iterations;
		long start,elapsed;
		byte[] RAW=new byte[100];
		RAND rng=new RAND();
		double dur;
		rsa_public_key pub=new rsa_public_key(ROM.FFLEN);
		rsa_private_key priv=new rsa_private_key(ROM.HFLEN);
		byte[] P=new byte[RSA.RFS];
		byte[] M=new byte[RSA.RFS];
		byte[] C=new byte[RSA.RFS];


		rng.clean();
		for (i=0;i<100;i++) RAW[i]=(byte)(i);

		rng.seed(100,RAW);	
		if (ROM.CURVETYPE==ROM.WEIERSTRASS)
		{
			System.out.print("Weierstrass parameterization\n");
		}		
		if (ROM.CURVETYPE==ROM.EDWARDS)
		{
			System.out.print("Edwards parameterization\n");
		}
		if (ROM.CURVETYPE==ROM.MONTGOMERY)
		{
			System.out.print("Montgomery parameterization\n");
		}

		if (ROM.MODTYPE==ROM.PSEUDO_MERSENNE)
		{
			System.out.print("Pseudo-Mersenne Modulus\n");
		}
		if (ROM.MODTYPE==ROM.MONTGOMERY_FRIENDLY)
		{
			System.out.print("Montgomery friendly Modulus\n");
		}
		if (ROM.MODTYPE==ROM.GENERALISED_MERSENNE)
		{
			System.out.print("Generalised-Mersenne Modulus\n");
		}
		if (ROM.MODTYPE==ROM.NOT_SPECIAL)
		{
			System.out.print("Not special Modulus\n");
		}

		System.out.format("Modulus size %d bits\n",ROM.MODBITS); 
		System.out.format("%d bit build\n",ROM.CHUNK); 
		BIG r,gx,gy,s,wx,wy;
		ECP G,WP;

		gx=new BIG(ROM.CURVE_Gx);
		if (ROM.CURVETYPE!=ROM.MONTGOMERY)
		{
			gy=new BIG(ROM.CURVE_Gy);
			G=new ECP(gx,gy);
		}
		else
			G=new ECP(gx);

		r=new BIG(ROM.CURVE_Order);
		s=BIG.randomnum(r,rng);

		WP=G.mul(r);
		if (!WP.is_infinity())
		{
			System.out.print("FAILURE - rG!=O\n");
			return;
		}

		start = System.currentTimeMillis();
		iterations=0;
		do {
			WP=G.mul(s);
			iterations++;
			elapsed=(System.currentTimeMillis()-start);
		} while (elapsed<MIN_TIME*1000 || iterations<MIN_ITERS);
		dur=(double)elapsed/iterations;
		System.out.format("EC  mul - %8d iterations  ",iterations);
		System.out.format(" %8.2f ms per iteration\n",dur);


		System.out.format("Generating %d-bit RSA public/private key pair\n",ROM.FFLEN*ROM.BIGBITS);

		iterations=0;
		start=System.currentTimeMillis();
		do {
			RSA.KEY_PAIR(rng,65537,priv,pub);
			iterations++;
			elapsed=(System.currentTimeMillis()-start);
		} while (elapsed<MIN_TIME*1000 || iterations<MIN_ITERS);
		dur=(double)elapsed/iterations;
		System.out.format("RSA gen - %8d iterations  ",iterations);
		System.out.format(" %8.2f ms per iteration\n",dur);

		for (i=0;i<RSA.RFS;i++) M[i]=(byte)(i%128);

		iterations=0;
		start=System.currentTimeMillis();
		do {
			RSA.ENCRYPT(pub,M,C);
			iterations++;
			elapsed=(System.currentTimeMillis()-start);
		} while (elapsed<MIN_TIME*1000 || iterations<MIN_ITERS);
		dur=(double)elapsed/iterations;
    	System.out.format("RSA enc - %8d iterations  ",iterations);
    	System.out.format(" %8.2f ms per iteration\n",dur);

		iterations=0;
		start=System.currentTimeMillis();
		do {
			RSA.DECRYPT(priv,C,P);
			iterations++;
			elapsed=(System.currentTimeMillis()-start);
		} while (elapsed<MIN_TIME*1000 || iterations<MIN_ITERS);
		dur=(double)elapsed/iterations;
    	System.out.format("RSA dec - %8d iterations  ",iterations);
    	System.out.format(" %8.2f ms per iteration\n",dur);

		for (i=0;i<RSA.RFS;i++)
		{
			if (P[i]!=M[i])
			{
				System.out.print("FAILURE - RSA decryption\n");
				return;
			}
		}

		System.out.print("All tests pass\n");
	}
}
