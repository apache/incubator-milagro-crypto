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
	mathrand "math/rand"
	"testing"

	"github.com/stretchr/testify/assert"
)

const nIter int = 1000

func TestCryptoGoodPIN(t *testing.T) {
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoBadPIN(t *testing.T) {
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	//////   Client   //////

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	//////   Server   //////
	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoBadToken(t *testing.T) {
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, _, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	// Send UT as V to model bad token
	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], UT[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoRandom(t *testing.T) {
	want := 0

	for i := 0; i < nIter; i++ {

		// Seed value for Random Number Generator (RNG)
		seed := make([]byte, 16)
		rand.Read(seed)
		rng := NewRAND()
		rng.Seed(len(seed), seed)

		// Epoch time in days
		date := MPIN_today()

		// Epoch time in seconds
		timeValue := MPIN_GET_TIME()

		// PIN variable to create token
		PIN1 := mathrand.Intn(10000)
		// PIN variable to authenticate
		PIN2 := PIN1

		// Assign the End-User a random ID
		ID := make([]byte, 16)
		rand.Read(ID)

		// Message to sign
		var MESSAGE []byte

		// Generate Master Secret Share 1
		_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Generate Master Secret Share 2
		_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Either Client or TA calculates Hash(ID)
		HCID := MPIN_HASH_ID(ID)

		// Generate server secret share 1
		_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

		// Generate server secret share 2
		_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

		// Combine server secret shares
		_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

		// Generate client secret share 1
		_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

		// Generate client secret share 2
		_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

		// Combine client secret shares
		CS := make([]byte, G1S)
		_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

		// Generate time permit share 1
		_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

		// Generate time permit share 2
		_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

		// Combine time permit shares
		_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

		// Create token
		_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

		// Send U, UT, V, timeValue and Message to server
		var X [EGS]byte
		_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

		got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
		assert.Equal(t, want, got, "Should be equal")
	}
}

func TestCryptoGoodSignature(t *testing.T) {
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	// Authenticate
	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoSignatureExpired(t *testing.T) {
	want := -19
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	timeValue += 10
	// Authenticate
	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoBadSignature(t *testing.T) {
	want := -19
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	// Authenticate
	MESSAGE[0] = 00
	got, _, _, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoPINError(t *testing.T) {
	want := 1
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, _, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	_, _, _, _, E, F := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])

	got := MPIN_KANGAROO(E[:], F[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoMPINFull(t *testing.T) {
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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Precomputation
	_, G1, G2 := MPIN_PRECOMPUTE_WRAP(TOKEN[:], HCID)

	// Send U, UT, V, timeValue and Message to server
	var X [EGS]byte
	_, XOut, _, V, U, UT := MPIN_CLIENT_WRAP(date, timeValue, PIN2, rng, ID[:], X[:], TOKEN[:], TP[:], MESSAGE[:])

	// Send Z=r.ID to Server
	var R [EGS]byte
	_, ROut, Z := MPIN_GET_G1_MULTIPLE_WRAP(rng, 1, R[:], HCID[:])

	// Authenticate
	_, _, HTID, _, _, _ := MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])

	// send T=w.ID to client
	var W [EGS]byte
	_, WOut, T := MPIN_GET_G1_MULTIPLE_WRAP(rng, 0, W[:], HTID[:])

	_, AES_KEY_SERVER := MPIN_SERVER_KEY_WRAP(Z[:], SS[:], WOut[:], U[:], UT[:])
	got := hex.EncodeToString(AES_KEY_SERVER[:])
	if got != want {
		t.Errorf("%s != %s", want, got)
	}

	_, AES_KEY_CLIENT := MPIN_CLIENT_KEY_WRAP(PIN2, G1[:], G2[:], ROut[:], XOut[:], T[:])
	got = hex.EncodeToString(AES_KEY_CLIENT[:])
	assert.Equal(t, want, got, "Should be equal")
}

// Subtract a 256 bit PIN
func TestCrypoSubBigPIN(t *testing.T) {
	want := "042182235070802ebc33633e70e6628f48fd896e86dfc40c81227caa2792367a581d461dbba6efa30896c71f427df335885142cc6fb64ba082ff9573b9276475c0"

	IDHex := "7465737455736572406365727469766f782e636f6d"
	ID, err := hex.DecodeString(IDHex)
	assert.Equal(t, nil, err, "Should be equal")

	TOKENHex := "0422a522b5c05d06cde3a65872656ab596e111c4ea7c0c349bac26f0bdaf7d5f0a1ea8a0cab99d06677cfbc3c8d667e7b0af33b9ed4df007b0ccc8c2b77353bbe6"
	TOKEN, err := hex.DecodeString(TOKENHex)
	assert.Equal(t, nil, err, "Should be equal")

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	assert.Equal(t, nil, err, "Should be equal")
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Generate big PIN - 256 bits
	errorCode, PIN := MPIN_RANDOM_GENERATE_WRAP(rng)
	assert.Equal(t, 0, errorCode, "Should be equal")

	// Extract big PIN
	errorCode, TK := MPIN_EXTRACT_BIG_PIN_WRAP(ID[:], PIN[:], TOKEN[:])
	assert.Equal(t, 0, errorCode, "Should be equal")
	got := hex.EncodeToString(TK[:])
	assert.Equal(t, want, got, "Should be equal")
}

// Add a 256 bit PIN
func TestCrypoAddBigPIN(t *testing.T) {
	want := "0422a522b5c05d06cde3a65872656ab596e111c4ea7c0c349bac26f0bdaf7d5f0a1ea8a0cab99d06677cfbc3c8d667e7b0af33b9ed4df007b0ccc8c2b77353bbe6"

	IDHex := "7465737455736572406365727469766f782e636f6d"
	ID, err := hex.DecodeString(IDHex)
	assert.Equal(t, nil, err, "Should be equal")

	TOKENHex := "042182235070802ebc33633e70e6628f48fd896e86dfc40c81227caa2792367a581d461dbba6efa30896c71f427df335885142cc6fb64ba082ff9573b9276475c0"
	TOKEN, err := hex.DecodeString(TOKENHex)
	assert.Equal(t, nil, err, "Should be equal")

	PINHex := "1b18b8b882daf76a18bf2278fe4e15c62eed8131e708573375fd81a8415014b3"
	PIN, err := hex.DecodeString(PINHex)
	assert.Equal(t, nil, err, "Should be equal")

	// Extract big PIN
	errorCode, TK := MPIN_ADD_BIG_PIN_WRAP(ID[:], PIN[:], TOKEN[:])
	assert.Equal(t, 0, errorCode, "Should be equal")
	got := hex.EncodeToString(TK[:])
	assert.Equal(t, want, got, "Should be equal")
}

// Split key
func TestCryptoSplitKey(t *testing.T) {
	want := "64b36b7a0395e61350de8839adb019d5ae2134052b8533e7c4bbab3965e0af1b"

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	assert.Equal(t, nil, err, "Should be equal")
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Generate big PIN - 256 bits
	errorCode, PIN := MPIN_RANDOM_GENERATE_WRAP(rng)
	assert.Equal(t, 0, errorCode, "Should be equal")
	PINHex := hex.EncodeToString(PIN[:])
	PINGoldHex := "1b18b8b882daf76a18bf2278fe4e15c62eed8131e708573375fd81a8415014b3"
	assert.Equal(t, PINGoldHex, PINHex, "Should be equal")

	n := len(PIN)
	// Split key by C = PIN ^ A ^ B
	A := GENERATE_RANDOM(rng, n)

	B := GENERATE_RANDOM(rng, n)

	C, errorCode := XORBytes(PIN[:], A[:], B[:])
	assert.Equal(t, 0, errorCode, "Should be equal")
	got := hex.EncodeToString(C[:])
	assert.Equal(t, want, got, "Should be equal")
}

// Combine key shares
func TestCryptoCombineKey(t *testing.T) {
	want := "1b18b8b882daf76a18bf2278fe4e15c62eed8131e708573375fd81a8415014b3"

	CHex := "64b36b7a0395e61350de8839adb019d5ae2134052b8533e7c4bbab3965e0af1b"
	C, err := hex.DecodeString(CHex)
	assert.Equal(t, nil, err, "Should be equal")

	AHex := "c5add1327790087193ae541acd6dc3264c19a12afaf196291d0820c611d3fcd4"
	A, err := hex.DecodeString(AHex)
	assert.Equal(t, nil, err, "Should be equal")

	BHex := "ba0602f0f6df1908dbcffe5b9e93cf35ccd5141e367cf2fdac4e0a573563477c"
	B, err := hex.DecodeString(BHex)
	assert.Equal(t, nil, err, "Should be equal")

	// Combine key shares PIN = A ^ B ^ C
	PIN, errorCode := XORBytes(C[:], A[:], B[:])
	assert.Equal(t, 0, errorCode, "Should be equal")
	got := hex.EncodeToString(PIN[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoTwoPassGoodPIN(t *testing.T) {
	want := 0
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Client Pass 1
	var X [EGS]byte
	_, _, SEC, U, UT := MPIN_CLIENT_1_WRAP(date, ID, rng, X[:], PIN2, TOKEN[:], TP[:])

	// Server Pass 1
	HID, HTID := MPIN_SERVER_1_WRAP(date, ID)
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoTwoPassBadPIN(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Client Pass 1
	var X [EGS]byte
	_, _, SEC, U, UT := MPIN_CLIENT_1_WRAP(date, ID, rng, X[:], PIN2, TOKEN[:], TP[:])

	// Server Pass 1
	HID, HTID := MPIN_SERVER_1_WRAP(date, ID)
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoTwoPassBadToken(t *testing.T) {
	want := -19
	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16660

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

	// Generate Master Secret Share 1
	_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Generate Master Secret Share 2
	_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Either Client or TA calculates Hash(ID)
	HCID := MPIN_HASH_ID(ID)

	// Generate server secret share 1
	_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

	// Generate server secret share 2
	_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

	// Combine server secret shares
	_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

	// Generate client secret share 1
	_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

	// Generate client secret share 2
	_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

	// Combine client secret shares
	CS := make([]byte, G1S)
	_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

	// Generate time permit share 1
	_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

	// Generate time permit share 2
	_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

	// Combine time permit shares
	_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

	// Create token
	_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

	// Client Pass 1
	var X [EGS]byte
	_, _, SEC, U, UT := MPIN_CLIENT_1_WRAP(date, ID, rng, X[:], PIN2, TOKEN[:], TP[:])

	// Server Pass 1
	HID, HTID := MPIN_SERVER_1_WRAP(date, ID)
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, _ = MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	// Send UT as V to model bad token
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], UT[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestCryptoRandomTwoPass(t *testing.T) {
	want := 0

	for i := 0; i < nIter; i++ {

		// Seed value for Random Number Generator (RNG)
		seed := make([]byte, 16)
		rand.Read(seed)
		rng := NewRAND()
		rng.Seed(len(seed), seed)

		// Epoch time in days
		date := MPIN_today()

		// PIN variable to create token
		PIN1 := mathrand.Intn(10000)
		// PIN variable to authenticate
		PIN2 := PIN1

		// Assign the End-User a random ID
		ID := make([]byte, 16)
		rand.Read(ID)

		// Generate Master Secret Share 1
		_, MS1 := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Generate Master Secret Share 2
		_, MS2 := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Either Client or TA calculates Hash(ID)
		HCID := MPIN_HASH_ID(ID)

		// Generate server secret share 1
		_, SS1 := MPIN_GET_SERVER_SECRET_WRAP(MS1[:])

		// Generate server secret share 2
		_, SS2 := MPIN_GET_SERVER_SECRET_WRAP(MS2[:])

		// Combine server secret shares
		_, SS := MPIN_RECOMBINE_G2_WRAP(SS1[:], SS2[:])

		// Generate client secret share 1
		_, CS1 := MPIN_GET_CLIENT_SECRET_WRAP(MS1[:], HCID)

		// Generate client secret share 2
		_, CS2 := MPIN_GET_CLIENT_SECRET_WRAP(MS2[:], HCID)

		// Combine client secret shares
		CS := make([]byte, G1S)
		_, CS = MPIN_RECOMBINE_G1_WRAP(CS1[:], CS2[:])

		// Generate time permit share 1
		_, TP1 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS1[:], HCID)

		// Generate time permit share 2
		_, TP2 := MPIN_GET_CLIENT_PERMIT_WRAP(date, MS2[:], HCID)

		// Combine time permit shares
		_, TP := MPIN_RECOMBINE_G1_WRAP(TP1[:], TP2[:])

		// Create token
		_, TOKEN := MPIN_EXTRACT_PIN_WRAP(ID[:], PIN1, CS[:])

		// Client Pass 1
		var X [EGS]byte
		_, _, SEC, U, UT := MPIN_CLIENT_1_WRAP(date, ID, rng, X[:], PIN2, TOKEN[:], TP[:])

		// Server Pass 1
		HID, HTID := MPIN_SERVER_1_WRAP(date, ID)
		_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Client Pass 2
		_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

		// Server Pass 2
		got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
		assert.Equal(t, want, got, "Should be equal")

	}
}
