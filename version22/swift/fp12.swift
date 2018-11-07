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
//  fp12.swift
//
//  Created by Michael Scott on 07/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

/* AMCL Fp^12 functions */
/* FP12 elements are of the form a+i.b+i^2.c */

final class FP12
{
    private final var a:FP4
    private final var b:FP4
    private final var c:FP4
    
    /* reduce all components of this mod Modulus */
    func reduce()
    {
        a.reduce()
        b.reduce()
        c.reduce()
    }
    /* normalise all components of this */
    func norm()
    {
        a.norm();
        b.norm();
        c.norm();
    }
    /* Constructors */
    init(_ d:FP4)
    {
        a=FP4(d)
        b=FP4(0)
        c=FP4(0)
    }
    
    init(_ d:Int)
    {
        a=FP4(d)
        b=FP4(0)
        c=FP4(0)
    }
    
    init(_ d:FP4,_ e:FP4,_ f:FP4)
    {
        a=FP4(d)
        b=FP4(e)
        c=FP4(f)
    }
    
    init(_ x:FP12)
    {
        a=FP4(x.a)
        b=FP4(x.b)
        c=FP4(x.c)
    }
    /* test x==0 ? */
    func iszilch() -> Bool
    {
        reduce();
        return a.iszilch() && b.iszilch() && c.iszilch()
    }
    /* test x==1 ? */
    func isunity() -> Bool
    {
        let one=FP4(1)
        return a.equals(one) && b.iszilch() && c.iszilch()
    }
    /* return 1 if x==y, else 0 */
    func equals(_ x:FP12) -> Bool
    {
        return a.equals(x.a) && b.equals(x.b) && c.equals(x.c)
    }
    /* extract a from self */
    func geta() -> FP4
    {
        return a
    }
    /* extract b */
    func getb()  -> FP4
    {
        return b
    }
    /* extract c */
    func getc() -> FP4
    {
        return c
    }
    /* copy self=x */
    func copy(_ x:FP12)
    {
        a.copy(x.a)
        b.copy(x.b)
        c.copy(x.c)
    }
    /* set self=1 */
    func one()
    {
        a.one()
        b.zero()
        c.zero()
    }
    /* self=conj(self) */
    func conj()
    {
        a.conj()
        b.nconj()
        c.conj()
    }
    /* Granger-Scott Unitary Squaring */
    func usqr()
    {
        let A=FP4(a)
        let B=FP4(c)
        let C=FP4(b)
        let D=FP4(0)
    
        a.sqr()
        D.copy(a); D.add(a)
        a.add(D)
    
        a.norm()
        A.nconj()
    
        A.add(A)
        a.add(A)
        B.sqr()
        B.times_i()
    
        D.copy(B); D.add(B)
        B.add(D)
        B.norm()
    
        C.sqr()
        D.copy(C); D.add(C)
        C.add(D)
        C.norm()
    
        b.conj()
        b.add(b)
        c.nconj()
    
        c.add(c)
        b.add(B)
        c.add(C)
        reduce()
    
    }
    /* Chung-Hasan SQR2 method from http://cacr.uwaterloo.ca/techreports/2006/cacr2006-24.pdf */
    func sqr()
    {
        let A=FP4(a)
        let B=FP4(b)
        let C=FP4(c)
        let D=FP4(a)
    
        A.sqr()
        B.mul(c)
        B.add(B)
        C.sqr()
        D.mul(b)
        D.add(D)

        c.add(a)
        c.add(b)
        c.sqr()
    
        a.copy(A)
    
        A.add(B)
        A.norm()
        A.add(C)
        A.add(D)
        A.norm()
    
        A.neg()
        B.times_i()
        C.times_i()
    
        a.add(B)

        b.copy(C); b.add(D)
        c.add(A)
    
        norm()
    }
    
    /* FP12 full multiplication this=this*y */
    func mul(_ y:FP12)
    {
        let z0=FP4(a)
        let z1=FP4(0)
        let z2=FP4(b)
        let z3=FP4(0)
        let t0=FP4(a)
        let t1=FP4(y.a)
    
        z0.mul(y.a)
        z2.mul(y.b)
    
        t0.add(b)
        t1.add(y.b)
    
        z1.copy(t0); z1.mul(t1)
        t0.copy(b); t0.add(c)
    
        t1.copy(y.b); t1.add(y.c)
        z3.copy(t0); z3.mul(t1)
    
        t0.copy(z0); t0.neg()
        t1.copy(z2); t1.neg()
    
        z1.add(t0)
        z1.norm()
        b.copy(z1); b.add(t1)
    
        z3.add(t1)
        z2.add(t0)
    
        t0.copy(a); t0.add(c)
        t1.copy(y.a); t1.add(y.c)
        t0.mul(t1)
        z2.add(t0)
    
        t0.copy(c); t0.mul(y.c)
        t1.copy(t0); t1.neg()
    
        z2.norm()
        z3.norm()
        b.norm()
    
        c.copy(z2); c.add(t1)
        z3.add(t1)
        t0.times_i()
        b.add(t0)
    
        z3.times_i()
        a.copy(z0); a.add(z3)
    
        norm()
    }
    
    /* Special case of multiplication arises from special form of ATE pairing line function */
    func smul(_ y:FP12)
    {
        let z0=FP4(a)
        let z2=FP4(b)
        let z3=FP4(b)
        let t0=FP4(0)
        let t1=FP4(y.a)
    
        z0.mul(y.a)
        z2.pmul(y.b.real())
        b.add(a)
        t1.real().add(y.b.real())
    
        b.mul(t1)
        z3.add(c)
        z3.pmul(y.b.real())
    
        t0.copy(z0); t0.neg()
        t1.copy(z2); t1.neg()
    
        b.add(t0)
        b.norm()
    
        b.add(t1)
        z3.add(t1)
        z2.add(t0)
    
        t0.copy(a); t0.add(c)
        t0.mul(y.a)
        c.copy(z2); c.add(t0)
    
        z3.times_i()
        a.copy(z0); a.add(z3)
    
        norm()
    }
    /* self=1/self */
    func inverse()
    {
        let f0=FP4(a)
        let f1=FP4(b)
        let f2=FP4(a)
        let f3=FP4(0)
    
        norm()
        f0.sqr()
        f1.mul(c)
        f1.times_i()
        f0.sub(f1)
    
        f1.copy(c); f1.sqr()
        f1.times_i()
        f2.mul(b)
        f1.sub(f2)
    
        f2.copy(b); f2.sqr()
        f3.copy(a); f3.mul(c)
        f2.sub(f3)
    
        f3.copy(b); f3.mul(f2)
        f3.times_i()
        a.mul(f0)
        f3.add(a)
        c.mul(f1)
        c.times_i()
    
        f3.add(c)
        f3.inverse()
        a.copy(f0); a.mul(f3)
        b.copy(f1); b.mul(f3)
        c.copy(f2); c.mul(f3)
    }
    
    /* self=self^p using Frobenius */
    func frob(_ f:FP2)
    {
        let f2=FP2(f)
        let f3=FP2(f)
    
        f2.sqr()
        f3.mul(f2)
    
        a.frob(f3)
        b.frob(f3)
        c.frob(f3)
    
        b.pmul(f)
        c.pmul(f2)
    }
    
    /* trace function */
    func trace() -> FP4
    {
        let t=FP4(0)
        t.copy(a)
        t.imul(3)
        t.reduce()
        return t
    }
    /* convert from byte array to FP12 */
    static func fromBytes(_ w:[UInt8]) -> FP12
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)
    
        for i in 0 ..< RM {t[i]=w[i]}
        var a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+RM]}
        var b=BIG.fromBytes(t)
        var c=FP2(a,b)
    
        for i in 0 ..< RM {t[i]=w[i+2*RM]}
        a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+3*RM]}
        b=BIG.fromBytes(t)
        var d=FP2(a,b)
    
        let e=FP4(c,d)
    
        for i in 0 ..< RM {t[i]=w[i+4*RM]}
        a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+5*RM]}
        b=BIG.fromBytes(t)
        c=FP2(a,b)
    
        for i in 0 ..< RM {t[i]=w[i+6*RM]}
        a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+7*RM]}
        b=BIG.fromBytes(t)
        d=FP2(a,b)
    
        let f=FP4(c,d)
    
    
        for i in 0 ..< RM {t[i]=w[i+8*RM]}
        a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+9*RM]}
        b=BIG.fromBytes(t)
        c=FP2(a,b)
    
        for i in 0 ..< RM {t[i]=w[i+10*RM]}
        a=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=w[i+11*RM]}
        b=BIG.fromBytes(t);
        d=FP2(a,b)
    
        let g=FP4(c,d)
    
        return FP12(e,f,g)
    }
    
    /* convert this to byte array */
    func toBytes(_ w:inout [UInt8])
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)

        a.geta().getA().toBytes(&t)
        for i in 0 ..< RM {w[i]=t[i]}
        a.geta().getB().toBytes(&t)
        for i in 0 ..< RM {w[i+RM]=t[i]}
        a.getb().getA().toBytes(&t)
        for i in 0 ..< RM {w[i+2*RM]=t[i]}
        a.getb().getB().toBytes(&t)
        for i in 0 ..< RM {w[i+3*RM]=t[i]}
    
        b.geta().getA().toBytes(&t)
        for i in 0 ..< RM {w[i+4*RM]=t[i]}
        b.geta().getB().toBytes(&t);
        for i in 0 ..< RM {w[i+5*RM]=t[i]}
        b.getb().getA().toBytes(&t)
        for i in 0 ..< RM {w[i+6*RM]=t[i]}
        b.getb().getB().toBytes(&t)
        for i in 0 ..< RM {w[i+7*RM]=t[i]}
    
        c.geta().getA().toBytes(&t)
        for i in 0 ..< RM {w[i+8*RM]=t[i]}
        c.geta().getB().toBytes(&t)
        for i in 0 ..< RM {w[i+9*RM]=t[i]}
        c.getb().getA().toBytes(&t)
        for i in 0 ..< RM {w[i+10*RM]=t[i]}
        c.getb().getB().toBytes(&t)
        for i in 0 ..< RM {w[i+11*RM]=t[i]}
    }
    /* convert to hex string */
    func toString() -> String
    {
        return ("["+a.toString()+","+b.toString()+","+c.toString()+"]")
    }
    
    /* self=self^e */
    /* Note this is simple square and multiply, so not side-channel safe */
    func pow(_ e:BIG) -> FP12
    {
        norm()
        e.norm()
        let w=FP12(self)
        let z=BIG(e)
        let r=FP12(1)
    
        while (true)
        {
            let bt=z.parity()
            z.fshr(1)
            if bt==1 {r.mul(w)}
            if z.iszilch() {break}
            w.usqr()
        }
        r.reduce()
        return r
    }
    /* constant time powering by small integer of max length bts */
    func pinpow(_ e:Int32,_ bts:Int32)
    {
        var R=[FP12]()
        R.append(FP12(1));
        R.append(FP12(self));

        //for var i=bts-1;i>=0;i--
        for i in (0...bts-1).reversed()
        {
            let b=Int((e>>i)&1)
            R[1-b].mul(R[b])
            R[b].usqr()
        }
        copy(R[0]);
    }
    
    /* p=q0^u0.q1^u1.q2^u2.q3^u3 */
    /* Timing attack secure, but not cache attack secure */
    
    static func pow4(_ q:[FP12],_ u:[BIG]) -> FP12
    {
        var a=[Int32](repeating: 0,count: 4)
        var g=[FP12]();
        
        for _ in 0 ..< 8 {g.append(FP12(0))}
        var s=[FP12]();
        for _ in 0 ..< 2 {s.append(FP12(0))}
        
        let c=FP12(1)
        let p=FP12(0)
        
        var t=[BIG]()
        for i in 0 ..< 4
            {t.append(BIG(u[i]))}
        
        let mt=BIG(0);
        var w=[Int8](repeating: 0,count: ROM.NLEN*Int(ROM.BASEBITS)+1)
    
        g[0].copy(q[0]); s[0].copy(q[1]); s[0].conj(); g[0].mul(s[0])
        g[1].copy(g[0])
        g[2].copy(g[0])
        g[3].copy(g[0])
        g[4].copy(q[0]); g[4].mul(q[1])
        g[5].copy(g[4])
        g[6].copy(g[4])
        g[7].copy(g[4])
    
        s[1].copy(q[2]); s[0].copy(q[3]); s[0].conj(); s[1].mul(s[0])
        s[0].copy(s[1]); s[0].conj(); g[1].mul(s[0])
        g[2].mul(s[1])
        g[5].mul(s[0])
        g[6].mul(s[1])
        s[1].copy(q[2]); s[1].mul(q[3])
        s[0].copy(s[1]); s[0].conj(); g[0].mul(s[0])
        g[3].mul(s[1])
        g[4].mul(s[0])
        g[7].mul(s[1])

    /* if power is even add 1 to power, and add q to correction */
    
        for i in 0 ..< 4
        {
            if t[i].parity()==0
            {
				t[i].inc(1); t[i].norm()
				c.mul(q[i])
            }
            mt.add(t[i]); mt.norm()
        }
        c.conj();
        let nb=1+mt.nbits();
    
    /* convert exponent to signed 1-bit window */
        for j in 0 ..< nb
        {
            for i in 0 ..< 4
            {
				a[i]=Int32(t[i].lastbits(2)-2)
				t[i].dec(Int(a[i]));
                t[i].norm()
				t[i].fshr(1)
            }
            w[j]=Int8(8*a[0]+4*a[1]+2*a[2]+a[3])
        }
        w[nb]=Int8(8*t[0].lastbits(2)+4*t[1].lastbits(2))
        w[nb]+=Int8(2*t[2].lastbits(2)+t[3].lastbits(2))
        p.copy(g[Int(w[nb]-1)/2])
    
        //for var i=nb-1;i>=0;i--
        for i in (0...nb-1).reversed()
        {
            let m=w[i]>>7
            let j=(w[i]^m)-m  /* j=abs(w[i]) */
            let k=Int((j-1)/2)
            s[0].copy(g[k]); s[1].copy(g[k]); s[1].conj()
            p.usqr()
            p.mul(s[Int(m&1)])
        }
        p.mul(c)  /* apply correction */
        p.reduce()
        return p
    }
    
    
    
    

}
