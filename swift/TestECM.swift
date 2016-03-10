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
//  TestECM.swift
//  
//
//  Created by Michael Scott on 03/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation

func TestECM()
{
    let pp=String("M0ng00se");

    let EGS=ECDH.EGS
    let EFS=ECDH.EFS
    let EAS=AES.KS

    var S1=[UInt8](count:EGS,repeatedValue:0)
    var W0=[UInt8](count:2*EFS+1,repeatedValue:0)
    var W1=[UInt8](count:2*EFS+1,repeatedValue:0)
    var Z0=[UInt8](count:EFS,repeatedValue:0)
    var Z1=[UInt8](count:EFS,repeatedValue:0)
    var RAW=[UInt8](count:100,repeatedValue:0)
    var SALT=[UInt8](count:8,repeatedValue:0)

    let rng=RAND()

    rng.clean();
    for var i=0;i<100;i++ {RAW[i]=UInt8(i&0xff)}

    rng.seed(100,RAW)


    for var i=0;i<8;i++ {SALT[i]=UInt8(i+1)}  // set Salt

    print("Alice's Passphrase= "+pp)
    let PW=[UInt8](pp.utf8)

    /* private key S0 of size EGS bytes derived from Password and Salt */

    var S0=ECDH.PBKDF2(PW,SALT,1000,EGS)
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
    for var i=0;i<EFS;i++
    {
        if Z0[i] != Z1[i] {same=false}
    }

    if (!same)
    {
        print("*** ECPSVDP-DH Failed")
        return
    }

    let KEY=ECDH.KDF1(Z0,EAS)

    print("Alice's DH Key=  0x",terminator: ""); ECDH.printBinary(KEY)
    print("Servers DH Key=  0x",terminator: ""); ECDH.printBinary(KEY)

}
