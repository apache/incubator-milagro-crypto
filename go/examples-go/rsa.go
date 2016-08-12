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

/* RSA API high-level functions  */

package main

import (
	"fmt"

	amcl "git.apache.org/incubator-milagro-crypto.git/go/amcl-go"
)

func main() {

	message := "Hello World\n"

	pub := amcl.New_rsa_public_key(amcl.FFLEN)
	priv := amcl.New_rsa_private_key(amcl.HFLEN)

	var ML [amcl.RSA_RFS]byte
	var C [amcl.RSA_RFS]byte
	var RAW [100]byte

	rng := amcl.NewRAND()

	rng.Clean()
	for i := 0; i < 100; i++ {
		RAW[i] = byte(i)
	}

	rng.Seed(100, RAW[:])
	//for (i=0;i<10;i++)
	//{
	fmt.Printf("Generating public/private key pair\n")
	amcl.RSA_KEY_PAIR(rng, 65537, priv, pub)

	M := []byte(message)

	fmt.Printf("Encrypting test string\n")
	E := amcl.RSA_OAEP_ENCODE(M, rng, nil) /* OAEP encode message M to E  */

	amcl.RSA_ENCRYPT(pub, E, C[:]) /* encrypt encoded message */
	fmt.Printf("Ciphertext= 0x")
	amcl.RSA_printBinary(C[:])

	fmt.Printf("Decrypting test string\n")
	amcl.RSA_DECRYPT(priv, C[:], ML[:])
	MS := amcl.RSA_OAEP_DECODE(nil, ML[:]) /* OAEP decode message  */

	message = string(MS)
	fmt.Printf(message)
	//}
	amcl.RSA_PRIVATE_KEY_KILL(priv)
}
