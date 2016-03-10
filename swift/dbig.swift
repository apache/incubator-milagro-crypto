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
//  dbig.swift
//  
//
//  Created by Michael Scott on 13/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

final class DBIG{
    var w=[Int32](count:ROM.DNLEN,repeatedValue:0)
    init() {
        for var i=0;i<ROM.DNLEN;i++ {w[i]=0}
    }
    init(_ x: Int32)
    {
        w[0]=x;
        for var i=1;i<ROM.DNLEN;i++ {w[i]=0}
    }
    init(_ x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i]=x.w[i]}
        w[ROM.NLEN-1]=x.w[ROM.NLEN-1]&ROM.MASK
        w[ROM.NLEN]=x.w[ROM.NLEN-1]>>ROM.BASEBITS
        for var i=ROM.NLEN+1;i<ROM.DNLEN;i++ {w[i]=0}
    }
    init(_ x: DBIG)
    {
        for var i=0;i<ROM.DNLEN;i++ {w[i]=x.w[i]}
    }
    init(_ x: [Int32])
    {
        for var i=0;i<ROM.DNLEN;i++ {w[i]=x[i]}
    }
    /* this-=x */
    func sub(x: DBIG)
    {
        for var i=0;i<ROM.DNLEN;i++
        {
            w[i]-=x.w[i]
        }
    }
    func muladd(x: Int32,_ y: Int32,_ c: Int32,_ i: Int) -> Int32
    {
        let prod:Int64 = Int64(x)*Int64(y)+Int64(c)+Int64(w[i])
        w[i]=Int32(prod&Int64(ROM.MASK))
        return Int32(prod>>Int64(ROM.BASEBITS))
    }
    /* general shift left */
    func shl(k: Int)
    {
        let n=Int32(k)%ROM.BASEBITS
        let m=(k/Int(ROM.BASEBITS))
        w[ROM.DNLEN-1]=((w[ROM.DNLEN-1-m]<<n))|(w[ROM.DNLEN-m-2]>>(ROM.BASEBITS-n))
        for var i=ROM.DNLEN-2;i>m;i--
        {
            w[i]=((w[i-m]<<n)&ROM.MASK)|(w[i-m-1]>>(ROM.BASEBITS-n))
        }
        w[m]=(w[0]<<n)&ROM.MASK
        for var i=0;i<m;i++ {w[i]=0}
    }
    /* general shift right */
    func shr(k: Int)
    {
        let n=Int32(k)%ROM.BASEBITS
        let m=(k/Int(ROM.BASEBITS))
        for var i=0;i<ROM.DNLEN-m-1;i++
        {
            w[i]=(w[m+i]>>n)|((w[m+i+1]<<(ROM.BASEBITS-n))&ROM.MASK)
        }
        w[ROM.DNLEN - m - 1]=w[ROM.DNLEN-1]>>n
        for var i=ROM.DNLEN - m;i<ROM.DNLEN;i++ {w[i]=0}
    }
    /* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
    static func comp(a: DBIG,_ b: DBIG) -> Int
    {
        for var i=ROM.DNLEN-1;i>=0;i--
        {
            if (a.w[i]==b.w[i]) {continue}
            if (a.w[i]>b.w[i]) {return 1}
            else  {return -1}
        }
        return 0;
    }
    /* normalise BIG - force all digits < 2^BASEBITS */
    func norm()
    {
        var carry:Int32=0
        for var i=0;i<ROM.DNLEN-1;i++
        {
            let d=w[i]+carry
            w[i]=d&ROM.MASK
            carry=d>>ROM.BASEBITS
        }
        w[ROM.DNLEN-1]+=carry
    }
    /* reduces this DBIG mod a BIG, and returns the BIG */
    func mod(c: BIG) -> BIG
    {
        var k:Int=0
        norm()
        let m=DBIG(c)

        if DBIG.comp(self,m)<0 {return BIG(self)}

        repeat
        {
            m.shl(1)
            k++
        }
        while (DBIG.comp(self,m)>=0);

        while (k>0)
        {
            m.shr(1)
            if (DBIG.comp(self,m)>=0)
            {
				sub(m)
				norm()
            }
            k--;
        }
        return BIG(self)
    }
    /* return this/c */
    func div(c:BIG) -> BIG
    {
        var k:Int=0
        let m=DBIG(c)
        let a=BIG(0)
        let e=BIG(1)
        norm()

        while (DBIG.comp(self,m)>=0)
        {
            e.fshl(1)
            m.shl(1)
            k++
        }

        while (k>0)
        {
            m.shr(1)
            e.shr(1)
            if (DBIG.comp(self,m)>0)
            {
				a.add(e)
				a.norm()
				sub(m)
				norm()
            }
            k--
        }
        return a
    }

    /* split DBIG at position n, return higher half, keep lower half */
    func split(n: Int32) -> BIG
    {
        let t=BIG(0)
        let m=n%ROM.BASEBITS
        var carry=w[ROM.DNLEN-1]<<(ROM.BASEBITS-m)

        for var i=ROM.DNLEN-2;i>=ROM.NLEN-1;i--
        {
            let nw=(w[i]>>m)|carry;
            carry=(w[i]<<(ROM.BASEBITS-m))&ROM.MASK;
            t.set(i-ROM.NLEN+1,nw);
        }
        w[ROM.NLEN-1]&=Int32((Int32(1)<<m)-1);
        return t;
    }
    /* return number of bits */
    func nbits() -> Int
    {
        var k=(ROM.DNLEN-1)
        norm()
        while k>=0 && w[k]==0 {k--}
        if k<0 {return 0}
        var bts=Int(ROM.BASEBITS)*k
        var c=w[k];
        while c != 0 {c/=2; bts++}
        return bts
    }
    /* Convert to Hex String */
    func toString() -> String
    {
        _ = DBIG()
        var s:String=""
        var len=nbits()
        if len%4 == 0 {len/=4}
        else {len/=4; len++}

        for var i=len-1;i>=0;i--
        {
            let b = DBIG(self)
            b.shr(i*4)
            let n=String(b.w[0]&15,radix:16,uppercase:false)
            s+=n
        }

        return s
    }

}
