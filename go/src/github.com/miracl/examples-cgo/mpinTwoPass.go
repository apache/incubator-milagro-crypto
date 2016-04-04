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
	"encoding/hex"
	"fmt"

	amclcgo "github.com/miracl/amcl-cgo"
	amclgo "github.com/miracl/amcl-go"
)

func main() {
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)
	fmt.Printf("ID: ")
	amclcgo.MPIN_printBinary(ID)
	fmt.Printf("\n")

	// Epoch time in days
	date := amclcgo.MPIN_today()

	// PIN variable to create token
	PIN1 := -1
	// PIN variable to authenticate
	PIN2 := -1

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := amclgo.NewRAND()
	rng.Seed(len(seed), seed)

	// Generate Master Secret Share 1
	rtn, MS1 := amclcgo.MPIN_RANDOM_GENERATE_WRAP(rng)
	if rtn != 0 {
		fmt.Println("MPIN_RANDOM_GENERATE Error:", rtn)
		return
	}
	fmt.Printf("MS1: 0x")
	amclcgo.MPIN_printBinary(MS1[:])

	// Generate Master Secret Share 2
	rtn, MS2 := amclcgo.MPIN_RANDOM_GENERATE_WRAP(rng)
	if rtn != 0 {
		fmt.Println("MPIN_RANDOM_GENERATE Error:", rtn)
		return
	}
	fmt.Printf("MS2: 0x")
	amclcgo.MPIN_printBinary(MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := amclcgo.MPIN_HASH_ID(ID)

	// Generate server secret share 1
	rtn, SS1 := amclcgo.MPIN_GET_SERVER_SECRET_WRAP(MS1[:])
	if rtn != 0 {
		fmt.Println("MPIN_GET_SERVER_SECRET Error:", rtn)
		return
	}
	fmt.Printf("SS1: 0x")
	amclcgo.MPIN_printBinary(SS1[:])

	// Generate server secret share 2
	rtn, SS2 := amclcgo.MPIN_GET_SERVER_SECRET_WRAP(MS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_GET_SERVER_SECRET Error:", rtn)
		return
	}
	fmt.Printf("SS2: 0x")
	amclcgo.MPIN_printBinary(SS2[:])

	// Combine server secret shares
	rtn, SS := amclcgo.MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G2(SS1, SS2) Error:", rtn)
		return
	}
	fmt.Printf("SS: 0x")
	amclcgo.MPIN_printBinary(SS[:])

	// Generate client secret share 1
	rtn, CS1 := amclcgo.MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_SECRET Error:", rtn)
		return
	}
	fmt.Printf("Client Secret Share CS1: 0x")
	amclcgo.MPIN_printBinary(CS1[:])

	// Generate client secret share 2
	rtn, CS2 := amclcgo.MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_SECRET Error:", rtn)
		return
	}
	fmt.Printf("Client Secret Share CS2: 0x")
	amclcgo.MPIN_printBinary(CS2[:])

	// Combine client secret shares
	CS := make([]byte, amclcgo.G1S)
	rtn, CS = amclcgo.MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G1 Error:", rtn)
		return
	}
	fmt.Printf("Client Secret CS: 0x")
	amclcgo.MPIN_printBinary(CS[:])

	// Generate time permit share 1
	rtn, TP1 := amclcgo.MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_PERMIT Error:", rtn)
		return
	}
	fmt.Printf("TP1: 0x")
	amclcgo.MPIN_printBinary(TP1[:])

	// Generate time permit share 2
	rtn, TP2 := amclcgo.MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)
	if rtn != 0 {
		fmt.Println("MPIN_GET_CLIENT_PERMIT Error:", rtn)
		return
	}
	fmt.Printf("TP2: 0x")
	amclcgo.MPIN_printBinary(TP2[:])

	// Combine time permit shares
	rtn, TP := amclcgo.MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])
	if rtn != 0 {
		fmt.Println("MPIN_RECOMBINE_G1(TP1, TP2) Error:", rtn)
		return
	}

	// Client extracts PIN1 from secret to create Token
	for PIN1 < 0 {
		fmt.Printf("Please enter PIN to create token: ")
		fmt.Scan(&PIN1)
	}

	rtn, TOKEN := amclcgo.MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: EXTRACT_PIN rtn: %d\n", rtn)
		return
	}
	fmt.Printf("Client Token TK: 0x")
	amclcgo.MPIN_printBinary(TOKEN[:])

	//////   Client   //////

	for PIN2 < 0 {
		fmt.Printf("Please enter PIN to authenticate: ")
		fmt.Scan(&PIN2)
	}

	////// Client Pass 1 //////
	// Send U and UT to server
	var X [amclcgo.EGS]byte
	fmt.Printf("X: 0x")
	amclcgo.MPIN_printBinary(X[:])
	rtn, XOut, SEC, U, UT := amclcgo.MPIN_CLIENT_1_WRAP(date, ID, rng, X[:], PIN2, TOKEN[:], TP[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: CLIENT rtn: %d\n", rtn)
		return
	}
	fmt.Printf("XOut: 0x")
	amclcgo.MPIN_printBinary(XOut[:])

	//////   Server Pass 1  //////
	/* Calculate H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
	HID, HTID := amclcgo.MPIN_SERVER_1_WRAP(date, ID)

	/* Send Y to Client */
	rtn, Y := amclcgo.MPIN_RANDOM_GENERATE_WRAP(rng)
	if rtn != 0 {
		fmt.Println("MPIN_RANDOM_GENERATE Error:", rtn)
		return
	}
	fmt.Printf("Y: 0x")
	amclcgo.MPIN_printBinary(Y[:])

	/* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
	rtn, V := amclcgo.MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: CLIENT_2 rtn: %d\n", rtn)
	}

	/* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
	/* If PIN error not required, set E and F = null */
	rtn, _, _ = amclcgo.MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
	if rtn != 0 {
		fmt.Printf("FAILURE: MPIN_SERVER_2 rtn: %d\n", rtn)
	}
	fmt.Printf("HID: 0x")
	amclcgo.MPIN_printBinary(HID[:])
	fmt.Printf("HTID: 0x")
	amclcgo.MPIN_printBinary(HTID[:])

	if rtn != 0 {
		fmt.Printf("Authentication failed Error Code %d\n", rtn)
		return
	} else {
		fmt.Printf("Authenticated ID: %s \n", IDstr)
	}
}
