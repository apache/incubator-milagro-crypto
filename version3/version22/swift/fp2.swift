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
//  fp2.swift
//
//  Created by Michael Scott on 07/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

/* Finite Field arithmetic  Fp^2 functions */

/* FP2 elements are of the form a+ib, where i is sqrt(-1) */


final class FP2
{
    private var a:FP
    private var b:FP

    /* Constructors */
    init(_ c: Int)
    {
        a=FP(c)
        b=FP(0)
    }
    
    init(_ x:FP2)
    {
        a=FP(x.a)
        b=FP(x.b)
    }

    init(_ c:FP,_ d:FP)
    {
        a=FP(c)
        b=FP(d)
    }
    
    init(_ c:BIG,_ d:BIG)
    {
        a=FP(c)
        b=FP(d)
    }

    init(_ c:FP)
    {
        a=FP(c)
        b=FP(0)
    }
    
    init(_ c:BIG)
    {
        a=FP(c)
        b=FP(0)
    }

    /* test this=0 ? */
    func iszilch() -> Bool
    {
        reduce()
        return (a.iszilch() && b.iszilch())
    }
    
    func cmove(_ g:FP2,_ d:Int)
    {
        a.cmove(g.a,d)
        b.cmove(g.b,d)
    }

    /* test this=1 ? */
    func isunity() -> Bool
    {
        let one=FP(1)
        return (a.equals(one) && b.iszilch())
    }
    
    /* test this=x */
    func equals(_ x:FP2) -> Bool
    {
        return (a.equals(x.a) && b.equals(x.b));
    }
    
    
    /* reduce components mod Modulus */
    func reduce()
    {
        a.reduce()
        b.reduce()
    }
    
    /* normalise components of w */
    func norm()
    {
        a.norm()
        b.norm()
    }
    
    /* extract a */
    func getA() -> BIG
    {
        return a.redc()
    }
    
    /* extract b */
    func getB() -> BIG
    {
        return b.redc()
    }

    /* copy self=x */
    func copy(_ x:FP2)
    {
        a.copy(x.a)
        b.copy(x.b)
    }
    
    /* set self=0 */
    func zero()
    {
        a.zero()
        b.zero()
    }
    
    /* set self=1 */
    func one()
    {
        a.one()
        b.zero()
    }
    
    /* negate self mod Modulus */
    func neg()
    {
        norm();
        let m=FP(a)
        let t=FP(0)
    
        m.add(b)
        m.neg()
        m.norm()
        t.copy(m); t.add(b)
        b.copy(m)
        b.add(a)
        a.copy(t)
    }
    
    /* set to a-ib */
    func conj()
    {
        b.neg()
    }

    /* self+=a */
    func add(_ x:FP2)
    {
        a.add(x.a)
        b.add(x.b)
    }
    
    /* self-=a */
    func sub(_ x:FP2)
    {
        let m=FP2(x)
        m.neg()
        add(m)
    }

    /* self*=s, where s is an FP */
    func pmul(_ s:FP)
    {
        a.mul(s)
        b.mul(s)
    }
    
    /* self*=i, where i is an int */
    func imul(_ c:Int)
    {
        a.imul(c);
        b.imul(c);
    }
    
    /* self*=self */
    func sqr()
    {
        norm();
    
        let w1=FP(a)
        let w3=FP(a)
        let mb=FP(b)
        w3.mul(b)
        w1.add(b)
        mb.neg()
        a.add(mb)
        a.mul(w1)
        b.copy(w3); b.add(w3)
        norm()
    }
    /* self*=y */
    func mul(_ y:FP2)
    {
        norm();  /* This is needed here as {a,b} is not normed before additions */
    
        let w1=FP(a)
        let w2=FP(b)
        let w5=FP(a)
        let mw=FP(0)
    
        w1.mul(y.a)  // w1=a*y.a  - this norms w1 and y.a, NOT a
        w2.mul(y.b)  // w2=b*y.b  - this norms w2 and y.b, NOT b
        w5.add(b)    // w5=a+b
        b.copy(y.a); b.add(y.b) // b=y.a+y.b
    
        b.mul(w5)
        mw.copy(w1); mw.add(w2); mw.neg()
    
        b.add(mw); mw.add(w1)
        a.copy(w1);	a.add(mw)
    
        norm()
    
    }
 
    /* sqrt(a+ib) = sqrt(a+sqrt(a*a-n*b*b)/2)+ib/(2*sqrt(a+sqrt(a*a-n*b*b)/2)) */
    /* returns true if this is QR */
    func sqrt() -> Bool
    {
        if iszilch() {return true}
        var w1=FP(b)
        var w2=FP(a)
        w1.sqr(); w2.sqr(); w1.add(w2)
        if w1.jacobi() != 1 { zero(); return false; }
        w1=w1.sqrt()
        w2.copy(a); w2.add(w1); w2.div2()
        if w2.jacobi() != 1
        {
            w2.copy(a); w2.sub(w1); w2.div2()
            if w2.jacobi() != 1 { zero(); return false }
        }
        w2=w2.sqrt()
        a.copy(w2)
        w2.add(w2)
        w2.inverse()
        b.mul(w2)
        return true
    }
    /* output to hex string */
    func toString() -> String
    {
        return ("["+a.toString()+","+b.toString()+"]")
    }
    
    func toRawString() -> String
    {
        return ("["+a.toRawString()+","+b.toRawString()+"]")
    }

    /* self=1/self */
    func inverse()
    {
        norm();
        let w1=FP(a)
        let w2=FP(b)
    
        w1.sqr()
        w2.sqr()
        w1.add(w2)
        w1.inverse()
        a.mul(w1)
        w1.neg()
        b.mul(w1)
    }

    /* self/=2 */
    func div2()
    {
        a.div2();
        b.div2();
    }
    
    /* self*=sqrt(-1) */
    func times_i()
    {
        let z=FP(a)
        a.copy(b); a.neg()
        b.copy(z)
    }

    /* w*=(1+sqrt(-1)) */
    /* where X*2-(1+sqrt(-1)) is irreducible for FP4, assumes p=3 mod 8 */
    func mul_ip()
    {
        norm();
        let t=FP2(self)
        let z=FP(a)
        a.copy(b)
        a.neg()
        b.copy(z)
        add(t)
        norm()
    }
    /* w/=(1+sqrt(-1)) */
    func div_ip()
    {
        let t=FP2(0)
        norm()
        t.a.copy(a); t.a.add(b)
        t.b.copy(b); t.b.sub(a)
        copy(t)
        div2()
    }
    
}
