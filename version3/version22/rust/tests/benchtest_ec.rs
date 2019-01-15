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

use  amcl::ecdh;
use  amcl::ecp;
use  amcl::ecp2;
use  amcl::fp;
use  amcl::fp2;
use  amcl::fp4;
use  amcl::fp12;
use  amcl::big;
use  amcl::dbig;
use  amcl::rand;
use  amcl::hash256;
use  amcl::hash384;
use  amcl::hash512;
use  amcl::aes;
use  amcl::rom;
use  amcl::ff;
use  amcl::rsa;

use rand::RAND;
use ecp::ECP;
use big::BIG;

use std::time::Instant;

const MIN_ITERS:isize=10;
const MIN_TIME: isize=10;

#[allow(non_snake_case)]
#[test]
fn benchtest_ec()
{
	let mut raw:[u8;100]=[0;100];	
	let mut fail=false;
	let mut pbc=rsa::new_public_key(rom::FFLEN);
	let mut prv=rsa::new_private_key(rom::HFLEN);	
	let mut c: [u8;rsa::RFS]=[0;rsa::RFS];
	let mut m: [u8;rsa::RFS]=[0;rsa::RFS];
	let mut p: [u8;rsa::RFS]=[0;rsa::RFS];	

	let mut rng=RAND::new();
	rng.clean();
	for i in 0..100 {raw[i]=i as u8}

	rng.seed(100,&raw);	

	if rom::CURVETYPE==rom::WEIERSTRASS {
		println!("Weierstrass parameterization");
	}		
	if rom::CURVETYPE==rom::EDWARDS {
		println!("Edwards parameterization");
	}
	if rom::CURVETYPE==rom::MONTGOMERY {
		println!("Montgomery parameterization");
	}

	if rom::MODTYPE==rom::PSEUDO_MERSENNE {
		println!("Pseudo-Mersenne Modulus");
	}
	if rom::MODTYPE==rom::MONTGOMERY_FRIENDLY {
		println!("Montgomery friendly Modulus");
	}
	if rom::MODTYPE==rom::GENERALISED_MERSENNE {
		println!("Generalised-Mersenne Modulus");
	}
	if rom::MODTYPE==rom::NOT_SPECIAL {
		println!("Not special Modulus");
	}

	println!("Modulus size {:} bits",rom::MODBITS); 
	println!("{:} bit build",rom::CHUNK); 

	let mut G:ECP;

	let gx=BIG::new_ints(&rom::CURVE_GX);
	
	if rom::CURVETYPE!=rom::MONTGOMERY {
		let gy=BIG::new_ints(&rom::CURVE_GY);
		G=ECP::new_bigs(&gx,&gy);
	} else {
		G=ECP::new_big(&gx);
	}

	let mut r=BIG::new_ints(&rom::CURVE_ORDER);
	let mut s=BIG::randomnum(&r,&mut rng);

	let mut P=G.mul(&mut r);
	if !P.is_infinity() {
		println!("FAILURE - rG!=O");
		fail=true;
	} 

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		P=G.mul(&mut s);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("EC  mul - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	println!("Generating {:}-bit RSA public/private key pair",rom::FFLEN*rom::BIGBITS);


	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		rsa::key_pair(&mut rng,65537,&mut prv,&mut pbc);
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("RSA gen - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	for i in 0..rsa::RFS {m[i]=(i%128) as u8;}

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		rsa::encrypt(&pbc,&m,&mut c); 
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("RSA enc - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	let start = Instant::now();
	let mut iterations=0;
	let mut dur=0 as u64;
	while dur<(MIN_TIME as u64)*1000 || iterations<MIN_ITERS {
		rsa::decrypt(&prv,&c,&mut p); 
		iterations+=1;
		let elapsed=start.elapsed();
		dur=(elapsed.as_secs() * 1_000) + (elapsed.subsec_nanos() / 1_000_000) as u64;
	} 
	let duration=(dur as f64)/(iterations as f64);
	print!("RSA dec - {:} iterations  ",iterations);
	println!(" {:0.2} ms per iteration",duration);

	let mut cmp=true;
	for i in 0..rsa::RFS {
			if p[i]!=m[i] {cmp=false;}
		}

	if !cmp {
		println!("FAILURE - RSA decryption");
		fail=true;
	}

	if !fail {
		println!("All tests pass");
	}

	assert!(!fail)
}
