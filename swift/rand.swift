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
//  rand.swift
//  
//
//  Created by Michael Scott on 17/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//
//  Cryptographic strong random number generator

/* Marsaglia & Zaman Random number generator constants */
final public class RAND {
    private static let NK:Int=21
    private static let NJ:Int=6
    private static let NV:Int=8
    private var ira=[UInt32](count:NK,repeatedValue:0)
    private var rndptr:Int=0
    private var borrow:UInt32=0
    private var pool_ptr:Int=0
    private var pool=[UInt8](count:32,repeatedValue:0)

    public func clean()
    {
        pool_ptr=0
        rndptr=0
        for var i=0;i<32;i++ {pool[i]=0}
        for var i=0;i<RAND.NK;i++ {ira[i]=0}
        borrow=0;
    }

    public init() {clean()}

    private func sbrand() -> UInt32
    { /* Marsaglia & Zaman random number generator */
        rndptr++;
        if rndptr<RAND.NK {return ira[rndptr]}
        rndptr=0;
        var k=RAND.NK-RAND.NJ
        for var i=0;i<RAND.NK;i++
        {
            if k==RAND.NK {k=0}
            let t=ira[k];
            let pdiff=t &- ira[i] &- borrow
            if pdiff<t {borrow=0}
            if pdiff>t {borrow=1}
            ira[i]=pdiff
            k++;
        }
        return ira[0]
    }

    func sirand(seed: UInt32)
    {
        var m:UInt32=1
        var s:UInt32=seed
        borrow=0;
        rndptr=0
        ira[0]^=s
        for var i=1;i<RAND.NK;i++
        { /* fill initialisation vector */
            let ipn=(RAND.NV*i)%RAND.NK
            ira[ipn]^=m
            let t=m
            m=s &- m
            s=t
        }
        for var i=0;i<10000;i++ {sbrand()}
    }

    private func fill_pool()
    {
        let sh=HASH()
        for var i=0;i<128;i++ {sh.process(UInt8(sbrand()&0xff))}
        pool=sh.hash()
        pool_ptr=0
    }

    private func pack(b: [UInt8]) -> UInt32
    {
        return (UInt32(b[3])<<24)|(UInt32(b[2])<<16)|(UInt32(b[1])<<8)|(UInt32(b[0]))
    }

/* Initialize RNG with some real entropy from some external source */
    public func seed(rawlen: Int,_ raw: [UInt8])
    { /* initialise from at least 128 byte string of raw random entropy */
        var digest=[UInt8]()
        var b=[UInt8](count:4, repeatedValue:0)
        let sh=HASH()
        pool_ptr=0
        for var i=0;i<RAND.NK;i++ {ira[i]=0}
        if rawlen>0
        {
            for var i=0;i<rawlen;i++ {sh.process(raw[i])}
            digest=sh.hash()

            for var i=0;i<8;i++
            {
                b[0]=digest[4*i]; b[1]=digest[4*i+1]; b[2]=digest[4*i+2]; b[3]=digest[4*i+3]
                sirand(pack(b))
            }

        }
        fill_pool()
    }

    public func getByte() -> UInt8
    {
        let r=pool[pool_ptr++]
        if pool_ptr>=32 {fill_pool()}
        return r
    }


}
