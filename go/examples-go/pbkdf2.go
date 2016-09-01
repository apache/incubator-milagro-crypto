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

	amcl "git.apache.org/incubator-milagro-crypto.git/go/amcl-go"
)

func main() {
	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := amcl.NewRAND()
	rng.Seed(len(seed), seed)

	// Password / Pass-phrase
	passwordStr := "#!qwerty"
	password := []byte(passwordStr)
	fmt.Printf("password: %s \n", password)
	fmt.Printf("PASSWORD: 0x")
	amcl.MPIN_printBinary(password[:])

	// Salt
	salt := amcl.GENERATE_RANDOM(rng, 16)
	fmt.Printf("salt: 0x")
	amcl.MPIN_printBinary(salt[:])

	// Number of repetitions
	rep := 1000

	KEY := amcl.PBKDF2(password[:], salt[:], rep, amcl.MPIN_PAS)

	// Initialization vector
	IV := amcl.GENERATE_RANDOM(rng, 12)
	fmt.Printf("IV: 0x")
	amcl.MPIN_printBinary(IV[:])

	// header
	HEADER := amcl.GENERATE_RANDOM(rng, 16)
	fmt.Printf("HEADER: 0x")
	amcl.MPIN_printBinary(HEADER[:])

	// Input plaintext
	plaintextStr := "A test message"
	PLAINTEXT1 := []byte(plaintextStr)
	fmt.Printf("String to encrypt: %s \n", plaintextStr)
	fmt.Printf("PLAINTEXT1: 0x")
	amcl.MPIN_printBinary(PLAINTEXT1[:])

	// AES-GCM Encryption
	CIPHERTEXT, TAG1 := amcl.AES_GCM_ENCRYPT(KEY[:], IV[:], HEADER[:], PLAINTEXT1[:])
	fmt.Printf("CIPHERTEXT:  0x")
	amcl.MPIN_printBinary(CIPHERTEXT[:])
	fmt.Printf("TAG1:  0x")
	amcl.MPIN_printBinary(TAG1[:])

	// AES-GCM Decryption
	PLAINTEXT2, TAG1 := amcl.AES_GCM_DECRYPT(KEY[:], IV[:], HEADER[:], CIPHERTEXT[:])
	fmt.Printf("PLAINTEXT2:  0x")
	amcl.MPIN_printBinary(PLAINTEXT2[:])
	fmt.Printf("TAG1:  0x")
	amcl.MPIN_printBinary(TAG1[:])
	fmt.Printf("Decrypted string: %s \n", string(PLAINTEXT2))
}
