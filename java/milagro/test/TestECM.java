package milagro.test;

import org.junit.Test;

import milagro.AES;
import milagro.ECDH;
import milagro.RAND;

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

/* test driver and function exerciser for ECDH API Function only - for use with Montgomery curves */

public class TestECM {
	public static void printBinary(byte[] array) {
		int i;
		for (i = 0; i < array.length; i++) {
			System.out.printf("%02x", array[i]);
		}
		System.out.println();
	}

	public static void main(String[] args) {
		new TestECM().main();
	}

	@Test
	public void main() {
		int i, res;
		String pp = new String("M0ng00se");

		int EGS = ECDH.EGS;
		int EFS = ECDH.EFS;
		int EAS = AES.KS;

		byte[] S1 = new byte[EGS];
		byte[] W0 = new byte[2 * EFS + 1];
		byte[] W1 = new byte[2 * EFS + 1];
		byte[] Z0 = new byte[EFS];
		byte[] Z1 = new byte[EFS];
		byte[] RAW = new byte[100];
		byte[] SALT = new byte[8];

		RAND rng = new RAND();

		rng.clean();
		for (i = 0; i < 100; i++)
			RAW[i] = (byte) (i);

		rng.seed(100, RAW);

		// for (j=0;j<100;j++)
		// {

		for (i = 0; i < 8; i++)
			SALT[i] = (byte) (i + 1); // set Salt

		System.out.println("Alice's Passphrase= " + pp);
		byte[] PW = pp.getBytes();

		/* private key S0 of size EGS bytes derived from Password and Salt */

		byte[] S0 = ECDH.PBKDF2(PW, SALT, 1000, EGS);

		System.out.print("Alice's private key= 0x");
		printBinary(S0);

		/* Generate Key pair S/W */
		ECDH.KEY_PAIR_GENERATE(null, S0, W0);

		System.out.print("Alice's public key= 0x");
		printBinary(W0);

		res = ECDH.PUBLIC_KEY_VALIDATE(true, W0);
		if (res != 0) {
			System.out.println("Alice's public Key is invalid!\n");
			return;
		}
		/* Random private key for other party */
		ECDH.KEY_PAIR_GENERATE(rng, S1, W1);

		System.out.print("Servers private key= 0x");
		printBinary(S1);

		System.out.print("Servers public key= 0x");
		printBinary(W1);

		res = ECDH.PUBLIC_KEY_VALIDATE(true, W1);
		if (res != 0) {
			System.out.print("Server's public Key is invalid!\n");
			return;
		}

		/* Calculate common key using DH - IEEE 1363 method */

		ECDH.ECPSVDP_DH(S0, W1, Z0);
		ECDH.ECPSVDP_DH(S1, W0, Z1);

		boolean same = true;
		for (i = 0; i < EFS; i++)
			if (Z0[i] != Z1[i])
				same = false;

		if (!same) {
			System.out.println("*** ECPSVDP-DH Failed");
			return;
		}

		byte[] KEY = ECDH.KDF1(Z0, EAS);

		System.out.print("Alice's DH Key=  0x");
		printBinary(KEY);
		System.out.print("Servers DH Key=  0x");
		printBinary(KEY);

		// }
		// System.out.println("Test Completed Successfully");
	}
}
