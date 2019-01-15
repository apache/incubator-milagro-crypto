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
//  TestRSA.swift
//
//  Created by Michael Scott on 25/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation
//import amcl   // comment out for Xcode

public func TestRSA()
{
    let RFS=RSA.RFS

    var message="Hello World\n"

    let pub=rsa_public_key(Int(ROM.FFLEN))
    let priv=rsa_private_key(Int(ROM.HFLEN))

    var ML=[UInt8](repeating: 0,count: RFS)
    var C=[UInt8](repeating: 0,count: RFS)
    var S=[UInt8](repeating: 0,count: RFS)
    var RAW=[UInt8](repeating: 0,count: 100)

    let rng=RAND()

    rng.clean()
    for i in 0 ..< 100 {RAW[i]=UInt8(i)}

    rng.seed(100,RAW)

    print("Generating public/private key pair")
    RSA.KEY_PAIR(rng,65537,priv,pub)

    let M=[UInt8](message.utf8)
    print("Encrypting test string\n");
    let E=RSA.OAEP_ENCODE(RSA.HASH_TYPE,M,rng,nil); /* OAEP encode message m to e  */

    RSA.ENCRYPT(pub,E,&C);     /* encrypt encoded message */
    print("Ciphertext= 0x", terminator: ""); RSA.printBinary(C)

    print("Decrypting test string\n");
    RSA.DECRYPT(priv,C,&ML)
    var MS=RSA.OAEP_DECODE(RSA.HASH_TYPE,nil,&ML) /* OAEP encode message m to e  */

    message=""
    for i in 0 ..< MS.count
    {
        message+=String(UnicodeScalar(MS[i]))
    }
    print(message);
    
    print("Signing message")
    RSA.PKCS15(RSA.HASH_TYPE,M,&C)
    
    RSA.DECRYPT(priv,C,&S); //  create signature in S
    print("Signature= 0x",terminator: ""); RSA.printBinary(S)
    
    RSA.ENCRYPT(pub,S,&ML);
    
    var cmp=true
    if C.count != ML.count {cmp=false}
    else
    {
        for j in 0 ..< C.count
        {
            if C[j] != ML[j] {cmp=false}
        }
    }
    
    if cmp {print("\nSignature is valid\n")}
    else {print("\nSignature is INVALID\n")}
    

    RSA.PRIVATE_KEY_KILL(priv);
}

//TestRSA() // comment out for Xcode



