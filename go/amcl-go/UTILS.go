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

// Generate random six digit value
func GENERATE_OTP(rng *RAND) int {
	OTP := 0
	mult := 1
	for i := 0; i < 6; i++ {
		val := int(rng.GetByte())
		if val < 0 {
			val = -val
		}
		val = val % 10
		OTP = val*mult + OTP
		mult = mult * 10
	}
	return OTP
}

// Generate a random byte array
func GENERATE_RANDOM(rng *RAND, randomLen int) []byte {
	random := make([]byte, randomLen)
	for i := 0; i < randomLen; i++ {
		random[i] = rng.GetByte()
	}
	return random
}
