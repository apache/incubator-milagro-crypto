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

extern crate amcl;

use amcl::mpin;
use amcl::ecp;
use amcl::ecp2;
use amcl::fp;
use amcl::fp2;
use amcl::fp4;
use amcl::fp12;
use amcl::pair;
use amcl::big;
use amcl::dbig;
use amcl::rand;
use amcl::hash256;
use amcl::hash384;
use amcl::hash512;
use amcl::aes;
use amcl::rom;

use rand::RAND;
use ecp::ECP;
use big::BIG;
use ecp2::ECP2;
use fp2::FP2;

use std::time::Instant;

const MIN_ITERS:isize=10;
const MIN_TIME: isize=10;

#[allow(non_snake_case)]
#[test]
fn benchtest_pair()
{
	let mut raw:[u8;100]=[0;100];	
	let mut fail=false;

	let mut rng=RAND::new();
	rng.clean();
	for i in 0..100 {raw[i]=i as u8}

	rng.seed(100,&raw);	

	if rom::CURVE_PAIRING_TYPE==rom::BN_CURVE {
		println!("BN Pairing-Friendly Curve");
	}
	if rom::CURVE_PAIRING_TYPE==rom::BLS_CURVE {
		println!("BLS Pairing-Friendly Curve");
	}

	println!("Modulus size {:} bits",rom::MODBITS); 
	println!("{:} bit build",rom::CHUNK); 

	let mut G:ECP;

	let gx=BIG::new_ints(&rom::CURVE_GX);
	
	let gy=BIG::new_ints(&rom::CURVE_GY);
	G=ECP::new_bigs(&gx,&gy);

	let mut r=BIG::new_ints(&rom::CURVE_ORDER);
	let mut s=BIG::randomnum(&r,&mut rng);

	let mut P=pair::g1mul(&mut G,&mut r);

	if !P.is_infinity() {
		println!("FAILURE - rP!=O");
		fail=true;
	}

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		P=pair::g1mul(&mut G,&mut s);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("G1  mul              - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	let mut Q=ECP2::new_fp2s(&FP2::new_bigs(&BIG::new_ints(&rom::CURVE_PXA),&BIG::new_ints(&rom::CURVE_PXB)),&FP2::new_bigs(&BIG::new_ints(&rom::CURVE_PYA),&BIG::new_ints(&rom::CURVE_PYB)));

	let mut W=pair::g2mul(&mut Q,&mut r);

	if !W.is_infinity() {
		println!("FAILURE - rQ!=O");
		fail=true;
	}

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		W=pair::g2mul(&mut Q,&mut s);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("G2  mul              - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	let mut w=pair::ate(&mut Q,&mut P);
	w=pair::fexp(&w);

	let mut g=pair::gtpow(&mut w,&mut r);

	if !g.isunity() {
		println!("FAILURE - g^r!=1");
		return;
	}

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		g=pair::gtpow(&mut w,&mut s);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("GT  pow              - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);


	let mut f = FP2::new_bigs(&BIG::new_ints(&rom::CURVE_FRA),&BIG::new_ints(&rom::CURVE_FRB));
	let q=BIG::new_ints(&rom::MODULUS);

	let mut m=BIG::new_copy(&q);
	m.rmod(&mut r);

	let mut a=BIG::new_copy(&s);
	a.rmod(&mut m);

	let mut b=BIG::new_copy(&s);
	b.div(&mut m);

	g.copy(&w);
	let mut c=g.trace();

	g.frob(&mut f);
	let cp=g.trace();

	w.conj();
	g.mul(&mut w);
	let cpm1=g.trace();
	g.mul(&mut w);
	let cpm2=g.trace();

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		c=c.xtr_pow2(&cp,&cpm1,&cpm2,&mut a,&mut b);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("GT  pow (compressed) - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		w=pair::ate(&mut Q,&mut P);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("PAIRing ATE          - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);


	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		g=pair::fexp(&w);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("PAIRing FEXP         - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	P.copy(&G);
	Q.copy(&W);

	P=pair::g1mul(&mut P,&mut s);
	g=pair::ate(&mut Q,&mut P);
	g=pair::fexp(&g);

	P.copy(&G);
	Q=pair::g2mul(&mut Q,&mut s);
	w=pair::ate(&mut Q,&mut P);
	w=pair::fexp(&w);

	if !g.equals(&mut w) {
		println!("FAILURE - e(sQ,p)!=e(Q,sP) ");
		fail=true;
	}

	Q.copy(&W);
	g=pair::ate(&mut Q,&mut P);
	g=pair::fexp(&g);
	g=pair::gtpow(&mut g,&mut s);

	if !g.equals(&mut w) {
		println!("FAILURE - e(sQ,p)!=e(Q,P)^s ");
		fail=true;
	}

	if !fail {
		println!("All tests pass");
	}

	assert!(!fail)
}