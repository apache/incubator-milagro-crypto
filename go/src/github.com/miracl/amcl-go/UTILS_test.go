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
	"encoding/hex"
	"fmt"
	"testing"
)

func TestGENERATE_OTP(t *testing.T) {
	cases := []int{751847, 625436, 628111, 611804, 148564, 202193, 794783, 631944, 544480, 384313}

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Generate the one time passwords
	for _, want := range cases {
		got := GENERATE_OTP(rng)
		if got != want {
			t.Errorf("One Time Passord %d != %d", got, want)
		}
	}
}

func TestGENERATE_RANDOM(t *testing.T) {
	cases := []string{"57d662d39b1b245da469e89c", "155babf8de4204e68a656f42", "727e1980e01f996d977a0a34", "7b6c39221d89546895153f10", "32e40e9ad6f50dab3f5ec63f", "f6962a1fc5add13277900871", "93ae541acd6dc3264c19a12a", "faf196291d0820c611d3fcd4", "ba0602f0f6df1908dbcffe5b", "9e93cf35ccd5141e367cf2fd"}

	// Seed value for Random Number Generator (RNG)
	seedHex := "9e8b4178790cd57a5761c4a6f164ba72"
	seed, err := hex.DecodeString(seedHex)
	if err != nil {
		fmt.Println("Error decoding seed value")
		return
	}
	rng := NewRAND()
	rng.Seed(len(seed), seed)

	// Generate the one time passwords
	for _, want := range cases {
		val := GENERATE_RANDOM(rng, 12)
		got := hex.EncodeToString(val)
		if got != want {
			t.Errorf("One Time Passord %s != %s", got, want)
		}
	}
}
