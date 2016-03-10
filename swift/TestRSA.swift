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
//
//  Created by Michael Scott on 25/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation
import clint   // comment out for Xcode

public func TestRSA()
{
    let RFS=RSA.RFS;

    var message="Hello World\n"

    let pub=rsa_public_key(ROM.FFLEN);
    let priv=rsa_private_key(ROM.HFLEN);

    var ML=[UInt8](count:RFS,repeatedValue:0)
    var C=[UInt8](count:RFS,repeatedValue:0)
    var RAW=[UInt8](count:100,repeatedValue:0)

    let rng=RAND()

    rng.clean();
    for var i=0;i<100;i++ {RAW[i]=UInt8(i)}

    rng.seed(100,RAW);

    print("Generating public/private key pair");
    RSA.KEY_PAIR(rng,65537,priv,pub);

    let M=[UInt8](message.utf8)
    print("Encrypting test string\n");
    let E=RSA.OAEP_ENCODE(M,rng,nil); /* OAEP encode message m to e  */

    RSA.ENCRYPT(pub,E,&C);     /* encrypt encoded message */
    print("Ciphertext= 0x", terminator: ""); RSA.printBinary(C);

    print("Decrypting test string\n");
    RSA.DECRYPT(priv,C,&ML);
    var MS=RSA.OAEP_DECODE(nil,&ML); /* OAEP encode message m to e  */

    message=""
    for var i=0;i<MS.count;i++
    {
        message+=String(UnicodeScalar(MS[i]))
    }
    print(message);

    RSA.PRIVATE_KEY_KILL(priv);
}

TestRSA() // comment out for Xcode



