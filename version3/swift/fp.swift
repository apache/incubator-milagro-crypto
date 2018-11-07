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
//  fp.swift
//
//  Created by Michael Scott on 20/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//  Small Finite Field arithmetic
//  AMCL mod p functions
//

public struct FP {


    static public let NOT_SPECIAL=0
    static public let PSEUDO_MERSENNE=1
    static public let MONTGOMERY_FRIENDLY=2
    static public let GENERALISED_MERSENNE=3

    static public let MODBITS:UInt = @NBT@
    static let MOD8:UInt = @M8@
    static public let MODTYPE =  @MT@   

    static let FEXCESS:Int32 = ((Int32(1)<<@SH@)-1)
    static let OMASK:Chunk=Chunk(-1)<<Chunk(FP.MODBITS%BIG.BASEBITS)
    static let TBITS:UInt=FP.MODBITS%BIG.BASEBITS; // Number of active bits in top word
    static let TMASK:Chunk=(1<<Chunk(FP.TBITS))-1

    var x:BIG
    var xes:Int32
    static let p=BIG(ROM.Modulus)
    static let r2modp=BIG(ROM.R2modp)

/* convert to Montgomery n-residue form */
    mutating func nres()
    {
        if FP.MODTYPE != FP.PSEUDO_MERSENNE && FP.MODTYPE != FP.GENERALISED_MERSENNE
        {
            var d=BIG.mul(x,FP.r2modp);
            x.copy(FP.mod(&d))
            xes=2
        } else {xes=1}
    }
/* convert back to regular form */
    func redc() -> BIG
    {
        if FP.MODTYPE != FP.PSEUDO_MERSENNE && FP.MODTYPE != FP.GENERALISED_MERSENNE
        {
            var d=DBIG(x)
            return FP.mod(&d)
        }
        else
        {
            let r=BIG(x)
            return r;
        }
    }
    
    /* reduce a DBIG to a BIG using the appropriate form of the modulus */
    static func mod(_ d: inout DBIG) -> BIG
    {
 
        if FP.MODTYPE==FP.PSEUDO_MERSENNE
        {
            var t=d.split(FP.MODBITS)
            let b=BIG(d)
            let v=t.pmul(Int(ROM.MConst))

            t.add(b)
            t.norm()


            let tw=t.w[BIG.NLEN-1]
            t.w[BIG.NLEN-1] &= TMASK
            t.inc(Int(ROM.MConst*((tw>>Chunk(FP.TBITS))+(v<<Chunk(BIG.BASEBITS-FP.TBITS)))))
    
            t.norm()
            return t

        }
        if FP.MODTYPE==FP.MONTGOMERY_FRIENDLY
        {
            for i in 0 ..< BIG.NLEN {
                let (top,bot)=BIG.muladd(d.w[i],ROM.MConst-1,d.w[i],d.w[BIG.NLEN+i-1])
                d.w[BIG.NLEN+i]+=top; d.w[BIG.NLEN+i-1]=bot
 //                   d.w[BIG.NLEN+i]+=d.muladd(d.w[i],ROM.MConst-1,d.w[i],BIG.NLEN+i-1)
            }
    
            var b=BIG(0);
    
            for i in 0 ..< BIG.NLEN
            {
                b.w[i]=d.w[BIG.NLEN+i]
            }
            b.norm()
            return b;
        }
        if FP.MODTYPE==FP.GENERALISED_MERSENNE
        { // GoldiLocks Only
            let t=d.split(FP.MODBITS)
            let RM2=FP.MODBITS/2
            var b=BIG(d)
            b.add(t)
            var dd=DBIG(t)
            dd.shl(RM2)
            
            var tt=dd.split(FP.MODBITS)
            let lo=BIG(dd)
            b.add(tt)
            b.add(lo)
            b.norm()
            tt.shl(RM2)
            b.add(tt)
            
            let carry=b.w[BIG.NLEN-1]>>Chunk(FP.TBITS)
            b.w[BIG.NLEN-1]&=TMASK
            b.w[0]+=carry
            
            b.w[Int(224/BIG.BASEBITS)]+=carry<<Chunk(224%BIG.BASEBITS)
            b.norm()
            return b;
        }
        if FP.MODTYPE==FP.NOT_SPECIAL
        {
            let md=BIG(ROM.Modulus);

            return BIG.monty(md,ROM.MConst,&d)
        }
        return BIG(0)
    }


    init()
    {
        x=BIG(0)
        xes=1
    }
    init(_ a: Int)
    {
        x=BIG(a)
        xes=1
        nres()
    }
    init(_ a: BIG)
    {
        x=BIG(a)
        xes=1
        nres()
    }
    init(_ a: FP)
    {
        x=BIG(a.x)
        xes=a.xes
    }
    /* convert to string */
    func toString() -> String
    {
        let s=redc().toString()
        return s
    }
    
    func toRawString() -> String
    {
        let s=x.toRawString()
        return s
    }



/* reduce this mod Modulus */
    mutating func reduce()
    {

        var m=BIG(FP.p)
        var r=BIG(FP.p)
        var sb:Int

        x.norm()

	   if xes>16 {
		  let q=FP.quo(x,m)
		  let carry=r.pmul(q)
		  r.w[BIG.NLEN-1]+=carry<<Chunk(BIG.BASEBITS); // correction - put any carry out back in again
		  x.sub(r)
		  x.norm()		
		  sb=2
	   } else {
		  sb=FP.logb2(UInt32(xes-Int32(1)))
	   }
        m.fshl(sb)

        while sb>0 {
            let sr=BIG.ssn(&r,x,&m)
            x.cmove(r,1-sr)
            sb -= 1
        }
	
        xes=1
    }
    
/* test this=0? */
    func iszilch() -> Bool
    {
        var z=FP(self)
        z.reduce()
        return z.x.iszilch()
    }
    
/* copy from FP b */
    mutating func copy(_ b: FP)
    {
        x.copy(b.x)
        xes=b.xes
    }
    
/* set this=0 */
    mutating func zero()
    {
        x.zero();
        xes=1;
    }
    
/* set this=1 */
    mutating func one()
    {
        x.one(); nres()
    }
    
/* normalise this */
    mutating func norm()
    {
        x.norm();
    }
/* swap FPs depending on d */
    mutating func cswap(_ b: inout FP,_ d: Int)
    {
        var c=Int32(d)
        x.cswap(&(b.x),d)
        c = ~(c-1)
        let t=c&(xes^b.xes)
        xes^=t
        b.xes^=t        
    }
    
/* copy FPs depending on d */
    mutating func cmove(_ b: FP,_ d:Int)
    {
        let c=Int32(-d)
        x.cmove(b.x,d)
        xes^=(xes^b.xes)&c        
    }
/* this*=b mod Modulus */
    mutating func mul(_ b: FP)
    {

        if Int64(xes)*Int64(b.xes) > Int64(FP.FEXCESS) {reduce()}
        
        var d=BIG.mul(x,b.x)
        x.copy(FP.mod(&d))
        xes=2
    }
    static func logb2(_ w: UInt32) -> Int
    {
        var v = w
        v |= (v >> 1)
        v |= (v >> 2)
        v |= (v >> 4)
        v |= (v >> 8)
        v |= (v >> 16)
        
        v = v - ((v >> 1) & 0x55555555)
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333)
        let r = Int((   ((v + (v >> 4)) & 0xF0F0F0F)   &* 0x1010101) >> 24)
        return (r)
    }

// find appoximation to quotient of a/m
// Out by at most 2.
// Note that MAXXES is bounded to be 2-bits less than half a word
    static func quo(_ n: BIG,_ m: BIG) -> Int
    {
        let hb=UInt(BIG.CHUNK)/2
        if FP.TBITS < hb {
		  let sh=Chunk(hb-FP.TBITS);
		  let num=((n.w[BIG.NLEN-1]<<sh))|(n.w[BIG.NLEN-2]>>(Chunk(BIG.BASEBITS)-sh));
		  let den=((m.w[BIG.NLEN-1]<<sh))|(m.w[BIG.NLEN-2]>>(Chunk(BIG.BASEBITS)-sh));
		  return Int(num/(den+1));
	   } else {
		  let num=n.w[BIG.NLEN-1];
		  let den=m.w[BIG.NLEN-1];
		  return Int(num/(den+1));
	   }
    }

    /* this = -this mod Modulus */
    mutating func neg()
    {
        var m=BIG(FP.p)
        let sb=FP.logb2(UInt32(xes-Int32(1)))
        m.fshl(sb)
        x.rsub(m)
        xes=(1<<Int32(sb))+1
        if xes>FP.FEXCESS {reduce()}
    }
    /* this*=c mod Modulus, where c is a small int */
    mutating func imul(_ c: Int)
    {
        var cc=c
    //    norm();
        var s=false
        if (cc<0)
        {
            cc = -cc
            s=true
        }

        if FP.MODTYPE==FP.PSEUDO_MERSENNE || FP.MODTYPE==FP.GENERALISED_MERSENNE
        {
            var d=x.pxmul(cc)
            x.copy(FP.mod(&d))
            xes=2
        }
        else {
            if xes*Int32(cc)<FP.FEXCESS
            {
                x.pmul(cc)
                xes*=Int32(cc);
            }
            else {
                let n=FP(cc)
                self.mul(n)
            }
        }

        if s {neg();  norm()}
       
    }
    
/* this*=this mod Modulus */
    mutating func sqr()
    {
        if Int64(xes)*Int64(xes) > Int64(FP.FEXCESS) {reduce()}   
        var d=BIG.sqr(x);
        x.copy(FP.mod(&d));
        xes=2
    }
    
    /* this+=b */
    mutating func add(_ b: FP)
    {
        x.add(b.x);
        xes+=b.xes
        if xes>FP.FEXCESS {reduce()}
    }
/* this-=b */
    mutating func sub(_ b: FP)
    {
        var n=FP(b)
        n.neg()
        self.add(n)
    }
/* this=b-this */
    mutating func rsub(_ b: FP)
    {
        self.neg();
        self.add(b)
    }
/* this/=2 mod Modulus */
    mutating func div2()
    {
    //    x.norm()
        if (x.parity()==0)
            {x.fshr(1)}
        else
        {
            x.add(FP.p)
            x.norm()
            x.fshr(1)
        }
    }
/* this=1/this mod Modulus */

    mutating func fpow() -> FP 
    {
        var ac: [Int] = [1, 2, 3, 6, 12, 15, 30, 60, 120, 240, 255]
        var xp=[FP]() 
// phase 1
        xp.append(FP(self))
        xp.append(FP(self)); xp[1].sqr()
        xp.append(FP(xp[1])); xp[2].mul(self)
        xp.append(FP(xp[2])); xp[3].sqr()
        xp.append(FP(xp[3])); xp[4].sqr()
        xp.append(FP(xp[4])); xp[5].mul(xp[2])
        xp.append(FP(xp[5])); xp[6].sqr()
        xp.append(FP(xp[6])); xp[7].sqr()
        xp.append(FP(xp[7])); xp[8].sqr()
        xp.append(FP(xp[8])); xp[9].sqr()
        xp.append(FP(xp[9])); xp[10].mul(xp[5])

        var n: Int
        var c: Int

        if (FP.MOD8==5)
        {
            n=Int(FP.MODBITS)-3
            c=(Int(ROM.MConst)+5)/8
        } else {
            n=Int(FP.MODBITS)-2
            c=(Int(ROM.MConst)+3)/4            
        }


        var bw=0; var w=1; while w<c {w*=2; bw+=1}
        var k=w-c

        var i=10; var key=FP(0)

        if k != 0 {
            while ac[i]>k {i-=1}
            key.copy(xp[i])
            k-=ac[i]
        }
        while k != 0 {
            i-=1
            if ac[i]>k {continue}
            key.mul(xp[i])
            k-=ac[i] 
        }

// phase 2 
        xp[1].copy(xp[2])
        xp[2].copy(xp[5])
        xp[3].copy(xp[10])

        var j=3; var m=8
        let nw=n-bw
        var t=FP(0)

        while 2*m<nw {
            t.copy(xp[j]); j+=1
            for _ in 0..<m {t.sqr()} 
            xp[j].copy(xp[j-1])
            xp[j].mul(t)
            m*=2
        }

        var lo=nw-m
        var r=FP(xp[j])

        while lo != 0 {
            m/=2; j-=1
            if lo<m {continue}
            lo-=m
            t.copy(r)
            for _ in 0..<m {t.sqr()}
            r.copy(t)
            r.mul(xp[j])
        }

        for _ in 0..<bw {r.sqr()}

        if w-c != 0 {
            r.mul(key)
        }
        return r        
    }

    mutating func inverse()
    {
        if FP.MODTYPE==FP.PSEUDO_MERSENNE {
            var y=fpow()
            if (FP.MOD8==5)
            {
                var t=FP(self)
                t.sqr()
                mul(t)
                y.sqr()

            } 
            y.sqr()
            y.sqr()
            mul(y)
        } else {
            var m2=BIG(ROM.Modulus)
            m2.dec(2); m2.norm()
            copy(pow(m2))
        }

    }
    
/* return TRUE if this==a */
    func equals(_ a: FP) -> Bool
    {
        var f=FP(self)
        var s=FP(a)
        f.reduce()
        s.reduce()
        if (BIG.comp(f.x,s.x)==0) {return true}
        return false;
    }


/* return this^e mod Modulus */
    mutating func pow(_ e: BIG) -> FP
    {
        var tb=[FP]() 
        let n=1+(BIG.NLEN*Int(BIG.BASEBITS)+3)/4
        var w=[Int8](repeating: 0,count: n)     
        norm()
        var t=BIG(e); t.norm()
        let nb=1+(t.nbits()+3)/4    

        for i in 0 ..< nb  {
            let lsbs=t.lastbits(4)
            t.dec(lsbs)
            t.norm()
            w[i]=Int8(lsbs)
            t.fshr(4);
        }
        tb.append(FP(1))
        tb.append(FP(self))
        for i in 2 ..< 16 {
            tb.append(FP(tb[i-1]))
            tb[i].mul(self)
        }
        var r=FP(tb[Int(w[nb-1])])
        for i in (0...nb-2).reversed() {
            r.sqr()
            r.sqr()
            r.sqr()
            r.sqr()
            r.mul(tb[Int(w[i])])
        }
        r.reduce()
        return r
    }


/* return this^e mod Modulus 
    func pow(_ e: BIG) -> FP
    {
        let r=FP(1)
        e.norm()
        x.norm()
	   let m=FP(self)
        while (true)
        {
            let bt=e.parity()
            e.fshr(1)
            if bt==1 {r.mul(m)}
            if e.iszilch() {break}
            m.sqr();
        }
        r.x.mod(FP.p);
        return r;
    } */

/* return sqrt(this) mod Modulus */
    mutating func sqrt() -> FP
    {
        reduce();
        if (FP.MOD8==5)
        {
            var v: FP
            var i=FP(self); i.x.shl(1)
            if FP.MODTYPE==FP.PSEUDO_MERSENNE {
                v=i.fpow()
            } else {       
                var b=BIG(FP.p)
                b.dec(5); b.norm(); b.shr(3)
                v=i.pow(b)
            }
            i.mul(v); i.mul(v)
            i.x.dec(1)
            var r=FP(self)
            r.mul(v); r.mul(i)
            r.reduce()
            return r
        }
        else
        {
           if FP.MODTYPE==FP.PSEUDO_MERSENNE {
                var r=fpow()
                r.mul(self)
                return r
            } else {                   
                var b=BIG(FP.p)
                b.inc(1); b.norm(); b.shr(2)
                return pow(b)
            }
        }
    }
/* return jacobi symbol (this/Modulus) */
    func jacobi() -> Int
    {
        var w=redc()
        return w.jacobi(FP.p)
    }
    
}
