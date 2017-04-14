package milagro.test;
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

/* test driver and function exerciser for MPIN API Functions */

import java.util.Scanner;

import org.junit.Test;

import milagro.MPIN;
import milagro.RAND;

public class TestMPIN {
	static boolean PERMITS = true;
	static boolean PINERROR = true;
	static boolean FULL = false;
	static boolean SINGLE_PASS = false;

	static void printBinary(byte[] array) {
		int i;
		for (i = 0; i < array.length; i++) {
			System.out.printf("%02x", array[i]);
		}
		System.out.println();
	}

	public static void main(String[] args) {
		new TestMPIN().main();
	}

	@Test
	public void main() {
		RAND rng = new RAND();
		byte[] raw = new byte[100];
		for (int i = 0; i < 100; i++)
			raw[i] = (byte) (i + 1);
		rng.seed(100, raw);

		int EGS = MPIN.EGS;
		int EFS = MPIN.EFS;
		int G1S = 2 * EFS + 1; /* Group 1 Size */
		int G2S = 4 * EFS; /* Group 2 Size */
		int EAS = 16;

		byte[] S = new byte[EGS];
		byte[] SST = new byte[G2S];
		byte[] TOKEN = new byte[G1S];
		byte[] PERMIT = new byte[G1S];
		byte[] SEC = new byte[G1S];
		byte[] xID = new byte[G1S];
		byte[] xCID = new byte[G1S];
		byte[] X = new byte[EGS];
		byte[] Y = new byte[EGS];
		byte[] E = new byte[12 * EFS];
		byte[] F = new byte[12 * EFS];
		byte[] HID = new byte[G1S];
		byte[] HTID = new byte[G1S];

		byte[] G1 = new byte[12 * EFS];
		byte[] G2 = new byte[12 * EFS];
		byte[] R = new byte[EGS];
		byte[] Z = new byte[G1S];
		byte[] W = new byte[EGS];
		byte[] T = new byte[G1S];
		byte[] CK = new byte[EAS];
		byte[] SK = new byte[EAS];

		/* Trusted Authority set-up */

		MPIN.RANDOM_GENERATE(rng, S);
		System.out.print("Master Secret s: 0x");
		printBinary(S);

		/* Create Client Identity */
		String IDstr = "testUser@miracl.com";
		byte[] CLIENT_ID = IDstr.getBytes();

		byte[] HCID = MPIN.HASH_ID(CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

		System.out.print("Client ID= ");
		printBinary(CLIENT_ID);

		/* Client and Server are issued secrets by DTA */
		MPIN.GET_SERVER_SECRET(S, SST);
		System.out.print("Server Secret SS: 0x");
		printBinary(SST);

		MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);
		System.out.print("Client Secret CS: 0x");
		printBinary(TOKEN);

		/* Client extracts PIN from secret to create Token */
		int pin = 1234;
		System.out.println("Client extracts PIN= " + pin);
		int rtn = MPIN.EXTRACT_PIN(CLIENT_ID, pin, TOKEN);
		if (rtn != 0)
			System.out.println("FAILURE: EXTRACT_PIN rtn: " + rtn);

		System.out.print("Client Token TK: 0x");
		printBinary(TOKEN);

		if (FULL) {
			MPIN.PRECOMPUTE(TOKEN, HCID, G1, G2);
		}
		int date;
		if (PERMITS) {
			date = MPIN.today();
			/* Client gets "Time Token" permit from DTA */
			MPIN.GET_CLIENT_PERMIT(date, S, HCID, PERMIT);
			System.out.print("Time Permit TP: 0x");
			printBinary(PERMIT);

			/* This encoding makes Time permit look random - Elligator squared */
			MPIN.ENCODING(rng, PERMIT);
			System.out.print("Encoded Time Permit TP: 0x");
			printBinary(PERMIT);
			MPIN.DECODING(PERMIT);
			System.out.print("Decoded Time Permit TP: 0x");
			printBinary(PERMIT);
		} else
			date = 0;

		System.out.print("\nPIN= ");
		try (Scanner scan = new Scanner(System.in)) {
			pin = scan.nextInt();
		}

		/*
		 * Set date=0 and PERMIT=null if time permits not in use
		 * 
		 * Client First pass: Inputs CLIENT_ID, optional RNG, pin, TOKEN and PERMIT. Output xID =x .H(CLIENT_ID) and re-combined secret SEC If PERMITS are is use, then date!=0 and PERMIT is added to secret and xCID = x.(H(CLIENT_ID)+H(date|H(CLIENT_ID))) Random value x is supplied externally if RNG=null, otherwise generated and passed out by RNG
		 * 
		 * IMPORTANT: To save space and time.. If Time Permits OFF set xCID = null, HTID=null and use xID and HID only If Time permits are ON, AND pin error detection is required then all of xID, xCID, HID and HTID are required If Time permits are ON, AND pin error detection is NOT required, set xID=null, HID=null and use xCID and HTID only.
		 * 
		 * 
		 */

		byte[] pxID = xID;
		byte[] pxCID = xCID;
		byte[] pHID = HID;
		byte[] pHTID = HTID;
		byte[] pE = E;
		byte[] pF = F;
		byte[] pPERMIT = PERMIT;
		byte[] prHID;

		if (date != 0) {

			prHID = pHTID;
			if (!PINERROR) {
				pxID = null;
				pHID = null;
			}
		} else {
			prHID = pHID;
			pPERMIT = null;
			pxCID = null;
			pHTID = null;
		}
		if (!PINERROR) {
			pE = null;
			pF = null;
		}

		if (SINGLE_PASS) {
			System.out.println("MPIN Single Pass");
			int timeValue = MPIN.GET_TIME();
			rtn = MPIN.CLIENT(date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT, timeValue, Y);
			if (rtn != 0)
				System.out.println("FAILURE: CLIENT rtn: " + rtn);

			if (FULL) {
				HCID = MPIN.HASH_ID(CLIENT_ID);
				MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */
			}

			rtn = MPIN.SERVER(date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, pE, pF, CLIENT_ID, timeValue);
			if (rtn != 0)
				System.out.println("FAILURE: SERVER rtn: " + rtn);

			if (FULL) {
				MPIN.GET_G1_MULTIPLE(rng, 0, W, prHID, T); /* Also send T=w.ID to client, remember random w */
			}
		} else {
			System.out.println("MPIN Multi Pass");
			/* Send U=x.ID to server, and recreate secret from token and pin */
			rtn = MPIN.CLIENT_1(date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT);
			if (rtn != 0)
				System.out.println("FAILURE: CLIENT_1 rtn: " + rtn);

			if (FULL) {
				HCID = MPIN.HASH_ID(CLIENT_ID);
				MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */
			}

			/* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
			MPIN.SERVER_1(date, CLIENT_ID, pHID, pHTID);

			/* Server generates Random number Y and sends it to Client */
			MPIN.RANDOM_GENERATE(rng, Y);

			if (FULL) {
				MPIN.GET_G1_MULTIPLE(rng, 0, W, prHID, T); /* Also send T=w.ID to client, remember random w */
			}

			/* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
			rtn = MPIN.CLIENT_2(X, Y, SEC);
			if (rtn != 0)
				System.out.println("FAILURE: CLIENT_2 rtn: " + rtn);

			/* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
			/* If PIN error not required, set E and F = null */

			rtn = MPIN.SERVER_2(date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, pE, pF);

			if (rtn != 0)
				System.out.println("FAILURE: SERVER_1 rtn: " + rtn);
		}

		if (rtn == MPIN.BAD_PIN) {
			System.out.println("Server says - Bad Pin. I don't know you. Feck off.\n");
			if (PINERROR) {
				int err = MPIN.KANGAROO(E, F);
				if (err != 0)
					System.out.format("(Client PIN is out by %d)\n", err);
			}
			return;
		} else
			System.out.println("Server says - PIN is good! You really are " + IDstr);

		if (FULL) {
			MPIN.CLIENT_KEY(G1, G2, pin, R, X, T, CK);
			System.out.print("Client Key =  0x");
			printBinary(CK);

			MPIN.SERVER_KEY(Z, SST, W, pxID, pxCID, SK);
			System.out.print("Server Key =  0x");
			printBinary(SK);
		}
	}
}
