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

public struct PAIR {
    
    // Line function
    static func linedbl(_ A: inout ECP2,_ Qx:FP,_ Qy:FP) -> FP12
    {
        var a:FP4
        var b:FP4
        var c:FP4

        var XX=FP2(A.getx())  //X
        var YY=FP2(A.gety())  //Y
        var ZZ=FP2(A.getz())  //Z
        var YZ=FP2(YY)        //Y 
        YZ.mul(ZZ)                //YZ
        XX.sqr()                  //X^2
        YY.sqr()                  //Y^2
        ZZ.sqr()                  //Z^2
            
        YZ.imul(4)
        YZ.neg(); YZ.norm()       //-2YZ
        YZ.pmul(Qy)               //-2YZ.Ys

        XX.imul(6)               //3X^2
        XX.pmul(Qx)              //3X^2.Xs

        let sb=3*ROM.CURVE_B_I
        ZZ.imul(sb)  
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {             
             ZZ.div_ip2();  
        }
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {
            ZZ.mul_ip()
            ZZ.add(ZZ)
            ZZ.norm()
            YZ.mul_ip()
            YZ.norm()
        }              
        ZZ.norm() // 3b.Z^2 

        YY.add(YY)
        ZZ.sub(YY); ZZ.norm()     // 3b.Z^2-Y^2

        a=FP4(YZ,ZZ)          // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs 
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {             
            b=FP4(XX)            // L(0,1) | L(0,0) | L(1,0)
            c=FP4(0)
        } else { 
            b=FP4(0)
            c=FP4(XX); c.times_i()
        }        
        A.dbl()

        return FP12(a,b,c)
    }


    // Line function
    static func lineadd(_ A: inout ECP2,_ B:ECP2,_ Qx:FP,_ Qy:FP) -> FP12
    {
        var a:FP4
        var b:FP4
        var c:FP4

        var X1=FP2(A.getx())    // X1
        var Y1=FP2(A.gety())    // Y1
        var T1=FP2(A.getz())    // Z1
        var T2=FP2(A.getz())    // Z1
            
        T1.mul(B.gety())    // T1=Z1.Y2 
        T2.mul(B.getx())    // T2=Z1.X2

        X1.sub(T2); X1.norm()  // X1=X1-Z1.X2
        Y1.sub(T1); Y1.norm()  // Y1=Y1-Z1.Y2

        T1.copy(X1)            // T1=X1-Z1.X2
        X1.pmul(Qy)            // X1=(X1-Z1.X2).Ys
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {
            X1.mul_ip()
            X1.norm()
        }              
        T1.mul(B.gety())       // T1=(X1-Z1.X2).Y2

        T2.copy(Y1)            // T2=Y1-Z1.Y2
        T2.mul(B.getx())       // T2=(Y1-Z1.Y2).X2
        T2.sub(T1); T2.norm()          // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
        Y1.pmul(Qx);  Y1.neg(); Y1.norm() // Y1=-(Y1-Z1.Y2).Xs

        a=FP4(X1,T2)       // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
        if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.D_TYPE {              
            b=FP4(Y1)
            c=FP4(0)
        } else {
            b=FP4(0)
            c=FP4(Y1); c.times_i()
         }  
        A.add(B)
        return FP12(a,b,c)
    }


    // Optimal R-ate pairing
    static public func ate(_ P1:ECP2,_ Q1:ECP) -> FP12
    {
        var f=FP2(BIG(ROM.Fra),BIG(ROM.Frb))
        let x=BIG(ROM.CURVE_Bnx)
        var n=BIG(x)
        var K=ECP2()
        
        var lv:FP12

        if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
            if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {  
                f.inverse()
                f.norm()
            }
            n.pmul(6);
            if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX { 
                n.dec(2)
            } else {
                n.inc(2)
            }
        } else {n.copy(x)}
	
        n.norm()

        var n3=BIG(n)
        n3.pmul(3)
        n3.norm()

        var P=ECP2(); P.copy(P1); P.affine()
        var Q=ECP(); Q.copy(Q1); Q.affine()


        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
    
        var A=ECP2()
        A.copy(P)

        var NP=ECP2()
        NP.copy(P)
        NP.neg()

        var r=FP12(1)
        let nb=n3.nbits()
    
        for i in (1...nb-2).reversed()
        //for var i=nb-2;i>=1;i--
        {
            r.sqr()            
            lv=linedbl(&A,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            let bt=n3.bit(UInt(i))-n.bit(UInt(i))
            if bt == 1 {
		      lv=lineadd(&A,P,Qx,Qy)
		      r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            }
            if bt == -1 {
                lv=lineadd(&A,NP,Qx,Qy)
                r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            }
        }
    
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
         }     

    // R-ate fixup required for BN curves

	   if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
            if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
                A.neg()
            }           
            K.copy(P)
            K.frob(f)

            lv=lineadd(&A,K,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            K.frob(f)
            K.neg()
            lv=lineadd(&A,K,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
        }
        return r
    }
    // Optimal R-ate double pairing e(P,Q).e(R,S)
    static public func ate2(_ P1:ECP2,_ Q1:ECP,_ R1:ECP2,_ S1:ECP) -> FP12
    {
        var f=FP2(BIG(ROM.Fra),BIG(ROM.Frb))
        let x=BIG(ROM.CURVE_Bnx)
        var n=BIG(x)
        var K=ECP2()
        var lv:FP12

        if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
            if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {  
                f.inverse()
                f.norm()
            }
            n.pmul(6); 
            if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX { 
                n.dec(2)
            } else {
                n.inc(2)
            }        
        } else {n.copy(x)}
	
        n.norm()
        var n3=BIG(n)
        n3.pmul(3)
        n3.norm()
    
	   var P=ECP2(); P.copy(P1); P.affine()
	   var Q=ECP(); Q.copy(Q1); Q.affine()
	   var R=ECP2(); R.copy(R1); R.affine()
	   var S=ECP(); S.copy(S1); S.affine()


        let Qx=FP(Q.getx())
        let Qy=FP(Q.gety())
        let Sx=FP(S.getx())
        let Sy=FP(S.gety())
    
        var A=ECP2()
        var B=ECP2()
        var r=FP12(1)
    
        A.copy(P)
        B.copy(R)
        var NP=ECP2()
        NP.copy(P)
        NP.neg()
        var NR=ECP2()
        NR.copy(R)
        NR.neg()


        let nb=n3.nbits()
    
        for i in (1...nb-2).reversed()
        {
            r.sqr()            
            lv=linedbl(&A,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            lv=linedbl(&B,Sx,Sy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            let bt=n3.bit(UInt(i))-n.bit(UInt(i))

            if bt == 1 {
                lv=lineadd(&A,P,Qx,Qy)
                r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
                lv=lineadd(&B,R,Sx,Sy)
                r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            }

            if bt == -1 {
                lv=lineadd(&A,NP,Qx,Qy)
                r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
                lv=lineadd(&B,NR,Sx,Sy)
                r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)              
            }            

        }
    
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
         }     

    // R-ate fixup required for BN curves

	   if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
            if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
                A.neg()                
                B.neg()
            }
            K.copy(P)
            K.frob(f)

            lv=lineadd(&A,K,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            K.frob(f)
            K.neg()
            lv=lineadd(&A,K,Qx,Qy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
    
            K.copy(R)
            K.frob(f)

            lv=lineadd(&B,K,Sx,Sy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
            K.frob(f)
            K.neg()
            lv=lineadd(&B,K,Sx,Sy)
            r.smul(lv,CONFIG_CURVE.SEXTIC_TWIST)
        }
        return r
    }
    
    // final exponentiation - keep separate for multi-pairings and to avoid thrashing stack
    static public func fexp(_ m:FP12) -> FP12
    {
        let f=FP2(BIG(ROM.Fra),BIG(ROM.Frb));
        let x=BIG(ROM.CURVE_Bnx)
        var r=FP12(m)
    
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
	if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
		lv.copy(r)
		lv.frob(f)
		var x0=FP12(lv)
		x0.frob(f)
		lv.mul(r)
		x0.mul(lv)
		x0.frob(f)
		var x1=FP12(r)
		x1.conj()
		var x4=r.pow(x)
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.POSITIVEX {
            x4.conj()
        }
		var x3=FP12(x4)
		x3.frob(f)
    
		var x2=x4.pow(x)
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.POSITIVEX {
            x2.conj()
        }    
		var x5=FP12(x2); x5.conj()
		lv=x2.pow(x)
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.POSITIVEX {
            lv.conj()
        }   
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
		var x0=FP12(r)
		var x1=FP12(r)
		lv.copy(r); lv.frob(f)
		var x3=FP12(lv); x3.conj(); x1.mul(x3)
		lv.frob(f); lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x)) 
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }          
		x3.copy(r); x3.conj(); x1.mul(x3)
		lv.copy(r); lv.frob(f)
		x0.mul(lv)
		lv.frob(f)
		x1.mul(lv)
		lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)

		r.copy(r.pow(x))
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }  
		x0.mul(r)
		lv.copy(r); lv.frob(f); lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)
		lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }          
		lv.copy(r); lv.frob(f)
		x3.copy(lv); x3.conj(); x0.mul(x3)
		lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }          
		x3.copy(r); x3.conj(); x0.mul(x3)
		lv.copy(r); lv.frob(f)
		x1.mul(lv)

		r.copy(r.pow(x))
        if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
            r.conj()
        }          
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
	if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
		var t=BIG(0)
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
			var d=BIG.mul(t,e)
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
        if CONFIG_CURVE.CURVE_PAIRING_TYPE == CONFIG_CURVE.BN {
		  var t=BIG(0)
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
			 var d=BIG.mul(t,e)
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
            let q=BIG(ROM.CURVE_Order)        
            let x=BIG(ROM.CURVE_Bnx)
            var w=BIG(e)
            for i in 0 ..< 3
            {
			     u.append(BIG(w))
			     u[i].mod(x)
			     w.div(x)
            }
            u.append(BIG(w))
            if CONFIG_CURVE.SIGN_OF_X == CONFIG_CURVE.NEGATIVEX {
                u[1].copy(BIG.modneg(u[1],q))
                u[3].copy(BIG.modneg(u[3],q))                
            }        
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
            var u=PAIR.glv(e)
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
    static public func G2mul(_ P:ECP2,_ e:BIG) -> ECP2
    {
        var R:ECP2
        if (CONFIG_CURVE.USE_GS_G2)
        {
            var Q=[ECP2]()
            var f=FP2(BIG(ROM.Fra),BIG(ROM.Frb));
            let q=BIG(ROM.CURVE_Order);
            var u=PAIR.gs(e);
    
            if CONFIG_CURVE.SEXTIC_TWIST == CONFIG_CURVE.M_TYPE {  
                f.inverse()
                f.norm()
            }

            var t=BIG(0)
            Q.append(ECP2())
            Q[0].copy(P);
            for i in 1 ..< 4
            {
                Q.append(ECP2()); Q[i].copy(Q[i-1]);
				Q[i].frob(f);
            }
            for i in 0 ..< 4
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
    
            R=ECP2.mul4(Q,u)
        }
        else
        {
            R=P.mul(e)
        }
        return R;
    }
    // f=f^e
    // Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java
    static public func GTpow(_ d:FP12,_ e:BIG) -> FP12
    {
        var r:FP12
        if (CONFIG_CURVE.USE_GS_GT)
        {
            var g=[FP12]()
            let f=FP2(BIG(ROM.Fra),BIG(ROM.Frb))
            let q=BIG(ROM.CURVE_Order)
            var t=BIG(0)
        
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
                u[i].norm()                
            }
            r=FP12.pow4(g,u)
        }
        else
        {
            r=d.pow(e)
        }
        return r
    }
}

