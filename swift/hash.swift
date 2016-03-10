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
//  hash.swift
//  
//
//  Created by Michael Scott on 17/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//  SHA256 Implementation
//

final class HASH{

    private var length=[UInt32](count:2,repeatedValue:0)
    private var h=[UInt32](count:8,repeatedValue:0)
    private var w=[UInt32](count:64,repeatedValue:0)
    static let H0:UInt32=0x6A09E667
    static let H1:UInt32=0xBB67AE85
    static let H2:UInt32=0x3C6EF372
    static let H3:UInt32=0xA54FF53A
    static let H4:UInt32=0x510E527F
    static let H5:UInt32=0x9B05688C
    static let H6:UInt32=0x1F83D9AB
    static let H7:UInt32=0x5BE0CD19

    static let len:Int=32

    static let K:[UInt32]=[
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2]


    private static func S(n: UInt32,_ x: UInt32) -> UInt32
    {
        return ((x>>n)|(x<<(32-n)))
    }

    private static func R(n: UInt32,_ x: UInt32) -> UInt32
    {
        return (x>>n)
    }

    private static func Ch(x: UInt32,_ y: UInt32,_ z:UInt32) -> UInt32
    {
        return ((x&y)^(~(x)&z))
    }

    private static func Maj(x: UInt32,_ y: UInt32,_ z:UInt32) -> UInt32
    {
        return ((x&y)^(x&z)^(y&z))
    }

    private static func Sig0(x: UInt32) -> UInt32
    {
        return (S(2,x)^S(13,x)^S(22,x))
    }

    private static func Sig1(x: UInt32) -> UInt32
    {
        return (S(6,x)^S(11,x)^S(25,x))
    }

    private static func theta0(x: UInt32) -> UInt32
    {
        return (S(7,x)^S(18,x)^R(3,x))
    }

    private static func theta1(x: UInt32) -> UInt32
    {
        return (S(17,x)^S(19,x)^R(10,x))
    }

    private func transform()
    { /* basic transformation step */
        var a,b,c,d,e,f,g,hh,t1,t2 :UInt32
        var j:Int
        for j=16;j<64;j++
        {
            w[j]=HASH.theta1(w[j-2])&+w[j-7]&+HASH.theta0(w[j-15])&+w[j-16]
        }
        a=h[0]; b=h[1]; c=h[2]; d=h[3]
        e=h[4]; f=h[5]; g=h[6]; hh=h[7]

        for j=0;j<64;j++
        { /* 64 times - mush it up */
            t1=hh&+HASH.Sig1(e)&+HASH.Ch(e,f,g)&+HASH.K[j]&+w[j]
            t2=HASH.Sig0(a)&+HASH.Maj(a,b,c)
            hh=g; g=f; f=e;
            e=d&+t1;
            d=c;
            c=b;
            b=a;
            a=t1&+t2;
        }
        h[0]=h[0]&+a; h[1]=h[1]&+b; h[2]=h[2]&+c; h[3]=h[3]&+d
        h[4]=h[4]&+e; h[5]=h[5]&+f; h[6]=h[6]&+g; h[7]=h[7]&+hh;
    }

    /* Re-Initialise Hash function */
    func init_it()
    { /* initialise */
        for var i=0;i<64;i++ {w[i]=0}
        length[0]=0; length[1]=0
        h[0]=HASH.H0;
        h[1]=HASH.H1;
        h[2]=HASH.H2;
        h[3]=HASH.H3;
        h[4]=HASH.H4;
        h[5]=HASH.H5;
        h[6]=HASH.H6;
        h[7]=HASH.H7;
    }

    init()
    {
        init_it()
    }

    /* process a single byte */
    func process(byt: UInt8)
    { /* process the next message byte */
        let cnt=Int((length[0]/32)%16)
        w[cnt]<<=8;
        w[cnt]|=(UInt32(byt)&0xFF);
        length[0]+=8;
        if (length[0]==0) { length[1]++; length[0]=0 }
        if ((length[0]%512)==0) {transform()}
    }

    /* process an array of bytes */
    func process_array(b: [UInt8])
    {
        for var i=0;i<b.count;i++ {process((b[i]))}
    }

    /* process a 32-bit integer */
    func process_num(n:Int32)
    {
        process(UInt8((n>>24)&0xff))
        process(UInt8((n>>16)&0xff))
        process(UInt8((n>>8)&0xff))
        process(UInt8(n&0xff))
    }

    /* Generate 32-byte Hash */
    func hash() -> [UInt8]
    { /* pad message and finish - supply digest */
        var digest=[UInt8](count:32,repeatedValue:0)

        let len0=length[0]
        let len1=length[1]
        process(0x80);
        while ((length[0]%512) != 448) {process(0)}
        w[14]=UInt32(len1)
        w[15]=len0;
        transform()
        for var i=0;i<HASH.len;i++
        { /* convert to bytes */
            digest[i]=UInt8((h[i/4]>>(8*(3-UInt32(i)%4))) & 0xff);
        }
        init_it();
    return digest;
    }


}
