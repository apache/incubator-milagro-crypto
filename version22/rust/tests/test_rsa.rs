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

use  amcl::ff;
use  amcl::big;
use  amcl::dbig;
use  amcl::rom;
use  amcl::rand;
use  amcl::hash256;
use  amcl::hash384;
use  amcl::hash512;
use  amcl::rsa;

use rand::RAND;

pub fn printbinary(array: &[u8]) {
	for i in 0..array.len() {
		print!("{:02X}", array[i]);
	}
	println!("");
} 

use std::str;
//use std::process;

#[test]
fn test_rsa()
{
	let sha=rsa::HASH_TYPE;
	let message:&[u8] = b"Hello World\n";

	let mut pbc=rsa::new_public_key(rom::FFLEN);
	let mut prv=rsa::new_private_key(rom::HFLEN);

	let mut ml:[u8;rsa::RFS]=[0;rsa::RFS];
	let mut ms:[u8;rsa::RFS]=[0;rsa::RFS];	
	let mut c: [u8;rsa::RFS]=[0;rsa::RFS];
	let mut s: [u8;rsa::RFS]=[0;rsa::RFS];
	let mut e: [u8;rsa::RFS]=[0;rsa::RFS];

	let mut raw:[u8;100]=[0;100];
	
	let mut rng=RAND::new();

	rng.clean();
	for i in 0..100 {raw[i]=i as u8}

	rng.seed(100,&raw);

	println!("Generating public/private key pair");
	rsa::key_pair(&mut rng,65537,&mut prv,&mut pbc);

	println!("Encrypting test string\n");
	rsa::oaep_encode(sha,&message,&mut rng,None,&mut e); /* OAEP encode message M to E  */

	rsa::encrypt(&pbc,&e,&mut c);    /* encrypt encoded message */
	print!("Ciphertext= 0x"); printbinary(&c);

	println!("Decrypting test string");
	rsa::decrypt(&prv,&c,&mut ml);
	let mlen=rsa::oaep_decode(sha,None,&mut ml); /* OAEP decode message  */

	let mess=str::from_utf8(&ml[0..mlen]).unwrap();
	print!("{}",&mess);

	println!("Signing message");
	rsa::pkcs15(sha,message,&mut c); 

	rsa::decrypt(&prv,&c,&mut s);  /* create signature in S */ 

	print!("Signature= 0x"); printbinary(&s);

	rsa::encrypt(&pbc,&s,&mut ms);

	let mut cmp=true;
	if c.len()!=ms.len() {
		cmp=false;
	} else {
		for j in 0..c.len() {
			if c[j]!=ms[j] {cmp=false}
		}
	}
	if cmp {
		println!("Signature is valid");
	} else {
		println!("Signature is INVALID");
	}

	rsa::private_key_kill(&mut prv);
}

