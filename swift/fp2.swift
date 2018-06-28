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


final public class FP2
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
    
    public init(_ c:BIG,_ d:BIG)
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
    //    norm();
        let m=FP(a)
        let t=FP(0)
    
        m.add(b)
        m.neg()
    //    m.norm()
        t.copy(m); t.add(b)
        b.copy(m)
        b.add(a)
        a.copy(t)
    }
    
    /* set to a-ib */
    func conj()
    {
        b.neg(); b.norm()
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

    /* self=a-self */
    func rsub(_ x:FP2)
    {
        self.neg()
        self.add(x)
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
        let w1=FP(a)
        let w3=FP(a)
        let mb=FP(b)

        w1.add(b)

        w3.add(a)
        w3.norm()
        b.mul(w3)

        mb.neg()
        a.add(mb)

        a.norm()
        w1.norm()

        a.mul(w1)

    }
    /* self*=y */
    func mul(_ y:FP2)
    { 
        if Int64(a.xes+b.xes)*Int64(y.a.xes+y.b.xes) > Int64(FP.FEXCESS)
        {
            if a.xes>1 {a.reduce()}
            if b.xes>1 {b.reduce()}       
        }

        let pR=DBIG(0)
        pR.ucopy(FP.p)

        let C=BIG(a.x)
        let D=BIG(y.a.x)

        let A=BIG.mul(a.x,y.a.x)
        let B=BIG.mul(b.x,y.b.x)

        C.add(b.x); C.norm()
        D.add(y.b.x); D.norm()

        let E=BIG.mul(C,D)
        let F=DBIG(A); F.add(B);
        B.rsub(pR);

        A.add(B); A.norm()
        E.sub(F); E.norm()

        a.x.copy(FP.mod(A)); a.xes=3
        b.x.copy(FP.mod(E)); b.xes=2
    
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
        w2.copy(a); w2.add(w1); w2.norm(); w2.div2()
        if w2.jacobi() != 1
        {
            w2.copy(a); w2.sub(w1); w2.norm(); w2.div2()
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
        w1.neg(); w1.norm()
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
    //    norm();
        let t=FP2(self)
        let z=FP(a)
        a.copy(b)
        a.neg()
        b.copy(z)
        add(t)
    //    norm()
    }
    /* w/=(1+sqrt(-1)) */
    func div_ip()
    {
        let t=FP2(0)
        norm()
        t.a.copy(a); t.a.add(b)
        t.b.copy(b); t.b.sub(a)
        copy(t); norm()
        div2()
    }
    
    func div_ip2()
    {
        let t=FP2(0)
	norm()
        t.a.copy(a); t.a.add(b)
        t.b.copy(b); t.b.sub(a)
        copy(t); norm()
    }

}
