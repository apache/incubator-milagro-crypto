using System;

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
	public static void printBinary(sbyte[] array)
	{
		int i;
		for (i = 0;i < array.Length;i++)
		{
			Console.Write("{0:x2}", array[i]);
		}
		Console.WriteLine();
	}

	public static void Main(string[] args)
	{
		int i , j = 0, res ;
		int result;
		string pp = "M0ng00se";

		int EGS = ECDH.EGS;
		int EFS = ECDH.EFS;
		int EAS = AES.KS;

		sbyte[] S1 = new sbyte[EGS];
		sbyte[] W0 = new sbyte[2 * EFS + 1];
		sbyte[] W1 = new sbyte[2 * EFS + 1];
		sbyte[] Z0 = new sbyte[EFS];
		sbyte[] Z1 = new sbyte[EFS];
		sbyte[] RAW = new sbyte[100];
		sbyte[] SALT = new sbyte[8];
		sbyte[] P1 = new sbyte[3];
		sbyte[] P2 = new sbyte[4];
		sbyte[] V = new sbyte[2 * EFS + 1];
		sbyte[] M = new sbyte[17];
		sbyte[] T = new sbyte[12];
		sbyte[] CS = new sbyte[EGS];
		sbyte[] DS = new sbyte[EGS];

		RAND rng = new RAND();

		rng.clean();
		for (i = 0;i < 100;i++)
		{
			RAW[i] = (sbyte)(i);
		}

		rng.seed(100,RAW);

//for (j=0;j<100;j++)
//{

		for (i = 0;i < 8;i++)
		{
			SALT[i] = (sbyte)(i + 1); // set Salt
		}

		Console.WriteLine("Alice's Passphrase= " + pp);
		sbyte[] PW = pp.GetBytes();

/* private key S0 of size EGS bytes derived from Password and Salt */

		sbyte[] S0 = ECDH.PBKDF2(PW,SALT,1000,EGS);

		Console.Write("Alice's private key= 0x");
		printBinary(S0);

/* Generate Key pair S/W */
		ECDH.KEY_PAIR_GENERATE(null,S0,W0);

		Console.Write("Alice's public key= 0x");
		printBinary(W0);

		res = ECDH.PUBLIC_KEY_VALIDATE(true,W0);
		if (res != 0)
		{
			Console.WriteLine("ECP Public Key is invalid!\n");
			return;
		}
/* Random private key for other party */
		ECDH.KEY_PAIR_GENERATE(rng,S1,W1);

		Console.Write("Servers private key= 0x");
		printBinary(S1);

		Console.Write("Servers public key= 0x");
		printBinary(W1);


		res = ECDH.PUBLIC_KEY_VALIDATE(true,W1);
		if (res != 0)
		{
			Console.Write("ECP Public Key is invalid!\n");
			return;
		}

/* Calculate common key using DH - IEEE 1363 method */

		ECDH.ECPSVDP_DH(S0,W1,Z0);
		ECDH.ECPSVDP_DH(S1,W0,Z1);

		bool same = true;
		for (i = 0;i < EFS;i++)
		{
			if (Z0[i] != Z1[i])
			{
				same = false;
			}
		}

		if (!same)
		{
			Console.WriteLine("*** ECPSVDP-DH Failed");
			return;
		}

		sbyte[] KEY = ECDH.KDF1(Z0,EAS);

		Console.Write("Alice's DH Key=  0x");
		printBinary(KEY);
		Console.Write("Servers DH Key=  0x");
		printBinary(KEY);

		Console.WriteLine("Testing ECIES");

		P1[0] = 0x0;
		P1[1] = 0x1;
		P1[2] = 0x2;
		P2[0] = 0x0;
		P2[1] = 0x1;
		P2[2] = 0x2;
		P2[3] = 0x3;

		for (i = 0;i <= 16;i++)
		{
			M[i] = (sbyte)i;
		}

		sbyte[] C = ECDH.ECIES_ENCRYPT(P1,P2,rng,W1,M,V,T);

		Console.WriteLine("Ciphertext= ");
		Console.Write("V= 0x");
		printBinary(V);
		Console.Write("C= 0x");
		printBinary(C);
		Console.Write("T= 0x");
		printBinary(T);


		M = ECDH.ECIES_DECRYPT(P1,P2,V,C,T,S1);
		if (M.Length == 0)
		{
			Console.WriteLine("*** ECIES Decryption Failed\n");
			return;
		}
		else
		{
			Console.WriteLine("Decryption succeeded");
		}

		Console.Write("Message is 0x");
		printBinary(M);

		Console.WriteLine("Testing ECDSA");

		if (ECDH.ECPSP_DSA(rng,S0,M,CS,DS) != 0)
		{
			Console.WriteLine("***ECDSA Signature Failed");
			return;
		}
		Console.WriteLine("Signature= ");
		Console.Write("C= 0x");
		printBinary(CS);
		Console.Write("D= 0x");
		printBinary(DS);

		if (ECDH.ECPVP_DSA(W0,M,CS,DS) != 0)
		{
			Console.WriteLine("***ECDSA Verification Failed");
			return;
		}
		else
		{
			Console.WriteLine("ECDSA Signature/Verification succeeded " + j);
		}
//}
//System.out.println("Test Completed Successfully");
	}
}
