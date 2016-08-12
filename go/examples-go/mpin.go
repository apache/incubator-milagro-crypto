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

package main

import (
	"fmt"

	amcl "git.apache.org/incubator-milagro-crypto.git/go/amcl-go"
)

/* Configure mode of operation */

const PERMITS bool = true
const PINERROR bool = true
const FULL bool = true
const SINGLE_PASS bool = true

func main() {
	rng := amcl.NewRAND()
	var raw [100]byte
	for i := 0; i < 100; i++ {
		raw[i] = byte(i + 1)
	}
	rng.Seed(100, raw[:])

	const EGS = amcl.MPIN_EGS
	const EFS = amcl.MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS int = 16

	var S [EGS]byte
	var SST [G2S]byte
	var TOKEN [G1S]byte
	var PERMIT [G1S]byte
	var SEC [G1S]byte
	var xID [G1S]byte
	var xCID [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	var G1 [12 * EFS]byte
	var G2 [12 * EFS]byte
	var R [EGS]byte
	var Z [G1S]byte
	var W [EGS]byte
	var T [G1S]byte
	var CK [EAS]byte
	var SK [EAS]byte
	var MESSAGE [256]byte

	/* Trusted Authority set-up */

	amcl.MPIN_RANDOM_GENERATE(rng, S[:])
	fmt.Printf("Master Secret s: 0x")
	amcl.MPIN_printBinary(S[:])

	/* Create Client Identity */
	IDstr := "testUser@miracl.com"
	CLIENT_ID := []byte(IDstr)

	HCID := amcl.MPIN_HASH_ID(CLIENT_ID) /* Either Client or TA calculates Hash(ID) - you decide! */

	fmt.Printf("Client ID= ")
	amcl.MPIN_printBinary(CLIENT_ID)
	fmt.Printf("\n")

	/* Client and Server are issued secrets by DTA */
	amcl.MPIN_GET_SERVER_SECRET(S[:], SST[:])
	fmt.Printf("Server Secret SS: 0x")
	amcl.MPIN_printBinary(SST[:])

	amcl.MPIN_GET_CLIENT_SECRET(S[:], HCID, TOKEN[:])
	fmt.Printf("Client Secret CS: 0x")
	amcl.MPIN_printBinary(TOKEN[:])

	/* Client extracts PIN from secret to create Token */
	pin := 1234
	fmt.Printf("Client extracts PIN= %d", pin)
	fmt.Printf("\n")
	rtn := amcl.MPIN_EXTRACT_PIN(CLIENT_ID, pin, TOKEN[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: EXTRACT_PIN rtn: %d", rtn)
		fmt.Printf("\n")
	}

	fmt.Printf("Client Token TK: 0x")
	amcl.MPIN_printBinary(TOKEN[:])

	if FULL {
		amcl.MPIN_PRECOMPUTE(TOKEN[:], HCID, G1[:], G2[:])
	}

	date := 0
	if PERMITS {
		date = amcl.MPIN_today()
		/* Client gets "Time Token" permit from DTA */
		amcl.MPIN_GET_CLIENT_PERMIT(date, S[:], HCID, PERMIT[:])
		fmt.Printf("Time Permit TP: 0x")
		amcl.MPIN_printBinary(PERMIT[:])

		/* This encoding makes Time permit look random - Elligator squared */
		amcl.MPIN_ENCODING(rng, PERMIT[:])
		fmt.Printf("Encoded Time Permit TP: 0x")
		amcl.MPIN_printBinary(PERMIT[:])
		amcl.MPIN_DECODING(PERMIT[:])
		fmt.Printf("Decoded Time Permit TP: 0x")
		amcl.MPIN_printBinary(PERMIT[:])
	}

	pin = -1
	for pin < 0 {
		fmt.Printf("\nPIN= ")
		fmt.Scanf("%d", &pin)
	}

	pxID := xID[:]
	pxCID := xCID[:]
	pHID := HID[:]
	pHTID := HTID[:]
	pE := E[:]
	pF := F[:]
	pPERMIT := PERMIT[:]
	var prHID []byte

	if date != 0 {
		prHID = pHTID
		if !PINERROR {
			pxID = nil
			pHID = nil
		}
	} else {
		prHID = pHID
		pPERMIT = nil
		pxCID = nil
		pHTID = nil
	}
	if !PINERROR {
		pE = nil
		pF = nil
	}

	if SINGLE_PASS {
		fmt.Printf("MPIN Single Pass\n")
		timeValue := amcl.MPIN_GET_TIME()
		rtn = amcl.MPIN_CLIENT(date, CLIENT_ID, rng, X[:], pin, TOKEN[:], SEC[:], pxID, pxCID, pPERMIT, MESSAGE[:], timeValue, Y[:])
		if rtn != 0 {
			fmt.Printf("FAILURE: CLIENT rtn: %d\n", rtn)
		}

		if FULL {
			HCID = amcl.MPIN_HASH_ID(CLIENT_ID)
			amcl.MPIN_GET_G1_MULTIPLE(rng, 1, R[:], HCID, Z[:]) /* Also Send Z=r.ID to Server, remember random r */
		}

		rtn = amcl.MPIN_SERVER(date, pHID, pHTID, Y[:], SST[:], pxID, pxCID, SEC[:], pE, pF, CLIENT_ID, MESSAGE[:], timeValue)
		if rtn != 0 {
			fmt.Printf("FAILURE: SERVER rtn: %d\n", rtn)
		}

		if FULL {
			amcl.MPIN_GET_G1_MULTIPLE(rng, 0, W[:], prHID, T[:]) /* Also send T=w.ID to client, remember random w  */
		}
	} else {
		fmt.Printf("MPIN Multi Pass\n")
		/* Send U=x.ID to server, and recreate secret from token and pin */
		rtn = amcl.MPIN_CLIENT_1(date, CLIENT_ID, rng, X[:], pin, TOKEN[:], SEC[:], pxID, pxCID, pPERMIT)
		if rtn != 0 {
			fmt.Printf("FAILURE: CLIENT_1 rtn: %d\n", rtn)
		}

		if FULL {
			HCID = amcl.MPIN_HASH_ID(CLIENT_ID)
			amcl.MPIN_GET_G1_MULTIPLE(rng, 1, R[:], HCID, Z[:]) /* Also Send Z=r.ID to Server, remember random r */
		}

		/* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
		amcl.MPIN_SERVER_1(date, CLIENT_ID, pHID, pHTID)

		/* Server generates Random number Y and sends it to Client */
		amcl.MPIN_RANDOM_GENERATE(rng, Y[:])

		if FULL {
			amcl.MPIN_GET_G1_MULTIPLE(rng, 0, W[:], prHID, T[:]) /* Also send T=w.ID to client, remember random w  */
		}

		/* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
		rtn = amcl.MPIN_CLIENT_2(X[:], Y[:], SEC[:])
		if rtn != 0 {
			fmt.Printf("FAILURE: CLIENT_2 rtn: %d\n", rtn)
		}

		/* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
		/* If PIN error not required, set E and F = null */

		rtn = amcl.MPIN_SERVER_2(date, pHID, pHTID, Y[:], SST[:], pxID, pxCID, SEC[:], pE, pF)
	}

	if rtn != 0 {
		fmt.Printf("FAILURE: SERVER_1 rtn: %d\n", rtn)
	}

	if rtn == amcl.MPIN_BAD_PIN {
		fmt.Printf("Server says - Bad Pin. I don't know you. Feck off.\n")
		if PINERROR {
			err := amcl.MPIN_KANGAROO(E[:], F[:])
			if err != 0 {
				fmt.Printf("(Client PIN is out by %d)\n", err)
			}
		}
		return
	} else {
		fmt.Printf("Server says - PIN is good! You really are " + IDstr)
		fmt.Printf("\n")
	}

	if FULL {
		amcl.MPIN_CLIENT_KEY(G1[:], G2[:], pin, R[:], X[:], T[:], CK[:])
		fmt.Printf("Client Key =  0x")
		amcl.MPIN_printBinary(CK[:])

		amcl.MPIN_SERVER_KEY(Z[:], SST[:], W[:], pxID, pxCID, SK[:])
		fmt.Printf("Server Key =  0x")
		amcl.MPIN_printBinary(SK[:])
	}

}
