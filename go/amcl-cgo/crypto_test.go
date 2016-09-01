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

	amclgo "git.apache.org/incubator-milagro-crypto.git/go/amcl-go"
)

const nIter int = 100

// Set to true if library built with "-D USE_ANONYMOUS=on"
const USE_ANONYMOUS = false

func TestGoodPIN(t *testing.T) {
	want := 0
	var got int

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
	rng := amclgo.NewRAND()
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

	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestBadPIN(t *testing.T) {
	want := -19
	var got int

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
	rng := amclgo.NewRAND()
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
	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestBadToken(t *testing.T) {
	want := -19
	var got int

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
	rng := amclgo.NewRAND()
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
	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], UT[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], UT[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestRandom(t *testing.T) {
	want := 0
	var got int

	for i := 0; i < nIter; i++ {

		// Seed value for Random Number Generator (RNG)
		seed := make([]byte, 16)
		rand.Read(seed)
		rng := amclgo.NewRAND()
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

		if USE_ANONYMOUS {
			got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
		} else {
			got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
		}
		assert.Equal(t, want, got, "Should be equal")
	}
}

func TestGoodSignature(t *testing.T) {
	want := 0
	var got int

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
	rng := amclgo.NewRAND()
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
	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestSignatureExpired(t *testing.T) {
	want := -19
	var got int

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
	rng := amclgo.NewRAND()
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
	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestBadSignature(t *testing.T) {
	want := -19
	var got int

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
	rng := amclgo.NewRAND()
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
	if USE_ANONYMOUS {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		got, _, _, _, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}
	assert.Equal(t, want, got, "Should be equal")
}

func TestPINError(t *testing.T) {
	want := 1001
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
	PIN2 := 2235

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := amclgo.NewRAND()
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

	var E []byte
	var F []byte
	if USE_ANONYMOUS {
		_, _, _, _, E, F = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		_, _, _, _, E, F = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}

	got := MPIN_KANGAROO(E[:], F[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestMPINFull(t *testing.T) {
	want := "4e0317c9962dc2944c121ec41c800e16"
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
	rng := amclgo.NewRAND()
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
	var HID []byte
	var HTID []byte
	var Y []byte
	if USE_ANONYMOUS {
		_, HID, HTID, Y, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], HCID[:], MESSAGE[:])
	} else {
		_, HID, HTID, Y, _, _ = MPIN_SERVER_WRAP(date, timeValue, SS[:], U[:], UT[:], V[:], ID[:], MESSAGE[:])
	}

	// send T=w.ID to client
	var W [EGS]byte
	_, WOut, T := MPIN_GET_G1_MULTIPLE_WRAP(rng, 0, W[:], HTID[:])

	// Hash all values
	HM := MPIN_HASH_ALL_WRAP(HCID[:], U[:], UT[:], Y[:], V[:], Z[:], T[:])

	_, AES_KEY_SERVER := MPIN_SERVER_KEY_WRAP(Z[:], SS[:], WOut[:], HM[:], HID[:], U[:], UT[:])
	got := hex.EncodeToString(AES_KEY_SERVER[:])
	if got != want {
		t.Errorf("%s != %s", want, got)
	}

	_, AES_KEY_CLIENT := MPIN_CLIENT_KEY_WRAP(PIN2, G1[:], G2[:], ROut[:], XOut[:], HM[:], T[:])
	got = hex.EncodeToString(AES_KEY_CLIENT[:])

	assert.Equal(t, want, got, "Should be equal")
}

func TestTwoPassGoodPIN(t *testing.T) {
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
	rng := amclgo.NewRAND()
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
	var HID []byte
	var HTID []byte
	if USE_ANONYMOUS {
		HID, HTID = MPIN_SERVER_1_WRAP(date, HCID)
	} else {
		HID, HTID = MPIN_SERVER_1_WRAP(date, ID)
	}
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestTwoPassBadPIN(t *testing.T) {
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
	rng := amclgo.NewRAND()
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
	var HID []byte
	var HTID []byte
	if USE_ANONYMOUS {
		HID, HTID = MPIN_SERVER_1_WRAP(date, HCID)
	} else {
		HID, HTID = MPIN_SERVER_1_WRAP(date, ID)
	}
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestTwoPassBadToken(t *testing.T) {
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
	rng := amclgo.NewRAND()
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
	var HID []byte
	var HTID []byte
	if USE_ANONYMOUS {
		HID, HTID = MPIN_SERVER_1_WRAP(date, HCID)
	} else {
		HID, HTID = MPIN_SERVER_1_WRAP(date, ID)
	}
	_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

	// Client Pass 2
	_, _ = MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

	// Server Pass 2
	// Send UT as V to model bad token
	got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], UT[:])
	assert.Equal(t, want, got, "Should be equal")
}

func TestRandomTwoPass(t *testing.T) {
	want := 0

	for i := 0; i < nIter; i++ {

		// Seed value for Random Number Generator (RNG)
		seed := make([]byte, 16)
		rand.Read(seed)
		rng := amclgo.NewRAND()
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
		var HID []byte
		var HTID []byte
		if USE_ANONYMOUS {
			HID, HTID = MPIN_SERVER_1_WRAP(date, HCID)
		} else {
			HID, HTID = MPIN_SERVER_1_WRAP(date, ID)
		}
		_, Y := MPIN_RANDOM_GENERATE_WRAP(rng)

		// Client Pass 2
		_, V := MPIN_CLIENT_2_WRAP(X[:], Y[:], SEC[:])

		// Server Pass 2
		got, _, _ := MPIN_SERVER_2_WRAP(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:])
		assert.Equal(t, want, got, "Should be equal")

	}
}
