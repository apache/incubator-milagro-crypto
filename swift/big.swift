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
//  big.swift
//  
//
//  Created by Michael Scott on 12/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//  BIG number class
//

final class BIG{
    var w=[Int32](count:ROM.NLEN,repeatedValue:0)
/* Constructors */
    init() {
        for var i=0;i<ROM.NLEN;i++ {w[i]=0}
    }
    init(_ x: Int32)
    {
        w[0]=x;
        for var i=1;i<ROM.NLEN;i++ {w[i]=0}
    }
    init(_ x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i]=x.w[i]}
    }
    init(_ x: DBIG)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i]=x.w[i]}
    }
    init(_ x: [Int32])
    {
        for var i=0;i<ROM.NLEN;i++ {w[i]=x[i]}
    }
    func get(i: Int) -> Int32
    {
        return w[i]
    }
    func set(i: Int,_ x: Int32)
    {
        w[i]=x
    }
    func xortop(x: Int32)
    {
        w[ROM.NLEN-1]^=x
    }
    func ortop(x: Int32)
    {
        w[ROM.NLEN-1]|=x
    }
/* calculate Field Excess */
    static func EXCESS(a: BIG) -> Int32
    {
        return ((a.w[ROM.NLEN-1] & ROM.OMASK)>>Int32(ROM.MODBITS%ROM.BASEBITS))
    }
/* test for zero */
    func iszilch() -> Bool
    {
        for var i=0;i<ROM.NLEN;i++ {if w[i] != 0 {return false}}
        return true
    }
/* set to zero */
    func zero()
    {
        for var i=0;i<ROM.NLEN;i++ {w[i] = 0}
    }
/* set to one */
    func one()
    {
        w[0]=1
        for var i=1;i<ROM.NLEN;i++ {w[i]=0}
    }
/* Test for equal to one */
    func isunity() -> Bool
    {
        for var i=1;i<ROM.NLEN;i++ {if w[i] != 0 {return false}}
        if w[0] != 1 {return false}
        return true
    }
/* Copy from another BIG */
    func copy(x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i] = x.w[i]}
    }
    func copy(x: DBIG)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i] = x.w[i]}
    }
/* Conditional swap of two bigs depending on d using XOR - no branches */
    func cswap(b: BIG,_ d: Int32)
    {
        var c:Int32 = d
        c = ~(c-1)
        for var i=0;i<ROM.NLEN;i++
        {
            let t=c&(w[i]^b.w[i])
            w[i]^=t
            b.w[i]^=t
        }
    }
    func cmove(g: BIG,_ d: Int32)
    {
        let b:Int32 = -d;

        for var i=0;i<ROM.NLEN;i++
        {
            w[i]^=(w[i]^g.w[i])&b;
        }
    }
/* normalise BIG - force all digits < 2^BASEBITS */
    func norm() -> Int32
    {
        var carry:Int32=0
        for var i=0;i<ROM.NLEN-1;i++
        {
            let d=w[i]+carry
            w[i]=d&ROM.MASK
            carry=d>>ROM.BASEBITS
        }
        w[ROM.NLEN-1]+=carry
        return (w[ROM.NLEN-1]>>((8*ROM.MODBYTES)%ROM.BASEBITS))
    }
/* Shift right by less than a word */
    func fshr(k: Int) -> Int32
    {
        let kw=Int32(k)
        let r=w[0]&((Int32(1)<<kw)-1)
        for var i=0;i<ROM.NLEN-1;i++
        {
            w[i]=(w[i]>>kw)|((w[i+1]<<(ROM.BASEBITS-kw))&ROM.MASK)
        }
        w[ROM.NLEN-1]>>=kw;
        return r
    }
/* general shift right */
    func shr(k: Int)
    {
        let n=Int32(k)%ROM.BASEBITS
        let m=(k/Int(ROM.BASEBITS))
        for var i=0;i<ROM.NLEN-m-1;i++
        {
            w[i]=(w[m+i]>>n)|((w[m+i+1]<<(ROM.BASEBITS-n))&ROM.MASK)
        }
        w[ROM.NLEN - m - 1]=w[ROM.NLEN-1]>>n
        for var i=ROM.NLEN - m;i<ROM.NLEN;i++ {w[i]=0}
    }
/* Shift right by less than a word */
    func fshl(k: Int) -> Int32
    {
        let kw=Int32(k)
        w[ROM.NLEN-1]=((w[ROM.NLEN-1]<<kw))|(w[ROM.NLEN-2]>>(ROM.BASEBITS-kw))
        for var i=ROM.NLEN-2;i>0;i--
        {
            w[i]=((w[i]<<kw)&ROM.MASK)|(w[i-1]>>(ROM.BASEBITS-kw))
        }
        w[0]=(w[0]<<kw)&ROM.MASK
        return (w[ROM.NLEN-1]>>((8*ROM.MODBYTES)%ROM.BASEBITS))
    }
/* general shift left */
    func shl(k: Int)
    {
        let n=Int32(k)%ROM.BASEBITS
        let m=(k/Int(ROM.BASEBITS))
        w[ROM.NLEN-1]=((w[ROM.NLEN-1-m]<<n))|(w[ROM.NLEN-m-2]>>(ROM.BASEBITS-n))
        for var i=ROM.NLEN-2;i>m;i--
        {
            w[i]=((w[i-m]<<n)&ROM.MASK)|(w[i-m-1]>>(ROM.BASEBITS-n))
        }
        w[m]=(w[0]<<n)&ROM.MASK
        for var i=0;i<m;i++ {w[i]=0}
    }
/* return number of bits */
    func nbits() -> Int
    {
        var k=(ROM.NLEN-1)
        norm()
        while k>=0 && w[k]==0 {k--}
        if k<0 {return 0}
        var bts=Int(ROM.BASEBITS)*k
        var c=w[k];
        while c != 0 {c/=2; bts++}
        return bts
    }
    func toRawString() -> String
    {
        var s:String="("
        for var i=0;i<ROM.NLEN-1;i++
        {
            let n=String(w[i],radix:16,uppercase:false)
            s+=n
            s+=","

        }
        let n=String(w[ROM.NLEN-1],radix:16,uppercase:false)
        s+=n
        s+=")"
        return s
    }
/* Convert to Hex String */
    func toString() -> String
    {
        _ = BIG()
        var s:String=""
        var len=nbits()
        if len%4 == 0 {len/=4}
        else {len/=4; len++}
        if len<2*Int(ROM.MODBYTES) {len=2*Int(ROM.MODBYTES)}

        for var i=len-1;i>=0;i--
        {
            let b = BIG(self)
            b.shr(i*4)
            let n=String(b.w[0]&15,radix:16,uppercase:false)
            s+=n
        }

        return s
    }
/* return this+x */
    func plus(x: BIG) -> BIG
    {
        let s=BIG()
        for var i=0;i<ROM.NLEN;i++
        {
            s.w[i]=w[i]+x.w[i]
        }
        return s
    }
/* this+=x */
    func add(x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++
        {
            w[i]+=x.w[i]
        }
    }
/* this+=x, where x is int */
    func inc(x: Int32) {
        norm();
        w[0]+=x;
    }
/* return this.x */
   	func minus(x: BIG) -> BIG
    {
        let d=BIG();
        for var i=0;i<ROM.NLEN;i++
        {
            d.w[i]=w[i]-x.w[i];
        }
        return d;
    }
/* this-=x */
    func sub(x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++
        {
            w[i]-=x.w[i]
        }
    }
/* reverse subtract this=x-this */
    func rsub(x: BIG)
    {
        for var i=0;i<ROM.NLEN;i++
        {
            w[i]=x.w[i]-w[i]
        }
    }
/* this-=x where x is int */
    func dec(x: Int32) {
        norm();
        w[0]-=x;
    }
/* this*=x, where x is small int<NEXCESS */
    func imul(c: Int32)
    {
        for var i=0;i<ROM.NLEN;i++ {w[i]*=c}
    }
/* convert this BIG to byte array */
    func tobytearray(inout b: [UInt8],_ n: Int)
    {
        norm();
        let c=BIG(self);

        for var i=Int(ROM.MODBYTES)-1;i>=0;i--
        {
            b[i+n]=UInt8(c.w[0]&0xff);
            c.fshr(8);
        }
    }
/* convert from byte array to BIG */
    static func frombytearray(b: [UInt8],_ n: Int) -> BIG
    {
        let m=BIG();

        for var i=0;i<Int(ROM.MODBYTES);i++
        {
            m.fshl(8)
            m.w[0]+=Int32(b[i+n])&0xff    //(int)b[i+n]&0xff;
        }
        return m;
    }
    func toBytes(inout b: [UInt8])
    {
        tobytearray(&b,0)
    }
    static func fromBytes(b: [UInt8]) -> BIG
    {
        return frombytearray(b,0)
    }
/* set this[i]+=x*y+c, and return high part */
    func muladd(x: Int32,_ y: Int32,_ c: Int32,_ i: Int) -> Int32
    {
        let prod:Int64 = Int64(x)*Int64(y)+Int64(c)+Int64(w[i])
        w[i]=Int32(prod&Int64(ROM.MASK))
        return Int32(prod>>Int64(ROM.BASEBITS))
    }
/* this*=x, where x is >NEXCESS */
    func pmul(c: Int32) -> Int32
    {
        var carry:Int32=0;
        norm();
        for var i=0;i<ROM.NLEN;i++
        {
            let ak=w[i]
            w[i]=0
            carry=muladd(ak,c,carry,i);
        }
        return carry;
    }
/* this*=c and catch overflow in DBIG */
    func pxmul(c: Int32) -> DBIG
    {
        let m=DBIG()
        var carry:Int32=0
        for var j=0;j<ROM.NLEN;j++
        {
            carry=m.muladd(w[j],c,carry,j)
        }
        m.w[ROM.NLEN]=carry
        return m;
    }
/* divide by 3 */
    func div3() -> Int32
    {
        var carry:Int32=0
        norm();
        let base=(1<<ROM.BASEBITS);
        for var i=ROM.NLEN-1;i>=0;i--
        {
            let ak=(carry*base+w[i]);
            w[i]=ak/3;
            carry=ak%3;
        }
        return carry;
    }
/* return a*b where result fits in a BIG */
    static func smul(a: BIG,_ b: BIG) -> BIG
    {
        let c=BIG()
        for var i=0;i<ROM.NLEN;i++
        {
            var carry:Int32=0
            for var j=0;j<ROM.NLEN;j++
            {
                if (i+j<ROM.NLEN) {carry=c.muladd(a.w[i],b.w[j],carry,i+j)}
            }
        }
        return c;
    }
/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
    static func comp(a: BIG,_ b: BIG) -> Int
    {
        for var i=ROM.NLEN-1;i>=0;i--
        {
            if (a.w[i]==b.w[i]) {continue}
            if (a.w[i]>b.w[i]) {return 1}
            else  {return -1}
        }
        return 0;
    }
/* set x = x mod 2^m */
    func mod2m(m: Int)
    {
        let wd=m/Int(ROM.BASEBITS)
        let bt=Int32(m)%ROM.BASEBITS
        let msk=(1<<bt)-1;
        w[wd]&=msk;
        for var i=wd+1;i<ROM.NLEN;i++ {w[i]=0}
    }
/* Arazi and Qi inversion mod 256 */
    static func invmod256(a: Int32) -> Int32
    {
        var t1:Int32=0
        var c=(a>>1)&1
        t1+=c
        t1&=1
        t1=2-t1
        t1<<=1
        var U=t1+1

    // i=2
        var b=a&3
        t1=U*b; t1>>=2
        c=(a>>2)&3
        var t2=(U*c)&3
        t1+=t2
        t1*=U; t1&=3
        t1=4-t1
        t1<<=2
        U+=t1

    // i=4
        b=a&15
        t1=U*b; t1>>=4
        c=(a>>4)&15
        t2=(U*c)&15
        t1+=t2
        t1*=U; t1&=15
        t1=16-t1
        t1<<=4
        U+=t1

        return U
    }
/* return parity */
    func parity() -> Int32
    {
        return Int32(w[0]%2)
    }

/* return n-th bit */
    func bit(n: Int) -> Int32
    {
        if ((w[n/Int(ROM.BASEBITS)]&(Int32(1)<<(Int32(n)%ROM.BASEBITS)))>0) {return 1;}
        else {return 0;}
    }

    /* return n last bits */
    func lastbits(n: Int) -> Int32
    {
        let msk=(1<<Int32(n))-1;
        norm();
        return Int32((w[0])&msk)
    }
/* a=1/a mod 2^256. This is very fast! */
    func invmod2m()
    {
        let U=BIG()
        var b=BIG()
        let c=BIG()

        U.inc(BIG.invmod256(Int32(lastbits(8))))

        for var i=8;i<256;i<<=1
        {
            b.copy(self)
            b.mod2m(i)
            let t1=BIG.smul(U,b)
            t1.shr(i)
            c.copy(self)
            c.shr(i)
            c.mod2m(i)

            let t2=BIG.smul(U,c)
            t2.mod2m(i)
            t1.add(t2)
            b=BIG.smul(t1,U)
            t1.copy(b)
            t1.mod2m(i)

            t2.one(); t2.shl(i); t1.rsub(t2); t1.norm()
            t1.shl(i)
            U.add(t1)
        }
        self.copy(U)
    }
    /* reduce this mod m */
    func mod(m: BIG)
    {
        var k=0
        norm()
        if (BIG.comp(self,m)<0) {return}
        repeat
        {
            m.fshl(1)
            k++
        } while (BIG.comp(self,m)>=0)

        while (k>0)
        {
            m.fshr(1)
            if (BIG.comp(self,m)>=0)
            {
				sub(m)
				norm()
            }
            k--
        }
    }
    /* divide this by m */
    func div(m: BIG)
    {
        var k=0
        norm()
        let e=BIG(1)
        let b=BIG(self)
        zero()

        while (BIG.comp(b,m)>=0)
        {
            e.fshl(1)
            m.fshl(1)
            k++
        }

        while (k>0)
        {
            m.fshr(1)
            e.fshr(1)
            if (BIG.comp(b,m)>=0)
            {
				add(e)
				norm()
				b.sub(m)
				b.norm()
            }
            k--;
        }
    }
    /* get 8*MODBYTES size random number */
    static func random(rng: RAND) -> BIG
    {
        let m=BIG();
        var j:Int=0
        var r:UInt8=0
        /* generate random BIG */
        for var i=0;i<Int(8*ROM.MODBYTES);i++
        {
            if (j==0) {r=rng.getByte()}
            else {r>>=1}

            let b=Int32(r&1);
            m.shl(1); m.w[0]+=b;// m.inc(b);
            j++; j&=7;
        }
        return m;
    }

    /* Create random BIG in portable way, one bit at a time, less than q */
    static func randomnum(q: BIG,_ rng: RAND) -> BIG
    {
        let d=DBIG(0);
        var j:Int=0
        var r:UInt8=0

        for var i=0;i<Int(2*ROM.MODBITS);i++
        {
            if (j==0) {r=rng.getByte()}
            else {r>>=1}

            let b=Int32(r&1);
            d.shl(1); d.w[0]+=b; // m.inc(b);
            j++; j&=7;
        }
        let m=d.mod(q);
        return m;
    }

    /* return NAF value as +/- 1, 3 or 5. x and x3 should be normed.
    nbs is number of bits processed, and nzs is number of trailing 0s detected */
    static func nafbits(x: BIG,_ x3:BIG ,i:Int) -> [Int32]
    {
        var j:Int
        var n=[Int32](count:3,repeatedValue:0)
        var nb=x3.bit(i)-x.bit(i)
        n[1]=1;
        n[0]=0;
        if (nb==0) {n[0]=0; return n}
        if (i==0) {n[0]=nb; return n}
        if (nb>0) {n[0]=1}
        else      {n[0]=(-1)}

        for j=i-1;j>0;j--
        {
            n[1]++
            n[0]*=2
            nb=x3.bit(j)-x.bit(j)
            if (nb>0) {n[0]+=1}
            if (nb<0) {n[0]-=1}
            if (n[0]>5 || n[0] < -5) {break}
        }

        if ((n[0]%2 != 0) && (j != 0))
        { /* backtrack */
            if (nb>0) {n[0]=(n[0]-1)/2}
            if (nb<0) {n[0]=(n[0]+1)/2}
            n[1]--;
        }
        while (n[0]%2==0)
        { /* remove trailing zeros */
            n[0]/=2
            n[2]++
            n[1]--
        }
        return n;
    }
    /* Jacobi Symbol (this/p). Returns 0, 1 or -1 */
    func jacobi(p: BIG) -> Int
    {
        var n8:Int32
        var k:Int
        var m:Int=0;
        let t=BIG()
        let x=BIG()
        let n=BIG()
        let zilch=BIG()
        let one=BIG(1)
        if (p.parity()==0 || BIG.comp(self,zilch)==0 || BIG.comp(p,one)<=0) {return 0}
        norm()
        x.copy(self)
        n.copy(p)
        x.mod(p)

        while (BIG.comp(n,one)>0)
        {
            if (BIG.comp(x,zilch)==0) {return 0}
            n8=n.lastbits(3)
            k=0
            while (x.parity()==0)
            {
				k++
				x.shr(1)
            }
            if (k%2==1) {m+=(n8*n8-1)/8}
            m+=(n8-1)*(x.lastbits(2)-1)/4
            t.copy(n)
            t.mod(x)
            n.copy(x)
            x.copy(t)
            m%=2

        }
        if (m==0) {return 1}
        else {return -1}
    }
    /* this=1/this mod p. Binary method */
    func invmodp(p: BIG)
    {
        mod(p)
        let u=BIG(self)
        let v=BIG(p)
        let x1=BIG(1)
        let x2=BIG()
        let t=BIG()
        let one=BIG(1)

        while ((BIG.comp(u,one) != 0 ) && (BIG.comp(v,one) != 0 ))
        {
            while (u.parity()==0)
            {
				u.shr(1);
				if (x1.parity() != 0 )
				{
                    x1.add(p);
                    x1.norm();
				}
				x1.shr(1);
            }
            while (v.parity()==0)
            {
				v.shr(1);
				if (x2.parity() != 0 )
				{
                    x2.add(p);
                    x2.norm();
				}
				x2.shr(1);
            }
            if (BIG.comp(u,v)>=0)
            {
				u.sub(v);
				u.norm();
                if (BIG.comp(x1,x2)>=0) {x1.sub(x2)}
				else
				{
                    t.copy(p);
                    t.sub(x2);
                    x1.add(t);
				}
				x1.norm();
            }
            else
            {
				v.sub(u);
				v.norm();
                if (BIG.comp(x2,x1)>=0) {x2.sub(x1)}
				else
				{
                    t.copy(p);
                    t.sub(x1);
                    x2.add(t);
				}
				x2.norm();
            }
        }
        if (BIG.comp(u,one)==0) {copy(x1)}
        else {copy(x2)}
    }
    /* return a*b as DBIG */
    static func mul(a: BIG,_ b:BIG) -> DBIG
    {
        var t:Int64
        var co:Int64
        let c=DBIG()
        let RM:Int64=Int64(ROM.MASK);
        let RB:Int64=Int64(ROM.BASEBITS)
        a.norm();
        b.norm();

        t=Int64(a.w[0])*Int64(b.w[0]); c.w[0]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[1])*Int64(b.w[0])+Int64(a.w[0])*Int64(b.w[1])+co; c.w[1]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[2])*Int64(b.w[0])+Int64(a.w[1])*Int64(b.w[1])+Int64(a.w[0])*Int64(b.w[2])+co; c.w[2]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[3])*Int64(b.w[0])+Int64(a.w[2])*Int64(b.w[1])+Int64(a.w[1])*Int64(b.w[2])+Int64(a.w[0])*Int64(b.w[3])+co; c.w[3]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[4])*Int64(b.w[0])+Int64(a.w[3])*Int64(b.w[1])+Int64(a.w[2])*Int64(b.w[2])+Int64(a.w[1])*Int64(b.w[3])+Int64(a.w[0])*Int64(b.w[4])+co; c.w[4]=Int32(t&RM); co=t>>RB;
        t=Int64(a.w[5])*Int64(b.w[0])+Int64(a.w[4])*Int64(b.w[1])+Int64(a.w[3])*Int64(b.w[2])+Int64(a.w[2])*Int64(b.w[3])+Int64(a.w[1])*Int64(b.w[4])+Int64(a.w[0])*Int64(b.w[5])+co; c.w[5]=Int32(t&RM); co=t>>RB;
        t=Int64(a.w[6])*Int64(b.w[0])+Int64(a.w[5])*Int64(b.w[1])+Int64(a.w[4])*Int64(b.w[2])+Int64(a.w[3])*Int64(b.w[3])+Int64(a.w[2])*Int64(b.w[4])+Int64(a.w[1])*Int64(b.w[5])+Int64(a.w[0])*Int64(b.w[6])+co; c.w[6]=Int32(t&RM); co=t>>RB;
        t=Int64(a.w[7])*Int64(b.w[0])+Int64(a.w[6])*Int64(b.w[1])+Int64(a.w[5])*Int64(b.w[2])+Int64(a.w[4])*Int64(b.w[3])+Int64(a.w[3])*Int64(b.w[4])+Int64(a.w[2])*Int64(b.w[5])+Int64(a.w[1])*Int64(b.w[6])+Int64(a.w[0])*Int64(b.w[7])+co; c.w[7]=Int32(t&RM); co=t>>RB;
        t=Int64(a.w[8])*Int64(b.w[0])+Int64(a.w[7])*Int64(b.w[1])+Int64(a.w[6])*Int64(b.w[2])+Int64(a.w[5])*Int64(b.w[3])+Int64(a.w[4])*Int64(b.w[4])+Int64(a.w[3])*Int64(b.w[5])+Int64(a.w[2])*Int64(b.w[6])+Int64(a.w[1])*Int64(b.w[7])+Int64(a.w[0])*Int64(b.w[8])+co; c.w[8]=Int32(t&RM); co=t>>RB;

        t=Int64(a.w[8])*Int64(b.w[1])+Int64(a.w[7])*Int64(b.w[2])+Int64(a.w[6])*Int64(b.w[3])+Int64(a.w[5])*Int64(b.w[4])+Int64(a.w[4])*Int64(b.w[5])+Int64(a.w[3])*Int64(b.w[6])+Int64(a.w[2])*Int64(b.w[7])+Int64(a.w[1])*Int64(b.w[8])+co; c.w[9]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[8])*Int64(b.w[2])+Int64(a.w[7])*Int64(b.w[3])+Int64(a.w[6])*Int64(b.w[4])+Int64(a.w[5])*Int64(b.w[5])+Int64(a.w[4])*Int64(b.w[6])+Int64(a.w[3])*Int64(b.w[7])+Int64(a.w[2])*Int64(b.w[8])+co; c.w[10]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[8])*Int64(b.w[3])+Int64(a.w[7])*Int64(b.w[4])+Int64(a.w[6])*Int64(b.w[5])+Int64(a.w[5])*Int64(b.w[6])+Int64(a.w[4])*Int64(b.w[7])+Int64(a.w[3])*Int64(b.w[8])+co; c.w[11]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[8])*Int64(b.w[4])+Int64(a.w[7])*Int64(b.w[5])+Int64(a.w[6])*Int64(b.w[6])+Int64(a.w[5])*Int64(b.w[7])+Int64(a.w[4])*Int64(b.w[8])+co; c.w[12]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(b.w[5])+Int64(a.w[7])*Int64(b.w[6])+Int64(a.w[6])*Int64(b.w[7])+Int64(a.w[5])*Int64(b.w[8])+co; c.w[13]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(b.w[6])+Int64(a.w[7])*Int64(b.w[7])+Int64(a.w[6])*Int64(b.w[8])+co; c.w[14]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(b.w[7])+Int64(a.w[7])*Int64(b.w[8])+co; c.w[15]=Int32(t&RM); co=t>>RB

        t=Int64(a.w[8])*Int64(b.w[8])+co; c.w[16]=Int32(t&RM); co=t>>RB
        c.w[17]=Int32(co)

        return c
    }

    /* return a^2 as DBIG */
    static func sqr(a: BIG) -> DBIG
    {
        var t:Int64
        var co:Int64
        let c=DBIG()
        let RM:Int64=Int64(ROM.MASK);
        let RB:Int64=Int64(ROM.BASEBITS)
        a.norm();

        t=Int64(a.w[0])*Int64(a.w[0]); c.w[0]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[1])*Int64(a.w[0]); t+=t; t+=co; c.w[1]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[2])*Int64(a.w[0]);t+=t; t+=Int64(a.w[1])*Int64(a.w[1]);t+=co;c.w[2]=Int32(t&RM);co=t>>RB
        t=Int64(a.w[3])*Int64(a.w[0])+Int64(a.w[2])*Int64(a.w[1]); t+=t; t+=co; c.w[3]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[4])*Int64(a.w[0])+Int64(a.w[3])*Int64(a.w[1]); t+=t; t+=Int64(a.w[2])*Int64(a.w[2]); t+=co; c.w[4]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[5])*Int64(a.w[0])+Int64(a.w[4])*Int64(a.w[1])
            t = t+Int64(a.w[3])*Int64(a.w[2])
            t+=t; t+=co; c.w[5]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[6])*Int64(a.w[0])+Int64(a.w[5])*Int64(a.w[1])
            t = t+Int64(a.w[4])*Int64(a.w[2])
            t+=t; t+=Int64(a.w[3])*Int64(a.w[3]); t+=co; c.w[6]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[7])*Int64(a.w[0])+Int64(a.w[6])*Int64(a.w[1])
        t = t+Int64(a.w[5])*Int64(a.w[2])+Int64(a.w[4])*Int64(a.w[3])
            t+=t; t+=co; c.w[7]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[0])+Int64(a.w[7])*Int64(a.w[1])
            t = t+Int64(a.w[6])*Int64(a.w[2])+Int64(a.w[5])*Int64(a.w[3])
            t+=t; t+=Int64(a.w[4])*Int64(a.w[4]); t+=co; c.w[8]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[1])+Int64(a.w[7])*Int64(a.w[2])
        t = t+Int64(a.w[6])*Int64(a.w[3])+Int64(a.w[5])*Int64(a.w[4])
            t+=t; t+=co; c.w[9]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[2])+Int64(a.w[7])*Int64(a.w[3])
            t = t+Int64(a.w[6])*Int64(a.w[4])
            t+=t; t+=Int64(a.w[5])*Int64(a.w[5]); t+=co; c.w[10]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[3])+Int64(a.w[7])*Int64(a.w[4])
            t = t+Int64(a.w[6])*Int64(a.w[5])
            t+=t; t+=co; c.w[11]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[4])+Int64(a.w[7])*Int64(a.w[5]); t+=t; t+=Int64(a.w[6])*Int64(a.w[6]); t+=co; c.w[12]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[5])+Int64(a.w[7])*Int64(a.w[6]); t+=t; t+=co; c.w[13]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[6]); t+=t; t+=Int64(a.w[7])*Int64(a.w[7]); t+=co; c.w[14]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[7]); t+=t; t+=co; c.w[15]=Int32(t&RM); co=t>>RB
        t=Int64(a.w[8])*Int64(a.w[8])+co; c.w[16]=Int32(t&RM); co=t>>RB
        c.w[17]=Int32(co)

    return c;
    }

    /* reduce a DBIG to a BIG using the appropriate form of the modulus */
    static func mod(d: DBIG) -> BIG
    {
        var b=BIG()
        if (ROM.MODTYPE==ROM.PSEUDO_MERSENNE)
        {
            let t=d.split(ROM.MODBITS)
            b=BIG(d)
            let v=t.pmul(ROM.MConst);
            let tw=t.w[ROM.NLEN-1];
            t.w[ROM.NLEN-1] &= ROM.TMASK;
            t.inc(ROM.MConst*((tw>>ROM.TBITS)+(v<<(ROM.BASEBITS-ROM.TBITS))));

            b.add(t);
            b.norm();
        }
        if (ROM.MODTYPE==ROM.MONTGOMERY_FRIENDLY)
        {
            for var i=0;i<ROM.NLEN;i++
                {d.w[ROM.NLEN+i]+=d.muladd(d.w[i],ROM.MConst-1,d.w[i],ROM.NLEN+i-1)}

            b=BIG(0);

            for var i=0;i<ROM.NLEN;i++
            {
                b.w[i]=d.w[ROM.NLEN+i]
            }
            b.norm()
        }

        if (ROM.MODTYPE==ROM.NOT_SPECIAL)
        {
            let md=BIG(ROM.Modulus);

            var sum=Int64(d.w[0])
            for var j=0;j<ROM.NLEN;j++
            {
                for var i=0;i<j;i++ {sum+=Int64(d.w[i])*Int64(md.w[j-i])}
                let sp=(Int32(sum&Int64(ROM.MASK))&*ROM.MConst)&ROM.MASK
                d.w[j]=sp; sum+=Int64(sp)*Int64(md.w[0])
                sum=Int64(d.w[j+1])+(sum>>Int64(ROM.BASEBITS))
            }

            for var j=ROM.NLEN;j<ROM.DNLEN-2;j++
            {
                for var i=j-ROM.NLEN+1;i<ROM.NLEN;i++ {sum+=Int64(d.w[i])*Int64(md.w[j-i])}
                    d.w[j]=Int32(sum&Int64(ROM.MASK))
                sum=Int64(d.w[j+1])+(sum>>Int64(ROM.BASEBITS))
            }

            sum+=Int64(d.w[ROM.NLEN-1])*Int64(md.w[ROM.NLEN-1])
            d.w[ROM.DNLEN-2]=Int32(sum&Int64(ROM.MASK))
            sum=Int64(d.w[ROM.DNLEN-1])+(sum>>Int64(ROM.BASEBITS))
            d.w[ROM.DNLEN-1]=Int32(sum&Int64(ROM.MASK))

            b=BIG(0);

            for var i=0;i<ROM.NLEN;i++
            {
                b.w[i]=d.w[ROM.NLEN+i];
            }
            b.norm();
        }

        return b;
    }

    /* return a*b mod m */
    static func modmul(a: BIG,_ b :BIG,_ m: BIG) -> BIG
    {
        a.mod(m)
        b.mod(m)
        let d=mul(a,b)
        return d.mod(m)
    }

    /* return a^2 mod m */
    static func modsqr(a: BIG,_ m: BIG) -> BIG
    {
        a.mod(m)
        let d=sqr(a)
        return d.mod(m)
    }

    /* return -a mod m */
    static func modneg(a: BIG,_ m: BIG) -> BIG
    {
        a.mod(m)
        return m.minus(a)
    }

    /* return this^e mod m */
    func powmod(e: BIG,_ m: BIG) -> BIG
    {
        norm();
        e.norm();
        var a=BIG(1)
        let z=BIG(e)
        var s=BIG(self)
        while (true)
        {
            let bt=z.parity();
            z.fshr(1)
            if bt==1 {a=BIG.modmul(a,s,m)}
            if (z.iszilch()) {break}
            s=BIG.modsqr(s,m)
        }
        return a
    }
}
