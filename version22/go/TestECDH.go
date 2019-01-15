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

/* test driver and function exerciser for ECDH/ECIES/ECDSA API Functions */

package main

import "fmt"

func ECDH_printBinary(array []byte) {
	for i:=0;i<len(array);i++ {
		fmt.Printf("%02x", array[i])
	}
	fmt.Printf("\n")
}  

func main() {

//	j:=0
	pp:="M0ng00se"
	res:=0

	var sha=ECDH_HASH_TYPE

	var S1 [ECDH_EGS]byte
	var W0 [2*ECDH_EFS+1]byte
	var W1 [2*ECDH_EFS+1]byte
	var Z0 [ECDH_EFS]byte
	var Z1 [ECDH_EFS]byte
	var RAW [100]byte
	var SALT [8]byte
	var P1 [3]byte
	var P2 [4]byte
	var V [2*ECDH_EFS+1]byte
	var M [17]byte
	var T [12]byte
	var CS [ECDH_EGS]byte
	var DS [ECDH_EGS]byte

	rng:=NewRAND()

	rng.Clean();
	for i:=0;i<100;i++ {RAW[i]=byte(i)}

	rng.Seed(100,RAW[:])

//for j:=0;j<100;j++ {

	for i:=0;i<8;i++ {SALT[i]=byte(i+1)}  // set Salt

	fmt.Printf("Alice's Passphrase= "+pp)
	fmt.Printf("\n");
	PW:=[]byte(pp)

/* private key S0 of size EGS bytes derived from Password and Salt */

	S0:=PBKDF2(sha,PW,SALT[:],1000,ECDH_EGS)

	fmt.Printf("Alice's private key= 0x")
	ECDH_printBinary(S0)

/* Generate Key pair S/W */
	ECDH_KEY_PAIR_GENERATE(nil,S0,W0[:])

	fmt.Printf("Alice's public key= 0x")
	ECDH_printBinary(W0[:]);

	res=ECDH_PUBLIC_KEY_VALIDATE(true,W0[:])
	if res!=0 {
		fmt.Printf("ECP Public Key is invalid!\n")
		return
	}

/* Random private key for other party */
	ECDH_KEY_PAIR_GENERATE(rng,S1[:],W1[:])

	fmt.Printf("Servers private key= 0x");
	ECDH_printBinary(S1[:])

	fmt.Printf("Servers public key= 0x")
	ECDH_printBinary(W1[:])


	res=ECDH_PUBLIC_KEY_VALIDATE(true,W1[:])
	if res!=0 {
		fmt.Printf("ECP Public Key is invalid!\n")
		return
	}
/* Calculate common key using DH - IEEE 1363 method */

	ECPSVDP_DH(S0,W1[:],Z0[:])
	ECPSVDP_DH(S1[:],W0[:],Z1[:])

	same:=true
	for i:=0;i<ECDH_EFS;i++ {
		if Z0[i]!=Z1[i] {same=false}
	}

	if !same {
		fmt.Printf("*** ECPSVDP-DH Failed\n");
		return
	}

	KEY:=KDF2(sha,Z0[:],nil,ECDH_EAS);

	fmt.Printf("Alice's DH Key=  0x"); ECDH_printBinary(KEY)
	fmt.Printf("Servers DH Key=  0x"); ECDH_printBinary(KEY)
	
	if CURVETYPE!=MONTGOMERY {
		fmt.Printf("Testing ECIES\n");

		P1[0]=0x0; P1[1]=0x1; P1[2]=0x2
		P2[0]=0x0; P2[1]=0x1; P2[2]=0x2; P2[3]=0x3

		for i:=0;i<=16;i++ {M[i]=byte(i)} 

		C:=ECIES_ENCRYPT(sha,P1[:],P2[:],rng,W1[:],M[:],V[:],T[:])

		fmt.Printf("Ciphertext= \n")
		fmt.Printf("V= 0x"); ECDH_printBinary(V[:])
		fmt.Printf("C= 0x"); ECDH_printBinary(C)
		fmt.Printf("T= 0x"); ECDH_printBinary(T[:])


		RM:=ECIES_DECRYPT(sha,P1[:],P2[:],V[:],C,T[:],S1[:])
		if RM==nil {
			fmt.Printf("*** ECIES Decryption Failed\n")
			return
		} else {fmt.Printf("Decryption succeeded\n")}

		fmt.Printf("Message is 0x"); ECDH_printBinary(RM)

		fmt.Printf("Testing ECDSA\n");

		if ECPSP_DSA(sha,rng,S0,M[:],CS[:],DS[:])!=0 {
			fmt.Printf("***ECDSA Signature Failed\n")
			return
		}
		fmt.Printf("Signature= \n")
		fmt.Printf("C= 0x"); ECDH_printBinary(CS[:])
		fmt.Printf("D= 0x"); ECDH_printBinary(DS[:])

		if ECPVP_DSA(sha,W0[:],M[:],CS[:],DS[:])!=0 {
			fmt.Printf("***ECDSA Verification Failed\n")
			return
		} else {fmt.Printf("ECDSA Signature/Verification succeeded \n")}
	}
}
