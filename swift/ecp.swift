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


final public class ECP {

    static public let WEIERSTRASS=0
    static public let EDWARDS=1
    static public let MONTGOMERY=2
    static public let NOT=0
    static public let BN=1
    static public let BLS=2
    static public let D_TYPE=0
    static public let M_TYPE=1
    static public let POSITIVEX=0
    static public let NEGATIVEX=1

    static public let CURVETYPE = @CT@
    static public let CURVE_PAIRING_TYPE = @PF@
    static public let SEXTIC_TWIST = @ST@
    static public let SIGN_OF_X = @SX@

    static public let HASH_TYPE = @HT@
    static public let AESKEY = @AK@

    private var x:FP
    private var y:FP
    private var z:FP
    //private var INF:Bool
    
   /* Constructor - set to O */
    init()
    {
        x=FP(0)
        y=FP(1)
        z=FP(0)
    //    INF=true
    }
    
    /* test for O point-at-infinity */
    public func is_infinity() -> Bool
    {
        //if INF {return true}        
        if (ECP.CURVETYPE==ECP.EDWARDS)
        {
            return x.iszilch() && y.equals(z)
        }
        if (ECP.CURVETYPE==ECP.WEIERSTRASS)
        {
            return x.iszilch() && z.iszilch()
        }        
        if (ECP.CURVETYPE==ECP.MONTGOMERY)     
        {
            return z.iszilch()
        }   
        return true
    }
 
    /* Conditional swap of P and Q dependant on d */
    private func cswap(_ Q: ECP,_ d:Int)
    {
        x.cswap(Q.x,d);
        if ECP.CURVETYPE != ECP.MONTGOMERY {y.cswap(Q.y,d)}
        z.cswap(Q.z,d);
/*
        var bd:Bool
        if d==0 {bd=false}
        else {bd=true}
        bd=bd && (INF != Q.INF)
        INF = (INF != bd)
        Q.INF = (Q.INF != bd) */
    }
    
    /* Conditional move of Q to P dependant on d */
    private func cmove(_ Q: ECP,_ d:Int)
    {
        x.cmove(Q.x,d);
        if ECP.CURVETYPE != ECP.MONTGOMERY {y.cmove(Q.y,d)}
        z.cmove(Q.z,d);
   /*     var bd:Bool
        if d==0 {bd=false}
        else {bd=true}
        INF = (INF != Q.INF) && bd; */
    }
    
    /* return 1 if b==c, no branching */
    private static func teq(_ b: Int32,_ c:Int32) -> Int
    {
        var x=b^c
        x-=1  // if x=0, x now -1
        return Int((x>>31)&1)
    }
 
    /* self=P */
    public func copy(_ P: ECP)
    {
        x.copy(P.x)
        if ECP.CURVETYPE != ECP.MONTGOMERY {y.copy(P.y)}
        z.copy(P.z)
    //    INF=P.INF
    }
    /* self=-self */
    func neg() {
    //    if is_infinity() {return}
        if (ECP.CURVETYPE == ECP.WEIERSTRASS)
        {
            y.neg(); y.norm();
        }
        if (ECP.CURVETYPE == ECP.EDWARDS)
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
    //    if (is_infinity() && Q.is_infinity()) {return true}
    //    if (is_infinity() || Q.is_infinity()) {return false}
 
        let a=FP(0)
        let b=FP(0)
        a.copy(x); a.mul(Q.z)
        b.copy(Q.x); b.mul(z)
        if !a.equals(b) {return false}
        if ECP.CURVETYPE != ECP.MONTGOMERY
        {
			a.copy(y); a.mul(Q.z); 
			b.copy(Q.y); b.mul(z); 
			if !a.equals(b) {return false}
        }
        return true
    }
  
/* set self=O */
    func inf()
    {
    //    INF=true;
        x.zero()
        if ECP.CURVETYPE != ECP.MONTGOMERY {y.one()}
        if ECP.CURVETYPE != ECP.EDWARDS {z.zero()}
        else {z.one()}
    }
    
    /* Calculate RHS of curve equation */
    static func RHS(_ x: FP) -> FP
    {
        x.norm();
        let r=FP(x);
        r.sqr();
    
        if ECP.CURVETYPE == ECP.WEIERSTRASS
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
        if (ECP.CURVETYPE == ECP.EDWARDS)
        { // (Ax^2-1)/(Bx^2-1)
            let b=FP(BIG(ROM.CURVE_B))
    
            let one=FP(1);
            b.mul(r);
            b.sub(one); b.norm()
            if ROM.CURVE_A == -1 {r.neg()}
            r.sub(one); r.norm()
            b.inverse()
            r.mul(b);
        }
        if ECP.CURVETYPE == ECP.MONTGOMERY
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
    public init(_ ix: BIG,_ iy: BIG)
    {
        x=FP(ix)
        y=FP(iy)
        z=FP(1)
    //    INF=true
        let rhs=ECP.RHS(x);
    
        if ECP.CURVETYPE == ECP.MONTGOMERY
        {
            if rhs.jacobi() != 1 {inf()}
        }
        else
        {
            let y2=FP(y)
            y2.sqr()
            if !y2.equals(rhs) {inf()}
        }
    }
    
    /* set (x,y) from BIG and a bit */
    public init(_ ix: BIG,_ s:Int)
    {
        x=FP(ix)
        let rhs=ECP.RHS(x)
        y=FP(0)
        z=FP(1)
    //    INF=true
        if rhs.jacobi()==1
        {
            let ny=rhs.sqrt()
            if (ny.redc().parity() != s) {ny.neg()}
            y.copy(ny)
   //         INF=false;
        }
        else {inf()}
    }
    
    /* set from x - calculate y from curve equation */
    public init(_ ix:BIG)
    {
        x=FP(ix)
        let rhs=ECP.RHS(x)
        y=FP(0)
        z=FP(1)
        if rhs.jacobi()==1
        {
            if ECP.CURVETYPE != ECP.MONTGOMERY {y.copy(rhs.sqrt())}
         //   INF=false;
        }
        else {inf()}
    }
    
    /* set to affine - from (x,y,z) to (x,y) */
    func affine()
    {
        if is_infinity() {return}
        let one=FP(1)
        if (z.equals(one)) {
            x.reduce(); y.reduce()
            return
        }
        z.inverse()

        x.mul(z); x.reduce()
        if ECP.CURVETYPE != ECP.MONTGOMERY
        {
            y.mul(z); y.reduce()
 
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
        let RM=Int(BIG.MODBYTES)
        var t=[UInt8](repeating: 0,count: RM)
        if ECP.CURVETYPE != ECP.MONTGOMERY {b[0]=0x04}
        else {b[0]=0x02}
    
        affine()
        x.redc().toBytes(&t)
        for i in 0 ..< RM {b[i+1]=t[i]}
        if ECP.CURVETYPE != ECP.MONTGOMERY
        {
            y.redc().toBytes(&t);
            for i in 0 ..< RM {b[i+RM+1]=t[i]}
        }
    }
    /* convert from byte array to point */
    static func fromBytes(_ b: [UInt8]) -> ECP
    {
        let RM=Int(BIG.MODBYTES)
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
        if ECP.CURVETYPE==ECP.MONTGOMERY {return "("+x.redc().toString()+")"}
        else {return "("+x.redc().toString()+","+y.redc().toString()+")"}
    }
    
    /* self*=2 */
    func dbl()
    {
//        if INF {return} 
        if (ECP.CURVETYPE == ECP.WEIERSTRASS)
        {

            if ROM.CURVE_A == 0
            {
                let t0=FP(y)        
                t0.sqr()
                let t1=FP(y)
                t1.mul(z);
                let t2=FP(z)
                t2.sqr()

                z.copy(t0)
                z.add(t0); z.norm() 
                z.add(z); z.add(z); z.norm()
                t2.imul(3*ROM.CURVE_B_I)

                let x3=FP(t2)
                x3.mul(z)

                let y3=FP(t0)
                y3.add(t2); y3.norm()
                z.mul(t1)
                t1.copy(t2); t1.add(t2); t2.add(t1)
                t0.sub(t2); t0.norm(); y3.mul(t0); y3.add(x3)
                t1.copy(x); t1.mul(y)
                x.copy(t0); x.norm(); x.mul(t1); x.add(x)
                x.norm()
                y.copy(y3); y.norm()
            }
            else {
                let t0=FP(x)
                let t1=FP(y)
                let t2=FP(z)
                let t3=FP(x)
                let z3=FP(z)
                let y3=FP(0)
                let x3=FP(0)
                let b=FP(0)

                if ROM.CURVE_B_I==0
                {
                    b.copy(FP(BIG(ROM.CURVE_B)))
                }

                t0.sqr()  //1    x^2
                t1.sqr()  //2    y^2
                t2.sqr()  //3

                t3.mul(y) //4
                t3.add(t3); t3.norm()//5
                z3.mul(x)   //6
                z3.add(z3);  z3.norm()//7
                y3.copy(t2) 
                
                if ROM.CURVE_B_I==0 {
                    y3.mul(b) //8
                }
                else { 
                    y3.imul(ROM.CURVE_B_I)
                }

                y3.sub(z3) //y3.norm(); //9  ***
                x3.copy(y3); x3.add(y3); x3.norm()//10

                y3.add(x3) //y3.norm();//11
                x3.copy(t1); x3.sub(y3); x3.norm()//12
                y3.add(t1); y3.norm()//13
                y3.mul(x3) //14
                x3.mul(t3) //15
                t3.copy(t2); t3.add(t2) //t3.norm(); //16
                t2.add(t3) //t2.norm(); //17

                if ROM.CURVE_B_I==0 {
                    z3.mul(b) //18
                }
                else {
                    z3.imul(ROM.CURVE_B_I)
                }

                z3.sub(t2) //z3.norm();//19
                z3.sub(t0); z3.norm()//20  ***
                t3.copy(z3); t3.add(z3) //t3.norm();//21

                z3.add(t3); z3.norm() //22
                t3.copy(t0); t3.add(t0) //t3.norm(); //23
                t0.add(t3) //t0.norm();//24
                t0.sub(t2); t0.norm()//25

                t0.mul(z3)//26
                y3.add(t0) //y3.norm();//27
                t0.copy(y); t0.mul(z)//28
                t0.add(t0); t0.norm() //29
                z3.mul(t0)//30
                x3.sub(z3) //x3.norm();//31
                t0.add(t0); t0.norm()//32
                t1.add(t1); t1.norm()//33
                z3.copy(t0); z3.mul(t1)//34

                x.copy(x3); x.norm()
                y.copy(y3); y.norm()
                z.copy(z3); z.norm()                
            }
        }
        if ECP.CURVETYPE == ECP.EDWARDS
        {
            let C=FP(x)
            let D=FP(y)
            let H=FP(z)
            let J=FP(0)
    
            x.mul(y); x.add(x); x.norm()
            C.sqr()
            D.sqr()
            if ROM.CURVE_A == -1 {C.neg()}
            y.copy(C); y.add(D); y.norm()
            H.sqr(); H.add(H)
            z.copy(y)
            J.copy(y); J.sub(H); J.norm()
            x.mul(J)
            C.sub(D); C.norm()
            y.mul(C)
            z.mul(J)
    
        }
        if ECP.CURVETYPE == ECP.MONTGOMERY
        {
            let A=FP(x)
            let B=FP(x);
            let AA=FP(0);
            let BB=FP(0);
            let C=FP(0);
        
            A.add(z); A.norm()
            AA.copy(A); AA.sqr()
            B.sub(z); B.norm()
            BB.copy(B); BB.sqr()
            C.copy(AA); C.sub(BB); C.norm()
    
            x.copy(AA); x.mul(BB)
    
            A.copy(C); A.imul((ROM.CURVE_A+2)/4)
    
            BB.add(A); BB.norm()
            z.copy(BB); z.mul(C)
        }
        return
    }
    
    /* self+=Q */
    func add(_ Q:ECP)
    {
    /*    if (INF)
        {
            copy(Q)
            return
        }
        if Q.INF {return} */

        if ECP.CURVETYPE == ECP.WEIERSTRASS
        {

                if ROM.CURVE_A == 0
                {
                    let b=3*ROM.CURVE_B_I
                    let t0=FP(x)
                    t0.mul(Q.x)
                    let t1=FP(y)
                    t1.mul(Q.y)
                    let t2=FP(z)
                    t2.mul(Q.z)
                    let t3=FP(x)
                    t3.add(y); t3.norm()
                    let t4=FP(Q.x)
                    t4.add(Q.y); t4.norm()
                    t3.mul(t4)
                    t4.copy(t0); t4.add(t1)

                    t3.sub(t4); t3.norm()
                    t4.copy(y)
                    t4.add(z); t4.norm()
                    let x3=FP(Q.y)
                    x3.add(Q.z); x3.norm()

                    t4.mul(x3)
                    x3.copy(t1)
                    x3.add(t2)
    
                    t4.sub(x3); t4.norm()
                    x3.copy(x); x3.add(z); x3.norm()
                    let y3=FP(Q.x)
                    y3.add(Q.z); y3.norm()
                    x3.mul(y3)
                    y3.copy(t0)
                    y3.add(t2)
                    y3.rsub(x3); y3.norm()
                    x3.copy(t0); x3.add(t0)
                    t0.add(x3); t0.norm()
                    t2.imul(b);

                    let z3=FP(t1); z3.add(t2); z3.norm()
                    t1.sub(t2); t1.norm()
                    y3.imul(b)
    
                    x3.copy(y3); x3.mul(t4); t2.copy(t3); t2.mul(t1); x3.rsub(t2)
                    y3.mul(t0); t1.mul(z3); y3.add(t1)
                    t0.mul(t3); z3.mul(t4); z3.add(t0)

                    x.copy(x3); x.norm() 
                    y.copy(y3); y.norm()
                    z.copy(z3); z.norm()
                } 
                else {

                    let t0=FP(x)
                    let t1=FP(y)
                    let t2=FP(z)
                    let t3=FP(x)
                    let t4=FP(Q.x)
                    let z3=FP(0)
                    let y3=FP(Q.x)
                    let x3=FP(Q.y)
                    let b=FP(0)

                    if ROM.CURVE_B_I==0
                    {
                        b.copy(FP(BIG(ROM.CURVE_B)))
                    }

                    t0.mul(Q.x) //1
                    t1.mul(Q.y) //2
                    t2.mul(Q.z) //3

                    t3.add(y); t3.norm() //4
                    t4.add(Q.y); t4.norm()//5
                    t3.mul(t4)//6
                    t4.copy(t0); t4.add(t1) //t4.norm(); //7
                    t3.sub(t4); t3.norm() //8
                    t4.copy(y); t4.add(z); t4.norm()//9
                    x3.add(Q.z); x3.norm()//10
                    t4.mul(x3) //11
                    x3.copy(t1); x3.add(t2) //x3.norm();//12

                    t4.sub(x3); t4.norm()//13
                    x3.copy(x); x3.add(z); x3.norm() //14
                    y3.add(Q.z); y3.norm()//15

                    x3.mul(y3) //16
                    y3.copy(t0); y3.add(t2) //y3.norm();//17

                    y3.rsub(x3); y3.norm() //18
                    z3.copy(t2)
                

                    if ROM.CURVE_B_I==0
                    {
                        z3.mul(b) //18
                    }
                    else {
                        z3.imul(ROM.CURVE_B_I)
                    }
                
                    x3.copy(y3); x3.sub(z3); x3.norm() //20
                    z3.copy(x3); z3.add(x3) //z3.norm(); //21

                    x3.add(z3) //x3.norm(); //22
                    z3.copy(t1); z3.sub(x3); z3.norm() //23
                    x3.add(t1); x3.norm() //24

                    if ROM.CURVE_B_I==0
                    {
                        y3.mul(b) //18
                    }
                    else {
                        y3.imul(ROM.CURVE_B_I)
                    }

                    t1.copy(t2); t1.add(t2) //t1.norm();//26
                    t2.add(t1) //t2.norm();//27

                    y3.sub(t2) //y3.norm(); //28

                    y3.sub(t0); y3.norm() //29
                    t1.copy(y3); t1.add(y3) //t1.norm();//30
                    y3.add(t1); y3.norm() //31

                    t1.copy(t0); t1.add(t0) //t1.norm(); //32
                    t0.add(t1) //t0.norm();//33
                    t0.sub(t2); t0.norm()//34
                    t1.copy(t4); t1.mul(y3)//35
                    t2.copy(t0); t2.mul(y3)//36
                    y3.copy(x3); y3.mul(z3)//37
                    y3.add(t2) //y3.norm();//38
                    x3.mul(t3)//39
                    x3.sub(t1)//40
                    z3.mul(t4)//41
                    t1.copy(t3); t1.mul(t0)//42
                    z3.add(t1)
                    x.copy(x3); x.norm() 
                    y.copy(y3); y.norm()
                    z.copy(z3); z.norm()
                }
        }
        if ECP.CURVETYPE == ECP.EDWARDS
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
            B.norm(); D.norm()
            B.mul(D)
            B.sub(C); B.norm(); F.norm()
            B.mul(F)
            x.copy(A); x.mul(B)
            G.norm()
            if ROM.CURVE_A==1
            {
				E.norm(); C.copy(E); C.mul(G)
            }
            if ROM.CURVE_A == -1
            {
				C.norm(); C.mul(G)
            }
            y.copy(A); y.mul(C)
            z.copy(F); z.mul(G)

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
        A.norm()
    
        D.norm()
        DA.copy(D); DA.mul(A)

        C.norm();
        B.norm();        
        CB.copy(C); CB.mul(B)
        
        A.copy(DA); A.add(CB); A.norm(); A.sqr()
        B.copy(DA); B.sub(CB); B.norm(); B.sqr()
    
        x.copy(A)
        z.copy(W.x); z.mul(B)

    }
    /* this-=Q */
    func sub(_ Q:ECP)
    {
        Q.neg()
        add(Q)
        Q.neg()
    }

    /* constant time multiply by small integer of length bts - use ladder */
    func pinmul(_ e:Int32,_ bts:Int32) -> ECP
    {
        if ECP.CURVETYPE == ECP.MONTGOMERY
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
    
    public func mul(_ e:BIG) -> ECP
    {
        if (e.iszilch() || is_infinity()) {return ECP()}
    
        let P=ECP()
        if ECP.CURVETYPE == ECP.MONTGOMERY
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
            let n=1+(BIG.NLEN*Int(BIG.BASEBITS)+3)/4
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
    
    public func mul2(_ e:BIG,_ Q:ECP,_ f:BIG) -> ECP
    {
        let te=BIG()
        let tf=BIG()
        let mt=BIG()
        let S=ECP()
        let T=ECP()
        let C=ECP()
        var W=[ECP]()
        let n=1+(BIG.NLEN*Int(BIG.BASEBITS)+1)/2
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
    
   func cfp()
   {

	let cf=ROM.CURVE_Cof_I;
	if cf==1 {return}
	if cf==4 {
		dbl(); dbl()
		affine()
		return
	} 
	if cf==8 {
		dbl(); dbl(); dbl()
		affine()
		return;
	}
	let c=BIG(ROM.CURVE_Cof);
	copy(mul(c));

   }

    static func mapit(_ h:[UInt8]) -> ECP
    {
        let q=BIG(ROM.Modulus)
        let x=BIG.fromBytes(h)
        x.mod(q)
        let P=ECP()
        while (true) {
		while (true) {
			if ECP.CURVETYPE != ECP.MONTGOMERY {
				P.copy(ECP(x,0))
			} else {
				P.copy(ECP(x))
			}
			x.inc(1); x.norm();
			if !P.is_infinity() {break}
		}
		P.cfp()
		if !P.is_infinity() {break}
	}

        return P
    }    
   
    static public func generator() -> ECP
    {
        let gx=BIG(ROM.CURVE_Gx);
        var G:ECP
        if ECP.CURVETYPE != ECP.MONTGOMERY
        {
            let gy=BIG(ROM.CURVE_Gy)
            G=ECP(gx,gy)
        }
        else
            {G=ECP(gx)}   
        return G     
    }
    
}
