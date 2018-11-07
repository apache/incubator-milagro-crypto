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

use amcl::ecdh;
use amcl::ecp;
use amcl::ecp2;
use amcl::fp;
use amcl::fp2;
use amcl::fp4;
use amcl::fp12;
use amcl::big;
use amcl::dbig;
use amcl::rand;
use amcl::hash256;
use amcl::hash384;
use amcl::hash512;
use amcl::aes;
use amcl::rom;
use rand::RAND;

pub fn printbinary(array: &[u8]) {
	for i in 0..array.len() {
		print!("{:02X}", array[i])
	}
	println!("")
}

#[test]
fn test_ecdh()
{
	let pw="M0ng00se";
	let pp:&[u8] = b"M0ng00se";
	let sha=ecdh::HASH_TYPE;
	let mut salt:[u8;8]=[0;8];
	let mut raw:[u8;100]=[0;100];	
	let mut s1:[u8;ecdh::EGS]=[0;ecdh::EGS];
	let mut w0:[u8;2*ecdh::EFS+1]=[0;2*ecdh::EFS+1];
	let mut w1:[u8;2*ecdh::EFS+1]=[0;2*ecdh::EFS+1];
	let mut z0:[u8;ecdh::EFS]=[0;ecdh::EFS];
	let mut z1:[u8;ecdh::EFS]=[0;ecdh::EFS];
	let mut key:[u8;ecdh::EAS]=[0;ecdh::EAS];
	let mut cs: [u8;ecdh::EGS]=[0;ecdh::EGS];
	let mut ds: [u8;ecdh::EGS]=[0;ecdh::EGS];	
	let mut m: Vec<u8> = vec![0;32];   // array that could be of any length. So use heap.
	let mut p1: [u8;3]=[0;3];
	let mut p2: [u8;4]=[0;4];	
	let mut v: [u8;2*ecdh::EFS+1]=[0;2*ecdh::EFS+1];
	let mut t: [u8;12]=[0;12];

	let mut rng=RAND::new();
	rng.clean();
	for i in 0..100 {raw[i]=i as u8}

	rng.seed(100,&raw);	

	for i in 0..8 {salt[i]=(i+1) as u8}  // set Salt	

	println!("Alice's Passphrase= {}",pw);

	let mut s0:[u8;ecdh::EFS]=[0;ecdh::EGS];
	ecdh::pbkdf2(sha,pp,&salt,1000,ecdh::EGS,&mut s0);

	print!("Alice's private key= 0x");
	printbinary(&s0);

/* Generate Key pair S/W */
	ecdh::key_pair_generate(None,&mut s0,&mut w0);

	print!("Alice's public key= 0x");
	printbinary(&w0);

	let mut res=ecdh::public_key_validate(true,&w0);
	if res!=0 {
		println!("ECP Public Key is invalid!");
		return;
	}

/* Random private key for other party */
	ecdh::key_pair_generate(Some(&mut rng),&mut s1,&mut w1);

	print!("Servers private key= 0x");
	printbinary(&s1);

	print!("Servers public key= 0x");
	printbinary(&w1);


	res=ecdh::public_key_validate(true,&w1);
	if res!=0 {
		println!("ECP Public Key is invalid!");
		return;
	}
/* Calculate common key using DH - IEEE 1363 method */

	ecdh::ecpsvdp_dh(&s0,&w1,&mut z0);
	ecdh::ecpsvdp_dh(&s1,&w0,&mut z1);

	let mut same=true;
	for i in 0..ecdh::EFS {
		if z0[i]!=z1[i] {same=false}
	}

	if !same {
		println!("*** ECPSVDP-DH Failed");
		return;
	}

	ecdh::kdf2(sha,&z0,None,ecdh::EAS,&mut key);

	print!("Alice's DH Key=  0x"); printbinary(&key);
	print!("Servers DH Key=  0x"); printbinary(&key);

	if rom::CURVETYPE!=rom::MONTGOMERY {

		for i in 0..17 {m[i]=i as u8} 

		println!("Testing ECIES");

		p1[0]=0x0; p1[1]=0x1; p1[2]=0x2;
		p2[0]=0x0; p2[1]=0x1; p2[2]=0x2; p2[3]=0x3;

		let cc=ecdh::ecies_encrypt(sha,&p1,&p2,&mut rng,&w1,&m[0..17],&mut v,&mut t);

		if let Some(mut c)=cc {
			println!("Ciphertext= ");
			print!("V= 0x"); printbinary(&v);
			print!("C= 0x"); printbinary(&c);
			print!("T= 0x"); printbinary(&t);
		

			let mm=ecdh::ecies_decrypt(sha,&p1,&p2,&v,&mut c,&t,&s1);
			if let Some(rm)=mm {
				println!("Decryption succeeded");
				println!("Message is 0x"); printbinary(&rm);				
			}
			else {
				println!("*** ECIES Decryption Failed");
				return;
			} 
		}
		else {
			println!("*** ECIES Encryption Failed");
			return;
		} 

		println!("Testing ECDSA");

		if ecdh::ecpsp_dsa(sha,&mut rng,&s0,&m[0..17],&mut cs,&mut ds)!=0 {
			println!("***ECDSA Signature Failed");
			return;
		}
		println!("Signature= ");
		print!("C= 0x"); printbinary(&cs);
		print!("D= 0x"); printbinary(&ds);

		if ecdh::ecpvp_dsa(sha,&w0,&m[0..17],&cs,&ds)!=0 {
			println!("***ECDSA Verification Failed");
			return;
		} else {println!("ECDSA Signature/Verification succeeded ")}
	}

}
