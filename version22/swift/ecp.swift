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
//  ecp.swift
//
//  Created by Michael Scott on 30/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

final class ECP {
    private var x:FP
    private var y:FP
    private var z:FP
    private var INF:Bool
    
   /* Constructor - set to O */
    init()
    {
        x=FP(0)
        y=FP(0)
        z=FP(1)
        INF=true
    }
    
    /* test for O point-at-infinity */
    func is_infinity() -> Bool
    {
        if (ROM.CURVETYPE==ROM.EDWARDS)
        {
            x.reduce(); y.reduce(); z.reduce()
            return x.iszilch() && y.equals(z)
        }
        else {return INF}
    }
 
    /* Conditional swap of P and Q dependant on d */
    private func cswap(_ Q: ECP,_ d:Int)
    {
        x.cswap(Q.x,d);
        if ROM.CURVETYPE != ROM.MONTGOMERY {y.cswap(Q.y,d)}
        z.cswap(Q.z,d);
        if (ROM.CURVETYPE != ROM.EDWARDS)
        {
            var bd:Bool
            if d==0 {bd=false}
            else {bd=true}
            bd=bd && (INF != Q.INF)
            INF = (INF != bd)
            Q.INF = (Q.INF != bd)
        }
    }
    
    /* Conditional move of Q to P dependant on d */
    private func cmove(_ Q: ECP,_ d:Int)
    {
        x.cmove(Q.x,d);
        if ROM.CURVETYPE != ROM.MONTGOMERY {y.cmove(Q.y,d)}
        z.cmove(Q.z,d);
        if (ROM.CURVETYPE != ROM.EDWARDS)
        {
            var bd:Bool
            if d==0 {bd=false}
            else {bd=true}
            INF = (INF != Q.INF) && bd;
        }
    }
    
    /* return 1 if b==c, no branching */
    private static func teq(_ b: Int32,_ c:Int32) -> Int
    {
        var x=b^c
        x-=1  // if x=0, x now -1
        return Int((x>>31)&1)
    }
 
    /* self=P */
    func copy(_ P: ECP)
    {
        x.copy(P.x)
        if ROM.CURVETYPE != ROM.MONTGOMERY {y.copy(P.y)}
        z.copy(P.z)
        INF=P.INF
    }
    /* self=-self */
    func neg() {
        if is_infinity() {return}
        if (ROM.CURVETYPE==ROM.WEIERSTRASS)
        {
            y.neg(); y.norm();
        }
        if (ROM.CURVETYPE==ROM.EDWARDS)
        {
            x.neg(); x.norm();
        }
        return;
    }
    
    /* Constant time select from pre-computed table */
    private func select(_ W:[ECP],_ b:Int32)
    {
        let MP=ECP()
        let m=b>>31
        var babs=(b^m)-m
    
        babs=(babs-1)/2
    
        cmove(W[0],ECP.teq(babs,0)); // conditional move
        cmove(W[1],ECP.teq(babs,1))
        cmove(W[2],ECP.teq(babs,2))
        cmove(W[3],ECP.teq(babs,3))
        cmove(W[4],ECP.teq(babs,4))
        cmove(W[5],ECP.teq(babs,5))
        cmove(W[6],ECP.teq(babs,6))
        cmove(W[7],ECP.teq(babs,7))
    
        MP.copy(self)
        MP.neg()
        cmove(MP,Int(m&1))
    }
    
    /* Test P == Q */
    func equals(_ Q: ECP) -> Bool
    {
        if (is_infinity() && Q.is_infinity()) {return true}
        if (is_infinity() || Q.is_infinity()) {return false}
        if (ROM.CURVETYPE==ROM.WEIERSTRASS)
        {
            let zs2=FP(z); zs2.sqr()
            let zo2=FP(Q.z); zo2.sqr()
            let zs3=FP(zs2); zs3.mul(z)
            let zo3=FP(zo2); zo3.mul(Q.z)
            zs2.mul(Q.x)
            zo2.mul(x)
            if !zs2.equals(zo2) {return false}
            zs3.mul(Q.y)
            zo3.mul(y)
            if !zs3.equals(zo3) {return false}
        }
        else
        {
            let a=FP(0)
            let b=FP(0)
            a.copy(x); a.mul(Q.z); a.reduce()
            b.copy(Q.x); b.mul(z); b.reduce()
            if !a.equals(b) {return false}
            if ROM.CURVETYPE==ROM.EDWARDS
            {
				a.copy(y); a.mul(Q.z); a.reduce()
				b.copy(Q.y); b.mul(z); b.reduce()
				if !a.equals(b) {return false}
            }
        }
        return true
    }
  
/* set self=O */
    func inf()
    {
        INF=true;
        x.zero()
        y.one()
        z.one()
    }
    
    /* Calculate RHS of curve equation */
    static func RHS(_ x: FP) -> FP
    {
        x.norm();
        let r=FP(x);
        r.sqr();
    
        if ROM.CURVETYPE==ROM.WEIERSTRASS
        { // x^3+Ax+B
            let b=FP(BIG(ROM.CURVE_B))
            r.mul(x)
            if (ROM.CURVE_A == -3)
            {
				let cx=FP(x)
				cx.imul(3)
				cx.neg(); cx.norm()
				r.add(cx)
            }
            r.add(b);
        }
        if (ROM.CURVETYPE==ROM.EDWARDS)
        { // (Ax^2-1)/(Bx^2-1)
            let b=FP(BIG(ROM.CURVE_B))
    
            let one=FP(1);
            b.mul(r);
            b.sub(one);
            if ROM.CURVE_A == -1 {r.neg()}
            r.sub(one)
            b.inverse()
            r.mul(b);
        }
        if ROM.CURVETYPE==ROM.MONTGOMERY
        { // x^3+Ax^2+x
            let x3=FP(0)
            x3.copy(r);
            x3.mul(x);
            r.imul(ROM.CURVE_A);
            r.add(x3);
            r.add(x);
        }
        r.reduce();
        return r;
    }
    
    /* set (x,y) from two BIGs */
    init(_ ix: BIG,_ iy: BIG)
    {
        x=FP(ix)
        y=FP(iy)
        z=FP(1)
        INF=true
        let rhs=ECP.RHS(x);
    
        if ROM.CURVETYPE==ROM.MONTGOMERY
        {
            if rhs.jacobi()==1 {INF=false}
            else {inf()}
        }
        else
        {
            let y2=FP(y)
            y2.sqr()
            if y2.equals(rhs) {INF=false}
            else {inf()}
        }
    }
    
    /* set (x,y) from BIG and a bit */
    init(_ ix: BIG,_ s:Int)
    {
        x=FP(ix)
        let rhs=ECP.RHS(x)
        y=FP(0)
        z=FP(1)
        INF=true
        if rhs.jacobi()==1
        {
            let ny=rhs.sqrt()
            if (ny.redc().parity() != s) {ny.neg()}
            y.copy(ny)
            INF=false;
        }
        else {inf()}
    }
    
    /* set from x - calculate y from curve equation */
    init(_ ix:BIG)
    {
        x=FP(ix)
        let rhs=ECP.RHS(x)
        y=FP(0)
        z=FP(1)
        if rhs.jacobi()==1
        {
            if ROM.CURVETYPE != ROM.MONTGOMERY {y.copy(rhs.sqrt())}
            INF=false;
        }
        else {INF=true}
    }
    
    /* set to affine - from (x,y,z) to (x,y) */
    func affine()
    {
        if is_infinity() {return}
        let one=FP(1)
        if (z.equals(one)) {return}
        z.inverse()
        if ROM.CURVETYPE==ROM.WEIERSTRASS
        {
            let z2=FP(z)
            z2.sqr()
            x.mul(z2); x.reduce()
            y.mul(z2)
            y.mul(z);  y.reduce()
        }
        if ROM.CURVETYPE==ROM.EDWARDS
        {
            x.mul(z); x.reduce()
            y.mul(z); y.reduce()
        }
        if ROM.CURVETYPE==ROM.MONTGOMERY
        {
            x.mul(z); x.reduce()
 
        }
        z.copy(one)
    }
    /* extract x as a BIG */
    func getX() -> BIG
    {
        affine()
        return x.redc()
    }
    /* extract y as a BIG */
    func getY() -> BIG
    {
        affine();
        return y.redc();
    }
    
    /* get sign of Y */
    func getS() -> Int
    {
        affine()
        let y=getY()
        return y.parity()
    }
    /* extract x as an FP */
    func getx() -> FP
    {
        return x;
    }
    /* extract y as an FP */
    func gety() -> FP
    {
        return y;
    }
    /* extract z as an FP */
    func getz() -> FP
    {
        return z;
    }
    /* convert to byte array */
    func toBytes(_ b:inout [UInt8])
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)
        if ROM.CURVETYPE != ROM.MONTGOMERY {b[0]=0x04}
        else {b[0]=0x02}
    
        affine()
        x.redc().toBytes(&t)
        for i in 0 ..< RM {b[i+1]=t[i]}
        if ROM.CURVETYPE != ROM.MONTGOMERY
        {
            y.redc().toBytes(&t);
            for i in 0 ..< RM {b[i+RM+1]=t[i]}
        }
    }
    /* convert from byte array to point */
    static func fromBytes(_ b: [UInt8]) -> ECP
    {
        let RM=Int(ROM.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)
        let p=BIG(ROM.Modulus);
    
        for i in 0 ..< RM {t[i]=b[i+1]}
        let px=BIG.fromBytes(t)
        if BIG.comp(px,p)>=0 {return ECP()}
    
        if (b[0]==0x04)
        {
            for i in 0 ..< RM {t[i]=b[i+RM+1]}
            let py=BIG.fromBytes(t)
            if BIG.comp(py,p)>=0 {return ECP()}
            return ECP(px,py)
        }
        else {return ECP(px)}
    }
    /* convert to hex string */
    func toString() -> String
    {
        if is_infinity() {return "infinity"}
        affine();
        if ROM.CURVETYPE==ROM.MONTGOMERY {return "("+x.redc().toString()+")"}
        else {return "("+x.redc().toString()+","+y.redc().toString()+")"}
    }
    
    /* self*=2 */
    func dbl()
    {
        if (ROM.CURVETYPE==ROM.WEIERSTRASS)
        {
            if INF {return}
            if y.iszilch()
            {
				inf()
				return
            }
    
            let w1=FP(x)
            let w6=FP(z)
            let w2=FP(0)
            let w3=FP(x)
            let w8=FP(x)
    
            if (ROM.CURVE_A == -3)
            {
				w6.sqr()
				w1.copy(w6)
				w1.neg()
				w3.add(w1)
				w8.add(w6)
				w3.mul(w8)
				w8.copy(w3)
				w8.imul(3)
            }
            else
            {
				w1.sqr()
				w8.copy(w1)
				w8.imul(3)
            }
    
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
            //w2.norm();
            y.sub(w2)
            y.norm()
            z.norm()
        }
        if ROM.CURVETYPE==ROM.EDWARDS
        {
            let C=FP(x)
            let D=FP(y)
            let H=FP(z)
            let J=FP(0)
    
            x.mul(y); x.add(x)
            C.sqr()
            D.sqr()
            if ROM.CURVE_A == -1 {C.neg()}
            y.copy(C); y.add(D)
            y.norm()
            H.sqr(); H.add(H)
            z.copy(y)
            J.copy(y); J.sub(H)
            x.mul(J)
            C.sub(D)
            y.mul(C)
            z.mul(J)
    
            x.norm();
            y.norm();
            z.norm();
        }
        if ROM.CURVETYPE==ROM.MONTGOMERY
        {
            let A=FP(x)
            let B=FP(x);
            let AA=FP(0);
            let BB=FP(0);
            let C=FP(0);
    
            if INF {return}
    
            A.add(z)
            AA.copy(A); AA.sqr()
            B.sub(z)
            BB.copy(B); BB.sqr()
            C.copy(AA); C.sub(BB)
    //C.norm();
    
            x.copy(AA); x.mul(BB)
    
            A.copy(C); A.imul((ROM.CURVE_A+2)/4)
    
            BB.add(A)
            z.copy(BB); z.mul(C)
            x.norm()
            z.norm()
        }
        return
    }
    
    /* self+=Q */
    func add(_ Q:ECP)
    {
        if ROM.CURVETYPE==ROM.WEIERSTRASS
        {
            if (INF)
            {
				copy(Q)
				return
            }
            if Q.INF {return}
    
            var aff=false;
    
            let one=FP(1);
            if Q.z.equals(one) {aff=true}
    
            var A:FP
            var C:FP
            let B=FP(z)
            let D=FP(z)
            if (!aff)
            {
				A=FP(Q.z)
				C=FP(Q.z)
    
				A.sqr(); B.sqr()
				C.mul(A); D.mul(B)
    
				A.mul(x)
				C.mul(y)
            }
            else
            {
				A=FP(x)
				C=FP(y)
    
				B.sqr()
				D.mul(B)
            }
    
            B.mul(Q.x); B.sub(A)
            D.mul(Q.y); D.sub(C)
    
            if B.iszilch()
            {
				if (D.iszilch())
				{
                    dbl()
                    return
				}
				else
				{
                    INF=true
                    return
				}
            }
    
            if !aff {z.mul(Q.z)}
            z.mul(B);
    
            let e=FP(B); e.sqr()
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
        }
        if ROM.CURVETYPE==ROM.EDWARDS
        {
            let b=FP(BIG(ROM.CURVE_B))
            let A=FP(z)
            let B=FP(0)
            let C=FP(x)
            let D=FP(y)
            let E=FP(0)
            let F=FP(0)
            let G=FP(0)
    
            A.mul(Q.z)
            B.copy(A); B.sqr()
            C.mul(Q.x)
            D.mul(Q.y)
    
            E.copy(C); E.mul(D); E.mul(b)
            F.copy(B); F.sub(E)
            G.copy(B); G.add(E)
    
            if ROM.CURVE_A==1
            {
				E.copy(D); E.sub(C)
            }
            C.add(D)
    
            B.copy(x); B.add(y)
            D.copy(Q.x); D.add(Q.y)
            B.mul(D)
            B.sub(C)
            B.mul(F)
            x.copy(A); x.mul(B)

            if ROM.CURVE_A==1
            {
				C.copy(E); C.mul(G)
            }
            if ROM.CURVE_A == -1
            {
				C.mul(G)
            }
            y.copy(A); y.mul(C)
            z.copy(F); z.mul(G)
            x.norm(); y.norm(); z.norm()
        }
        return;
    }
    
    /* Differential Add for Montgomery curves. self+=Q where W is self-Q and is affine. */
    func dadd(_ Q:ECP,_ W:ECP)
    {
        let A=FP(x)
        let B=FP(x)
        let C=FP(Q.x)
        let D=FP(Q.x)
        let DA=FP(0)
        let CB=FP(0)
    
        A.add(z)
        B.sub(z)
    
        C.add(Q.z)
        D.sub(Q.z)
    
        DA.copy(D); DA.mul(A)
        CB.copy(C); CB.mul(B)
        
        A.copy(DA); A.add(CB); A.sqr()
        B.copy(DA); B.sub(CB); B.sqr()
    
        x.copy(A)
        z.copy(W.x); z.mul(B)
    
        if z.iszilch() {inf()}
        else {INF=false}
    
        x.norm()
    }
    /* this-=Q */
    func sub(_ Q:ECP)
    {
        Q.neg()
        add(Q)
        Q.neg()
    }
    static func multiaffine(_ m: Int,_ P:[ECP])
    {
        let t1=FP(0)
        let t2=FP(0)
    
        var work=[FP]()
        
        for _ in 0 ..< m
            {work.append(FP(0))}
    
        work[0].one()
        work[1].copy(P[0].z)
    
        for i in 2 ..< m
        {
            work[i].copy(work[i-1])
            work[i].mul(P[i-1].z)
        }
    
        t1.copy(work[m-1]);
        t1.mul(P[m-1].z);
        t1.inverse();
        t2.copy(P[m-1].z);
        work[m-1].mul(t1);
        var i=m-2;
        while (true)
        {
            if i==0
            {
				work[0].copy(t1)
				work[0].mul(t2)
				break
            }
            work[i].mul(t2);
            work[i].mul(t1);
            t2.mul(P[i].z);
            i=i-1;
        }
    /* now work[] contains inverses of all Z coordinates */
    
        for i in 0 ..< m
        {
            P[i].z.one();
            t1.copy(work[i]);
            t1.sqr();
            P[i].x.mul(t1);
            t1.mul(work[i]);
            P[i].y.mul(t1);
        }
    }
    /* constant time multiply by small integer of length bts - use ladder */
    func pinmul(_ e:Int32,_ bts:Int32) -> ECP
    {
        if ROM.CURVETYPE==ROM.MONTGOMERY
            {return self.mul(BIG(Int(e)))}
        else
        {
            let P=ECP()
            let R0=ECP()
            let R1=ECP(); R1.copy(self)
    
            for i in (0...bts-1).reversed()
            {
				let b=Int(e>>i)&1;
				P.copy(R1);
				P.add(R0);
				R0.cswap(R1,b);
				R1.copy(P);
				R0.dbl();
				R0.cswap(R1,b);
            }
            P.copy(R0);
            P.affine();
            return P;
        }
    }
    
    /* return e.self */
    
    func mul(_ e:BIG) -> ECP
    {
        if (e.iszilch() || is_infinity()) {return ECP()}
    
        let P=ECP()
        if ROM.CURVETYPE==ROM.MONTGOMERY
        {
            /* use Ladder */
            let D=ECP()
            let R0=ECP(); R0.copy(self)
            let R1=ECP(); R1.copy(self)
            R1.dbl();
            D.copy(self); D.affine();
            let nb=e.nbits();
            
            for i in (0...nb-2).reversed()
            {
				let b=e.bit(UInt(i))
                //print("\(b)")
				P.copy(R1)
				P.dadd(R0,D)
				R0.cswap(R1,b)
				R1.copy(P)
				R0.dbl()
				R0.cswap(R1,b)
            }
            P.copy(R0)
        }
        else
        {
    // fixed size windows
            let mt=BIG()
            let t=BIG()
            let Q=ECP()
            let C=ECP()
            var W=[ECP]()
            let n=1+(ROM.NLEN*Int(ROM.BASEBITS)+3)/4
            var w=[Int8](repeating: 0,count: n)
    
            affine();
    
    // precompute table
            Q.copy(self)
            Q.dbl()
            W.append(ECP())
            
            W[0].copy(self)
    
            for i in 1 ..< 8
            {
                W.append(ECP())
				W[i].copy(W[i-1])
				W[i].add(Q)
            }
    
    // convert the table to affine
            if ROM.CURVETYPE==ROM.WEIERSTRASS
                {ECP.multiaffine(8,W)}
    
    // make exponent odd - add 2P if even, P if odd
            t.copy(e);
            let s=t.parity();
            t.inc(1); t.norm(); let ns=t.parity();
            mt.copy(t); mt.inc(1); mt.norm();
            t.cmove(mt,s);
            Q.cmove(self,ns);
            C.copy(Q);
    
            let nb=1+(t.nbits()+3)/4;
    
    // convert exponent to signed 4-bit window
            for i in 0 ..< nb
            {
				w[i]=Int8(t.lastbits(5)-16);
				t.dec(Int(w[i]));
                t.norm();
				t.fshr(4);
            }
            w[nb]=Int8(t.lastbits(5))
    
            P.copy(W[Int((w[nb])-1)/2]);
            for i in (0...nb-1).reversed()
            {
				Q.select(W,Int32(w[i]));
				P.dbl();
				P.dbl();
				P.dbl();
				P.dbl();
				P.add(Q);
            }
            P.sub(C); /* apply correction */
        }
        P.affine();
        return P;
    }
    
    /* Return e.this+f.Q */
    
    func mul2(_ e:BIG,_ Q:ECP,_ f:BIG) -> ECP
    {
        let te=BIG()
        let tf=BIG()
        let mt=BIG()
        let S=ECP()
        let T=ECP()
        let C=ECP()
        var W=[ECP]()
        let n=1+(ROM.NLEN*Int(ROM.BASEBITS)+1)/2
        var w=[Int8](repeating: 0,count: n);
        
        affine();
        Q.affine();
    
        te.copy(e);
        tf.copy(f);
    
    // precompute table
        for _ in 0 ..< 8 {W.append(ECP())}
        W[1].copy(self); W[1].sub(Q)
        W[2].copy(self); W[2].add(Q)
        S.copy(Q); S.dbl();
        W[0].copy(W[1]); W[0].sub(S)
        W[3].copy(W[2]); W[3].add(S)
        T.copy(self); T.dbl()
        W[5].copy(W[1]); W[5].add(T)
        W[6].copy(W[2]); W[6].add(T)
        W[4].copy(W[5]); W[4].sub(S)
        W[7].copy(W[6]); W[7].add(S)
    
    // convert the table to affine
        if ROM.CURVETYPE==ROM.WEIERSTRASS
            {ECP.multiaffine(8,W)}
    
    // if multiplier is odd, add 2, else add 1 to multiplier, and add 2P or P to correction
    
        var s=te.parity()
        te.inc(1); te.norm(); var ns=te.parity(); mt.copy(te); mt.inc(1); mt.norm()
        te.cmove(mt,s)
        T.cmove(self,ns)
        C.copy(T)
    
        s=tf.parity()
        tf.inc(1); tf.norm(); ns=tf.parity(); mt.copy(tf); mt.inc(1); mt.norm()
        tf.cmove(mt,s)
        S.cmove(Q,ns)
        C.add(S)
    
        mt.copy(te); mt.add(tf); mt.norm()
        let nb=1+(mt.nbits()+1)/2
    
    // convert exponent to signed 2-bit window
        for i in 0 ..< nb
        {
            let a=(te.lastbits(3)-4);
            te.dec(a); te.norm();
            te.fshr(2);
            let b=(tf.lastbits(3)-4);
            tf.dec(b); tf.norm();
            tf.fshr(2);
            w[i]=Int8(4*a+b);
        }
        w[nb]=Int8(4*te.lastbits(3)+tf.lastbits(3));
        S.copy(W[Int(w[nb]-1)/2]);
        for i in (0...nb-1).reversed()
        {
            T.select(W,Int32(w[i]));
            S.dbl();
            S.dbl();
            S.add(T);
        }
        S.sub(C); /* apply correction */
        S.affine();
        return S;
    }
    
    
   
    
}
