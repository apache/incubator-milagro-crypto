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
//  ecdh.swift
//  
//
//  Created by Michael Scott on 30/06/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation

/* Elliptic Curve API high-level functions  */

final public class ECDH
{
    static let INVALID_PUBLIC_KEY:Int = -2
    static let ERROR:Int = -3
    static let INVALID:Int = -4
    static public let EFS=Int(ROM.MODBYTES);
    static public let EGS=Int(ROM.MODBYTES);
    static public let EAS=AES.KS;
    static public let EBS=AES.BS;

    /* Convert Integer to n-byte array */
    private static func inttoBytes(n: Int,_ len:Int) -> [UInt8]
    {
        var b=[UInt8](count:len,repeatedValue:0)
        var nn=n

        var i=len;
        while (nn>0 && i>0)
        {
            i--;
            b[i]=UInt8(nn&0xff);
            nn /= 256;
        }
        return b;
    }

    /* Key Derivation Functions */
    /* Input octet Z */
    /* Output key of length olen */
    static public func KDF1(Z: [UInt8],_ olen:Int) -> [UInt8]
    {
    /* NOTE: the parameter olen is the length of the output K in bytes */
        let H=HASH()
        let hlen=HASH.len
        var K=[UInt8](count:olen,repeatedValue:0)
        var B=[UInt8](count:hlen,repeatedValue:0)

        var k=0;

        var cthreshold=olen/hlen; if (olen%hlen) != 0 {cthreshold++}

        for var counter=0;counter<cthreshold;counter++
        {
            H.process_array(Z); if counter>0 {H.process_num(Int32(counter))}
            B=H.hash();
            if k+hlen>olen {for var i=0;i<olen%hlen;i++ {K[k++]=B[i]}}
            else {for var i=0;i<hlen;i++ {K[k++]=B[i]}}
        }
        return K;
    }

    static public func KDF2(Z:[UInt8],_ P:[UInt8],_ olen:Int) -> [UInt8]
    {
    /* NOTE: the parameter olen is the length of the output k in bytes */
        let H=HASH();
        let hlen=HASH.len;
        var K=[UInt8](count:olen,repeatedValue:0)
        var B=[UInt8](count:hlen,repeatedValue:0)

        var k=0;

        var cthreshold=olen/hlen; if (olen%hlen) != 0 {cthreshold++}

        for var counter=1;counter<=cthreshold;counter++
        {
            H.process_array(Z); H.process_num(Int32(counter)); H.process_array(P)
            B=H.hash();
            if k+hlen>olen {for var i=0;i<olen%hlen;i++ {K[k++]=B[i]}}
            else {for var i=0;i<hlen;i++ {K[k++]=B[i]}}
        }
        return K;
    }

    /* Password based Key Derivation Function */
    /* Input password p, salt s, and repeat count */
    /* Output key of length olen */
    static public func PBKDF2(Pass:[UInt8],_ Salt:[UInt8],_ rep:Int,_ olen:Int) -> [UInt8]
    {
        var d=olen/32;
        if (olen%32) != 0 {d++}
        var F=[UInt8](count:ECDH.EFS,repeatedValue:0)
        var U=[UInt8](count:ECDH.EFS,repeatedValue:0)
        var S=[UInt8](count:Salt.count+4,repeatedValue:0)

        var K=[UInt8](count:d*ECDH.EFS,repeatedValue:0)

        var opt=0;

        for var i=1;i<=d;i++
        {
            for var j=0;j<Salt.count;j++ {S[j]=Salt[j]}
            var N=ECDH.inttoBytes(i,4);
            for var j=0;j<4;j++ {S[Salt.count+j]=N[j]}

            ECDH.HMAC(S,Pass,&F);

            for var j=0;j<EFS;j++ {U[j]=F[j]}
            for var j=2;j<=rep;j++
            {
				ECDH.HMAC(U,Pass,&U);
                for var k=0;k<ECDH.EFS;k++ {F[k]^=U[k]}
            }
            for var j=0;j<EFS;j++ {K[opt++]=F[j]}
        }
        var key=[UInt8](count:olen,repeatedValue:0)
        for var i=0;i<olen;i++ {key[i]=K[i]}
        return key;
    }

    /* Calculate HMAC of m using key k. HMAC is tag of length olen */
    static public func HMAC(M:[UInt8],_ K:[UInt8],inout _ tag:[UInt8]) -> Int
    {
    /* Input is from an octet m        *
    * olen is requested output length in bytes. k is the key  *
    * The output is the calculated tag */
        var K0=[UInt8](count:64,repeatedValue:0)
        let olen=tag.count;

        let b=K0.count;
        if olen<4 || olen>HASH.len {return 0}

        let H=HASH();

        if (K.count > b)
        {
            H.process_array(K); var B=H.hash();
            for var i=0;i<32;i++ {K0[i]=B[i]}
        }
        else
        {
            for var i=0;i<K.count;i++ {K0[i]=K[i]}
        }
        for var i=0;i<b;i++ {K0[i]^=0x36}
        H.process_array(K0); H.process_array(M); var B=H.hash();

        for var i=0;i<b;i++ {K0[i]^=0x6a}
        H.process_array(K0); H.process_array(B); B=H.hash();

        for var i=0;i<olen;i++ {tag[i]=B[i]}

        return 1;
    }
    /* AES encryption/decryption. Encrypt byte array M using key K and returns ciphertext */
    static public func AES_CBC_IV0_ENCRYPT(K:[UInt8],_ M:[UInt8]) -> [UInt8]
    { /* AES CBC encryption, with Null IV and key K */
    /* Input is from an octet string M, output is to an octet string C */
    /* Input is padded as necessary to make up a full final block */
        let a=AES();
        var buff=[UInt8](count:16,repeatedValue:0)
        let clen=16+(M.count/16)*16;

        var C=[UInt8](count:clen,repeatedValue:0)

        a.init_it(AES.CBC,K,nil)

        var ipt=0; var opt=0;
        var fin=false;
        var i:Int=0
        while true
        {
            for i=0;i<16;i++
            {
                if (ipt<M.count) {buff[i]=M[ipt++]}
				else {fin=true; break;}
            }
            if fin {break}
            a.encrypt(&buff);
            for var i=0;i<16;i++
                {C[opt++]=buff[i]}
        }

    /* last block, filled up to i-th index */

        let padlen=16-i;
        for var j=i;j<16;j++ {buff[j]=UInt8(padlen&0xff)}

        a.encrypt(&buff);

        for var i=0;i<16;i++
            {C[opt++]=buff[i]}
        a.end();
        return C;
    }

    /* returns plaintext if all consistent, else returns null string */
    static public func AES_CBC_IV0_DECRYPT(K:[UInt8],_ C:[UInt8]) -> [UInt8]
    { /* padding is removed */
        let a=AES();

        var buff=[UInt8](count:16,repeatedValue:0)
        var MM=[UInt8](count:C.count,repeatedValue:0)

        var ipt=0; var opt=0;

        a.init_it(AES.CBC,K,nil);

        if C.count==0 {return [UInt8]()}
        var ch=C[ipt++];

        var fin=false;
        var i:Int=0
        while true
        {
            for i=0;i<16;i++
            {
				buff[i]=ch;
				if ipt>=C.count {fin=true; break;}
                else {ch=C[ipt++]}
            }
            a.decrypt(&buff);
            if fin {break}
            for var i=0;i<16;i++
                {MM[opt++]=buff[i]}
        }

        a.end();
        var bad=false;
        let padlen:Int=Int(buff[15]);
        if i != 15 || padlen<1 || padlen>16 {bad=true}
        if padlen>=2 && padlen<=16
        {
            for var i=16-padlen;i<16;i++ {if buff[i] != buff[15] {bad=true}}
        }
        if !bad
        {
            for var i=0;i<16-padlen;i++
                {MM[opt++]=buff[i]}
        }

        if bad {return [UInt8]()}

        var M=[UInt8](count:opt,repeatedValue:0)
        for var i=0;i<opt;i++ {M[i]=MM[i]}

        return M;
    }

    /* Calculate a public/private EC GF(p) key pair W,S where W=S.G mod EC(p),
    * where S is the secret key and W is the public key
    * and G is fixed generator.
    * If RNG is NULL then the private key is provided externally in S
    * otherwise it is generated randomly internally */
    static public func KEY_PAIR_GENERATE(RNG:RAND?,inout _ S:[UInt8],inout _ W:[UInt8]) -> Int
    {
        let res=0;
        var T=[UInt8](count:ECDH.EFS,repeatedValue:0)
        let gx=BIG(ROM.CURVE_Gx);
        var s:BIG
        var G:ECP
        if ROM.CURVETYPE != ROM.MONTGOMERY
        {
            let gy=BIG(ROM.CURVE_Gy)
            G=ECP(gx,gy)
        }
        else
            {G=ECP(gx)}

        let r=BIG(ROM.CURVE_Order);

        if (RNG==nil)
        {
            s=BIG.fromBytes(S);
        }
        else
        {
            s=BIG.randomnum(r,RNG!)

            s.toBytes(&T)
            for var i=0;i<EGS;i++ {S[i]=T[i]}
        }

        let WP=G.mul(s)
        WP.toBytes(&W)

        return res;
    }

    /* validate public key. Set full=true for fuller check */
    static public func PUBLIC_KEY_VALIDATE(full:Bool,_ W:[UInt8]) -> Int
    {
        var WP=ECP.fromBytes(W);
        var res=0;

        let r=BIG(ROM.CURVE_Order)

        if WP.is_infinity() {res=INVALID_PUBLIC_KEY}

        if res==0 && full
        {
            WP=WP.mul(r)
            if !WP.is_infinity() {res=INVALID_PUBLIC_KEY}
        }
        return res;
    }
    /* IEEE-1363 Diffie-Hellman online calculation Z=S.WD */
    static public func ECPSVDP_DH(S:[UInt8],_ WD:[UInt8],inout _ Z:[UInt8]) -> Int
    {
        var res=0
        var T=[UInt8](count:ECDH.EFS,repeatedValue:0)

        let s=BIG.fromBytes(S)

        var W=ECP.fromBytes(WD)
        if W.is_infinity() {res=ECDH.ERROR}

        if (res==0)
        {
            let r=BIG(ROM.CURVE_Order)
            s.mod(r)

            W=W.mul(s);
            if W.is_infinity() {res=ERROR}
            else
            {
				W.getX().toBytes(&T);
                for var i=0;i<ECDH.EFS;i++ {Z[i]=T[i]}
            }
        }
        return res;
    }
    /* IEEE ECDSA Signature, C and D are signature on F using private key S */
    static public func ECPSP_DSA(RNG:RAND,_ S:[UInt8],_ F:[UInt8],inout _ C:[UInt8],inout _ D:[UInt8]) -> Int
    {
        var T=[UInt8](count:ECDH.EFS,repeatedValue:0)
        let H=HASH()
        H.process_array(F)
        let B=H.hash()

        let gx=BIG(ROM.CURVE_Gx)
        let gy=BIG(ROM.CURVE_Gy)

        let G=ECP(gx,gy)
        let r=BIG(ROM.CURVE_Order)

        let s=BIG.fromBytes(S)
        let f=BIG.fromBytes(B)

        let c=BIG(0)
        let d=BIG(0)
        var V=ECP()

        repeat {
            let u=BIG.randomnum(r,RNG);

            V.copy(G)
            V=V.mul(u)
            let vx=V.getX()
            c.copy(vx)
            c.mod(r)
            if c.iszilch() {continue}
            u.invmodp(r)
            d.copy(BIG.modmul(s,c,r))
            d.add(f)
            d.copy(BIG.modmul(u,d,r))
        } while d.iszilch()

        c.toBytes(&T)
        for var i=0;i<ECDH.EFS;i++ {C[i]=T[i]}
        d.toBytes(&T)
        for var i=0;i<ECDH.EFS;i++ {D[i]=T[i]}
        return 0;
    }

    /* IEEE1363 ECDSA Signature Verification. Signature C and D on F is verified using public key W */
    static public func ECPVP_DSA(W:[UInt8],_ F:[UInt8],_ C:[UInt8],_ D:[UInt8]) -> Int
    {
        var res=0

        let H=HASH()
        H.process_array(F)
        let B=H.hash()

        let gx=BIG(ROM.CURVE_Gx)
        let gy=BIG(ROM.CURVE_Gy)

        let G=ECP(gx,gy)
        let r=BIG(ROM.CURVE_Order)

        let c=BIG.fromBytes(C)
        var d=BIG.fromBytes(D)
        let f=BIG.fromBytes(B)

        if c.iszilch() || BIG.comp(c,r)>=0 || d.iszilch() || BIG.comp(d,r)>=0
            {res=ECDH.INVALID}

        if res==0
        {
            d.invmodp(r);
            f.copy(BIG.modmul(f,d,r))
            let h2=BIG.modmul(c,d,r)

            let WP=ECP.fromBytes(W)
            if WP.is_infinity() {res=ECDH.ERROR}
            else
            {
				var P=ECP();
				P.copy(WP);
				P=P.mul2(h2,G,f);
                if P.is_infinity() {res=INVALID}
				else
				{
                    d=P.getX();
                    d.mod(r);
                    if (BIG.comp(d,c) != 0) {res=ECDH.INVALID}
				}
            }
        }

        return res;
    }

    /* IEEE1363 ECIES encryption. Encryption of plaintext M uses public key W and produces ciphertext V,C,T */
    static public func ECIES_ENCRYPT(P1:[UInt8],_ P2:[UInt8],_ RNG:RAND,_ W:[UInt8],_ M:[UInt8],inout _ V:[UInt8],inout _ T:[UInt8]) -> [UInt8]
    {
        var Z=[UInt8](count:ECDH.EFS,repeatedValue:0)
        var VZ=[UInt8](count:3*ECDH.EFS+1,repeatedValue:0)
        var K1=[UInt8](count:ECDH.EAS,repeatedValue:0)
        var K2=[UInt8](count:ECDH.EAS,repeatedValue:0)
        var U=[UInt8](count:ECDH.EGS,repeatedValue:0)

        if ECDH.KEY_PAIR_GENERATE(RNG,&U,&V) != 0 {return [UInt8]()}
        if ECDH.ECPSVDP_DH(U,W,&Z) != 0 {return [UInt8]()}

        for var i=0;i<2*ECDH.EFS+1;i++ {VZ[i]=V[i]}
        for var i=0;i<ECDH.EFS;i++ {VZ[2*ECDH.EFS+1+i]=Z[i]}


        var K=KDF2(VZ,P1,ECDH.EFS)

        for var i=0;i<ECDH.EAS;i++ {K1[i]=K[i]; K2[i]=K[EAS+i];}

        var C=AES_CBC_IV0_ENCRYPT(K1,M)

        var L2=inttoBytes(P2.count,8)

        var AC=[UInt8](count:C.count+P2.count+8,repeatedValue:0)

        for var i=0;i<C.count;i++ {AC[i]=C[i]}
        for var i=0;i<P2.count;i++ {AC[C.count+i]=P2[i]}
        for var i=0;i<8;i++ {AC[C.count+P2.count+i]=L2[i]}

        ECDH.HMAC(AC,K2,&T)

        return C
    }

    /* IEEE1363 ECIES decryption. Decryption of ciphertext V,C,T using private key U outputs plaintext M */
    static public func ECIES_DECRYPT(P1:[UInt8],_ P2:[UInt8],_ V:[UInt8],_ C:[UInt8],_ T:[UInt8],_ U:[UInt8]) -> [UInt8]
    {
        var Z=[UInt8](count:ECDH.EFS,repeatedValue:0)
        var VZ=[UInt8](count:3*ECDH.EFS+1,repeatedValue:0)
        var K1=[UInt8](count:ECDH.EAS,repeatedValue:0)
        var K2=[UInt8](count:ECDH.EAS,repeatedValue:0)

        var TAG=[UInt8](count:T.count,repeatedValue:0)

        if ECPSVDP_DH(U,V,&Z) != 0 {return [UInt8]()}

        for var i=0;i<2*ECDH.EFS+1;i++ {VZ[i]=V[i]}
        for var i=0;i<ECDH.EFS;i++ {VZ[2*EFS+1+i]=Z[i]}

        var K=KDF2(VZ,P1,ECDH.EFS)

        for var i=0;i<ECDH.EAS;i++ {K1[i]=K[i]; K2[i]=K[ECDH.EAS+i]}

        let M=ECDH.AES_CBC_IV0_DECRYPT(K1,C)

        if M.count==0 {return M}

        var L2=inttoBytes(P2.count,8)

        var AC=[UInt8](count:C.count+P2.count+8,repeatedValue:0)

        for var i=0;i<C.count;i++ {AC[i]=C[i]}
        for var i=0;i<P2.count;i++ {AC[C.count+i]=P2[i]}
        for var i=0;i<8;i++ {AC[C.count+P2.count+i]=L2[i]}

        ECDH.HMAC(AC,K2,&TAG)

        var same=true
        for var i=0;i<T.count;i++
        {
            if T[i] != TAG[i] {same=false}
        }
        if !same {return [UInt8]()}

        return M;

    }

    static public func printBinary(array: [UInt8])
    {
        for var i=0;i<array.count;i++
        {
            let h=String(array[i],radix:16);
            print("\(h)", terminator: "")
        }
        print("");
    }

}
