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

public func BenchtestEC()
{
    let pub=rsa_public_key(Int(ROM.FFLEN))
    let priv=rsa_private_key(Int(ROM.HFLEN))
    var C=[UInt8](repeating: 0,count: RSA.RFS)
    var P=[UInt8](repeating: 0,count: RSA.RFS)
    var M=[UInt8](repeating: 0,count: RSA.RFS)
    let MIN_TIME=10.0
    let MIN_ITERS=10
    
    var fail=false;
    var RAW=[UInt8](repeating: 0,count: 100)
    
    let rng=RAND()
    rng.clean();
    for i in 0 ..< 100 {RAW[i]=UInt8(i&0xff)}
    
    rng.seed(100,RAW)
    
    if ROM.CURVETYPE==ROM.WEIERSTRASS {
        print("Weierstrass parameterisation")
    }
    if ROM.CURVETYPE==ROM.EDWARDS {
        print("Edwards parameterisation")
    }
    if ROM.CURVETYPE==ROM.MONTGOMERY {
        print("Montgomery representation")
    }
    if ROM.MODTYPE==ROM.PSEUDO_MERSENNE {
        print("Pseudo-Mersenne Modulus")
    }
    if ROM.MODTYPE==ROM.MONTGOMERY_FRIENDLY {
        print("Montgomery Friendly Modulus")
    }
    if ROM.MODTYPE==ROM.GENERALISED_MERSENNE {
        print("Generalised-Mersenne Modulus")
    }
    if ROM.MODTYPE==ROM.NOT_SPECIAL {
        print("Not special Modulus")
    }
    print("Modulus size \(ROM.MODBITS) bits")
    print("\(ROM.CHUNK) bit build")
    
    let gx=BIG(ROM.CURVE_Gx);
    var s:BIG
    var G:ECP
    if ROM.CURVETYPE != ROM.MONTGOMERY
    {
        let gy=BIG(ROM.CURVE_Gy)
        G=ECP(gx,gy)
    }
    else
        {G=ECP(gx)}
    
    let r=BIG(ROM.CURVE_Order)
    s=BIG.randomnum(r,rng)
    
    var W=G.mul(r)
    if !W.is_infinity() {
        print("FAILURE - rG!=O")
        fail=true;
    }
    
    var start=Date()
    var iterations=0
    var elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        W=G.mul(s)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "EC  mul - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
    
    print("Generating \(ROM.FFLEN*ROM.BIGBITS) RSA public/private key pair")
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        RSA.KEY_PAIR(rng,65537,priv,pub)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "RSA gen - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
    
    for i in 0..<RSA.RFS {M[i]=UInt8(i%128)}
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        RSA.ENCRYPT(pub,M,&C)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "RSA enc - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
   
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        RSA.DECRYPT(priv,C,&P)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "RSA dec - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
   
    var cmp=true
    for i in 0..<RSA.RFS {
        if P[i] != M[i] {cmp=false}
    }
    
    if !cmp {
        print("FAILURE - RSA decryption")
        fail=true;
    }
    
    if !fail {
       print("All tests pass")
    }
}

//BenchtestEC()

