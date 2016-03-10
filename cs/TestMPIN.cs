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

/* test driver and function exerciser for MPIN API Functions */


public class TestMPIN
{
	internal static bool PERMITS = true;
	internal static bool PINERROR = true;
	internal static bool FULL = true;
	internal static bool SINGLE_PASS = false;

	internal static void printBinary(sbyte[] array)
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
		RAND rng = new RAND();
		sbyte[] raw = new sbyte[100];
		for (int i = 0;i < 100;i++)
		{
			raw[i] = (sbyte)(i + 1);
		}
		rng.seed(100,raw);

		int EGS = MPIN.EGS;
		int EFS = MPIN.EFS;
		int G1S = 2 * EFS + 1; // Group 1 Size
		int G2S = 4 * EFS; // Group 2 Size
		int EAS = 16;

		sbyte[] S = new sbyte[EGS];
		sbyte[] SST = new sbyte[G2S];
		sbyte[] TOKEN = new sbyte[G1S];
		sbyte[] PERMIT = new sbyte[G1S];
		sbyte[] SEC = new sbyte[G1S];
		sbyte[] xID = new sbyte[G1S];
		sbyte[] xCID = new sbyte[G1S];
		sbyte[] X = new sbyte[EGS];
		sbyte[] Y = new sbyte[EGS];
		sbyte[] E = new sbyte[12 * EFS];
		sbyte[] F = new sbyte[12 * EFS];
		sbyte[] HID = new sbyte[G1S];
		sbyte[] HTID = new sbyte[G1S];

		sbyte[] G1 = new sbyte[12 * EFS];
		sbyte[] G2 = new sbyte[12 * EFS];
		sbyte[] R = new sbyte[EGS];
		sbyte[] Z = new sbyte[G1S];
		sbyte[] W = new sbyte[EGS];
		sbyte[] T = new sbyte[G1S];
		sbyte[] CK = new sbyte[EAS];
		sbyte[] SK = new sbyte[EAS];

/* Trusted Authority set-up */

		MPIN.RANDOM_GENERATE(rng,S);
		Console.Write("Master Secret s: 0x");
		printBinary(S);

 /* Create Client Identity */
		 string IDstr = "testUser@miracl.com";
		sbyte[] CLIENT_ID = IDstr.GetBytes();

		sbyte[] HCID = MPIN.HASH_ID(CLIENT_ID); // Either Client or TA calculates Hash(ID) - you decide!

		Console.Write("Client ID= ");
		printBinary(CLIENT_ID);

/* Client and Server are issued secrets by DTA */
		MPIN.GET_SERVER_SECRET(S,SST);
		Console.Write("Server Secret SS: 0x");
		printBinary(SST);

		MPIN.GET_CLIENT_SECRET(S,HCID,TOKEN);
		Console.Write("Client Secret CS: 0x");
		printBinary(TOKEN);

/* Client extracts PIN from secret to create Token */
		int pin = 1234;
		Console.WriteLine("Client extracts PIN= " + pin);
		int rtn = MPIN.EXTRACT_PIN(CLIENT_ID,pin,TOKEN);
		if (rtn != 0)
		{
			Console.WriteLine("FAILURE: EXTRACT_PIN rtn: " + rtn);
		}

		Console.Write("Client Token TK: 0x");
		printBinary(TOKEN);

		if (FULL)
		{
			MPIN.PRECOMPUTE(TOKEN,HCID,G1,G2);
		}
		int date;
		if (PERMITS)
		{
			date = MPIN.today();
			Console.WriteLine("Date= "+date);
/* Client gets "Time Token" permit from DTA */
			MPIN.GET_CLIENT_PERMIT(date,S,HCID,PERMIT);
			Console.Write("Time Permit TP: 0x");
			printBinary(PERMIT);

/* This encoding makes Time permit look random - Elligator squared */
			MPIN.ENCODING(rng,PERMIT);
			Console.Write("Encoded Time Permit TP: 0x");
			printBinary(PERMIT);
			MPIN.DECODING(PERMIT);
			Console.Write("Decoded Time Permit TP: 0x");
			printBinary(PERMIT);
		}
		else
		{
			date = 0;
		}

		Console.Write("\nPIN= ");
//		Scanner scan = new Scanner(System.in);
//		pin = scan.Next();

		pin=int.Parse(Console.ReadLine());

/* Set date=0 and PERMIT=null if time permits not in use

Client First pass: Inputs CLIENT_ID, optional RNG, pin, TOKEN and PERMIT. Output xID =x .H(CLIENT_ID) and re-combined secret SEC
If PERMITS are is use, then date!=0 and PERMIT is added to secret and xCID = x.(H(CLIENT_ID)+H(date|H(CLIENT_ID)))
Random value x is supplied externally if RNG=null, otherwise generated and passed out by RNG

IMPORTANT: To save space and time..
If Time Permits OFF set xCID = null, HTID=null and use xID and HID only
If Time permits are ON, AND pin error detection is required then all of xID, xCID, HID and HTID are required
If Time permits are ON, AND pin error detection is NOT required, set xID=null, HID=null and use xCID and HTID only.


*/

		sbyte[] pxID = xID;
		sbyte[] pxCID = xCID;
		sbyte[] pHID = HID;
		sbyte[] pHTID = HTID;
		sbyte[] pE = E;
		sbyte[] pF = F;
		sbyte[] pPERMIT = PERMIT;
		sbyte[] prHID;

		if (date != 0)
		{

			prHID = pHTID;
			if (!PINERROR)
			{
				pxID = null;
				pHID = null;
			}
		}
		else
		{
			prHID = pHID;
			pPERMIT = null;
			pxCID = null;
			pHTID = null;
		}
		if (!PINERROR)
		{
			pE = null;
			pF = null;
		}

				if (SINGLE_PASS)
				{
			Console.WriteLine("MPIN Single Pass");
				  int timeValue = MPIN.GET_TIME();
				  rtn = MPIN.CLIENT(date,CLIENT_ID,rng,X,pin,TOKEN,SEC,pxID,pxCID,pPERMIT,timeValue,Y);
			if (rtn != 0)
			{
			  Console.WriteLine("FAILURE: CLIENT rtn: " + rtn);
			}

				  if (FULL)
				  {
					HCID = MPIN.HASH_ID(CLIENT_ID);
					MPIN.GET_G1_MULTIPLE(rng,1,R,HCID,Z); // Also Send Z=r.ID to Server, remember random r
				  }

				  rtn = MPIN.SERVER(date,pHID,pHTID,Y,SST,pxID,pxCID,SEC,pE,pF,CLIENT_ID,timeValue);
				  if (rtn != 0)
				  {
			  Console.WriteLine("FAILURE: SERVER rtn: " + rtn);
				  }

				  if (FULL)
				  {
					MPIN.GET_G1_MULTIPLE(rng,0,W,prHID,T); // Also send T=w.ID to client, remember random w
				  }
				}
				else
				{
			Console.WriteLine("MPIN Multi Pass");
				  /* Send U=x.ID to server, and recreate secret from token and pin */
			rtn = MPIN.CLIENT_1(date,CLIENT_ID,rng,X,pin,TOKEN,SEC,pxID,pxCID,pPERMIT);
			if (rtn != 0)
			{
			  Console.WriteLine("FAILURE: CLIENT_1 rtn: " + rtn);
			}

			if (FULL)
			{
			  HCID = MPIN.HASH_ID(CLIENT_ID);
			  MPIN.GET_G1_MULTIPLE(rng,1,R,HCID,Z); // Also Send Z=r.ID to Server, remember random r
			}

				  /* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
			MPIN.SERVER_1(date,CLIENT_ID,pHID,pHTID);

				  /* Server generates Random number Y and sends it to Client */
			MPIN.RANDOM_GENERATE(rng,Y);

				  if (FULL)
				  {
			  MPIN.GET_G1_MULTIPLE(rng,0,W,prHID,T); // Also send T=w.ID to client, remember random w
				  }

				  /* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
			rtn = MPIN.CLIENT_2(X,Y,SEC);
			if (rtn != 0)
			{
			  Console.WriteLine("FAILURE: CLIENT_2 rtn: " + rtn);
			}

				  /* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
				  /* If PIN error not required, set E and F = null */

			rtn = MPIN.SERVER_2(date,pHID,pHTID,Y,SST,pxID,pxCID,SEC,pE,pF);

			if (rtn != 0)
			{
			  Console.WriteLine("FAILURE: SERVER_1 rtn: " + rtn);
			}
				}

		if (rtn == MPIN.BAD_PIN)
		{
		  Console.WriteLine("Server says - Bad Pin. I don't know you. Feck off.\n");
		  if (PINERROR)
		  {
			int err = MPIN.KANGAROO(E,F);
			if (err != 0)
			{
				Console.Write("(Client PIN is out by {0:D})\n",err);
			}
		  }
		  return;
		}
		else
		{
			Console.WriteLine("Server says - PIN is good! You really are " + IDstr);
		}


		if (FULL)
		{
			MPIN.CLIENT_KEY(G1,G2,pin,R,X,T,CK);
			Console.Write("Client Key =  0x");
			printBinary(CK);

			MPIN.SERVER_KEY(Z,SST,W,pxID,pxCID,SK);
			Console.Write("Server Key =  0x");
			printBinary(SK);
		}
	}
}
