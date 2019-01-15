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
//  pair.swift
//
//  Created by Michael Scott on 07/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

/* AMCL BN Curve Pairing functions */

final class PAIR {
    
    // Line function
    static func line(_ A:ECP2,_ B:ECP2,_ Qx:FP,_ Qy:FP) -> FP12
    {
        let P=ECP2()
        var a:FP4
        var b:FP4
        var c:FP4
        P.copy(A);
        let ZZ=FP2(P.getz())
        ZZ.sqr();
        var D:Int
        if A===B {D=A.dbl()} // Check this return value in ecp2.c
        else {D=A.add(B)}
        if (D<0) {return FP12(1)}
        let Z3=FP2(A.getz())
        c=FP4(0)
        if D==0
        { /* Addition */
            let X=FP2(B.getx())
            let Y=FP2(B.gety())
            let T=FP2(P.getz())
            T.mul(Y)
            ZZ.mul(T)
    
            let NY=FP2(P.gety()); NY.neg()
            ZZ.add(NY)
            Z3.pmul(Qy)
            T.mul(P.getx())
            X.mul(NY)
            T.add(X)
            a=FP4(Z3,T)
            ZZ.neg()
            ZZ.pmul(Qx)
            b=FP4(ZZ)
        }
        else
        { // Doubling
            let X=FP2(P.getx())
            let Y=FP2(P.gety())
            let T=FP2(P.getx())
            T.sqr()
            T.imul(3)
    
            Y.sqr()
            Y.add(Y)
            Z3.mul(ZZ)
            Z3.pmul(Qy)
    
            X.mul(T)
            X.sub(Y)
            a=FP4(Z3,X)
            T.neg()
            ZZ.mul(T)
            ZZ.pmul(Qx)
            b=FP4(ZZ)
        }
        return FP12(a,b,c)
    }
    // Optimal R-ate pairing
    static func ate(_ P:ECP2,_ Q:ECP) -> FP12
    {
        let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
        let x=BIG(ROM.CURVE_Bnx)
        let n=BIG(x)
        let K=ECP2()
        
        var lv:FP12

	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		n.pmul(6); n.dec(2)
	} else {n.copy(x)}
	
	n.norm()
        P.affine()
        Q.affine()
        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
    
        let A=ECP2()
        let r=FP12(1)
    
        A.copy(P)
        let nb=n.nbits()
    
        for i in (1...nb-2).reversed()
        //for var i=nb-2;i>=1;i--
        {
            lv=line(A,A,Qx,Qy)
            r.smul(lv)
    
            if (n.bit(UInt(i))==1)
            {
		lv=line(A,P,Qx,Qy)
		r.smul(lv)
            }
            r.sqr()
        }
    
        lv=line(A,A,Qx,Qy)
        r.smul(lv)
	if n.parity()==1 {
		lv=line(A,P,Qx,Qy)
		r.smul(lv)
	}
    
    // R-ate fixup required for BN curves

	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		r.conj()
		K.copy(P)
		K.frob(f)
		A.neg()
		lv=line(A,K,Qx,Qy)
		r.smul(lv)
		K.frob(f)
		K.neg()
		lv=line(A,K,Qx,Qy)
		r.smul(lv)
	}
        return r
    }
    // Optimal R-ate double pairing e(P,Q).e(R,S)
    static func ate2(_ P:ECP2,_ Q:ECP,_ R:ECP2,_ S:ECP) -> FP12
    {
        let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
        let x=BIG(ROM.CURVE_Bnx)
        let n=BIG(x)
        let K=ECP2()
        var lv:FP12

 	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		n.pmul(6); n.dec(2)
	} else {n.copy(x)}
	
	n.norm()
        P.affine()
        Q.affine()
        R.affine()
        S.affine()
    
        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
        let Sx=FP(S.getx())
        let Sy=FP(S.gety())
    
        let A=ECP2()
        let B=ECP2()
        let r=FP12(1)
    
        A.copy(P)
        B.copy(R)
        let nb=n.nbits()
    
        for i in (1...nb-2).reversed()
        //for var i=nb-2;i>=1;i--
        {
            lv=line(A,A,Qx,Qy)
            r.smul(lv)
            lv=line(B,B,Sx,Sy)
            r.smul(lv)
            if n.bit(UInt(i))==1
            {
		lv=line(A,P,Qx,Qy)
		r.smul(lv)
		lv=line(B,R,Sx,Sy)
		r.smul(lv)
            }
            r.sqr()
        }
    
        lv=line(A,A,Qx,Qy)
        r.smul(lv)
        lv=line(B,B,Sx,Sy)
        r.smul(lv)
	if n.parity()==1 {
		lv=line(A,P,Qx,Qy)
		r.smul(lv)
		lv=line(B,R,Sx,Sy)
		r.smul(lv)
	}
    
    // R-ate fixup required for BN curves

	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		r.conj()
    
		K.copy(P)
		K.frob(f)
		A.neg()
		lv=line(A,K,Qx,Qy)
		r.smul(lv)
		K.frob(f)
		K.neg()
		lv=line(A,K,Qx,Qy)
		r.smul(lv)
    
		K.copy(R)
		K.frob(f)
		B.neg()
		lv=line(B,K,Sx,Sy)
		r.smul(lv)
		K.frob(f)
		K.neg()
		lv=line(B,K,Sx,Sy)
		r.smul(lv)
	}
        return r
    }
    
    // final exponentiation - keep separate for multi-pairings and to avoid thrashing stack
    static func fexp(_ m:FP12) -> FP12
    {
        let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb));
        let x=BIG(ROM.CURVE_Bnx)
        let r=FP12(m)
    
    // Easy part of final exp
        var lv=FP12(r)
        lv.inverse()
        r.conj()
    
        r.mul(lv)
        lv.copy(r)
        r.frob(f)
        r.frob(f)
        r.mul(lv)
        
    // Hard part of final exp
	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		lv.copy(r)
		lv.frob(f)
		let x0=FP12(lv)
		x0.frob(f)
		lv.mul(r)
		x0.mul(lv)
		x0.frob(f)
		let x1=FP12(r)
		x1.conj()
		let x4=r.pow(x)

		let x3=FP12(x4)
		x3.frob(f)
    
		let x2=x4.pow(x)
    
		let x5=FP12(x2); x5.conj()
		lv=x2.pow(x)
    
		x2.frob(f)
		r.copy(x2); r.conj()
    
		x4.mul(r)
		x2.frob(f)
    
		r.copy(lv)
		r.frob(f)
		lv.mul(r)
    
		lv.usqr()
		lv.mul(x4)
		lv.mul(x5)
		r.copy(x3)
		r.mul(x5)
		r.mul(lv)
		lv.mul(x2)
		r.usqr()
		r.mul(lv)
		r.usqr()
		lv.copy(r)
		lv.mul(x1)
		r.mul(x0)
		lv.usqr()
		r.mul(lv)
		r.reduce()
	} else {
		let x0=FP12(r)
		let x1=FP12(r)
		lv.copy(r); lv.frob(f)
		let x3=FP12(lv); x3.conj(); x1.mul(x3)
		lv.frob(f); lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))  //r=r.pow(x);
		x3.copy(r); x3.conj(); x1.mul(x3)
		lv.copy(r); lv.frob(f)
		x0.mul(lv)
		lv.frob(f)
		x1.mul(lv)
		lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)

		r.copy(r.pow(x))
		x0.mul(r)
		lv.copy(r); lv.frob(f); lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)
		lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
		lv.copy(r); lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)
		lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
		x3.copy(r); x3.conj(); x0.mul(x3)
		lv.copy(r); lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
		x1.mul(r)

		x0.usqr()
		x0.mul(x1)
		r.copy(x0)
		r.reduce()
	}
        return r
    }
    
    // GLV method
    static func glv(_ e:BIG) -> [BIG]
    {
	var u=[BIG]();
	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		let t=BIG(0)
		let q=BIG(ROM.CURVE_Order)
		var v=[BIG]();
		for _ in 0 ..< 2
		{
			u.append(BIG(0))
			v.append(BIG(0))
		}
        
		for i in 0 ..< 2
		{
			t.copy(BIG(ROM.CURVE_W[i]))
			let d=BIG.mul(t,e)
			v[i].copy(d.div(q))
		}
		u[0].copy(e);
		for i in 0 ..< 2
		{
			for j in 0 ..< 2
			{
				t.copy(BIG(ROM.CURVE_SB[j][i]))
				t.copy(BIG.modmul(v[j],t,q))
				u[i].add(q)
				u[i].sub(t)
				u[i].mod(q)
			}
		}
	} else { // -(x^2).P = (Beta.x,y)
		let q=BIG(ROM.CURVE_Order)
		let x=BIG(ROM.CURVE_Bnx)
		let x2=BIG.smul(x,x)
		u.append(BIG(e))
		u[0].mod(x2)
		u.append(BIG(e))
		u[1].div(x2)
		u[1].rsub(q)

	}
        return u
    }
    // Galbraith & Scott Method
    static func gs(_ e:BIG) -> [BIG]
    {
        var u=[BIG]();
	if ROM.CURVE_PAIRING_TYPE == ROM.BN_CURVE {
		let t=BIG(0)
		let q=BIG(ROM.CURVE_Order)
		var v=[BIG]();
		for _ in 0 ..< 4
		{
			u.append(BIG(0))
			v.append(BIG(0))
		}
        
		for i in 0 ..< 4
		{
			t.copy(BIG(ROM.CURVE_WB[i]))
			let d=BIG.mul(t,e)
			v[i].copy(d.div(q))
		}
		u[0].copy(e);
		for i in 0 ..< 4
		{
			for j in 0 ..< 4
			{
				t.copy(BIG(ROM.CURVE_BB[j][i]))
				t.copy(BIG.modmul(v[j],t,q))
				u[i].add(q)
				u[i].sub(t)
				u[i].mod(q)
			}
		}
	} else {
		let x=BIG(ROM.CURVE_Bnx)
		var w=BIG(e)
		for i in 0 ..< 4
		{
			u.append(BIG(w))
			u[i].mod(x)
			w.div(x)
		}
	}
        return u
    }	
    
    // Multiply P by e in group G1
    static func G1mul(_ P:ECP,_ e:BIG) -> ECP
    {
        var R:ECP
        if (ROM.USE_GLV)
        {
            P.affine()
            R=ECP()
            R.copy(P)
            let Q=ECP()
            Q.copy(P)
            let q=BIG(ROM.CURVE_Order)
            let cru=FP(BIG(ROM.CURVE_Cru))
            let t=BIG(0)
            var u=PAIR.glv(e)
            Q.getx().mul(cru);
    
            var np=u[0].nbits()
            t.copy(BIG.modneg(u[0],q))
            var nn=t.nbits()
            if (nn<np)
            {
				u[0].copy(t)
				R.neg()
            }
    
            np=u[1].nbits()
            t.copy(BIG.modneg(u[1],q))
            nn=t.nbits()
            if (nn<np)
            {
				u[1].copy(t)
				Q.neg()
            }
    
            R=R.mul2(u[0],Q,u[1])
        }
        else
        {
            R=P.mul(e)
        }
        return R
    }
    
    // Multiply P by e in group G2
    static func G2mul(_ P:ECP2,_ e:BIG) -> ECP2
    {
        var R:ECP2
        if (ROM.USE_GS_G2)
        {
            var Q=[ECP2]()
            let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb));
            let q=BIG(ROM.CURVE_Order);
            var u=PAIR.gs(e);
    
            let t=BIG(0);
            P.affine()
            Q.append(ECP2())
            Q[0].copy(P);
            for i in 1 ..< 4
            {
                Q.append(ECP2()); Q[i].copy(Q[i-1]);
				Q[i].frob(f);
            }
            for i in 0 ..< 4
            {
				let np=u[i].nbits();
				t.copy(BIG.modneg(u[i],q));
				let nn=t.nbits();
				if (nn<np)
				{
                    u[i].copy(t);
                    Q[i].neg();
				}
            }
    
            R=ECP2.mul4(Q,u);
        }
        else
        {
            R=P.mul(e);
        }
        return R;
    }
    // f=f^e
    // Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java
    static func GTpow(_ d:FP12,_ e:BIG) -> FP12
    {
        var r:FP12
        if (ROM.USE_GS_GT)
        {
            var g=[FP12]()
            let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
            let q=BIG(ROM.CURVE_Order)
            let t=BIG(0)
        
            var u=gs(e)
            g.append(FP12(0))
            g[0].copy(d);
            for i in 1 ..< 4
            {
                g.append(FP12(0)); g[i].copy(g[i-1])
				g[i].frob(f)
            }
            for i in 0 ..< 4
            {
				let np=u[i].nbits()
				t.copy(BIG.modneg(u[i],q))
				let nn=t.nbits()
				if (nn<np)
				{
                    u[i].copy(t)
                    g[i].conj()
				}
            }
            r=FP12.pow4(g,u)
        }
        else
        {
            r=d.pow(e)
        }
        return r
    }
    // test group membership - no longer needed
    // with GT-Strong curve, now only check that m!=1, conj(m)*m==1, and m.m^{p^4}=m^{p^2}
/*
    static func GTmember(m:FP12) -> Bool
    {
        if m.isunity() {return false}
        let r=FP12(m)
        r.conj()
        r.mul(m)
        if !r.isunity() {return false}
    
        let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
    
        r.copy(m); r.frob(f); r.frob(f)
        var w=FP12(r); w.frob(f); w.frob(f)
        w.mul(m)
        if !ROM.GT_STRONG
        {
            if !w.equals(r) {return false}
            let x=BIG(ROM.CURVE_Bnx)
            r.copy(m); w=r.pow(x); w=w.pow(x)
            r.copy(w); r.sqr(); r.mul(w); r.sqr()
            w.copy(m); w.frob(f)
        }
        return w.equals(r)
    }
*/   
}

