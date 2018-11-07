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

//
//  TestECDH.swift
//
//  Created by Michael Scott on 02/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation
//import amcl // comment out for Xcode

public func TestECDH()
{
    let pp=String("M0ng00se");
    
    let EGS=ECDH.EGS
    let EFS=ECDH.EFS
    let EAS=16
    let sha=ECDH.HASH_TYPE
    
    var S1=[UInt8](repeating: 0,count: EGS)
    var W0=[UInt8](repeating: 0,count: 2*EFS+1)
    var W1=[UInt8](repeating: 0,count: 2*EFS+1)
    var Z0=[UInt8](repeating: 0,count: EFS)
    var Z1=[UInt8](repeating: 0,count: EFS)
    var RAW=[UInt8](repeating: 0,count: 100)
    var SALT=[UInt8](repeating: 0,count: 8)
    var P1=[UInt8](repeating: 0,count: 3)
    var P2=[UInt8](repeating: 0,count: 4)
    var V=[UInt8](repeating: 0,count: 2*EFS+1)
    var M=[UInt8](repeating: 0,count: 17)
    var T=[UInt8](repeating: 0,count: 12)
    var CS=[UInt8](repeating: 0,count: EGS)
    var DS=[UInt8](repeating: 0,count: EGS)
    
    let rng=RAND()
    
    rng.clean();
    for i in 0 ..< 100 {RAW[i]=UInt8(i&0xff)}
    
    rng.seed(100,RAW)
    
    
    for i in 0 ..< 8 {SALT[i]=UInt8(i+1)}  // set Salt
    
    print("Alice's Passphrase= " + pp!)
    let PW=[UInt8]( (!pp).utf8)
    
    /* private key S0 of size EGS bytes derived from Password and Salt */
    
    var S0=ECDH.PBKDF2(sha,PW,SALT,1000,EGS)
    print("Alice's private key= 0x",terminator: ""); ECDH.printBinary(S0)
    
    /* Generate Key pair S/W */
    ECDH.KEY_PAIR_GENERATE(nil,&S0,&W0);
    
    print("Alice's public key= 0x",terminator: ""); ECDH.printBinary(W0)
    
    var res=ECDH.PUBLIC_KEY_VALIDATE(true,W0);

    if res != 0
    {
        print("ECP Public Key is invalid!");
        return;
    }
    
    /* Random private key for other party */
    ECDH.KEY_PAIR_GENERATE(rng,&S1,&W1)
    
    print("Servers private key= 0x",terminator: ""); ECDH.printBinary(S1)
    
    print("Servers public key= 0x",terminator: ""); ECDH.printBinary(W1);
    
    res=ECDH.PUBLIC_KEY_VALIDATE(true,W1)
    if res != 0
    {
        print("ECP Public Key is invalid!")
        return
    }
    
    /* Calculate common key using DH - IEEE 1363 method */
    
    ECDH.ECPSVDP_DH(S0,W1,&Z0)
    ECDH.ECPSVDP_DH(S1,W0,&Z1)
    
    var same=true
    for i in 0 ..< EFS
    {
        if Z0[i] != Z1[i] {same=false}
    }
    
    if (!same)
    {
        print("*** ECPSVDP-DH Failed")
        return
    }
    
    let KEY=ECDH.KDF2(sha,Z0,nil,EAS)
    
    print("Alice's DH Key=  0x",terminator: ""); ECDH.printBinary(KEY)
    print("Servers DH Key=  0x",terminator: ""); ECDH.printBinary(KEY)

    if ROM.CURVETYPE != ROM.MONTGOMERY
    {
        print("Testing ECIES")
    
        P1[0]=0x0; P1[1]=0x1; P1[2]=0x2
        P2[0]=0x0; P2[1]=0x1; P2[2]=0x2; P2[3]=0x3
    
        for i in 0...16 {M[i]=UInt8(i&0xff)}
    
        let C=ECDH.ECIES_ENCRYPT(sha,P1,P2,rng,W1,M,&V,&T)

        print("Ciphertext= ")
        print("V= 0x",terminator: ""); ECDH.printBinary(V)
        print("C= 0x",terminator: ""); ECDH.printBinary(C)
        print("T= 0x",terminator: ""); ECDH.printBinary(T)

        M=ECDH.ECIES_DECRYPT(sha,P1,P2,V,C,T,S1)
        if M.count==0
        {
            print("*** ECIES Decryption Failed\n")
            return
        }
        else {print("Decryption succeeded")}
    
        print("Message is 0x"); ECDH.printBinary(M)
    
        print("Testing ECDSA")

        if ECDH.ECPSP_DSA(sha,rng,S0,M,&CS,&DS) != 0
        {
            print("***ECDSA Signature Failed")
            return
        }
        print("Signature= ")
        print("C= 0x",terminator: ""); ECDH.printBinary(CS)
        print("D= 0x",terminator: ""); ECDH.printBinary(DS)
    
        if ECDH.ECPVP_DSA(sha,W0,M,CS,DS) != 0
        {
            print("***ECDSA Verification Failed")
            return
        }
        else {print("ECDSA Signature/Verification succeeded ")}
    }
}

//TestECDH()  // comment out for Xcode
