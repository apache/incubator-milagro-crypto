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

final class FP {
    var x:BIG
    static let p=BIG(ROM.Modulus)
/* convert to Montgomery n-residue form */
    func nres()
    {
        if ROM.MODTYPE != ROM.PSEUDO_MERSENNE && ROM.MODTYPE != ROM.GENERALISED_MERSENNE
        {
            let d=DBIG(x)
            d.shl(UInt(ROM.NLEN)*ROM.BASEBITS)
            x.copy(d.mod(FP.p))
        }
    }
/* convert back to regular form */
    func redc() -> BIG
    {
        if ROM.MODTYPE != ROM.PSEUDO_MERSENNE && ROM.MODTYPE != ROM.GENERALISED_MERSENNE
        {
            let d=DBIG(x)
            return BIG.mod(d)
        }
        else
        {
            let r=BIG(x)
            return r;
        }
    }
    
    init()
    {
        x=BIG(0)
    }
    init(_ a: Int)
    {
        x=BIG(a)
        nres()
    }
    init(_ a: BIG)
    {
        x=BIG(a)
        nres()
    }
    init(_ a: FP)
    {
        x=BIG(a.x)
    }
    /* convert to string */
    func toString() -> String
    {
        let s=redc().toString();
        return s;
    }
    
    func toRawString() -> String
    {
        let s=x.toRawString();
        return s;
    }
/* reduce this mod Modulus */
    func reduce()
    {
        x.mod(FP.p)
    }
    
/* test this=0? */
    func iszilch() -> Bool
    {
        reduce();
        return x.iszilch()
    }
    
/* copy from FP b */
    func copy(_ b: FP)
    {
        x.copy(b.x);
    }
    
/* set this=0 */
    func zero()
    {
        x.zero();
    }
    
/* set this=1 */
    func one()
    {
        x.one(); nres()
    }
    
/* normalise this */
    func norm()
    {
        x.norm();
    }
/* swap FPs depending on d */
    func cswap(_ b: FP,_ d: Int)
    {
        x.cswap(b.x,d)
    }
    
/* copy FPs depending on d */
    func cmove(_ b: FP,_ d:Int)
    {
        x.cmove(b.x,d);
    }
/* this*=b mod Modulus */
    func mul(_ b: FP)
    {
        norm()
        b.norm()
        let ea=BIG.EXCESS(x)
        let eb=BIG.EXCESS(b.x)
        
        if Int64(ea+1)*Int64(eb+1)>Int64(ROM.FEXCESS) {reduce()}
        /*if (ea+1)>=(ROM.FEXCESS-1)/(eb+1) {reduce()}*/
        
        let d=BIG.mul(x,b.x)
        x.copy(BIG.mod(d))
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
        return (r+1)
    }
    /* this = -this mod Modulus */
    func neg()
    {
        let m=BIG(FP.p);
    
        norm();
        let sb=FP.logb2(UInt32(BIG.EXCESS(x)))
 //       var ov=BIG.EXCESS(x);
 //       var sb=1; while(ov != 0) {sb += 1;ov>>=1}
    
        m.fshl(sb)
        x.rsub(m)
    
        if BIG.EXCESS(x)>=ROM.FEXCESS {reduce()}
    }
    /* this*=c mod Modulus, where c is a small int */
    func imul(_ c: Int)
    {
        var cc=c
        norm();
        var s=false
        if (cc<0)
        {
            cc = -cc
            s=true
        }
        let afx=(BIG.EXCESS(x)+1)*(cc+1)+1;
        if cc<ROM.NEXCESS && afx<ROM.FEXCESS
        {
            x.imul(cc);
        }
        else
        {
            if afx<ROM.FEXCESS {x.pmul(cc)}
            else
            {
				let d=x.pxmul(cc);
				x.copy(d.mod(FP.p));
            }
        }
        if s {neg()}
        norm();
    }
    
/* this*=this mod Modulus */
    func sqr()
    {
        norm()
        let ea=BIG.EXCESS(x);
        
        if Int64(ea+1)*Int64(ea+1)>Int64(ROM.FEXCESS) {reduce()}
        /*if (ea+1)>=(ROM.FEXCESS-1)/(ea+1) {reduce()}*/
        
        let d=BIG.sqr(x);
        x.copy(BIG.mod(d));
    }
    
    /* this+=b */
    func add(_ b: FP)
    {
        x.add(b.x);
        if BIG.EXCESS(x)+2>=ROM.FEXCESS {reduce()}
    }
/* this-=b */
    func sub(_ b: FP)
    {
        let n=FP(b)
        n.neg()
        self.add(n)
    }
/* this/=2 mod Modulus */
    func div2()
    {
        x.norm()
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
    func inverse()
    {
        let r=redc()
        r.invmodp(FP.p)
        x.copy(r)
        nres()
    }
    
/* return TRUE if this==a */
    func equals(_ a: FP) -> Bool
    {
        a.reduce()
        reduce()
        if (BIG.comp(a.x,x)==0) {return true}
        return false;
    }
/* return this^e mod Modulus */
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
    }
/* return sqrt(this) mod Modulus */
    func sqrt() -> FP
    {
        reduce();
        let b=BIG(FP.p)
        if (ROM.MOD8==5)
        {
            b.dec(5); b.norm(); b.shr(3)
            let i=FP(self); i.x.shl(1)
            let v=i.pow(b)
            i.mul(v); i.mul(v)
            i.x.dec(1)
            let r=FP(self)
            r.mul(v); r.mul(i)
            r.reduce()
            return r
        }
        else
        {
            b.inc(1); b.norm(); b.shr(2)
            return pow(b)
        }
    }
/* return jacobi symbol (this/Modulus) */
    func jacobi() -> Int
    {
        let w=redc()
        return w.jacobi(FP.p)
    }
    
}
