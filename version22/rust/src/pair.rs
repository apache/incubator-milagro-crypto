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

//mod fp;
use fp::FP;
//mod ecp;
use ecp::ECP;
//mod fp2;
use fp2::FP2;
//mod ecp2;
use ecp2::ECP2;
//mod fp4;
use fp4::FP4;
//mod fp12;
use fp12::FP12;
//mod big;
use big::BIG;
//mod dbig;
use dbig::DBIG;
//mod rand;
//mod hash256;
//mod rom;
use rom;

#[allow(non_snake_case)]

fn linedbl(A: &mut ECP2,qx: &mut FP,qy: &mut  FP) -> FP12 {
	let mut P=ECP2::new();

	P.copy(A);
	let mut zz=FP2::new_copy(&P.getpz());
	zz.sqr();
	let d=A.dbl();	

	if d<0 {return FP12::new_int(1)}

	let mut z3=FP2::new_copy(&A.getpz());
	let mut a=FP4::new();
	let mut b=FP4::new();
	let c=FP4::new();	

	let mut x=FP2::new_copy(&P.getpx());
	let mut y=FP2::new_copy(&P.getpy());
	let mut t=FP2::new_copy(&P.getpx());
	t.sqr();
	t.imul(3);

	y.sqr();
	y.dbl();
	z3.mul(&mut zz);
	z3.pmul(qy);

	x.mul(&mut t);
	x.sub(&y);
	a.copy(&FP4::new_fp2s(&z3,&x));
	t.neg();
	zz.mul(&mut t);
	zz.pmul(qx);
	b.copy(&FP4::new_fp2(&zz));

	return FP12::new_fp4s(&a,&b,&c);
}

#[allow(non_snake_case)]
fn lineadd(A: &mut ECP2,B: &mut ECP2,qx: &mut FP,qy: &mut  FP) -> FP12 {

	let mut P=ECP2::new();

	P.copy(A);
	let mut zz=FP2::new_copy(&P.getpz());
	zz.sqr();

	let d=A.add(B);
	if d<0 {return FP12::new_int(1)}	

	let mut z3=FP2::new_copy(&A.getpz());
	let mut a=FP4::new();
	let mut b=FP4::new();
	let c=FP4::new();	

	if d==0 { /* Addition */
		let mut x=FP2::new_copy(&B.getpx());
		let mut y=FP2::new_copy(&B.getpy());
		let mut t=FP2::new_copy(&P.getpz()); 
		t.mul(&mut y);
		zz.mul(&mut t);

		let mut ny=FP2::new_copy(&P.getpy()); ny.neg();
		zz.add(&ny); 
		z3.pmul(qy);
		t.mul(&mut P.getpx());
		x.mul(&mut ny);
		t.add(&x);
		a.copy(&FP4::new_fp2s(&z3,&t));
		zz.neg();
		zz.pmul(qx);
		b.copy(&FP4::new_fp2(&zz));
	} else { /* Doubling */
		let mut x=FP2::new_copy(&P.getpx());
		let mut y=FP2::new_copy(&P.getpy());
		let mut t=FP2::new_copy(&P.getpx());
		t.sqr();
		t.imul(3);

		y.sqr();
		y.dbl();
		z3.mul(&mut zz);
		z3.pmul(qy);

		x.mul(&mut t);
		x.sub(&y);
		a.copy(&FP4::new_fp2s(&z3,&x));
		t.neg();
		zz.mul(&mut t);
		zz.pmul(qx);
		b.copy(&FP4::new_fp2(&zz));
	}
	return FP12::new_fp4s(&a,&b,&c);
}

#[allow(non_snake_case)]
/* Optimal R-ate pairing */
pub fn ate(P: & mut ECP2,Q: &mut ECP) -> FP12 {
	let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
	let x = BIG::new_ints(&rom::CURVE_BNX);
	let mut n = BIG::new_copy(&x);
	let mut K = ECP2::new();

	
	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		n.pmul(6); n.dec(2);
	} else {n.copy(&x)}
	
	n.norm();
	P.affine();
	Q.affine();
	let mut qx=FP::new_copy(&Q.getpx());
	let mut qy=FP::new_copy(&Q.getpy());

	let mut A=ECP2::new();
	let mut r=FP12::new_int(1);

	A.copy(&P);
	let nb=n.nbits();

	for i in (1..nb-1).rev() {
		let mut lv=linedbl(&mut A,&mut qx,&mut qy);
		r.smul(&mut lv);
		if n.bit(i)==1 {
		
			lv=lineadd(&mut A,P,&mut qx,&mut qy);
		
			r.smul(&mut lv);
		}		
		r.sqr();	
	}

	let mut lv=linedbl(&mut A,&mut qx,&mut qy);
	r.smul(&mut lv);

	if n.parity()==1 {
		lv=lineadd(&mut A,P,&mut qx,&mut qy);
		r.smul(&mut lv);
	}

/* R-ate fixup required for BN curves */

	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		r.conj();
		K.copy(&P);
		K.frob(&mut f);
		A.neg();
		lv=lineadd(&mut A,&mut K,&mut qx,&mut qy);
		r.smul(&mut lv);
		K.frob(&mut f);
		K.neg();
		lv=lineadd(&mut A,&mut K,&mut qx,&mut qy);
		r.smul(&mut lv);
	}

	return r;
}

#[allow(non_snake_case)]
/* Optimal R-ate double pairing e(P,Q).e(R,S) */
pub fn ate2(P: &mut ECP2,Q: &mut ECP,R: &mut ECP2,S: &mut ECP) -> FP12 {
	let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
	let x = BIG::new_ints(&rom::CURVE_BNX);
	let mut n = BIG::new_copy(&x);
	let mut K = ECP2::new();


	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		n.pmul(6); n.dec(2);
	} else {n.copy(&x)}
	
	n.norm();
	P.affine();
	Q.affine();
	R.affine();
	S.affine();

	let mut qx=FP::new_copy(&Q.getpx());
	let mut qy=FP::new_copy(&Q.getpy());

	let mut sx=FP::new_copy(&S.getpx());
	let mut sy=FP::new_copy(&S.getpy());

	let mut A=ECP2::new();
	let mut B=ECP2::new();
	let mut r=FP12::new_int(1);

	A.copy(&P);
	B.copy(&R);
	let nb=n.nbits();

	for i in (1..nb-1).rev() {
		let mut lv=linedbl(&mut A,&mut qx,&mut qy);
		r.smul(&mut lv);
		lv=linedbl(&mut B,&mut sx,&mut sy);
		r.smul(&mut lv);

		if n.bit(i)==1 {
			lv=lineadd(&mut A,P,&mut qx,&mut qy);
			r.smul(&mut lv);
			lv=lineadd(&mut B,R,&mut sx,&mut sy);
			r.smul(&mut lv);
		}
		r.sqr();
	}

	let mut lv=linedbl(&mut A,&mut qx,&mut qy);
	r.smul(&mut lv);
	lv=linedbl(&mut B,&mut sx,&mut sy);
	r.smul(&mut lv);
	if n.parity()==1 {
		lv=lineadd(&mut A,P,&mut qx,&mut qy);
		r.smul(&mut lv);
		lv=lineadd(&mut B,R,&mut sx,&mut sy);
		r.smul(&mut lv);
	}

/* R-ate fixup */
	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		r.conj();
		K.copy(&P);
		K.frob(&mut f);
		A.neg();
		lv=lineadd(&mut A,&mut K,&mut qx,&mut qy);
		r.smul(&mut lv);
		K.frob(&mut f);
		K.neg();
		lv=lineadd(&mut A,&mut K,&mut qx,&mut qy);
		r.smul(&mut lv);

		K.copy(&R);
		K.frob(&mut f);
		B.neg();
		lv=lineadd(&mut B,&mut K,&mut sx,&mut sy);
		r.smul(&mut lv);
		K.frob(&mut f);
		K.neg();
		lv=lineadd(&mut B,&mut K,&mut sx,&mut sy);
		r.smul(&mut lv);
	}

	return r;
}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
pub fn fexp(m: &FP12) -> FP12 {
	let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
	let mut x = BIG::new_ints(&rom::CURVE_BNX);
	let mut r=FP12::new_copy(m);
		
/* Easy part of final exp */
	let mut lv=FP12::new_copy(&r);
	lv.inverse();
	r.conj();

	r.mul(&mut lv);
	lv.copy(&r);
	r.frob(&mut f);
	r.frob(&mut f);
	r.mul(&mut lv);
/* Hard part of final exp */
	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		lv.copy(&r);
		lv.frob(&mut f);
		let mut x0=FP12::new_copy(&lv);
		x0.frob(&mut f);
		lv.mul(&mut r);
		x0.mul(&mut lv);
		x0.frob(&mut f);
		let mut x1=FP12::new_copy(&r);
		x1.conj();
		let mut x4=r.pow(&mut x);

		let mut x3=FP12::new_copy(&x4);
		x3.frob(&mut f);

		let mut x2=x4.pow(&mut x);

		let mut x5=FP12::new_copy(&x2); x5.conj();
		lv=x2.pow(&mut x);

		x2.frob(&mut f);
		r.copy(&x2); r.conj();

		x4.mul(&mut r);
		x2.frob(&mut f);

		r.copy(&lv);
		r.frob(&mut f);
		lv.mul(&mut r);

		lv.usqr();
		lv.mul(&mut x4);
		lv.mul(&mut x5);
		r.copy(&x3);
		r.mul(&mut x5);
		r.mul(&mut lv);
		lv.mul(&mut x2);
		r.usqr();
		r.mul(&mut lv);
		r.usqr();
		lv.copy(&r);
		lv.mul(&mut x1);
		r.mul(&mut x0);
		lv.usqr();
		r.mul(&mut lv);
		r.reduce();
	} else {

// Ghamman & Fouotsa Method

		let mut y0=FP12::new_copy(&r); y0.usqr();
		let mut y1=y0.pow(&mut x);
		x.fshr(1); let mut y2=y1.pow(&mut x); x.fshl(1);
		let mut y3=FP12::new_copy(&r); y3.conj();
		y1.mul(&mut y3);

		y1.conj();
		y1.mul(&mut y2);

		y2=y1.pow(&mut x);

		y3=y2.pow(&mut x);
		y1.conj();
		y3.mul(&mut y1);

		y1.conj();
		y1.frob(&mut f); y1.frob(&mut f); y1.frob(&mut f);
		y2.frob(&mut f); y2.frob(&mut f);
		y1.mul(&mut y2);

		y2=y3.pow(&mut x);
		y2.mul(&mut y0);
		y2.mul(&mut r);

		y1.mul(&mut y2);
		y2.copy(&y3); y2.frob(&mut f);
		y1.mul(&mut y2);
		r.copy(&y1);
		r.reduce();


/*
		let mut x0=FP12::new_copy(&r);
		let mut x1=FP12::new_copy(&r);
		lv.copy(&r); lv.frob(&mut f);
		let mut x3=FP12::new_copy(&lv); x3.conj(); x1.mul(&mut x3);
		lv.frob(&mut f); lv.frob(&mut f);
		x1.mul(&mut lv);

		r=r.pow(&mut x);  //r=r.pow(x);
		x3.copy(&r); x3.conj(); x1.mul(&mut x3);
		lv.copy(&r); lv.frob(&mut f);
		x0.mul(&mut lv);
		lv.frob(&mut f);
		x1.mul(&mut lv);
		lv.frob(&mut f);
		x3.copy(&lv); x3.conj(); x0.mul(&mut x3);

		r=r.pow(&mut x);
		x0.mul(&mut r);
		lv.copy(&r); lv.frob(&mut f); lv.frob(&mut f);
		x3.copy(&lv); x3.conj(); x0.mul(&mut x3);
		lv.frob(&mut f);
		x1.mul(&mut lv);

		r=r.pow(&mut x);
		lv.copy(&r); lv.frob(&mut f);
		x3.copy(&lv); x3.conj(); x0.mul(&mut x3);
		lv.frob(&mut f);
		x1.mul(&mut lv);

		r=r.pow(&mut x);
		x3.copy(&r); x3.conj(); x0.mul(&mut x3);
		lv.copy(&r); lv.frob(&mut f);
		x1.mul(&mut lv);

		r=r.pow(&mut x);
		x1.mul(&mut r);

		x0.usqr();
		x0.mul(&mut x1);
		r.copy(&x0);
		r.reduce();  */
	}
	return r;
}

#[allow(non_snake_case)]
/* GLV method */
fn glv(e: &mut BIG) -> [BIG;2] {
	let mut u:[BIG;2]=[BIG::new(),BIG::new()];
	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		let mut t=BIG::new();
		let q=BIG::new_ints(&rom::CURVE_ORDER);
		let mut v:[BIG;2]=[BIG::new(),BIG::new()];

		for i in 0..2 {
			t.copy(&BIG::new_ints(&rom::CURVE_W[i]));  // why not just t=new BIG(ROM.CURVE_W[i]); 
			let mut d:DBIG = BIG::mul(&mut t,e);
			v[i].copy(&d.div(&q));
		}
		u[0].copy(&e);
		for i in 0..2 {
			for j in 0..2 {
				t=BIG::new_ints(&rom::CURVE_SB[j][i]);
				t=BIG::modmul(&mut v[j],&mut t,&q);
				u[i].add(&q);
				u[i].sub(&t);
				u[i].rmod(&q);
			}
		}
	} else {
		let q=BIG::new_ints(&rom::CURVE_ORDER);
		let x=BIG::new_ints(&rom::CURVE_BNX);
		let x2=BIG::smul(&x,&x);
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
pub fn gs(e: &mut BIG) -> [BIG;4] {
	let mut u:[BIG;4]=[BIG::new(),BIG::new(),BIG::new(),BIG::new()];
	if rom::CURVE_PAIRING_TYPE == rom::BN_CURVE {
		let mut t=BIG::new();
		let q=BIG::new_ints(&rom::CURVE_ORDER);

		let mut v:[BIG;4]=[BIG::new(),BIG::new(),BIG::new(),BIG::new()];
		for i in 0..4 {
			t.copy(&BIG::new_ints(&rom::CURVE_WB[i]));
			let mut d:DBIG=BIG::mul(&mut t,e);
			v[i].copy(&d.div(&q));
		}
		u[0].copy(&e);
		for i in 0..4 {
			for j in 0..4 {
				t=BIG::new_ints(&rom::CURVE_BB[j][i]);
				t=BIG::modmul(&mut v[j],&mut t,&q);
				u[i].add(&q);
				u[i].sub(&t);
				u[i].rmod(&q);
			}
		}
	} else {
		let x=BIG::new_ints(&rom::CURVE_BNX);
		let mut w=BIG::new_copy(&e);
		for i in 0..4 {
			u[i].copy(&w);
			u[i].rmod(&x);
			w.div(&x);
		}
	}
	return u;
}	

#[allow(non_snake_case)]
/* Multiply P by e in group G1 */
pub fn g1mul(P: &mut ECP,e: &mut BIG) -> ECP {
	let mut R=ECP::new();
	if rom::USE_GLV {
		P.affine();
		R.copy(P);
		let mut Q=ECP::new();
		Q.copy(P);
		let q=BIG::new_ints(&rom::CURVE_ORDER);
		let mut cru=FP::new_big(&BIG::new_ints(&rom::CURVE_CRU));
		let mut u=glv(e);
		Q.mulx(&mut cru);

		let mut np=u[0].nbits();
		let mut t:BIG=BIG::modneg(&mut u[0],&q);
		let mut nn=t.nbits();
		if nn<np {
			u[0].copy(&t);
			R.neg();
		}

		np=u[1].nbits();
		t=BIG::modneg(&mut u[1],&q);
		nn=t.nbits();
		if nn<np {
			u[1].copy(&t);
			Q.neg();
		}

		R=R.mul2(&u[0],&mut Q,&u[1]);
			
	} else {
		R=P.mul(e);
	}
	return R;
}

#[allow(non_snake_case)]
/* Multiply P by e in group G2 */
pub fn g2mul(P: &mut ECP2,e: &mut BIG) -> ECP2 {
	let mut R=ECP2::new();
	if rom::USE_GS_G2 {
		let mut Q:[ECP2;4]=[ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new()];
		let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
		let q=BIG::new_ints(&rom::CURVE_ORDER);
		let mut u=gs(e);
		let mut T=ECP2::new();

		let mut t=BIG::new();
		P.affine();
		Q[0].copy(&P);
		for i in 1..4 {
			T.copy(&Q[i-1]);
			Q[i].copy(&T);
			Q[i].frob(&mut f);
		}
		for i in 0..4 {
			let np=u[i].nbits();
			t.copy(&BIG::modneg(&mut u[i],&q));
			let nn=t.nbits();
			if nn<np {
				u[i].copy(&t);
				Q[i].neg();
			}
		}

		R.copy(&ECP2::mul4(&mut Q,&u));

	} else {
		R.copy(&P.mul(e));
	}
	return R;
}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java */
pub fn gtpow(d: &mut FP12,e: &mut BIG) -> FP12 {
	let mut r=FP12::new();
	if rom::USE_GS_GT {
		let mut g:[FP12;4]=[FP12::new(),FP12::new(),FP12::new(),FP12::new()];
		let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
		let q=BIG::new_ints(&rom::CURVE_ORDER);
		let mut t=BIG::new();
		let mut u=gs(e);
		let mut w=FP12::new();

		g[0].copy(&d);
		for i in 1..4 {
			w.copy(&g[i-1]);
			g[i].copy(&w);
			g[i].frob(&mut f);
		}
		for i in 0..4 {
			let np=u[i].nbits();
			t.copy(&BIG::modneg(&mut u[i],&q));
			let nn=t.nbits();
			if nn<np {
				u[i].copy(&t);
				g[i].conj();
			}
		}
		r.copy(&FP12::pow4(&mut g,&u));
	} else {
		r.copy(&d.pow(e));
	}
	return r;
}

/*
#[allow(non_snake_case)]
fn main()
{
	let mut Q=ECP::new_bigs(&BIG::new_ints(&rom::CURVE_GX),&BIG::new_ints(&rom::CURVE_GY));
	let mut P=ECP2::new_fp2s(&FP2::new_bigs(&BIG::new_ints(&rom::CURVE_PXA),&BIG::new_ints(&rom::CURVE_PXB)),&FP2::new_bigs(&BIG::new_ints(&rom::CURVE_PYA),&BIG::new_ints(&rom::CURVE_PYB)));

	let mut r=BIG::new_ints(&rom::CURVE_ORDER);
	
	println!("P= {}",P.tostring());
	println!("Q= {}",Q.tostring());

	//m:=NewBIGint(17)

	let mut e=ate(&mut P,&mut Q);
	println!("\ne= {}",e.tostring());

	e=fexp(&e);

	for i in 1..10 {
		e=ate(&mut P,&mut Q);
		e=fexp(&e);
	}


	//	e=GTpow(e,m);

	println!("\ne= {}",e.tostring());

	
	fmt.Printf("\n");
	GLV:=glv(r)

	fmt.Printf("GLV[0]= "+GLV[0].toString())
	fmt.Printf("\n")

	fmt.Printf("GLV[0]= "+GLV[1].toString())
	fmt.Printf("\n")

	G:=NewECP(); G.copy(Q)
	R:=NewECP2(); R.copy(P)


	e=ate(R,Q)
	e=fexp(e)

	e=GTpow(e,xa)
	fmt.Printf("\ne= "+e.toString());
	fmt.Printf("\n")

	R=G2mul(R,xa)
	e=ate(R,G)
	e=fexp(e)

	fmt.Printf("\ne= "+e.toString())
	fmt.Printf("\n")

	G=G1mul(G,xa)
	e=ate(P,G)
	e=fexp(e)
	fmt.Printf("\ne= "+e.toString())
	fmt.Printf("\n") 
}*/
