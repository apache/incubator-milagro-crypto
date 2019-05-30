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


use super::fp::FP;
use super::ecp::ECP;
use super::fp2::FP2;
use super::ecp2::ECP2;
use super::fp4::FP4;
use super::fp12;
use super::fp12::FP12;
use super::big::BIG;
use super::dbig::DBIG;
use super::ecp;
use super::rom;
use types::{SexticTwist, CurvePairingType, SignOfX};

#[allow(non_snake_case)]
fn linedbl(A: &mut ECP2, qx: &FP, qy: &FP) -> FP12 {
    let mut a = FP4::new();
    let mut b = FP4::new();
    let mut c = FP4::new();

    let mut xx = FP2::new_copy(&A.getpx()); //X
    let mut yy = FP2::new_copy(&A.getpy()); //Y
    let mut zz = FP2::new_copy(&A.getpz()); //Z
    let mut yz = FP2::new_copy(&yy); //Y
    yz.mul(&zz); //YZ
    xx.sqr(); //X^2
    yy.sqr(); //Y^2
    zz.sqr(); //Z^2

    yz.imul(4);
    yz.neg();
    yz.norm(); //-2YZ
    yz.pmul(qy); //-2YZ.Ys

    xx.imul(6); //3X^2
    xx.pmul(qx); //3X^2.Xs

    let sb = 3 * rom::CURVE_B_I;
    zz.imul(sb);
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        zz.div_ip2();
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        zz.mul_ip();
        zz.dbl();
        yz.mul_ip();
        yz.norm();
    }

    zz.norm(); // 3b.Z^2

    yy.dbl();
    zz.sub(&yy);
    zz.norm(); // 3b.Z^2-Y^2

    a.copy(&FP4::new_fp2s(&yz, &zz)); // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP4::new_fp2(&xx)); // L(0,1) | L(0,0) | L(1,0)
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP4::new_fp2(&xx));
        c.times_i();
    }
    A.dbl();
    let mut res= FP12::new_fp4s(&a, &b, &c);
    res.settype(fp12::SPARSER);
    return res;
}

#[allow(non_snake_case)]
fn lineadd(A: &mut ECP2, B: &ECP2, qx: &FP, qy: &FP) -> FP12 {
    let mut a = FP4::new();
    let mut b = FP4::new();
    let mut c = FP4::new();

    let mut x1 = FP2::new_copy(&A.getpx()); // X1
    let mut y1 = FP2::new_copy(&A.getpy()); // Y1
    let mut t1 = FP2::new_copy(&A.getpz()); // Z1
    let mut t2 = FP2::new_copy(&A.getpz()); // Z1

    t1.mul(&B.getpy()); // T1=Z1.Y2
    t2.mul(&B.getpx()); // T2=Z1.X2

    x1.sub(&t2);
    x1.norm(); // X1=X1-Z1.X2
    y1.sub(&t1);
    y1.norm(); // Y1=Y1-Z1.Y2

    t1.copy(&x1); // T1=X1-Z1.X2
    x1.pmul(qy); // X1=(X1-Z1.X2).Ys
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        x1.mul_ip();
        x1.norm();
    }

    t1.mul(&B.getpy()); // T1=(X1-Z1.X2).Y2

    t2.copy(&y1); // T2=Y1-Z1.Y2
    t2.mul(&B.getpx()); // T2=(Y1-Z1.Y2).X2
    t2.sub(&t1);
    t2.norm(); // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
    y1.pmul(qx);
    y1.neg();
    y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

    a.copy(&FP4::new_fp2s(&x1, &t2)); // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP4::new_fp2(&y1));
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP4::new_fp2(&y1));
        c.times_i();
    }

    A.add(B);
    let mut res= FP12::new_fp4s(&a, &b, &c);
    res.settype(fp12::SPARSER);
    return res;
}

/* prepare ate parameter, n=6u+2 (BN) or n=u (BLS), n3=3*n */
#[allow(non_snake_case)]
fn lbits(n3: &mut BIG,n: &mut BIG) -> usize {
    n.copy(&BIG::new_ints(&rom::CURVE_BNX));
    if ecp::CURVE_PAIRING_TYPE==CurvePairingType::BN {
        n.pmul(6);
        if ecp::SIGN_OF_X==SignOfX::POSITIVEX {
            n.inc(2);
        } else {
            n.dec(2);
        }
    }
    n.norm();
    n3.copy(&n);
    n3.pmul(3);
    n3.norm();
    return n3.nbits();
}

/* prepare for multi-pairing */
pub fn initmp() -> [FP12; rom::ATE_BITS] {
    let r: [FP12; rom::ATE_BITS] = [FP12::new_int(1); rom::ATE_BITS];
    return r
}

/* basic Miller loop */
pub fn miller(r:&[FP12]) -> FP12 {
    let mut res=FP12::new_int(1);
    for i in (1..rom::ATE_BITS).rev() {
        res.sqr();
        res.ssmul(&r[i]);
    }

    if ecp::SIGN_OF_X==SignOfX::NEGATIVEX {
        res.conj();
    }
    res.ssmul(&r[0]);
    return res;
}

/* Accumulate another set of line functions for n-pairing */
#[allow(non_snake_case)]
pub fn another(r:&mut [FP12],P1: &ECP2,Q1: &ECP) {
    let mut f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut n = BIG::new();
    let mut n3 = BIG::new();
    let mut K = ECP2::new();


// P is needed in affine form for line function, Q for (Qx,Qy) extraction
    let mut P = ECP2::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();

    if ecp::CURVE_PAIRING_TYPE==CurvePairingType::BN {
        if ecp::SEXTIC_TWIST==SexticTwist::M_TYPE {
            f.inverse();
            f.norm();
        }
    }

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());
    let mut A = ECP2::new();

    A.copy(&P);
    let mut NP = ECP2::new();
    NP.copy(&P);
    NP.neg();

    let nb=lbits(&mut n3,&mut n);

    for i in (1..nb-1).rev() {
        let mut lv=linedbl(&mut A,&qx,&qy);

	let bt=n3.bit(i)-n.bit(i);
        if bt==1 {
            let lv2=lineadd(&mut A,&P,&qx,&qy);
            lv.smul(&lv2);
        }
        if bt==-1 {
            let lv2=lineadd(&mut A,&NP,&qx,&qy);
            lv.smul(&lv2);
        }
        r[i].ssmul(&lv);
    }

/* R-ate fixup required for BN curves */
    if ecp::CURVE_PAIRING_TYPE==CurvePairingType::BN {
        if ecp::SIGN_OF_X==SignOfX::NEGATIVEX {
            A.neg();
        }
        K.copy(&P);
        K.frob(&f);
        let mut lv=lineadd(&mut A,&K,&qx,&qy);
        K.frob(&f);
        K.neg();
        let lv2=lineadd(&mut A,&K,&qx,&qy);
        lv.smul(&lv2);
	r[0].ssmul(&lv);
    } 
}

#[allow(non_snake_case)]
/* Optimal R-ate pairing */
pub fn ate(P1: &ECP2, Q1: &ECP) -> FP12 {
    let mut f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut n = BIG::new();
    let mut n3 = BIG::new();
    let mut K = ECP2::new();

    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
            f.inverse();
            f.norm();
        }
    } 
    let mut P = ECP2::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let mut A = ECP2::new();
    let mut r = FP12::new_int(1);

    A.copy(&P);
    let mut NP = ECP2::new();
    NP.copy(&P);
    NP.neg();

    let nb=lbits(&mut n3,&mut n);

    for i in (1..nb - 1).rev() {
        r.sqr();
        let mut lv = linedbl(&mut A, &qx, &qy);
        let bt = n3.bit(i) - n.bit(i);
        if bt == 1 {
            let lv2 = lineadd(&mut A, &P, &qx, &qy);
            lv.smul(&lv2);
        }
        if bt == -1 {
            let lv2 = lineadd(&mut A, &NP, &qx, &qy);
            lv.smul(&lv2);
        }
        r.ssmul(&lv);
    }

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        r.conj();
    }

    /* R-ate fixup required for BN curves */

    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            A.neg();
        }

        K.copy(&P);
        K.frob(&f);

        let mut lv = lineadd(&mut A, &K, &qx, &qy);
        K.frob(&f);
        K.neg();
        let lv2 = lineadd(&mut A, &K, &qx, &qy);
	lv.smul(&lv2);
        r.ssmul(&lv);
    }

    return r;
}

#[allow(non_snake_case)]
/* Optimal R-ate double pairing e(P,Q).e(R,S) */
pub fn ate2(P1: &ECP2, Q1: &ECP, R1: &ECP2, S1: &ECP) -> FP12 {
    let mut f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut n = BIG::new();
    let mut n3 = BIG::new();
    let mut K = ECP2::new();

    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
            f.inverse();
            f.norm();
        }
    } 

    let mut P = ECP2::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();
    let mut R = ECP2::new();
    R.copy(R1);
    R.affine();
    let mut S = ECP::new();
    S.copy(S1);
    S.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let sx = FP::new_copy(&S.getpx());
    let sy = FP::new_copy(&S.getpy());

    let mut A = ECP2::new();
    let mut B = ECP2::new();
    let mut r = FP12::new_int(1);

    A.copy(&P);
    B.copy(&R);

    let mut NP = ECP2::new();
    NP.copy(&P);
    NP.neg();
    let mut NR = ECP2::new();
    NR.copy(&R);
    NR.neg();

    let nb=lbits(&mut n3,&mut n);

    for i in (1..nb - 1).rev() {
        r.sqr();
        let mut lv = linedbl(&mut A, &qx, &qy);
        let lv2 = linedbl(&mut B, &sx, &sy);
	lv.smul(&lv2);
        r.ssmul(&lv);
        let bt = n3.bit(i) - n.bit(i);
        if bt == 1 {
            lv = lineadd(&mut A, &P, &qx, &qy);
            let lv2 = lineadd(&mut B, &R, &sx, &sy);
	    lv.smul(&lv2);
            r.ssmul(&lv);
        }
        if bt == -1 {
            lv = lineadd(&mut A, &NP, &qx, &qy);
            let lv2 = lineadd(&mut B, &NR, &sx, &sy);
	    lv.smul(&lv2);
            r.ssmul(&lv);
        }
    }

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        r.conj();
    }

    /* R-ate fixup */
    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            A.neg();
            B.neg();
        }
        K.copy(&P);
        K.frob(&f);

        let mut lv = lineadd(&mut A, &K, &qx, &qy);
        K.frob(&f);
        K.neg();
        let mut lv2 = lineadd(&mut A, &K, &qx, &qy);
	lv.smul(&lv2);
        r.ssmul(&lv);

        K.copy(&R);
        K.frob(&f);

        lv = lineadd(&mut B, &K, &sx, &sy);
        K.frob(&f);
        K.neg();
        lv2 = lineadd(&mut B, &K, &sx, &sy);
	lv.smul(&lv2);
        r.ssmul(&lv);

    }

    return r;
}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
pub fn fexp(m: &FP12) -> FP12 {
    let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut x = BIG::new_ints(&rom::CURVE_BNX);
    let mut r = FP12::new_copy(m);

    /* Easy part of final exp */
    let mut lv = FP12::new_copy(&r);
    lv.inverse();
    r.conj();

    r.mul(&lv);
    lv.copy(&r);
    r.frob(&f);
    r.frob(&f);
    r.mul(&lv);
    if r.isunity() {
	r.zero();
	return r;
    }

    /* Hard part of final exp */
    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        lv.copy(&r);
        lv.frob(&f);
        let mut x0 = FP12::new_copy(&lv);
        x0.frob(&f);
        lv.mul(&r);
        x0.mul(&lv);
        x0.frob(&f);
        let mut x1 = FP12::new_copy(&r);
        x1.conj();
        let mut x4 = r.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::POSITIVEX {
            x4.conj();
        }

        let mut x3 = FP12::new_copy(&x4);
        x3.frob(&f);

        let mut x2 = x4.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::POSITIVEX {
            x2.conj();
        }
        let mut x5 = FP12::new_copy(&x2);
        x5.conj();
        lv = x2.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::POSITIVEX {
            lv.conj();
        }
        x2.frob(&f);
        r.copy(&x2);
        r.conj();

        x4.mul(&r);
        x2.frob(&f);

        r.copy(&lv);
        r.frob(&f);
        lv.mul(&r);

        lv.usqr();
        lv.mul(&x4);
        lv.mul(&x5);
        r.copy(&x3);
        r.mul(&x5);
        r.mul(&lv);
        lv.mul(&x2);
        r.usqr();
        r.mul(&lv);
        r.usqr();
        lv.copy(&r);
        lv.mul(&x1);
        r.mul(&x0);
        lv.usqr();
        r.mul(&lv);
        r.reduce();
    } else {
        // Ghamman & Fouotsa Method

        let mut y0 = FP12::new_copy(&r);
        y0.usqr();
        let mut y1 = y0.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            y1.conj();
        }
        x.fshr(1);
        let mut y2 = y1.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            y2.conj();
        }
        x.fshl(1);
        let mut y3 = FP12::new_copy(&r);
        y3.conj();
        y1.mul(&y3);

        y1.conj();
        y1.mul(&y2);

        y2 = y1.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            y2.conj();
        }
        y3 = y2.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            y3.conj();
        }
        y1.conj();
        y3.mul(&y1);

        y1.conj();
        y1.frob(&f);
        y1.frob(&f);
        y1.frob(&f);
        y2.frob(&f);
        y2.frob(&f);
        y1.mul(&y2);

        y2 = y3.pow(&mut x);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            y2.conj();
        }
        y2.mul(&y0);
        y2.mul(&r);

        y1.mul(&y2);
        y2.copy(&y3);
        y2.frob(&f);
        y1.mul(&y2);
        r.copy(&y1);
        r.reduce();
    }
    return r;
}

#[allow(non_snake_case)]
/* GLV method */
fn glv(e: &BIG) -> [BIG; 2] {
    let mut u: [BIG; 2] = [BIG::new(), BIG::new()];
    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        let mut t = BIG::new();
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut v: [BIG; 2] = [BIG::new(), BIG::new()];

        for i in 0..2 {
            t.copy(&BIG::new_ints(&rom::CURVE_W[i])); // why not just t=new BIG(ROM.CURVE_W[i]);
            let mut d: DBIG = BIG::mul(&t, e);
            v[i].copy(&d.div(&q));
        }
        u[0].copy(&e);
        for i in 0..2 {
            for j in 0..2 {
                t = BIG::new_ints(&rom::CURVE_SB[j][i]);
                t = BIG::modmul(&mut v[j], &mut t, &q);
                u[i].add(&q);
                u[i].sub(&t);
                u[i].rmod(&q);
            }
        }
    } else {
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let x = BIG::new_ints(&rom::CURVE_BNX);
        let x2 = BIG::smul(&x, &x);
        u[0].copy(&e);
        u[0].rmod(&x2);
        u[1].copy(&e);
        u[1].div(&x2);
        u[1].rsub(&q);
    }
    return u;
}

#[allow(non_snake_case)]
/* Galbraith & Scott Method */
pub fn gs(e: &BIG) -> [BIG; 4] {
    let mut u: [BIG; 4] = [BIG::new(), BIG::new(), BIG::new(), BIG::new()];
    if ecp::CURVE_PAIRING_TYPE == CurvePairingType::BN {
        let mut t = BIG::new();
        let q = BIG::new_ints(&rom::CURVE_ORDER);

        let mut v: [BIG; 4] = [BIG::new(), BIG::new(), BIG::new(), BIG::new()];
        for i in 0..4 {
            t.copy(&BIG::new_ints(&rom::CURVE_WB[i]));
            let mut d: DBIG = BIG::mul(&t, e);
            v[i].copy(&d.div(&q));
        }
        u[0].copy(&e);
        for i in 0..4 {
            for j in 0..4 {
                t = BIG::new_ints(&rom::CURVE_BB[j][i]);
                t = BIG::modmul(&mut v[j], &mut t, &q);
                u[i].add(&q);
                u[i].sub(&t);
                u[i].rmod(&q);
            }
        }
    } else {
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let x = BIG::new_ints(&rom::CURVE_BNX);
        let mut w = BIG::new_copy(&e);
        for i in 0..3 {
            u[i].copy(&w);
            u[i].rmod(&x);
            w.div(&x);
        }
        u[3].copy(&w);
        if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
            let mut t = BIG::new();
            t.copy(&BIG::modneg(&mut u[1], &q));
            u[1].copy(&t);
            t.copy(&BIG::modneg(&mut u[3], &q));
            u[3].copy(&t);
        }
    }
    return u;
}

#[allow(non_snake_case)]
/* Multiply P by e in group G1 */
pub fn g1mul(P: &ECP, e: &mut BIG) -> ECP {
    let mut R = ECP::new();
    if rom::USE_GLV {
        R.copy(P);
        let mut Q = ECP::new();
        Q.copy(P);
        Q.affine();
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut cru = FP::new_big(&BIG::new_ints(&rom::CURVE_CRU));
        let mut u = glv(e);
        Q.mulx(&mut cru);

        let mut np = u[0].nbits();
        let mut t: BIG = BIG::modneg(&mut u[0], &q);
        let mut nn = t.nbits();
        if nn < np {
            u[0].copy(&t);
            R.neg();
        }

        np = u[1].nbits();
        t = BIG::modneg(&mut u[1], &q);
        nn = t.nbits();
        if nn < np {
            u[1].copy(&t);
            Q.neg();
        }
        u[0].norm();
        u[1].norm();
        R = R.mul2(&u[0], &mut Q, &u[1]);
    } else {
        R = P.mul(e);
    }
    return R;
}

#[allow(non_snake_case)]
/* Multiply P by e in group G2 */
pub fn g2mul(P: &ECP2, e: &BIG) -> ECP2 {
    let mut R = ECP2::new();
    if rom::USE_GS_G2 {
        let mut Q: [ECP2; 4] = [ECP2::new(), ECP2::new(), ECP2::new(), ECP2::new()];
        let mut f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut u = gs(e);
        let mut T = ECP2::new();

        if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
            f.inverse();
            f.norm();
        }

        let mut t = BIG::new();
        Q[0].copy(&P);
        for i in 1..4 {
            T.copy(&Q[i - 1]);
            Q[i].copy(&T);
            Q[i].frob(&f);
        }
        for i in 0..4 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                Q[i].neg();
            }
            u[i].norm();
        }

        R.copy(&ECP2::mul4(&mut Q, &u));
    } else {
        R.copy(&P.mul(e));
    }
    return R;
}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java */
pub fn gtpow(d: &FP12, e: &BIG) -> FP12 {
    let mut r = FP12::new();
    if rom::USE_GS_GT {
        let mut g: [FP12; 4] = [FP12::new(), FP12::new(), FP12::new(), FP12::new()];
        let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut t = BIG::new();
        let mut u = gs(e);
        let mut w = FP12::new();

        g[0].copy(&d);
        for i in 1..4 {
            w.copy(&g[i - 1]);
            g[i].copy(&w);
            g[i].frob(&f);
        }
        for i in 0..4 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                g[i].conj();
            }
            u[i].norm();
        }
        r.copy(&FP12::pow4(&mut g, &u));
    } else {
        r.copy(&d.pow(e));
    }
    return r;
}
