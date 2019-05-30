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
//  pair256.swift
//
//  Created by Michael Scott on 07/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

/* AMCL BLS Curve Pairing functions */

public struct PAIR256 {
    
    // Line function
    static func linedbl(_ A: inout ECP8,_ Qx:FP,_ Qy:FP) -> FP48
    {
        var a:FP16
        var b:FP16
        var c:FP16

        var XX=FP8(A.getx())  //X
        var YY=FP8(A.gety())  //Y
        var ZZ=FP8(A.getz())  //Z
        var YZ=FP8(YY)        //Y 
        YZ.mul(ZZ)                //YZ
        XX.sqr()                  //X^2
        YY.sqr()                  //Y^2
        ZZ.sqr()                  //Z^2
            
        YZ.imul(4)
        YZ.neg(); YZ.norm()       //-2YZ
        YZ.tmul(Qy)               //-2YZ.Ys

        XX.imul(6)               //3X^2
        XX.tmul(Qx)              //3X^2.Xs

        let sb=3*ROM.CURVE_B_I
        ZZ.imul(sb)  
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {             
            ZZ.div_2i();  
        }
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {
            ZZ.times_i()
            ZZ.add(ZZ)
            YZ.times_i()
        }              
        ZZ.norm() // 3b.Z^2 

        YY.add(YY)
        ZZ.sub(YY); ZZ.norm()     // 3b.Z^2-Y^2

        a=FP16(YZ,ZZ)          // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs 
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {             
            b=FP16(XX)            // L(0,1) | L(0,0) | L(1,0)
            c=FP16()
        } else { 
            b=FP16()
            c=FP16(XX); c.times_i()
        }        
        A.dbl()

        var res=FP48(a,b,c)
        res.settype(FP48.SPARSER)
        return res
    }

    static func lineadd(_ A: inout ECP8,_ B:ECP8,_ Qx:FP,_ Qy:FP) -> FP48
    {
        var a:FP16
        var b:FP16
        var c:FP16

        var X1=FP8(A.getx())    // X1
        var Y1=FP8(A.gety())    // Y1
        var T1=FP8(A.getz())    // Z1
        var T2=FP8(A.getz())    // Z1
            
        T1.mul(B.gety())    // T1=Z1.Y2 
        T2.mul(B.getx())    // T2=Z1.X2

        X1.sub(T2); X1.norm()  // X1=X1-Z1.X2
        Y1.sub(T1); Y1.norm()  // Y1=Y1-Z1.Y2

        T1.copy(X1)            // T1=X1-Z1.X2
        X1.tmul(Qy)            // X1=(X1-Z1.X2).Ys
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {
            X1.times_i()
        }              
        T1.mul(B.gety())       // T1=(X1-Z1.X2).Y2

        T2.copy(Y1)            // T2=Y1-Z1.Y2
        T2.mul(B.getx())       // T2=(Y1-Z1.Y2).X2
        T2.sub(T1); T2.norm()          // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
        Y1.tmul(Qx);  Y1.neg(); Y1.norm() // Y1=-(Y1-Z1.Y2).Xs

        a=FP16(X1,T2)       // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {              
            b=FP16(Y1)
            c=FP16()
        } else {
            b=FP16()
            c=FP16(Y1); c.times_i()
        }  
        A.add(B)

        var res=FP48(a,b,c)
        res.settype(FP48.SPARSER)
        return res
    }

    static private func lbits(_ n3:inout BIG,_ n:inout BIG) -> Int
    {
        n.copy(BIG(ROM.CURVE_Bnx))
        n3.copy(n)
        n3.pmul(3)
        n3.norm()
        return n3.nbits()          
    }

    static public func initmp() -> [FP48]
    {
        var r=[FP48]();
        for _ in (0...CONFIG_CURVE.ATE_BITS-1).reversed() {
            r.append(FP48(1))
        }
        return r
    }

/* basic Miller loop */
    static public func miller(_ r: [FP48]) -> FP48 {
        var res=FP48(1)
        for i in (1...CONFIG_CURVE.ATE_BITS-1).reversed() {
            res.sqr()
            res.ssmul(r[i])
        }

        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            res.conj();
        }
        res.ssmul(r[0])
        return res
    }

/* Accumulate another set of line functions for n-pairing */
    static public func another(_ r: inout [FP48],_ P1: ECP8,_ Q1: ECP) {
        var n = BIG();
        var n3 = BIG();

// P is needed in affine form for line function, Q for (Qx,Qy) extraction
        var P=ECP8(); P.copy(P1); P.affine()
        var Q=ECP(); Q.copy(Q1); Q.affine()

        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
    
        var A=ECP8()
        A.copy(P)
        var NP=ECP8()
        NP.copy(P)
        NP.neg()

        let nb=lbits(&n3,&n)

        for i in (1...nb-2).reversed() {
            var lv=linedbl(&A,Qx,Qy)

            let bt=n3.bit(UInt(i))-n.bit(UInt(i))
            if bt == 1 {
                let lv2=lineadd(&A,P,Qx,Qy)
                lv.smul(lv2)
            }
            if bt == -1 {
                let lv2=lineadd(&A,NP,Qx,Qy)
                lv.smul(lv2)
            }
            r[i].ssmul(lv)
        }
    }

    // Optimal R-ate pairing
    static public func ate(_ P1:ECP8,_ Q1:ECP) -> FP48
    {
        var n = BIG();
        var n3 = BIG();
        
        var lv:FP48

        var P=ECP8(); P.copy(P1); P.affine()
        var Q=ECP(); Q.copy(Q1); Q.affine()

        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
    
        var A=ECP8()
        var r=FP48(1)
    
        A.copy(P)
        var NP=ECP8()
        NP.copy(P)
        NP.neg()

        let nb=lbits(&n3,&n)
    
        for i in (1...nb-2).reversed()
        {
            r.sqr()            
            lv=linedbl(&A,Qx,Qy)

            let bt=n3.bit(UInt(i))-n.bit(UInt(i))
            if bt == 1 {
                let lv2=lineadd(&A,P,Qx,Qy)
                lv.smul(lv2)
            }
            if bt == -1 {
                let lv2=lineadd(&A,NP,Qx,Qy)
                lv.smul(lv2)
            }
            r.ssmul(lv)        
        }
    
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
         }     

        return r
    }    

    // Optimal R-ate double pairing e(P,Q).e(R,S)
    static public func ate2(_ P1:ECP8,_ Q1:ECP,_ R1:ECP8,_ S1:ECP) -> FP48
    {
        var n = BIG();
        var n3 = BIG();
        var lv:FP48
    
        var P=ECP8(); P.copy(P1); P.affine()
        var Q=ECP(); Q.copy(Q1); Q.affine()
        var R=ECP8(); R.copy(R1); R.affine()
        var S=ECP(); S.copy(S1); S.affine()

        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
        let Sx=FP(S.getx())
        let Sy=FP(S.gety())
    
        var A=ECP8()
        var B=ECP8()
        var r=FP48(1)
    
        A.copy(P)
        B.copy(R)
        var NP=ECP8()
        NP.copy(P)
        NP.neg()
        var NR=ECP8()
        NR.copy(R)
        NR.neg()

        let nb=lbits(&n3,&n)
    
        for i in (1...nb-2).reversed()
        {
            r.sqr()            
            lv=linedbl(&A,Qx,Qy)
            var lv2=linedbl(&B,Sx,Sy)
            lv.smul(lv2)
            r.ssmul(lv)
            let bt=n3.bit(UInt(i))-n.bit(UInt(i))

            if bt == 1 {
                lv=lineadd(&A,P,Qx,Qy)
                lv2=lineadd(&B,R,Sx,Sy)
                lv.smul(lv2)
                r.ssmul(lv)
            }

            if bt == -1 {
                lv=lineadd(&A,NP,Qx,Qy)
                lv2=lineadd(&B,NR,Sx,Sy)
                lv.smul(lv2)
                r.ssmul(lv)              
            }            
        }
    
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }     

        return r
    }

    // final exponentiation - keep separate for multi-pairings and to avoid thrashing stack
    static public func fexp(_ m:FP48) -> FP48
    {
        let f=FP2(BIG(ROM.Fra),BIG(ROM.Frb));
        var x=BIG(ROM.CURVE_Bnx)
        var r=FP48(m)
    
    // Easy part of final exp
        var lv=FP48(r)
        lv.inverse()
        r.conj()
    
        r.mul(lv)
        lv.copy(r)
        r.frob(f,8)
        r.mul(lv)
        if r.isunity() {
		r.zero()
		return r
	}        
    // Hard part of final exp

        var t7=FP48(r); t7.usqr()
        var t1=t7.pow(x)

        x.fshr(1)
        var t2=t1.pow(x)
        x.fshl(1)

        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        var t3=FP48(t1); t3.conj()
        t2.mul(t3)
        t2.mul(r)

        r.mul(t7)

        t1.copy(t2.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }
        t3.copy(t1)
        t3.frob(f,14)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,13)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,12)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,11)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,10)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,9)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,8)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t2); t3.conj()
        t1.mul(t3)
        t3.copy(t1);
        t3.frob(f,7)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,6)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,5)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,4)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,3)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,2)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }

        t3.copy(t1)
        t3.frob(f,1)
        r.mul(t3)
        t1.copy(t1.pow(x))
        if CONFIG_CURVE.SIGN_OF_X==CONFIG_CURVE.NEGATIVEX {
            t1.conj()
        }
    
        r.mul(t1)
        t2.frob(f,15)
        r.mul(t2)

        r.reduce()
        return r
    }

    // GLV method
    static func glv(_ e:BIG) -> [BIG]
    {
        var u=[BIG]();
        let q=BIG(ROM.CURVE_Order)
        var x=BIG(ROM.CURVE_Bnx)
        var x2=BIG.smul(x,x)
        x.copy(BIG.smul(x2,x2))
        x2.copy(BIG.smul(x,x))
        u.append(BIG(e))
        u[0].mod(x2)
        u.append(BIG(e))
        u[1].div(x2)
        u[1].rsub(q)

        return u
    }

    // Galbraith & Scott Method
    static func gs(_ e:BIG) -> [BIG]
    {
        var u=[BIG]();
        let q=BIG(ROM.CURVE_Order)        
        let x=BIG(ROM.CURVE_Bnx)
        var w=BIG(e)
        for i in 0 ..< 15
        {
            u.append(BIG(w))
            u[i].mod(x)
            w.div(x)
        }
        u.append(BIG(w))
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            u[1].copy(BIG.modneg(u[1],q))
            u[3].copy(BIG.modneg(u[3],q))         
            u[5].copy(BIG.modneg(u[5],q))
            u[7].copy(BIG.modneg(u[7],q))             
            u[9].copy(BIG.modneg(u[9],q))
            u[11].copy(BIG.modneg(u[11],q))         
            u[13].copy(BIG.modneg(u[13],q))
            u[15].copy(BIG.modneg(u[15],q))                              
        }        

        return u
    }   

    // Multiply P by e in group G1
    static public func G1mul(_ P:ECP,_ e:BIG) -> ECP
    {
        var R:ECP
        if (CONFIG_CURVE.USE_GLV)
        {
            R=ECP()
            R.copy(P)
            var Q=ECP()
            Q.copy(P); Q.affine()
            let q=BIG(ROM.CURVE_Order)
            let cru=FP(BIG(ROM.CURVE_Cru))
            var t=BIG(0)
            var u=PAIR256.glv(e)
            Q.mulx(cru);            
    
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
            u[0].norm()
            u[1].norm()
            R=R.mul2(u[0],Q,u[1])
        }
        else
        {
            R=P.mul(e)
        }
        return R
    }

    // Multiply P by e in group G2
    static public func G2mul(_ P:ECP8,_ e:BIG) -> ECP8
    {
        var R:ECP8
        if (CONFIG_CURVE.USE_GS_G2)
        {
            var Q=[ECP8]()
            let F=ECP8.frob_constants()
            let q=BIG(ROM.CURVE_Order);
            var u=PAIR256.gs(e);
    
            var t=BIG(0)
            Q.append(ECP8())
            Q[0].copy(P);
            for i in 1 ..< 16
            {
                Q.append(ECP8()); Q[i].copy(Q[i-1]);
                Q[i].frob(F,1);
            }
            for i in 0 ..< 16
            {
                let np=u[i].nbits()
                t.copy(BIG.modneg(u[i],q))
                let nn=t.nbits()
                if (nn<np)
                {
                    u[i].copy(t)
                    Q[i].neg()
                }
                u[i].norm()
            }
    
            R=ECP8.mul16(Q,u)
        }
        else
        {
            R=P.mul(e)
        }
        return R;
    }

    // f=f^e
    // Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP16.swift
    static public func GTpow(_ d:FP48,_ e:BIG) -> FP48
    {
        var r:FP48
        if (CONFIG_CURVE.USE_GS_GT)
        {
            var g=[FP48]()
            let f=FP2(BIG(ROM.Fra),BIG(ROM.Frb))
            let q=BIG(ROM.CURVE_Order)
            var t=BIG(0)
        
            var u=gs(e)
            g.append(FP48())
            g[0].copy(d);
            for i in 1 ..< 16
            {
                g.append(FP48()); g[i].copy(g[i-1])
                g[i].frob(f,1)
            }
            for i in 0 ..< 16
            {
                let np=u[i].nbits()
                t.copy(BIG.modneg(u[i],q))
                let nn=t.nbits()
                if (nn<np)
                {
                    u[i].copy(t)
                    g[i].conj()
                }
                u[i].norm()                
            }
            r=FP48.pow16(g,u)
        }
        else
        {
            r=d.pow(e)
        }
        return r
    }
}
