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

package amcl

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"testing"
)

func TestGoodPIN(t *testing.T) {
	want := 0
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("MPIN GOOD PIN %d != %d", want, got)
	}
}

func TestBadPIN(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1235

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestBadPIN %d != %d", want, got)
	}
}

func TestBadToken(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	// Send UT as V to model bad token
	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], UT[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestBadToken %d != %d", want, got)
	}
}

func TestRandom(t *testing.T) {
	want := 0
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seed := make([]byte, 16)
	rand.Read(seed)
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestRandom %d != %d", want, got)
	}
}

func TestGoodSignature(t *testing.T) {
	want := 0
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Message to sign
	MESSAGE := []byte("test message to sign")

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestGoodSignature %d != %d", want, got)
	}
}

func TestSignatureExpired(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	MESSAGE := []byte("test message to sign")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	timeValue += 10
	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestSignatureExpired %d != %d", want, got)
	}
}

func TestBadSignature(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	MESSAGE := []byte("test message to sign")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	var X [EGS]byte
	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	MESSAGE[0] = 00
	got := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	if got != want {
		t.Errorf("TestBadSignature %d != %d", want, got)
	}
}

func TestMPINFull(t *testing.T) {
	want := "0afc948b03b2733a0663571f86411a07"
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

	// Epoch time in seconds
	timeValue := 1439465203

	// PIN variable to create token
	PIN1 := 1234
	// PIN variable to authenticate
	PIN2 := 1234

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Message to sign
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = MPIN_EGS
	const EFS = MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */
	const EAS = MPIN_PAS

	var MS1 [EGS]byte
	var SS1 [G2S]byte
	var CS1 [G1S]byte
	var TP1 [G1S]byte
	var MS2 [EGS]byte
	var SS2 [G2S]byte
	var CS2 [G1S]byte
	var TP2 [G1S]byte
	var SS [G2S]byte
	var TP [G1S]byte
	var TOKEN [G1S]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
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
	var AES_KEY_CLIENT [EAS]byte
	var AES_KEY_SERVER [EAS]byte

	// Generate Master Secret Share 1
	MPIN_RANDOM_GENERATE(rng, MS1[:])

	// Generate Master Secret Share 2
	MPIN_RANDOM_GENERATE(rng, MS2[:])

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	MPIN_GET_SERVER_SECRET(MS1[:], SS1[:])

	// Generate server secret share 2
	MPIN_GET_SERVER_SECRET(MS2[:], SS2[:])

	// Combine server secret shares
	MPIN_RECOMBINE_G2(SS1[:], SS2[:], SS[:])

	// Generate client secret share 1
	MPIN_GET_CLIENT_SECRET(MS1[:], HCID, CS1[:])

	// Generate client secret share 2
	MPIN_GET_CLIENT_SECRET(MS2[:], HCID, CS2[:])

	// Combine client secret shares : TOKEN is the full client secret
	MPIN_RECOMBINE_G1(CS1[:], CS2[:], TOKEN[:])

	// Generate time permit share 1
	MPIN_GET_CLIENT_PERMIT(date, MS1[:], HCID, TP1[:])

	// Generate time permit share 2
	MPIN_GET_CLIENT_PERMIT(date, MS2[:], HCID, TP2[:])

	// Combine time permit shares
	MPIN_RECOMBINE_G1(TP1[:], TP2[:], TP[:])

	// Create token
	MPIN_EXTRACT_PIN(ID, PIN1, TOKEN[:])

	// precomputation
	MPIN_PRECOMPUTE(TOKEN[:], HCID, G1[:], G2[:])

	// Authenticate
	MPIN_CLIENT(date, ID, rng, X[:], PIN2, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, timeValue, Y[:])

	// Send Z=r.ID to Server
	MPIN_GET_G1_MULTIPLE(rng, 1, R[:], HCID, Z[:])

	MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)

	// send T=w.ID to client
	MPIN_GET_G1_MULTIPLE(rng, 0, W[:], HTID[:], T[:])

	MPIN_SERVER_KEY(Z[:], SS[:], W[:], U[:], UT[:], AES_KEY_SERVER[:])
	got := hex.EncodeToString(AES_KEY_SERVER[:])
	if got != want {
		t.Errorf("TestMPINFull %s != %s", want, got)
	}

	MPIN_CLIENT_KEY(G1[:], G2[:], PIN2, R[:], X[:], T[:], AES_KEY_CLIENT[:])
	got = hex.EncodeToString(AES_KEY_CLIENT[:])
	if got != want {
		t.Errorf("TestMPINFull %s != %s", want, got)
	}
}
