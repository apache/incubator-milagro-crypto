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
use super::ecp8::ECP8;
use super::fp8::FP8;
use super::fp16::FP16;
use super::fp48::FP48;
use super::big::BIG;
use super::ecp;
use super::rom;
use types::{SignOfX, SexticTwist};

#[allow(non_snake_case)]
fn linedbl(A: &mut ECP8, qx: &FP, qy: &FP) -> FP48 {
    let mut a = FP16::new();
    let mut b = FP16::new();
    let mut c = FP16::new();

    let mut xx = FP8::new_copy(&A.getpx()); //X
    let mut yy = FP8::new_copy(&A.getpy()); //Y
    let mut zz = FP8::new_copy(&A.getpz()); //Z
    let mut yz = FP8::new_copy(&yy); //Y
    yz.mul(&zz); //YZ
    xx.sqr(); //X^2
    yy.sqr(); //Y^2
    zz.sqr(); //Z^2

    yz.imul(4);
    yz.neg();
    yz.norm(); //-2YZ
    yz.tmul(qy); //-2YZ.Ys

    xx.imul(6); //3X^2
    xx.tmul(qx); //3X^2.Xs

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

    a.copy(&FP16::new_fp8s(&yz, &zz)); // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP16::new_fp8(&xx)); // L(0,1) | L(0,0) | L(1,0)
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP16::new_fp8(&xx));
        c.times_i();
    }
    A.dbl();
    return FP48::new_fp16s(&a, &b, &c);
}

#[allow(non_snake_case)]
fn lineadd(A: &mut ECP8, B: &ECP8, qx: &FP, qy: &FP) -> FP48 {
    let mut a = FP16::new();
    let mut b = FP16::new();
    let mut c = FP16::new();

    let mut x1 = FP8::new_copy(&A.getpx()); // X1
    let mut y1 = FP8::new_copy(&A.getpy()); // Y1
    let mut t1 = FP8::new_copy(&A.getpz()); // Z1
    let mut t2 = FP8::new_copy(&A.getpz()); // Z1

    t1.mul(&B.getpy()); // T1=Z1.Y2
    t2.mul(&B.getpx()); // T2=Z1.X2

    x1.sub(&t2);
    x1.norm(); // X1=X1-Z1.X2
    y1.sub(&t1);
    y1.norm(); // Y1=Y1-Z1.Y2

    t1.copy(&x1); // T1=X1-Z1.X2
    x1.tmul(qy); // X1=(X1-Z1.X2).Ys
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        x1.times_i();
    }

    t1.mul(&B.getpy()); // T1=(X1-Z1.X2).Y2

    t2.copy(&y1); // T2=Y1-Z1.Y2
    t2.mul(&B.getpx()); // T2=(Y1-Z1.Y2).X2
    t2.sub(&t1);
    t2.norm(); // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
    y1.tmul(qx);
    y1.neg();
    y1.norm(); // Y1=-(Y1-Z1.Y2).Xs

    a.copy(&FP16::new_fp8s(&x1, &t2)); // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
    if ecp::SEXTIC_TWIST == SexticTwist::D_TYPE {
        b.copy(&FP16::new_fp8(&y1));
    }
    if ecp::SEXTIC_TWIST == SexticTwist::M_TYPE {
        c.copy(&FP16::new_fp8(&y1));
        c.times_i();
    }

    A.add(B);
    return FP48::new_fp16s(&a, &b, &c);
}

#[allow(non_snake_case)]
/* Optimal R-ate pairing */
pub fn ate(P1: &ECP8, Q1: &ECP) -> FP48 {
    let x = BIG::new_ints(&rom::CURVE_BNX);
    let n = BIG::new_copy(&x);

    let mut n3 = BIG::new_copy(&n);
    n3.pmul(3);
    n3.norm();

    let mut P = ECP8::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let mut A = ECP8::new();
    let mut r = FP48::new_int(1);

    A.copy(&P);
    let mut NP = ECP8::new();
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
pub fn ate2(P1: &ECP8, Q1: &ECP, R1: &ECP8, S1: &ECP) -> FP48 {
    let x = BIG::new_ints(&rom::CURVE_BNX);
    let n = BIG::new_copy(&x);

    let mut n3 = BIG::new_copy(&n);
    n3.pmul(3);
    n3.norm();

    let mut P = ECP8::new();
    P.copy(P1);
    P.affine();
    let mut Q = ECP::new();
    Q.copy(Q1);
    Q.affine();
    let mut R = ECP8::new();
    R.copy(R1);
    R.affine();
    let mut S = ECP::new();
    S.copy(S1);
    S.affine();

    let qx = FP::new_copy(&Q.getpx());
    let qy = FP::new_copy(&Q.getpy());

    let sx = FP::new_copy(&S.getpx());
    let sy = FP::new_copy(&S.getpy());

    let mut A = ECP8::new();
    let mut B = ECP8::new();
    let mut r = FP48::new_int(1);

    A.copy(&P);
    B.copy(&R);

    let mut NP = ECP8::new();
    NP.copy(&P);
    NP.neg();
    let mut NR = ECP8::new();
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
pub fn fexp(m: &FP48) -> FP48 {
    let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
    let mut x = BIG::new_ints(&rom::CURVE_BNX);
    let mut r = FP48::new_copy(m);

    /* Easy part of final exp */
    let mut lv = FP48::new_copy(&r);
    lv.inverse();
    r.conj();

    r.mul(&lv);
    lv.copy(&r);
    r.frob(&f, 8);
    r.mul(&lv);

    /* Hard part of final exp */
    // Ghamman & Fouotsa Method

    let mut t7 = FP48::new_copy(&r);
    t7.usqr();
    let mut t1 = t7.pow(&mut x);

    x.fshr(1);
    let mut t2 = t1.pow(&mut x);
    x.fshl(1);

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    let mut t3 = FP48::new_copy(&t1);
    t3.conj();
    t2.mul(&t3);
    t2.mul(&r);

    r.mul(&t7);

    t1.copy(&t2.pow(&mut x));

    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }
    t3.copy(&t1);
    t3.frob(&f, 14);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 13);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 12);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 11);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 10);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 9);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 8);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t2);
    t3.conj();
    t1.mul(&t3);
    t3.copy(&t1);
    t3.frob(&f, 7);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 6);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 5);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 4);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 3);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 2);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    t3.copy(&t1);
    t3.frob(&f, 1);
    r.mul(&t3);
    lv.copy(&t1.pow(&mut x));
    t1.copy(&lv);
    if ecp::SIGN_OF_X == SignOfX::NEGATIVEX {
        t1.conj();
    }

    r.mul(&t1);
    t2.frob(&f, 15);
    r.mul(&t2);

    r.reduce();
    return r;
}

#[allow(non_snake_case)]
/* GLV method */
fn glv(e: &BIG) -> [BIG; 2] {
    let mut u: [BIG; 2] = [BIG::new(), BIG::new()];
    let q = BIG::new_ints(&rom::CURVE_ORDER);
    let mut x = BIG::new_ints(&rom::CURVE_BNX);
    let mut x2 = BIG::smul(&x, &x);
    x.copy(&BIG::smul(&x2, &x2));
    x2.copy(&BIG::smul(&x, &x));
    u[0].copy(&e);
    u[0].rmod(&x2);
    u[1].copy(&e);
    u[1].div(&x2);
    u[1].rsub(&q);

    return u;
}

#[allow(non_snake_case)]
/* Galbraith & Scott Method */
pub fn gs(e: &BIG) -> [BIG; 16] {
    let mut u: [BIG; 16] = [
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
        BIG::new(),
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
    for i in 0..15 {
        u[i].copy(&w);
        u[i].rmod(&x);
        w.div(&x);
    }
    u[15].copy(&w);
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
        t.copy(&BIG::modneg(&mut u[9], &q));
        u[9].copy(&t);
        t.copy(&BIG::modneg(&mut u[11], &q));
        u[11].copy(&t);
        t.copy(&BIG::modneg(&mut u[13], &q));
        u[13].copy(&t);
        t.copy(&BIG::modneg(&mut u[15], &q));
        u[15].copy(&t);
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
pub fn g2mul(P: &ECP8, e: &BIG) -> ECP8 {
    let mut R = ECP8::new();
    if rom::USE_GS_G2 {
        let mut Q: [ECP8; 16] = [
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
            ECP8::new(),
        ];
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut u = gs(e);
        let mut T = ECP8::new();

        let f = ECP8::frob_constants();

        let mut t = BIG::new();
        Q[0].copy(&P);
        for i in 1..16 {
            T.copy(&Q[i - 1]);
            Q[i].copy(&T);
            Q[i].frob(&f, 1);
        }
        for i in 0..16 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                Q[i].neg();
            }
            u[i].norm();
        }

        R.copy(&ECP8::mul16(&mut Q, &u));
    } else {
        R.copy(&P.mul(e));
    }
    return R;
}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java */
pub fn gtpow(d: &FP48, e: &BIG) -> FP48 {
    let mut r = FP48::new();
    if rom::USE_GS_GT {
        let mut g: [FP48; 16] = [
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
            FP48::new(),
        ];
        let f = FP2::new_bigs(&BIG::new_ints(&rom::FRA), &BIG::new_ints(&rom::FRB));
        let q = BIG::new_ints(&rom::CURVE_ORDER);
        let mut t = BIG::new();
        let mut u = gs(e);
        let mut w = FP48::new();

        g[0].copy(&d);
        for i in 1..16 {
            w.copy(&g[i - 1]);
            g[i].copy(&w);
            g[i].frob(&f, 1);
        }
        for i in 0..16 {
            let np = u[i].nbits();
            t.copy(&BIG::modneg(&mut u[i], &q));
            let nn = t.nbits();
            if nn < np {
                u[i].copy(&t);
                g[i].conj();
            }
            u[i].norm();
        }
        r.copy(&FP48::pow16(&mut g, &u));
    } else {
        r.copy(&d.pow(e));
    }
    return r;
}
