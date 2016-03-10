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
//  mpin.swift
//  
//
//  Created by Michael Scott on 08/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation

final public class MPIN
{
    static public let EFS=Int(ROM.MODBYTES)
    static public let EGS=Int(ROM.MODBYTES)
    static public let PAS:Int=16
    static let INVALID_POINT:Int = -14
    static let BAD_PARAMS:Int = -11
    static let WRONG_ORDER:Int = -18
    static public let BAD_PIN:Int = -19

    /* Configure your PIN here */

    static let MAXPIN:Int32 = 10000  /* PIN less than this */
    static let PBLEN:Int32 = 14      /* Number of bits in PIN */
    static let TS:Int = 10         /* 10 for 4 digit PIN, 14 for 6-digit PIN - 2^TS/TS approx = sqrt(MAXPIN) */
    static let TRAP:Int = 200      /* 200 for 4 digit PIN, 2000 for 6-digit PIN  - approx 2*sqrt(MAXPIN) */

    /* Hash number (optional) and string to point on curve */

    private static func hashit(n:Int32,_ ID:[UInt8]) -> [UInt8]
    {
        let H=HASH()
        if n != 0 {H.process_num(n)}
        H.process_array(ID)
        let h=H.hash()
        return h
    }

    static func mapit(h:[UInt8]) -> ECP
    {
        let q=BIG(ROM.Modulus)
        let x=BIG.fromBytes(h)
        x.mod(q)
        var P=ECP(x,0)
        while (true)
        {
            if !P.is_infinity() {break}
            x.inc(1); x.norm();
            P=ECP(x,0);
        }
        return P
    }

    /* needed for SOK */
    static func mapit2(h:[UInt8]) -> ECP2
    {
        let q=BIG(ROM.Modulus)
        var x=BIG.fromBytes(h)
        let one=BIG(1)
        var Q=ECP2()
        x.mod(q);
        while (true)
        {
            let X=FP2(one,x);
            Q=ECP2(X);
            if !Q.is_infinity() {break}
            x.inc(1); x.norm();
        }
    /* Fast Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez */
        let Fra=BIG(ROM.CURVE_Fra);
        let Frb=BIG(ROM.CURVE_Frb);
        let X=FP2(Fra,Frb);
        x=BIG(ROM.CURVE_Bnx);

        let T=ECP2(); T.copy(Q)
        T.mul(x); T.neg()
        let K=ECP2(); K.copy(T)
        K.dbl(); K.add(T); K.affine()

        K.frob(X)
        Q.frob(X); Q.frob(X); Q.frob(X)
        Q.add(T); Q.add(K)
        T.frob(X); T.frob(X)
        Q.add(T)
        Q.affine()
        return Q
    }

    /* return time in slots since epoch */
    static public func today() -> Int32
    {
        let date=NSDate()
        return (Int32(date.timeIntervalSince1970/(60*1440)))
    }

    /* these next two functions help to implement elligator squared - http://eprint.iacr.org/2014/043 */
    /* maps a random u to a point on the curve */
    static func map(u:BIG,_ cb:Int32) -> ECP
    {
        let x=BIG(u)
        let p=BIG(ROM.Modulus)
        x.mod(p)
        var P=ECP(x,cb)
        while (true)
        {
            if !P.is_infinity() {break}
            x.inc(1);  x.norm()
            P=ECP(x,cb)
        }
        return P
    }

    /* returns u derived from P. Random value in range 1 to return value should then be added to u */
    static func unmap(inout u:BIG,_ P:ECP) -> Int32
    {
        let s=P.getS()
        var r:Int32=0
        let x=P.getX()
        u.copy(x)
        var R=ECP()
        while (true)
        {
            u.dec(1); u.norm()
            r++
            R=ECP(u,s)
            if !R.is_infinity() {break}
        }
        return r
    }

    static public func HASH_ID(ID:[UInt8]) -> [UInt8]
    {
        return hashit(0,ID)
    }

    /* these next two functions implement elligator squared - http://eprint.iacr.org/2014/043 */
    /* Elliptic curve point E in format (0x04,x,y} is converted to form {0x0-,u,v} */
    /* Note that u and v are indistinguisible from random strings */
    static public func ENCODING(rng:RAND,inout _ E:[UInt8]) -> Int
    {
        var T=[UInt8](count:EFS,repeatedValue:0)

        for var i=0;i<EFS;i++ {T[i]=E[i+1]}
        var u=BIG.fromBytes(T);
        for var i=0;i<EFS;i++ {T[i]=E[i+EFS+1]}
        var v=BIG.fromBytes(T)

        let P=ECP(u,v);
        if P.is_infinity() {return INVALID_POINT}

        let p=BIG(ROM.Modulus)
        u=BIG.randomnum(p,rng)

        var su=rng.getByte();
        su%=2

        let W=MPIN.map(u,Int32(su))
        P.sub(W);
        let sv=P.getS();
        let rn=MPIN.unmap(&v,P)
        let m=rng.getByte();
        let incr:Int32=1+Int32(m)%rn
        v.inc(incr)
        E[0]=(su+UInt8(2*sv))
        u.toBytes(&T)
        for var i=0;i<EFS;i++ {E[i+1]=T[i]}
        v.toBytes(&T)
        for var i=0;i<EFS;i++ {E[i+EFS+1]=T[i]}

        return 0;
    }

    static public func DECODING(inout D:[UInt8]) -> Int
    {
        var T=[UInt8](count:EFS,repeatedValue:0)

        if (D[0]&0x04) != 0 {return INVALID_POINT}

        for var i=0;i<EFS;i++ {T[i]=D[i+1]}
        var u=BIG.fromBytes(T)
        for var i=0;i<EFS;i++ {T[i]=D[i+EFS+1]}
        var v=BIG.fromBytes(T)

        let su=D[0]&1
        let sv=(D[0]>>1)&1
        let W=map(u,Int32(su))
        let P=map(v,Int32(sv))
        P.add(W)
        u=P.getX()
        v=P.getY()
        D[0]=0x04
        u.toBytes(&T);
        for var i=0;i<EFS;i++ {D[i+1]=T[i]}
        v.toBytes(&T)
        for var i=0;i<EFS;i++ {D[i+EFS+1]=T[i]}

        return 0
    }
    /* R=R1+R2 in group G1 */
    static public func RECOMBINE_G1(R1:[UInt8],_ R2:[UInt8],inout _ R:[UInt8]) -> Int
    {
        let P=ECP.fromBytes(R1)
        let Q=ECP.fromBytes(R2)

        if P.is_infinity() || Q.is_infinity() {return INVALID_POINT}

        P.add(Q)

        P.toBytes(&R)
        return 0;
    }
    /* W=W1+W2 in group G2 */
    static public func RECOMBINE_G2(W1:[UInt8],_ W2:[UInt8],inout _  W:[UInt8]) -> Int
    {
        let P=ECP2.fromBytes(W1)
        let Q=ECP2.fromBytes(W2)

        if P.is_infinity() || Q.is_infinity() {return INVALID_POINT}

        P.add(Q)

        P.toBytes(&W)
        return 0
    }
    /* create random secret S */
    static public func RANDOM_GENERATE(rng:RAND,inout _ S:[UInt8]) -> Int
    {
        let r=BIG(ROM.CURVE_Order)
        let s=BIG.randomnum(r,rng)

        s.toBytes(&S);
        return 0;
    }
    /* Extract PIN from TOKEN for identity CID */
    static public func EXTRACT_PIN(CID:[UInt8],_ pin:Int32,inout _ TOKEN:[UInt8]) -> Int
    {
        let P=ECP.fromBytes(TOKEN)
        if P.is_infinity() {return INVALID_POINT}
        let h=MPIN.hashit(0,CID)
        var R=MPIN.mapit(h)


        R=R.pinmul(pin%MAXPIN,MPIN.PBLEN)
        P.sub(R)

        P.toBytes(&TOKEN)

        return 0
    }
    /* Implement step 2 on client side of MPin protocol */
    static public func CLIENT_2(X:[UInt8],_ Y:[UInt8],inout _ SEC:[UInt8]) -> Int
    {
        let r=BIG(ROM.CURVE_Order)
        let P=ECP.fromBytes(SEC)
        if P.is_infinity() {return INVALID_POINT}

        let px=BIG.fromBytes(X)
        let py=BIG.fromBytes(Y)
        px.add(py)
        px.mod(r)
        px.rsub(r)

        PAIR.G1mul(P,px).toBytes(&SEC)
        return 0
    }

    /* Implement step 1 on client side of MPin protocol */
    static public func CLIENT_1(date:Int32,_ CLIENT_ID:[UInt8],_ rng:RAND?,inout _ X:[UInt8],_ pin:Int32,_ TOKEN:[UInt8],inout _ SEC:[UInt8],inout _ xID:[UInt8]?,inout _ xCID:[UInt8]?,_ PERMIT:[UInt8]) -> Int
    {
        let r=BIG(ROM.CURVE_Order)
   //     let q=BIG(ROM.Modulus)
        var x:BIG
        if rng != nil
        {
            x=BIG.randomnum(r,rng!)
            x.toBytes(&X);
        }
        else
        {
            x=BIG.fromBytes(X);
        }
    //    var t=[UInt8](count:EFS,repeatedValue:0)

        var h=MPIN.hashit(0,CLIENT_ID)
        var P=mapit(h);

        let T=ECP.fromBytes(TOKEN);
        if T.is_infinity() {return INVALID_POINT}

        var W=P.pinmul(pin%MPIN.MAXPIN,MPIN.PBLEN)
        T.add(W)
        if date != 0
        {
            W=ECP.fromBytes(PERMIT)
            if W.is_infinity() {return INVALID_POINT}
            T.add(W);
            h=MPIN.hashit(date,h)
            W=MPIN.mapit(h);
            if xID != nil
            {
				P=PAIR.G1mul(P,x)
				P.toBytes(&xID!)
				W=PAIR.G1mul(W,x)
				P.add(W)
            }
            else
            {
				P.add(W);
				P=PAIR.G1mul(P,x);
            }
            if xCID != nil {P.toBytes(&xCID!)}
        }
        else
        {
            if xID != nil
            {
				P=PAIR.G1mul(P,x)
				P.toBytes(&xID!)
            }
        }


        T.toBytes(&SEC);
        return 0;
    }
    /* Extract Server Secret SST=S*Q where Q is fixed generator in G2 and S is master secret */
    static public func GET_SERVER_SECRET(S:[UInt8],inout _ SST:[UInt8]) -> Int
    {
        var Q=ECP2(FP2(BIG(ROM.CURVE_Pxa),BIG(ROM.CURVE_Pxb)),FP2(BIG(ROM.CURVE_Pya),BIG(ROM.CURVE_Pyb)))

        let s=BIG.fromBytes(S)
        Q=PAIR.G2mul(Q,s)
        Q.toBytes(&SST)
        return 0
    }

    /*
    W=x*H(G);
    if RNG == NULL then X is passed in
    if RNG != NULL the X is passed out
    if type=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
    */
    static public func GET_G1_MULTIPLE(rng:RAND?,_ type:Int,inout _ X:[UInt8],_ G:[UInt8],inout _ W:[UInt8]) -> Int
    {
        var x:BIG
        let r=BIG(ROM.CURVE_Order)
        if rng != nil
        {
            x=BIG.randomnum(r,rng!)
            x.toBytes(&X)
        }
        else
        {
            x=BIG.fromBytes(X);
        }
        var P:ECP
        if type==0
        {
            P=ECP.fromBytes(G)
            if P.is_infinity() {return INVALID_POINT}
        }
        else
            {P=MPIN.mapit(G)}

        PAIR.G1mul(P,x).toBytes(&W)
        return 0;
    }
    /* Client secret CST=S*H(CID) where CID is client ID and S is master secret */
    /* CID is hashed externally */
    static public func GET_CLIENT_SECRET(inout S:[UInt8],_ CID:[UInt8],inout _ CST:[UInt8]) -> Int
    {
        return GET_G1_MULTIPLE(nil,1,&S,CID,&CST)
    }
    /* Time Permit CTT=S*(date|H(CID)) where S is master secret */
    static public func GET_CLIENT_PERMIT(date:Int32,_ S:[UInt8],_ CID:[UInt8],inout _ CTT:[UInt8]) -> Int
    {
        let h=MPIN.hashit(date,CID)
        let P=MPIN.mapit(h)

        let s=BIG.fromBytes(S)
        PAIR.G1mul(P,s).toBytes(&CTT)
        return 0;
    }

    /* Outputs H(CID) and H(T|H(CID)) for time permits. If no time permits set HID=HTID */
    static public func SERVER_1(date:Int32,_ CID:[UInt8],inout _ HID:[UInt8]?,inout _ HTID:[UInt8])
    {
        var h=MPIN.hashit(0,CID)
        let P=MPIN.mapit(h)

        if date != 0
        {
            if HID != nil {P.toBytes(&HID!)}
            h=hashit(date,h)
            let R=MPIN.mapit(h)
            P.add(R)
            P.toBytes(&HTID)
        }
        else {P.toBytes(&HID!)}
    }
    /* Implement step 2 of MPin protocol on server side */
    static public func SERVER_2(date:Int32,_ HID:[UInt8]?,_ HTID:[UInt8],_ Y:[UInt8],_ SST:[UInt8],_ xID:[UInt8]?,_ xCID:[UInt8],_ mSEC:[UInt8],inout _ E:[UInt8]?,inout _ F:[UInt8]?) -> Int
    {
        _=BIG(ROM.Modulus);
        let Q=ECP2(FP2(BIG(ROM.CURVE_Pxa),BIG(ROM.CURVE_Pxb)),FP2(BIG(ROM.CURVE_Pya),BIG(ROM.CURVE_Pyb)))
        let sQ=ECP2.fromBytes(SST)
        if sQ.is_infinity() {return INVALID_POINT}

        var R:ECP
        if date != 0
            {R=ECP.fromBytes(xCID)}
        else
        {
            if xID==nil {return MPIN.BAD_PARAMS}
            R=ECP.fromBytes(xID!)
        }
        if R.is_infinity() {return INVALID_POINT}

        let y=BIG.fromBytes(Y)
        var P:ECP
        if date != 0 {P=ECP.fromBytes(HTID)}
        else
        {
            if HID==nil {return MPIN.BAD_PARAMS}
            P=ECP.fromBytes(HID!)
        }

        if P.is_infinity() {return INVALID_POINT}

        P=PAIR.G1mul(P,y)
        P.add(R)
        R=ECP.fromBytes(mSEC)
        if R.is_infinity() {return MPIN.INVALID_POINT}


        var g=PAIR.ate2(Q,R,sQ,P)
        g=PAIR.fexp(g)

        if !g.isunity()
        {
            if HID != nil && xID != nil && E != nil && F != nil
            {
				g.toBytes(&E!)
				if date != 0
				{
                    P=ECP.fromBytes(HID!)
                    if P.is_infinity() {return MPIN.INVALID_POINT}
                    R=ECP.fromBytes(xID!)
                    if R.is_infinity() {return MPIN.INVALID_POINT}

                    P=PAIR.G1mul(P,y);
                    P.add(R);
				}
				g=PAIR.ate(Q,P);
				g=PAIR.fexp(g);
				g.toBytes(&F!);
            }
            return MPIN.BAD_PIN;
        }

        return 0
    }
    /* Pollards kangaroos used to return PIN error */
    static public func KANGAROO(E:[UInt8],_ F:[UInt8]) -> Int
    {
        let ge=FP12.fromBytes(E)
        let gf=FP12.fromBytes(F)
        var distance=[Int]();
        let t=FP12(gf);
        var table=[FP12]()

        var s:Int=1
        for var m=0;m<Int(TS);m++
        {
            distance.append(s)
            table.append(FP12(t))
            s*=2
            t.usqr()

        }
        t.one()
        var dn:Int=0
        for var j=0;j<TRAP;j++
        {
            let i=Int(t.geta().geta().getA().lastbits(8))%TS
            t.mul(table[i])
            dn+=distance[i]
        }
        gf.copy(t); gf.conj()
        var steps=0; var dm:Int=0
        var res=0;
        while (dm-dn<Int(MAXPIN))
        {
            steps++;
            if steps>4*TRAP {break}
            let i=Int(ge.geta().geta().getA().lastbits(8))%TS
            ge.mul(table[i])
            dm+=distance[i]
            if (ge.equals(t))
            {
				res=dm-dn;
				break;
            }
            if (ge.equals(gf))
            {
				res=dn-dm
				break
            }

        }
        if steps>4*TRAP || dm-dn>=Int(MAXPIN) {res=0 }    // Trap Failed  - probable invalid token
        return res
    }
    /* Functions to support M-Pin Full */

    static public func PRECOMPUTE(TOKEN:[UInt8],_ CID:[UInt8],inout _ G1:[UInt8],inout _ G2:[UInt8]) -> Int
    {
        let T=ECP.fromBytes(TOKEN);
        if T.is_infinity() {return INVALID_POINT}

        let P=MPIN.mapit(CID)

        let Q=ECP2(FP2(BIG(ROM.CURVE_Pxa),BIG(ROM.CURVE_Pxb)),FP2(BIG(ROM.CURVE_Pya),BIG(ROM.CURVE_Pyb)))

        var g=PAIR.ate(Q,T)
        g=PAIR.fexp(g)
        g.toBytes(&G1)

        g=PAIR.ate(Q,P)
        g=PAIR.fexp(g)
        g.toBytes(&G2)

        return 0
    }

    /* calculate common key on client side */
    /* wCID = w.(A+AT) */
    static public func CLIENT_KEY(G1:[UInt8],_ G2:[UInt8],_ pin:Int32,_ R:[UInt8],_ X:[UInt8],_ wCID:[UInt8],inout _ CK:[UInt8]) -> Int
    {
        let H=HASH()
        var t=[UInt8](count:EFS,repeatedValue:0)

        let g1=FP12.fromBytes(G1)
        let g2=FP12.fromBytes(G2)
        let z=BIG.fromBytes(R)
        let x=BIG.fromBytes(X)

        var W=ECP.fromBytes(wCID)
        if W.is_infinity() {return INVALID_POINT}

        W=PAIR.G1mul(W,x)

        let f=FP2(BIG(ROM.CURVE_Fra),BIG(ROM.CURVE_Frb))
        let r=BIG(ROM.CURVE_Order)
        let q=BIG(ROM.Modulus)

        let m=BIG(q)
        m.mod(r)

        let a=BIG(z)
        a.mod(m)

        let b=BIG(z)
        b.div(m);

        g2.pinpow(pin,PBLEN);
        g1.mul(g2);

        var c=g1.trace()
        g2.copy(g1)
        g2.frob(f)
        let cp=g2.trace()
        g1.conj()
        g2.mul(g1)
        let cpm1=g2.trace()
        g2.mul(g1)
        let cpm2=g2.trace()

        c=c.xtr_pow2(cp,cpm1,cpm2,a,b)

        c.geta().getA().toBytes(&t)
        H.process_array(t)
        c.geta().getB().toBytes(&t)
        H.process_array(t)
        c.getb().getA().toBytes(&t)
        H.process_array(t)
        c.getb().getB().toBytes(&t)
        H.process_array(t);

        W.getX().toBytes(&t)
        H.process_array(t)
        W.getY().toBytes(&t)
        H.process_array(t)

        t=H.hash()
        for var i=0;i<MPIN.PAS;i++ {CK[i]=t[i]}

        return 0
    }
    /* calculate common key on server side */
    /* Z=r.A - no time permits involved */

    static public func SERVER_KEY(Z:[UInt8],_ SST:[UInt8],_ W:[UInt8],_ xID:[UInt8],_ xCID:[UInt8]?,inout _ SK:[UInt8]) -> Int
    {
        let H=HASH();
        var t=[UInt8](count:EFS,repeatedValue:0)

        let sQ=ECP2.fromBytes(SST)
        if sQ.is_infinity() {return INVALID_POINT}
        let R=ECP.fromBytes(Z)
        if R.is_infinity() {return INVALID_POINT}

        var U:ECP
        if xCID != nil
            {U=ECP.fromBytes(xCID!)}
        else
            {U=ECP.fromBytes(xID)}

        if U.is_infinity() {return INVALID_POINT}

        let w=BIG.fromBytes(W)
        U=PAIR.G1mul(U,w)
        var g=PAIR.ate(sQ,R)
        g=PAIR.fexp(g)

        let c=g.trace()
        c.geta().getA().toBytes(&t)
        H.process_array(t)
        c.geta().getB().toBytes(&t)
        H.process_array(t)
        c.getb().getA().toBytes(&t)
        H.process_array(t)
        c.getb().getB().toBytes(&t)
        H.process_array(t);

        U.getX().toBytes(&t)
        H.process_array(t)
        U.getY().toBytes(&t)
        H.process_array(t)

        t=H.hash()
        for var i=0;i<MPIN.PAS;i++ {SK[i]=t[i]}

        return 0
    }

    /* return time since epoch */
    static public func GET_TIME() -> Int32
    {
        let date=NSDate()
        return (Int32(date.timeIntervalSince1970))
    }

    /* Generate Y = H(epoch, xCID/xID) */
    static public func GET_Y(TimeValue:Int32,_ xCID:[UInt8],inout _ Y:[UInt8])
    {
        let h = MPIN.hashit(TimeValue,xCID)
        let y = BIG.fromBytes(h)
        let q=BIG(ROM.CURVE_Order)
        y.mod(q)
        y.toBytes(&Y)
    }
    /* One pass MPIN Client */
    static public func CLIENT(date:Int32,_ CLIENT_ID:[UInt8],_ RNG:RAND?,inout _ X:[UInt8],_ pin:Int32,_ TOKEN:[UInt8],inout _  SEC:[UInt8],inout _ xID:[UInt8]?,inout _ xCID:[UInt8]?,_ PERMIT:[UInt8],_ TimeValue:Int32,inout _ Y:[UInt8]) -> Int
    {
        var rtn=0

        rtn = MPIN.CLIENT_1(date,CLIENT_ID,RNG,&X,pin,TOKEN,&SEC,&xID,&xCID,PERMIT)

        if rtn != 0 {return rtn}

        if date==0 {MPIN.GET_Y(TimeValue,xID!,&Y)}
        else {MPIN.GET_Y(TimeValue,xCID!,&Y)}

        rtn = MPIN.CLIENT_2(X,Y,&SEC)
        if (rtn != 0) {return rtn}

        return 0
    }
    /* One pass MPIN Server */
    static public func SERVER(date:Int32,inout _ HID:[UInt8]?,inout _ HTID:[UInt8],inout _ Y:[UInt8],_ SST:[UInt8],_ xID:[UInt8]?,_ xCID:[UInt8],_ SEC:[UInt8],inout _ E:[UInt8]?,inout _ F:[UInt8]?,_ CID:[UInt8],_ TimeValue:Int32) -> Int
    {
        var rtn=0

        var pID:[UInt8]
        if date == 0
            {pID = xID!}
        else
            {pID = xCID}

        SERVER_1(date,CID,&HID,&HTID);

        GET_Y(TimeValue,pID,&Y);

        rtn = SERVER_2(date,HID,HTID,Y,SST,xID,xCID,SEC,&E,&F);
        if rtn != 0 {return rtn}

        return 0
    }

    static public func printBinary(array: [UInt8])
    {
        for var i=0;i<array.count;i++
        {
            let h=String(format:"%02x",array[i])
            print("\(h)", terminator: "")
        }
        print(" ");
    }
}
