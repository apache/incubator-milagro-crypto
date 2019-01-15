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
//  TestMPIN.swift
//
//  Created by Michael Scott on 08/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation
//import amcl  // comment out for Xcode

public func BenchtestPAIR()
{
    let MIN_TIME=10.0
    let MIN_ITERS=10
    
    let rng=RAND()
    var fail=false;
    
    var RAW=[UInt8](repeating: 0,count: 100)
    
    for i in 0 ..< 100 {RAW[i]=UInt8((i+1)&0xff)}
    rng.seed(100,RAW)

    if ROM.CURVE_PAIRING_TYPE==ROM.BN_CURVE {
        print("BN Pairing-Friendly Curve")
    }
    if ROM.CURVE_PAIRING_TYPE==ROM.BLS_CURVE {
        print("BLS Pairing-Friendly Curve")
    }
    print("Modulus size \(ROM.MODBITS) bits")
    print("\(ROM.CHUNK) bit build")
    
    let gx=BIG(ROM.CURVE_Gx);

    let gy=BIG(ROM.CURVE_Gy)
    let G=ECP(gx,gy)
    
    let r=BIG(ROM.CURVE_Order)
    let s=BIG.randomnum(r,rng)
    
    var P=PAIR.G1mul(G,r);
    
    if !P.is_infinity() {
        print("FAILURE - rP!=O")
        fail=true
    }
    
    var start=Date()
    var iterations=0
    var elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        P=PAIR.G1mul(G,s)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "G1  mul              - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
    
    var Q=ECP2(FP2(BIG(ROM.CURVE_Pxa),BIG(ROM.CURVE_Pxb)),FP2(BIG(ROM.CURVE_Pya),BIG(ROM.CURVE_Pyb)))
    
    var W=PAIR.G2mul(Q,r)
    
    if !W.is_infinity() {
        print("FAILURE - rQ!=O")
        fail=true
    }
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        W=PAIR.G2mul(Q,s)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "G2  mul              - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
    
    var w=PAIR.ate(Q,P)
    w=PAIR.fexp(w)
    
    var g=PAIR.GTpow(w,r)
    
    if !g.isunity() {
        print("FAILURE - g^r!=1")
        fail=true
    }
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        g=PAIR.GTpow(w,s)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "GT  pow              - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
   
    let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
    let q=BIG(ROM.Modulus)
    
    var m=BIG(q)
    m.mod(r)
    
    let a=BIG(s)
    a.mod(m)
    
    let b=BIG(s)
    b.div(m)
    
    g.copy(w)
    var c=g.trace()
    
    g.frob(f)
    let cp=g.trace()
    
    w.conj()
    g.mul(w);
    let cpm1=g.trace()
    
    g.mul(w)
    let cpm2=g.trace()
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        c=c.xtr_pow2(cp,cpm1,cpm2,a,b)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "GT  pow (compressed) - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
    
    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        w=PAIR.ate(Q,P)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "PAIRing ATE          - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))

    start=Date()
    iterations=0
    elapsed=0.0
    while elapsed<MIN_TIME || iterations<MIN_ITERS {
        g=PAIR.fexp(w)
        iterations+=1
        elapsed = -start.timeIntervalSinceNow
    }
    elapsed=1000.0*elapsed/Double(iterations)
    print(String(format: "PAIRing FEXP         - %d iterations",iterations),terminator: "");
    print(String(format: " %.2f ms per iteration",elapsed))
 
    P.copy(G)
    Q.copy(W)
    
    P=PAIR.G1mul(P,s)
    g=PAIR.ate(Q,P)
    g=PAIR.fexp(g)
    
    P.copy(G)
    Q=PAIR.G2mul(Q,s)
    w=PAIR.ate(Q,P)
    w=PAIR.fexp(w)
    
    if !g.equals(w) {
        print("FAILURE - e(sQ,P)!=e(Q,sP)")
        fail=true
    }
    
    if !fail {
        print("All tests pass")
    }
}

//BenchtestPAIR()

