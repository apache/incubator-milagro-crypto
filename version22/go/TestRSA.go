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

/* test driver and function exerciser for RSA API Functions */

package main

import "fmt"

func RSA_printBinary(array []byte) {
	for i:=0;i<len(array);i++ {
		fmt.Printf("%02x", array[i])
	}
	fmt.Printf("\n")
}  

func main() {

	var sha=RSA_HASH_TYPE
	message:="Hello World\n"

	pub:=New_rsa_public_key(FFLEN)
	priv:=New_rsa_private_key(HFLEN)

	var ML [RSA_RFS]byte
	var C [RSA_RFS]byte
	var S [RSA_RFS]byte
	var RAW [100]byte
	
	rng:=NewRAND()

	rng.Clean();
	for i:=0;i<100;i++ {RAW[i]=byte(i)}

	rng.Seed(100,RAW[:]);
//for (i=0;i<10;i++)
//{
	fmt.Printf("Generating public/private key pair\n")
	RSA_KEY_PAIR(rng,65537,priv,pub)

	M:=[]byte(message)

	fmt.Printf("Encrypting test string\n")
	E:=RSA_OAEP_ENCODE(sha,M,rng,nil) /* OAEP encode message M to E  */

	RSA_ENCRYPT(pub,E,C[:])    /* encrypt encoded message */
	fmt.Printf("Ciphertext= 0x"); RSA_printBinary(C[:])

	fmt.Printf("Decrypting test string\n");
	RSA_DECRYPT(priv,C[:],ML[:])
	MS:=RSA_OAEP_DECODE(sha,nil,ML[:]) /* OAEP decode message  */

	message=string(MS)
	fmt.Printf(message)

	fmt.Printf("Signing message\n")
	PKCS15(sha,M,C[:]); 

	RSA_DECRYPT(priv,C[:],S[:])  /* create signature in S */ 

	fmt.Printf("Signature= 0x"); RSA_printBinary(S[:])

	RSA_ENCRYPT(pub,S[:],ML[:])

	cmp:=true
	if len(C)!=len(ML) {
		cmp=false
	} else {
		for j:=0;j<len(C);j++ {
			if C[j]!=ML[j] {cmp=false}
		}
	}
	if cmp {
		fmt.Printf("Signature is valid")
	} else {
		fmt.Printf("Signature is INVALID")
	}


//}
	RSA_PRIVATE_KEY_KILL(priv)
}
