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
//  ecp2.swift
//
//  Created by Michael Scott on 07/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

/* AMCL Weierstrass elliptic curve functions over FP2 */

final class ECP2 {
    private var x:FP2
    private var y:FP2
    private var z:FP2
    private var INF:Bool
    
    /* Constructor - set self=O */
    init()
    {
        INF=true
        x=FP2(0)
        y=FP2(1)
        z=FP2(1)
    }
    /* Test self=O? */
    func is_infinity() -> Bool
    {
        return INF
    }
    /* copy self=P */
    func copy(_ P:ECP2)
    {
        x.copy(P.x)
        y.copy(P.y)
        z.copy(P.z)
        INF=P.INF
    }
    /* set self=O */
    func inf() {
        INF=true
        x.zero()
        y.zero()
        z.zero()
    }
    /* Conditional move of Q to P dependant on d */
    func cmove(_ Q:ECP2,_ d:Int)
    {
        x.cmove(Q.x,d);
        y.cmove(Q.y,d);
        z.cmove(Q.z,d);
    
        var bd:Bool
        if d==0 {bd=false}
        else {bd=true}
        INF = (INF != ((INF != Q.INF) && bd))
    }
    
    /* return 1 if b==c, no branching */
    private static func teq(_ b:Int32,_ c:Int32) -> Int
    {
        var x=b^c
        x-=1  // if x=0, x now -1
        return Int((x>>31)&1)
    }
    /* Constant time select from pre-computed table */
    func select(_ W:[ECP2],_ b:Int32)
    {
        let MP=ECP2()
        let m=b>>31
        var babs=(b^m)-m
        
        babs=(babs-1)/2
    
        cmove(W[0],ECP2.teq(babs,0)) // conditional move
        cmove(W[1],ECP2.teq(babs,1))
        cmove(W[2],ECP2.teq(babs,2))
        cmove(W[3],ECP2.teq(babs,3))
        cmove(W[4],ECP2.teq(babs,4))
        cmove(W[5],ECP2.teq(babs,5))
        cmove(W[6],ECP2.teq(babs,6))
        cmove(W[7],ECP2.teq(babs,7))
    
        MP.copy(self)
        MP.neg()
        cmove(MP,Int(m&1))
    }
 
    /* Test if P == Q */
    func equals(_ Q:ECP2) -> Bool
    {
        if is_infinity() && Q.is_infinity() {return true}
        if is_infinity() || Q.is_infinity() {return false}
    
        let zs2=FP2(z); zs2.sqr()
        let zo2=FP2(Q.z); zo2.sqr()
        let zs3=FP2(zs2); zs3.mul(z)
        let zo3=FP2(zo2); zo3.mul(Q.z)
        zs2.mul(Q.x)
        zo2.mul(x)
        if !zs2.equals(zo2) {return false}
        zs3.mul(Q.y)
        zo3.mul(y)
        if !zs3.equals(zo3) {return false}
    
        return true;
    }
    /* set self=-self */
    func neg()
    {
        if is_infinity() {return}
        y.neg(); y.norm()
        return
    }
    /* set to Affine - (x,y,z) to (x,y) */
    func affine() {
        if is_infinity() {return}
        let one=FP2(1)
        if z.equals(one) {return}
        z.inverse()
    
        let z2=FP2(z)
        z2.sqr()
        x.mul(z2); x.reduce()
        y.mul(z2)
        y.mul(z);  y.reduce()
        z.copy(one)
    }
    /* extract affine x as FP2 */
    func getX() -> FP2
    {
        affine()
        return x
    }
    /* extract affine y as FP2 */
    func getY() -> FP2
    {
        affine()
        return y
    }
    /* extract projective x */
    func getx() -> FP2
    {
        return x
    }
    /* extract projective y */
    func gety() -> FP2
    {
        return y
    }
    /* extract projective z */
    func getz() -> FP2
    {
        return z
    }
    /* convert to byte array */
    func toBytes(_ b:inout [UInt8])
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)

        affine();
        x.getA().toBytes(&t)
        for i in 0 ..< RM
            {b[i]=t[i]}
        x.getB().toBytes(&t);
        for i in 0 ..< RM
            {b[i+RM]=t[i]}
    
        y.getA().toBytes(&t);
        for i in 0 ..< RM
            {b[i+2*RM]=t[i]}
        y.getB().toBytes(&t);
        for i in 0 ..< RM
            {b[i+3*RM]=t[i]}
    }
    /* convert from byte array to point */
    static func fromBytes(_ b:[UInt8]) -> ECP2
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)

    
        for i in 0 ..< RM {t[i]=b[i]}
        var ra=BIG.fromBytes(t);
        for i in 0 ..< RM {t[i]=b[i+RM]}
        var rb=BIG.fromBytes(t);
        let rx=FP2(ra,rb)
    
        for i in 0 ..< RM {t[i]=b[i+2*RM]}
        ra=BIG.fromBytes(t)
        for i in 0 ..< RM {t[i]=b[i+3*RM]}
        rb=BIG.fromBytes(t)
        let ry=FP2(ra,rb)
    
        return ECP2(rx,ry)
    }
/* convert self to hex string */
    func toString() -> String
    {
        if is_infinity() {return "infinity"}
        affine()
        return "("+x.toString()+","+y.toString()+")"
    }
    
/* Calculate RHS of twisted curve equation x^3+B/i */
    static func RHS(_ x:FP2) -> FP2
    {
        x.norm()
        let r=FP2(x)
        r.sqr()
        let b=FP2(BIG(ROM.CURVE_B))
        b.div_ip();
        r.mul(x);
        r.add(b);
    
        r.reduce();
        return r;
    }
/* construct self from (x,y) - but set to O if not on curve */
    init(_ ix:FP2,_ iy:FP2)
    {
        x=FP2(ix)
        y=FP2(iy)
        z=FP2(1)
        let rhs=ECP2.RHS(x)
        let y2=FP2(y)
        y2.sqr()
        if y2.equals(rhs) {INF=false}
        else {x.zero(); INF=true}
    }
    /* construct this from x - but set to O if not on curve */
    init(_ ix:FP2)
    {
        x=FP2(ix)
        y=FP2(1)
        z=FP2(1)
        let rhs=ECP2.RHS(x)
        if rhs.sqrt()
        {
            y.copy(rhs);
            INF=false;
        }
        else {x.zero(); INF=true;}
    }
    
    /* this+=this */
    func dbl() -> Int
    {
        if (INF) {return -1}
        if y.iszilch()
        {
            inf();
            return -1;
        }
    
        let w1=FP2(x)
        let w2=FP2(0)
        let w3=FP2(x)
        let w8=FP2(x)
    
        w1.sqr()
        w8.copy(w1)
        w8.imul(3)
    
        w2.copy(y); w2.sqr()
        w3.copy(x); w3.mul(w2)
        w3.imul(4)
        w1.copy(w3); w1.neg()
        w1.norm()
    
        x.copy(w8); x.sqr()
        x.add(w1)
        x.add(w1)
        x.norm()
    
        z.mul(y)
        z.add(z)
    
        w2.add(w2)
        w2.sqr()
        w2.add(w2)
        w3.sub(x)
        y.copy(w8); y.mul(w3)
        w2.norm()
        y.sub(w2)
        y.norm()
        z.norm()
    
        return 1
    }
/* this+=Q - return 0 for add, 1 for double, -1 for O */
    func add(_ Q:ECP2) -> Int
    {
        if INF
        {
            copy(Q)
            return -1
        }
        if Q.INF {return -1}
    
        var aff=false
    
        if Q.z.isunity() {aff=true}
    
        var A:FP2
        var C:FP2
        let B=FP2(z)
        let D=FP2(z)
        if (!aff)
        {
            A=FP2(Q.z)
            C=FP2(Q.z)
    
            A.sqr(); B.sqr()
            C.mul(A); D.mul(B)
    
            A.mul(x)
            C.mul(y)
        }
        else
        {
            A=FP2(x)
            C=FP2(y)
    
            B.sqr()
            D.mul(B)
        }
    
        B.mul(Q.x); B.sub(A)
        D.mul(Q.y); D.sub(C)
    
        if B.iszilch()
        {
            if D.iszilch()
            {
				dbl()
				return 1
            }
            else
            {
				INF=true
				return -1
            }
        }
    
        if !aff {z.mul(Q.z)}
        z.mul(B)
    
        let e=FP2(B); e.sqr()
        B.mul(e)
        A.mul(e)
    
        e.copy(A)
        e.add(A); e.add(B)
        x.copy(D); x.sqr(); x.sub(e)
    
        A.sub(x)
        y.copy(A); y.mul(D)
        C.mul(B); y.sub(C)
    
        x.norm()
        y.norm()
        z.norm()
    
        return 0
    }

    /* set self-=Q */
    func sub(_ Q:ECP2) -> Int
    {
        Q.neg()
        let D=add(Q)
        Q.neg()
        return D
    }
/* set self*=q, where q is Modulus, using Frobenius */
    func frob(_ X:FP2)
    {
        if INF {return}
        let X2=FP2(X)
        X2.sqr()
        x.conj()
        y.conj()
        z.conj()
        z.reduce()
        x.mul(X2)
        y.mul(X2)
        y.mul(X)
    }
    /* normalises m-array of ECP2 points. Requires work vector of m FP2s */
    
    private static func multiaffine(_ m:Int,_ P:[ECP2])
    {
        let t1=FP2(0)
        let t2=FP2(0)
    
        var work=[FP2]()
        for _ in 0 ..< m
            {work.append(FP2(0))}
     
        work[0].one()
        work[1].copy(P[0].z)
        
        for i in 2 ..< m
        {
            work[i].copy(work[i-1])
            work[i].mul(P[i-1].z)
        }
    
        t1.copy(work[m-1]); t1.mul(P[m-1].z)
    
        t1.inverse()
    
        t2.copy(P[m-1].z)
        work[m-1].mul(t1)
    
        var i=m-2
        while true
        {
            if (i==0)
            {
				work[0].copy(t1)
				work[0].mul(t2)
				break;
            }
            work[i].mul(t2)
            work[i].mul(t1)
            t2.mul(P[i].z)
            i-=1
        }
    /* now work[] contains inverses of all Z coordinates */
    
        for i in 0 ..< m
        {
            P[i].z.one()
            t1.copy(work[i]); t1.sqr()
            P[i].x.mul(t1)
            t1.mul(work[i])
            P[i].y.mul(t1)
        }
    }
    
    /* P*=e */
    func mul(_ e:BIG) -> ECP2
    {
    /* fixed size windows */
        let mt=BIG()
        let t=BIG()
        let P=ECP2()
        let Q=ECP2()
        let C=ECP2()
        
        var W=[ECP2]();
        for _ in 0 ..< 8 {W.append(ECP2())}
        
        var w=[Int8](repeating: 0,count: 1+(ROM.NLEN*Int(ROM.BASEBITS)+3)/4)
    
        if is_infinity() {return ECP2()}
    
        affine()
    
    /* precompute table */
        Q.copy(self)
        Q.dbl()
        W[0].copy(self)
    
        for i in 1 ..< 8
        {
            W[i].copy(W[i-1])
            W[i].add(Q)
        }
    
    /* convert the table to affine */
 
        ECP2.multiaffine(8,W);
    
    /* make exponent odd - add 2P if even, P if odd */
        t.copy(e)
        let s=t.parity()
        t.inc(1); t.norm(); let ns=t.parity(); mt.copy(t); mt.inc(1); mt.norm()
        t.cmove(mt,s)
        Q.cmove(self,ns)
        C.copy(Q)
    
        let nb=1+(t.nbits()+3)/4
    /* convert exponent to signed 4-bit window */
        for i in 0 ..< nb
        {
            w[i]=Int8(t.lastbits(5)-16)
            t.dec(Int(w[i])); t.norm()
            t.fshr(4)
        }
        w[nb]=Int8(t.lastbits(5))
    
        P.copy(W[Int(w[nb]-1)/2])
        for i in (0...nb-1).reversed()
        //for var i=nb-1;i>=0;i--
        {
            Q.select(W,Int32(w[i]))
            P.dbl()
            P.dbl()
            P.dbl()
            P.dbl()
            P.add(Q)
        }
        P.sub(C);
        P.affine()
        return P;
    }
    
    /* P=u0.Q0+u1*Q1+u2*Q2+u3*Q3 */
    static func mul4(_ Q:[ECP2],_ u:[BIG]) -> ECP2
    {
        var a=[Int32](repeating: 0,count: 4)
        let T=ECP2()
        let C=ECP2()
        let P=ECP2()
        
        var W=[ECP2]();
        for _ in 0 ..< 8 {W.append(ECP2())}
    
        let mt=BIG()
        var t=[BIG]()
    
        var w=[Int8](repeating: 0,count: ROM.NLEN*Int(ROM.BASEBITS)+1)
    
        for i in 0 ..< 4
        {
            t.append(BIG(u[i]))
            Q[i].affine()
        }
    
    /* precompute table */
    
        W[0].copy(Q[0]); W[0].sub(Q[1])
        W[1].copy(W[0])
        W[2].copy(W[0])
        W[3].copy(W[0])
        W[4].copy(Q[0]); W[4].add(Q[1])
        W[5].copy(W[4])
        W[6].copy(W[4])
        W[7].copy(W[4])
        T.copy(Q[2]); T.sub(Q[3])
        W[1].sub(T)
        W[2].add(T)
        W[5].sub(T)
        W[6].add(T)
        T.copy(Q[2]); T.add(Q[3])
        W[0].sub(T)
        W[3].add(T)
        W[4].sub(T)
        W[7].add(T)
    
        ECP2.multiaffine(8,W);
    
    /* if multiplier is even add 1 to multiplier, and add P to correction */
        mt.zero(); C.inf()
        for i in 0 ..< 4
        {
            if (t[i].parity()==0)
            {
				t[i].inc(1); t[i].norm()
                C.add(Q[i])
            }
            mt.add(t[i]); mt.norm()
        }
    
        let nb=1+mt.nbits();
    
    /* convert exponent to signed 1-bit window */
        for j in 0 ..< nb
        {
            for i in 0 ..< 4 {
				a[i]=Int32(t[i].lastbits(2)-2)
                
				t[i].dec(Int(a[i]))
                t[i].norm()
				t[i].fshr(1)
            }
            w[j]=Int8(8*a[0]+4*a[1]+2*a[2]+a[3])
        }
        w[nb]=Int8(8*t[0].lastbits(2)+4*t[1].lastbits(2))
        w[nb]+=Int8(2*t[2].lastbits(2)+t[3].lastbits(2))
    
        P.copy(W[Int(w[nb]-1)/2])
        for i in (0...nb-1).reversed()
        //for var i=nb-1;i>=0;i--
        {
            T.select(W,Int32(w[i]))
            P.dbl()
            P.add(T)
        }
        P.sub(C) /* apply correction */
    
        P.affine()
        return P
    }
    
    
    
}
