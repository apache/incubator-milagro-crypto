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
//  rsa.swift
//  
//
//  Created by Michael Scott on 25/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation

/* RSA API high-level functions  */

final public class rsa_private_key {
    var p:FF
    var q:FF
    var dp:FF
    var dq:FF
    var c:FF

    public init(_ n: Int)
    {
    p=FF(n);
    q=FF(n);
    dp=FF(n);
    dq=FF(n);
    c=FF(n);
    }
}

final public class rsa_public_key
{
    var e:Int32
    var n:FF

    public init(_ m:Int)
    {
        e=0;
        n=FF(m);
    }
}

final public class RSA {

    static public let RFS=Int(ROM.MODBYTES)*ROM.FFLEN

    /* generate an RSA key pair */

    static public func KEY_PAIR(rng: RAND,_ e:Int32,_ PRIV:rsa_private_key,_ PUB:rsa_public_key)
    { /* IEEE1363 A16.11/A16.12 more or less */

        let n=PUB.n.getlen()/2;
        let t = FF(n);
        let p1=FF(n);
        let q1=FF(n);

        while true
        {

            PRIV.p.random(rng);
            while PRIV.p.lastbits(2) != 3 {PRIV.p.inc(1)}
            while !FF.prime(PRIV.p,rng) {PRIV.p.inc(4)}

            p1.copy(PRIV.p);
            p1.dec(1);

            if p1.cfactor(e) {continue}
            break;
        }

        while true
        {
            PRIV.q.random(rng);
            while PRIV.q.lastbits(2) != 3 {PRIV.q.inc(1)}
            while !FF.prime(PRIV.q,rng) {PRIV.q.inc(4)}

            q1.copy(PRIV.q);
            q1.dec(1);

            if q1.cfactor(e) {continue}

            break;
        }

        PUB.n=FF.mul(PRIV.p,PRIV.q);
        PUB.e=e;

        t.copy(p1);
        t.shr();
        PRIV.dp.set(e);
        PRIV.dp.invmodp(t);
        if (PRIV.dp.parity()==0) {PRIV.dp.add(t)}
        PRIV.dp.norm();

        t.copy(q1);
        t.shr();
        PRIV.dq.set(e);
        PRIV.dq.invmodp(t);
        if (PRIV.dq.parity()==0) {PRIV.dq.add(t)}
        PRIV.dq.norm();

        PRIV.c.copy(PRIV.p);
        PRIV.c.invmodp(PRIV.q);

        return;
    }
    /* Mask Generation Function */

    static func MGF1(Z: [UInt8],_ olen:Int,inout _ K:[UInt8])
    {
        let H=HASH();
        let hlen=HASH.len;

        var k=0;
        for var i=0;i<K.count;i++ {K[i]=0}

        var cthreshold=Int32(olen/hlen); if (olen%hlen != 0) {cthreshold++}
        for var counter:Int32=0;counter<cthreshold;counter++
        {
            H.process_array(Z);
            H.process_num(counter);
            var B=H.hash();

            if (k+hlen>olen) {for var i=0;i<olen%hlen;i++ {K[k++]=B[i]}}
            else {for var i=0;i<hlen;i++ {K[k++]=B[i]}}
        }
    }

    static public func printBinary(array: [UInt8])
    {
        for var i=0;i<array.count;i++
        {
            let h=String(array[i],radix:16)
            print("\(h)", terminator: "")
        }
        print("");
    }
    /* OAEP Message Encoding for Encryption */
    static public func OAEP_ENCODE(m:[UInt8],_ rng:RAND,_ p:[UInt8]?) -> [UInt8]
    {
        let olen=RFS-1;
        let mlen=m.count;
        var f=[UInt8](count:RSA.RFS,repeatedValue:0)

        let H=HASH();
        let hlen=HASH.len;
        var SEED=[UInt8](count:hlen,repeatedValue:0)
        let seedlen=hlen;
        if (mlen>olen-hlen-seedlen-1) {return [UInt8]()}

        var DBMASK=[UInt8](count:olen-seedlen,repeatedValue:0)

        if ((p) != nil) {H.process_array(p!)}
        var h=H.hash();
        for var i=0;i<hlen;i++ {f[i]=h[i]}

        let slen=olen-mlen-hlen-seedlen-1;

        for var i=0;i<slen;i++ {f[hlen+i]=0}
        f[hlen+slen]=1;
        for var i=0;i<mlen;i++ {f[hlen+slen+1+i]=m[i]}

        for var i=0;i<seedlen;i++ {SEED[i]=rng.getByte()}
        RSA.MGF1(SEED,olen-seedlen,&DBMASK)

        for var i=0;i<olen-seedlen;i++ {DBMASK[i]^=f[i]}
        RSA.MGF1(DBMASK,seedlen,&f)

        for var i=0;i<seedlen;i++ {f[i]^=SEED[i]}

        for var i=0;i<olen-seedlen;i++ {f[i+seedlen]=DBMASK[i]}

    /* pad to length RFS */
        let d:Int=1;
        for var i=RFS-1;i>=d;i--
            {f[i]=f[i-d]}
        for var i=d-1;i>=0;i--
            {f[i]=0}

        return f;
    }

    /* OAEP Message Decoding for Decryption */
    static public func OAEP_DECODE(p: [UInt8]?,inout _ f:[UInt8]) -> [UInt8]
    {
        let olen=RFS-1
        var k:Int
        let H=HASH()
        var hlen=HASH.len
        var SEED=[UInt8](count:hlen,repeatedValue:0)
        var seedlen=hlen
        var CHASH=[UInt8](count:hlen,repeatedValue:0)
        seedlen=32; hlen=32
        if olen<seedlen+hlen+1 {return [UInt8()]}
        var DBMASK=[UInt8](count:olen-seedlen,repeatedValue:0)
        for var i=0;i<olen-seedlen;i++ {DBMASK[i]=0}

        if (f.count<RSA.RFS)
        {
            let d=RSA.RFS-f.count;
            for var i=RSA.RFS-1;i>=d;i--
                {f[i]=f[i-d]}
            for var i=d-1;i>=0;i--
                {f[i]=0}

        }

        if (p != nil) {H.process_array(p!)}
        var h=H.hash();
        for var i=0;i<hlen;i++ {CHASH[i]=h[i]}

        let x=f[0];

        for var i=seedlen;i<olen;i++
            {DBMASK[i-seedlen]=f[i+1]}

        RSA.MGF1(DBMASK,seedlen,&SEED);
        for var i=0;i<seedlen;i++ {SEED[i]^=f[i+1]}
        RSA.MGF1(SEED,olen-seedlen,&f);
        for var i=0;i<olen-seedlen;i++ {DBMASK[i]^=f[i]}

        var comp=true;
        for var i=0;i<hlen;i++
        {
            if (CHASH[i] != DBMASK[i]) {comp=false}
        }

        for var i=0;i<olen-seedlen-hlen;i++
        {DBMASK[i]=DBMASK[i+hlen]}

        for var i=0;i<hlen;i++
            {SEED[i]=0;CHASH[i]=0;}

        for k=0;;k++
        {
            if (k>=olen-seedlen-hlen) {return [UInt8]()}
            if (DBMASK[k] != 0) {break}
        }

        let t=DBMASK[k];
        if (!comp || x != 0 || t != 0x01)
        {
            for var i=0;i<olen-seedlen;i++ {DBMASK[i]=0}
            return [UInt8]()
        }

        var r=[UInt8](count:olen-seedlen-hlen-k-1,repeatedValue:0)

        for var i=0;i<olen-seedlen-hlen-k-1;i++
            {r[i]=DBMASK[i+k+1]}

        for var i=0;i<olen-seedlen;i++ {DBMASK[i]=0}

        return r;
    }
    /* destroy the Private Key structure */
    static public func PRIVATE_KEY_KILL(PRIV: rsa_private_key)
    {
        PRIV.p.zero();
        PRIV.q.zero();
        PRIV.dp.zero();
        PRIV.dq.zero();
        PRIV.c.zero();
    }
    /* RSA encryption with the public key */
    static public func ENCRYPT(PUB: rsa_public_key,_ F:[UInt8],inout _ G:[UInt8])
    {
        let n=PUB.n.getlen()
        let f=FF(n)

        FF.fromBytes(f,F)
        f.power(PUB.e,PUB.n)
        f.toBytes(&G)
    }
    /* RSA decryption with the private key */
    static public func DECRYPT(PRIV: rsa_private_key,_ G:[UInt8],inout _ F:[UInt8])
    {
        let n=PRIV.p.getlen()
        let g=FF(2*n)

        FF.fromBytes(g,G)
        let jp=g.dmod(PRIV.p)
        var jq=g.dmod(PRIV.q)

        jp.skpow(PRIV.dp,PRIV.p)
        jq.skpow(PRIV.dq,PRIV.q)

        g.zero()
        g.dscopy(jp)
        jp.mod(PRIV.q)
        if (FF.comp(jp,jq)>0) {jq.add(PRIV.q)}
        jq.sub(jp)
        jq.norm()

        var t=FF.mul(PRIV.c,jq)
        jq=t.dmod(PRIV.q)

        t=FF.mul(jq,PRIV.p)
        g.add(t);
        g.norm();

        g.toBytes(&F);
    }

}

