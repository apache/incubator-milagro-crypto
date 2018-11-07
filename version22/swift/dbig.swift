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
//  Created by Michael Scott on 13/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

final class DBIG{
    var w=[Chunk](repeating: 0,count: ROM.DNLEN)
    init() {
        for i in 0 ..< ROM.DNLEN {w[i]=0}
    }
    init(_ x: Int)
    {
        w[0]=Chunk(x);
        for i in 1 ..< ROM.DNLEN {w[i]=0}
    }
    init(_ x: BIG)
    {
        for i in 0 ..< ROM.NLEN {w[i]=x.w[i]}
        w[ROM.NLEN-1]=x.w[ROM.NLEN-1]&ROM.BMASK
        w[ROM.NLEN]=x.w[ROM.NLEN-1]>>Chunk(ROM.BASEBITS)
        for i in ROM.NLEN+1 ..< ROM.DNLEN {w[i]=0}
    }
    init(_ x: DBIG)
    {
        for i in 0 ..< ROM.DNLEN {w[i]=x.w[i]}
    }
    init(_ x: [Chunk])
    {
        for i in 0 ..< ROM.DNLEN {w[i]=x[i]}
    }

    func cmove(_ g: DBIG,_ d: Int)
    {
        let b = Chunk(-d)
    
        for i in 0 ..< ROM.DNLEN
        {
            w[i]^=(w[i]^g.w[i])&b;
        }
    }

/* Copy from another DBIG */
    func copy(_ x: DBIG)
    {
        for i in 0 ..< ROM.DNLEN {w[i] = x.w[i]}
    }

    /* this-=x */
    func sub(_ x: DBIG)
    {
        for i in 0 ..< ROM.DNLEN
        {
            w[i]-=x.w[i]
        }
    }
/*    func muladd(_ x: Int32,_ y: Int32,_ c: Int32,_ i: Int) -> Int32
    {
        let prod:Int64 = Int64(x)*Int64(y)+Int64(c)+Int64(w[i])
        w[i]=Int32(prod&Int64(ROM.BMASK))
        return Int32(prod>>Int64(ROM.BASEBITS))
    } */
    /* general shift left */
    func shl(_ k: UInt)
    {
        let n=k%ROM.BASEBITS
        let m=Int(k/ROM.BASEBITS)
        w[ROM.DNLEN-1]=((w[ROM.DNLEN-1-m]<<Chunk(n)))|(w[ROM.DNLEN-m-2]>>Chunk(ROM.BASEBITS-n))
        for i in (m+1...ROM.DNLEN-2).reversed()
     //   for var i=ROM.DNLEN-2;i>m;i--
        {
            w[i]=((w[i-m]<<Chunk(n))&ROM.BMASK)|(w[i-m-1]>>Chunk(ROM.BASEBITS-n))
        }
        w[m]=(w[0]<<Chunk(n))&ROM.BMASK
        for i in 0 ..< m {w[i]=0}
    }
    /* general shift right */
    func shr(_ k: UInt)
    {
        let n=k%ROM.BASEBITS
        let m=Int(k/ROM.BASEBITS)
        for i in 0 ..< ROM.DNLEN-m-1
        {
            w[i]=(w[m+i]>>Chunk(n))|((w[m+i+1]<<Chunk(ROM.BASEBITS-n))&ROM.BMASK)
        }
        w[ROM.DNLEN - m - 1]=w[ROM.DNLEN-1]>>Chunk(n)
        for i in ROM.DNLEN - m ..< ROM.DNLEN {w[i]=0}
    }
    /* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
    static func comp(_ a: DBIG,_ b: DBIG) -> Int
    {
        for i in (0...ROM.DNLEN-1).reversed()
       // for var i=ROM.DNLEN-1;i>=0;i--
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
        var carry:Chunk=0
        for i in 0 ..< ROM.DNLEN-1
        {
            let d=w[i]+carry
            w[i]=d&ROM.BMASK
            carry=d>>Chunk(ROM.BASEBITS)
        }
        w[ROM.DNLEN-1]+=carry
    }
    /* reduces this DBIG mod a BIG, and returns the BIG */
    func mod(_ c: BIG) -> BIG
    {
        var k:Int=0
        norm()
        let m=DBIG(c)
        let r=DBIG(0)
    
        if DBIG.comp(self,m)<0 {return BIG(self)}
    
        repeat
        {
            m.shl(1)
            k += 1
        }
        while (DBIG.comp(self,m)>=0);
    
        while (k>0)
        {
            m.shr(1)

		r.copy(self)
		r.sub(m)
		r.norm()
		cmove(r,Int(1-((r.w[ROM.DNLEN-1]>>Chunk(ROM.CHUNK-1))&1)))
/*

            if (DBIG.comp(self,m)>=0)
            {
				sub(m)
				norm()
            } */
            k -= 1;
        }
        return BIG(self)
    }
    /* return this/c */
    func div(_ c:BIG) -> BIG
    {
        var k:Int=0
        let m=DBIG(c)
        let a=BIG(0)
        let e=BIG(1)
        let r=BIG(0)
        let dr=DBIG(0)

        norm()
    
        while (DBIG.comp(self,m)>=0)
        {
            e.fshl(1)
            m.shl(1)
            k += 1
        }
    
        while (k>0)
        {
            m.shr(1)
            e.shr(1)

		dr.copy(self)
		dr.sub(m)
		dr.norm()
		let d=Int(1-((dr.w[ROM.DNLEN-1]>>Chunk(ROM.CHUNK-1))&1))
		cmove(dr,d)
		r.copy(a)
		r.add(e)
		r.norm()
		a.cmove(r,d)
/*
            if (DBIG.comp(self,m)>0)
            {
				a.add(e)
				a.norm()
				sub(m)
				norm()
            } */
            k -= 1
        }
        return a
    }
    
    /* split DBIG at position n, return higher half, keep lower half */
    func split(_ n: UInt) -> BIG
    {
        let t=BIG(0)
        let m=n%ROM.BASEBITS
        var carry=w[ROM.DNLEN-1]<<Chunk(ROM.BASEBITS-m)
    
        for i in (ROM.NLEN-1...ROM.DNLEN-2).reversed()
      //  for var i=ROM.DNLEN-2;i>=ROM.NLEN-1;i--
        {
            let nw=(w[i]>>Chunk(m))|carry;
            carry=(w[i]<<Chunk(ROM.BASEBITS-m))&ROM.BMASK;
            t.set(i-ROM.NLEN+1,nw);
        }
        w[ROM.NLEN-1]&=((1<<Chunk(m))-1);
        return t;
    }
    /* return number of bits */
    func nbits() -> Int
    {
        var k=(ROM.DNLEN-1)
        norm()
        while k>=0 && w[k]==0 {k -= 1}
        if k<0 {return 0}
        var bts=Int(ROM.BASEBITS)*k
        var c=w[k];
        while c != 0 {c/=2; bts+=1}
        return bts
    }
    /* Convert to Hex String */
    func toString() -> String
    {
        _ = DBIG()
        var s:String=""
        var len=nbits()
        if len%4 == 0 {len/=4}
        else {len/=4; len += 1}
        
        for i in (0...len-1).reversed()
    //    for var i=len-1;i>=0;i--
        {
            let b = DBIG(self)
            b.shr(UInt(i*4))
            let n=String(b.w[0]&15,radix:16,uppercase:false)
            s+=n
        }
        
        return s
    }
    
}
