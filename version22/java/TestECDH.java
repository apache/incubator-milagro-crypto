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

/* test driver and function exerciser for ECDH/ECIES/ECDSA API Functions */

public class TestECDH
{
	public static void printBinary(byte[] array)
	{
		int i;
		for (i=0;i<array.length;i++)
		{
			System.out.printf("%02x", array[i]);
		}
		System.out.println();
	}    

	public static void main(String[] args) 
	{
		int i,j=0,res;
		int result;
		String pp=new String("M0ng00se");

		int EGS=ECDH.EGS;
		int EFS=ECDH.EFS;
		int EAS=AES.KS;
		int sha=ECDH.HASH_TYPE;

		byte[] S1=new byte[EGS];
		byte[] W0=new byte[2*EFS+1];
		byte[] W1=new byte[2*EFS+1];
		byte[] Z0=new byte[EFS];
		byte[] Z1=new byte[EFS];
		byte[] RAW=new byte[100];
		byte[] SALT=new byte[8];
		byte[] P1=new byte[3];
		byte[] P2=new byte[4];
		byte[] V=new byte[2*EFS+1];
		byte[] M=new byte[17];
		byte[] T=new byte[12];
		byte[] CS=new byte[EGS];
		byte[] DS=new byte[EGS];

		RAND rng=new RAND();

		rng.clean();
		for (i=0;i<100;i++) RAW[i]=(byte)(i);

		rng.seed(100,RAW);

//for (j=0;j<100;j++)
//{

		for (i=0;i<8;i++) SALT[i]=(byte)(i+1);  // set Salt

		System.out.println("Alice's Passphrase= "+pp);
		byte[] PW=pp.getBytes();

/* private key S0 of size EGS bytes derived from Password and Salt */

		byte[] S0=ECDH.PBKDF2(sha,PW,SALT,1000,EGS);

		System.out.print("Alice's private key= 0x");
		printBinary(S0);

/* Generate Key pair S/W */
		ECDH.KEY_PAIR_GENERATE(null,S0,W0); 

		System.out.print("Alice's public key= 0x");
		printBinary(W0);

		res=ECDH.PUBLIC_KEY_VALIDATE(true,W0);
		if (res!=0)
		{
			System.out.println("ECP Public Key is invalid!\n");
			return;
		}
/* Random private key for other party */
		ECDH.KEY_PAIR_GENERATE(rng,S1,W1);

		System.out.print("Servers private key= 0x");
		printBinary(S1);

		System.out.print("Servers public key= 0x");
		printBinary(W1);


		res=ECDH.PUBLIC_KEY_VALIDATE(true,W1);
		if (res!=0)
		{
			System.out.print("ECP Public Key is invalid!\n");
			return;
		}

/* Calculate common key using DH - IEEE 1363 method */

		ECDH.ECPSVDP_DH(S0,W1,Z0);
		ECDH.ECPSVDP_DH(S1,W0,Z1);

		boolean same=true;
		for (i=0;i<EFS;i++)
			if (Z0[i]!=Z1[i]) same=false;

		if (!same)
		{
			System.out.println("*** ECPSVDP-DH Failed");
			return;
		}

		byte[] KEY=ECDH.KDF2(sha,Z0,null,EAS);

		System.out.print("Alice's DH Key=  0x"); printBinary(KEY);
		System.out.print("Servers DH Key=  0x"); printBinary(KEY);

		if (ROM.CURVETYPE!=ROM.MONTGOMERY)
		{
			System.out.println("Testing ECIES");

			P1[0]=0x0; P1[1]=0x1; P1[2]=0x2; 
			P2[0]=0x0; P2[1]=0x1; P2[2]=0x2; P2[3]=0x3; 

			for (i=0;i<=16;i++) M[i]=(byte)i; 

			byte[] C=ECDH.ECIES_ENCRYPT(sha,P1,P2,rng,W1,M,V,T);

			System.out.println("Ciphertext= ");
			System.out.print("V= 0x"); printBinary(V);
			System.out.print("C= 0x"); printBinary(C);
			System.out.print("T= 0x"); printBinary(T);


			M=ECDH.ECIES_DECRYPT(sha,P1,P2,V,C,T,S1);
			if (M.length==0)
			{
				System.out.println("*** ECIES Decryption Failed\n");
				return;
			}
			else System.out.println("Decryption succeeded");

			System.out.print("Message is 0x"); printBinary(M);

			System.out.println("Testing ECDSA");

			if (ECDH.ECPSP_DSA(sha,rng,S0,M,CS,DS)!=0)
			{
				System.out.println("***ECDSA Signature Failed");
				return;
			}
			System.out.println("Signature= ");
			System.out.print("C= 0x"); printBinary(CS);
			System.out.print("D= 0x"); printBinary(DS);

			if (ECDH.ECPVP_DSA(sha,W0,M,CS,DS)!=0)
			{
				System.out.println("***ECDSA Verification Failed");
				return;
			}
			else System.out.println("ECDSA Signature/Verification succeeded "+j);
//}
//System.out.println("Test Completed Successfully");
		}
	}
}
