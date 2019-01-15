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
use super::ecp4::ECP4;
use super::fp4::FP4;
use super::fp8::FP8;
use super::fp24::FP24;
use super::big::BIG;
use super::ecp;
use super::rom;
use types::{SexticTwist, SignOfX};

#[allow(non_snake_case)]
fn linedbl(A: &mut ECP4, qx: &FP, qy: &FP) -> FP24 {
    let mut a = FP8::new();
    let mut b = FP8::new();
    let mut c = FP8::new();

    let mut xx = FP4::new_copy(&A.getpx()); //X
    let mut yy = FP4::new_copy(&A.getpy()); //Y
    let mut zz = FP4::new_copy(&A.getpz()); //Z
    let mut yz = FP4::new_copy(&yy); //Y
    yz.mul(&zz); //YZ
    xx.sqr(); //X^2
    yy.sqr(); //Y^2
    zz.sqr(); //Z^2

    yz.imul(4);
    yz.neg();
    yz.norm(); //-2YZ
    yz.qmul(qy); //-2YZ.Ys

    xx.imul(6); //3X^2
    xx.qmul(qx); //3X^2.Xs

    let sb = 3 * rom::CURVE_B_I;
    zz.imul(sb);
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        zz.div_2i();
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        zz.times_i();
        zz.dbl();
        yz.times_i();
    }

    zz.norm(); // 3b.Z^2

    yy.dbl();
    zz.sub(&yy);
    zz.norm(); // 3b.Z^2-Y^2

    a.copy(&FP8::new_fp4s(&yz, &zz)); // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP8::new_fp4(&xx)); // L(0,1) | L(0,0) | L(1,0)
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP8::new_fp4(&xx));
        c.times_i();
    }
    A.dbl();
    return FP24::new_fp8s(&a, &b, &c);
}

#[allow(non_snake_case)]
fn lineadd(A: &mut ECP4, B: &ECP4, qx: &FP, qy: &FP) -> FP24 {
    let mut a = FP8::new();
    let mut b = FP8::new();
    let mut c = FP8::new();

    let mut x1 = FP4::new_copy(&A.getpx()); // X1
    let mut y1 = FP4::new_copy(&A.getpy()); // Y1
    let mut t1 = FP4::new_copy(&A.getpz()); // Z1
    let mut t2 = FP4::new_copy(&A.getpz()); // Z1

    t1.mul(&B.getpy()); // T1=Z1.Y2
    t2.mul(&B.getpx()); // T2=Z1.X2

    x1.sub(&t2);
    x1.norm(); // X1=X1-Z1.X2
    y1.sub(&t1);
    y1.norm(); // Y1=Y1-Z1.Y2

    t1.copy(&x1); // T1=X1-Z1.X2
    x1.qmul(qy); // X1=(X1-Z1.X2).Ys
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        x1.times_i();
    }

    t1.mul(&B.getpy()); // T1=(X1-Z1.X2).Y2

    t2.copy(&y1); // T2=Y1-Z1.Y2
    t2.mul(&B.getpx()); // T2=(Y1-Z1.Y2).X2
    t2.sub(&t1);
    t2.norm(); // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
    y1.qmul(qx);
    y1.neg();
    y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

    a.copy(&FP8::new_fp4s(&x1, &t2)); // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP8::new_fp4(&y1));
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP8::new_fp4(&y1));
        c.times_i();
    }

    A.add(B);
    return FP24::new_fp8s(&a, &b, &c);
}

#[allow(non_snake_case)]
/* Optimal R-ate pairing */
pub fn ate(P1: &ECP4, Q1: &ECP) -> FP24 {
    let x = BIG::new_ints(&rom::CURVE_BNX);
    let n = BIG::new_copy(&x);

    let mut n3 = BIG::new_copy(&n);
    n3.pmul(3);
    n3.norm();

    let mut P = ECP4::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let mut A = ECP4::new();
    let mut r = FP24::new_int(1);

    A.copy(&P);
    let mut NP = ECP4::new();
    NP.copy(&P);
    NP.neg();

    let nb = n3.nbits();

    for i in (1..nb - 1).rev() {
        r.sqr();
        let mut lv = linedbl(&mut A, &qx, &qy);
        r.smul(&lv, ecp::SEXTIC_TWIST.into());
        let bt = n3.bit(i) - n.bit(i);
        if bt == 1 {
            lv = lineadd(&mut A, &P, &qx, &qy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
        }
        if bt == -1 {
            lv = lineadd(&mut A, &NP, &qx, &qy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
        }
    }

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        r.conj();
    }

    return r;
}

#[allow(non_snake_case)]
/* Optimal R-ate double pairing e(P,Q).e(R,S) */
pub fn ate2(P1: &ECP4, Q1: &ECP, R1: &ECP4, S1: &ECP) -> FP24 {
    let x = BIG::new_ints(&rom::CURVE_BNX);
    let n = BIG::new_copy(&x);

    let mut n3 = BIG::new_copy(&n);
    n3.pmul(3);
    n3.norm();

    let mut P = ECP4::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();
    let mut R = ECP4::new();
    R.copy(R1);
    R.affine();
    let mut S = ECP::new();
    S.copy(S1);
    S.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let sx = FP::new_copy(&S.getpx());
    let sy = FP::new_copy(&S.getpy());

    let mut A = ECP4::new();
    let mut B = ECP4::new();
    let mut r = FP24::new_int(1);

    A.copy(&P);
    B.copy(&R);

    let mut NP = ECP4::new();
    NP.copy(&P);
    NP.neg();
    let mut NR = ECP4::new();
    NR.copy(&R);
    NR.neg();

    let nb = n3.nbits();

    for i in (1..nb - 1).rev() {
        r.sqr();
        let mut lv = linedbl(&mut A, &qx, &qy);
        r.smul(&lv, ecp::SEXTIC_TWIST.into());
        lv = linedbl(&mut B, &sx, &sy);
        r.smul(&lv, ecp::SEXTIC_TWIST.into());
        let bt = n3.bit(i) - n.bit(i);
        if bt == 1 {
            lv = lineadd(&mut A, &P, &qx, &qy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
            lv = lineadd(&mut B, &R, &sx, &sy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
        }
        if bt == -1 {
            lv = lineadd(&mut A, &NP, &qx, &qy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
            lv = lineadd(&mut B, &NR, &sx, &sy);
            r.smul(&lv, ecp::SEXTIC_TWIST.into());
        }
    }

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        r.conj();
    }

    return r;
}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
pub fn fexp(m: &FP24) -> FP24 {
    let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut x = BIG::new_ints(&rom::CURVE_BNX);
    let mut r = FP24::new_copy(m);

    /* Easy part of final exp */
    let mut lv = FP24::new_copy(&r);
    lv.inverse();
    r.conj();

    r.mul(&lv);
    lv.copy(&r);
    r.frob(&f, 4);
    r.mul(&lv);

    /* Hard part of final exp */
    // Ghamman & Fouotsa Method

    let mut t7 = FP24::new_copy(&r);
    t7.usqr();
    let mut t1 = t7.pow(&mut x);

    x.fshr(1);
    let mut t2 = t1.pow(&mut x);
    x.fshl(1);

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }
    let mut t3 = FP24::new_copy(&t1);
    t3.conj();
    t2.mul(&t3);
    t2.mul(&r);

    t3.copy(&t2.pow(&mut x));
    let mut t4 = t3.pow(&mut x);
    let mut t5 = t4.pow(&mut x);

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t3.conj();
        t5.conj();
    }

    t3.frob(&f, 6);
    t4.frob(&f, 5);
    t3.mul(&t4);

    let mut t6 = t5.pow(&mut x);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t6.conj();
    }

    t5.frob(&f, 4);
    t3.mul(&t5);

    let mut t0 = FP24::new_copy(&t2);
    t0.conj();
    t6.mul(&t0);

    t5.copy(&t6);
    t5.frob(&f, 3);

    t3.mul(&t5);
    t5.copy(&t6.pow(&mut x));
    t6.copy(&t5.pow(&mut x));

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t5.conj();
    }

    t0.copy(&t5);
    t0.frob(&f, 2);
    t3.mul(&t0);
    t0.copy(&t6);
    t0.frob(&f, 1);

    t3.mul(&t0);
    t5.copy(&t6.pow(&mut x));

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t5.conj();
    }
    t2.frob(&f, 7);

    t5.mul(&t7);
    t3.mul(&t2);
    t3.mul(&t5);

    r.mul(&t3);

    r.reduce();
    return r;
}

#[allow(non_snake_case)]
/* GLV method */
fn glv(e: &BIG) -> [BIG; 2] {
    let mut u: [BIG; 2] = [BIG::new(), BIG::new()];
    let q = BIG::new_ints(&rom::CURVE_ORDER);
    let mut x = BIG::new_ints(&rom::CURVE_BNX);
    let x2 = BIG::smul(&x, &x);
    x.copy(&BIG::smul(&x2, &x2));
    u[0].copy(&e);
    u[0].rmod(&x);
    u[1].copy(&e);
    u[1].div(&x);
    u[1].rsub(&q);

    return u;
}

#[allow(non_snake_case)]
/* Galbraith & Scott Method */
pub fn gs(e: &BIG) -> [BIG; 8] {
    let mut u: [BIG; 8] = [
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
    ];
    let q = BIG::new_ints(&rom::CURVE_ORDER);
    let x = BIG::new_ints(&rom::CURVE_BNX);
    let mut w = BIG::new_copy(&e);
    for i in 0..7 {
        u[i].copy(&w);
        u[i].rmod(&x);
        w.div(&x);
    }
    u[7].copy(&w);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        let mut t = BIG::new();
        t.copy(&BIG::modneg(&mut u[1], &q));
        u[1].copy(&t);
        t.copy(&BIG::modneg(&mut u[3], &q));
        u[3].copy(&t);
        t.copy(&BIG::modneg(&mut u[5], &q));
        u[5].copy(&t);
        t.copy(&BIG::modneg(&mut u[7], &q));
        u[7].copy(&t);
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
pub fn g2mul(P: &ECP4, e: &BIG) -> ECP4 {
    let mut R = ECP4::new();
    if rom::USE_GS_G2 {
        let mut Q: [ECP4; 8] = [
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
            ECP4::new(),
        ];
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut u = gs(e);
        let mut T = ECP4::new();

        let f = ECP4::frob_constants();

        let mut t = BIG::new();
        Q[0].copy(&P);
        for i in 1..8 {
            T.copy(&Q[i - 1]);
            Q[i].copy(&T);
            Q[i].frob(&f, 1);
        }
        for i in 0..8 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                Q[i].neg();
            }
            u[i].norm();
        }

        R.copy(&ECP4::mul8(&mut Q, &u));
    } else {
        R.copy(&P.mul(e));
    }
    return R;
}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java */
pub fn gtpow(d: &FP24, e: &BIG) -> FP24 {
    let mut r = FP24::new();
    if rom::USE_GS_GT {
        let mut g: [FP24; 8] = [
            FP24::new(),
            FP24::new(),
            FP24::new(),
            FP24::new(),
            FP24::new(),
            FP24::new(),
            FP24::new(),
            FP24::new(),
        ];
        let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut t = BIG::new();
        let mut u = gs(e);
        let mut w = FP24::new();

        g[0].copy(&d);
        for i in 1..8 {
            w.copy(&g[i - 1]);
            g[i].copy(&w);
            g[i].frob(&f, 1);
        }
        for i in 0..8 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                g[i].conj();
            }
            u[i].norm();
        }
        r.copy(&FP24::pow8(&mut g, &u));
    } else {
        r.copy(&d.pow(e));
    }
    return r;
}
