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

use std::fmt;
use std::str::SplitWhitespace;

#[derive(Copy, Clone)]
pub struct FP2 {
	a:FP,
	b:FP,
}

use rom::BIG_HEX_STRING_LEN;
//mod fp;
use fp::FP;
//mod big;
use big::BIG;
//mod dbig;
//use dbig::DBIG;
//mod rand;
//mod hash256;
//mod rom;
//use rom;

impl fmt::Display for FP2 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "FP2: [ {}, {} ]", self.a, self.b)
	}
}

impl fmt::Debug for FP2 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "FP2: [ {}, {} ]", self.a, self.b)
	}
}

impl PartialEq for FP2 {
	fn eq(&self, other: &FP2) -> bool {
		return (self.a == other.a) &&
			(self.b == other.b);
	}
}

impl FP2 {

	pub fn new() -> FP2 {
		FP2 {
				a: FP::new(),
				b: FP::new(),
		}
	}

	pub fn new_int(a: isize) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(&FP::new_int(a));
		f.b.zero();
		return f;
	}	

	pub fn new_copy(x: &FP2) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(&x.a);
		f.b.copy(&x.b);
		return f
	}	

	pub fn new_fps(c: &FP,d: &FP) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(c);
		f.b.copy(d);
		return f;
	}	

	pub fn new_bigs(c: &BIG,d: &BIG) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(&FP::new_big(c));
		f.b.copy(&FP::new_big(d));
		return f;
	}	

	pub fn new_fp(c: &FP) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(c);
		f.b.zero();
		return f;
	}	

	pub fn new_big(c: &BIG) -> FP2 {
		let mut f=FP2::new();
		f.a.copy(&FP::new_big(c));
		f.b.zero();
		return f;
	}	

/* reduce components mod Modulus */
	pub fn reduce(&mut self) {
		self.a.reduce();
		self.b.reduce();
	}

/* normalise components of w */
	pub fn norm(&mut self) {
		self.a.norm();
		self.b.norm();
	}	

/* test self=0 ? */
	pub fn iszilch(&mut self) -> bool {
		self.reduce();
		return self.a.iszilch() && self.b.iszilch();
	}	

	pub fn cmove(&mut self,g:&FP2,d: isize) {
		self.a.cmove(&g.a,d);
		self.b.cmove(&g.b,d);
	}		

/* test self=1 ? */
	pub fn isunity(&mut self) -> bool {
		let mut one=FP::new_int(1);
		return self.a.equals(&mut one) && self.b.iszilch();
	}

/* test self=x */
	pub fn equals(&mut self,x:&mut FP2) -> bool {
		return self.a.equals(&mut x.a) && self.b.equals(&mut x.b);
	}

/* extract a */
	pub fn geta(&mut self) -> BIG { 
		return self.a.redc();
	}

/* extract b */
	pub fn getb(&mut self) -> BIG {
		return self.b.redc();
	}

/* copy self=x */
	pub fn copy(&mut self,x :&FP2) {
		self.a.copy(&x.a);
		self.b.copy(&x.b);
	}

/* set self=0 */
	pub fn zero(&mut self) {
		self.a.zero();
		self.b.zero();
	}

/* set self=1 */
	pub fn one(&mut self) {
		self.a.one();
		self.b.zero();
	}	

/* negate self mod Modulus */
	pub fn neg(&mut self) {
		self.norm();
		let mut m=FP::new_copy(&self.a);
		let mut t=FP::new();

		m.add(&self.b);
		m.neg();
		m.norm();
		t.copy(&m); t.add(&self.b);
		self.b.copy(&m);
		self.b.add(&self.a);
		self.a.copy(&t);
	}	

/* set to a-ib */
	pub fn conj(&mut self) {
		self.b.neg();
	}

/* self+=a */
	pub fn add(&mut self,x:&FP2) {
		self.a.add(&x.a);
		self.b.add(&x.b);
	}

	pub fn dbl(&mut self) {
		self.a.dbl();
		self.b.dbl();
	}

/* self-=a */
	pub fn sub(&mut self,x:&FP2) {
		let mut m=FP2::new_copy(x);
		m.neg();
		self.add(&m);
	}

/* self*=s, where s is an FP */
	pub fn pmul(&mut self,s:&mut FP) {
		self.a.mul(s);
		self.b.mul(s);
	}

/* self*=i, where i is an int */
	pub fn imul(&mut self,c: isize) {
		self.a.imul(c);
		self.b.imul(c);
	}

/* self*=self */
	pub fn sqr(&mut self) {
		self.norm();
		let mut w1=FP::new_copy(&self.a);
		let mut w3=FP::new_copy(&self.a);
		let mut mb=FP::new_copy(&self.b);

		w3.mul(&mut self.b);
		w1.add(&self.b);
		mb.neg();
		self.a.add(&mb);
		self.a.mul(&mut w1);
		self.b.copy(&w3); self.b.add(&w3);

		self.norm();
	}	

/* this*=y */
	pub fn mul(&mut self,y :&mut FP2) {
		self.norm();  /* This is needed here as {a,b} is not normed before additions */

		let mut w1=FP::new_copy(&self.a);
		let mut w2=FP::new_copy(&self.b);
		let mut w5=FP::new_copy(&self.a);
		let mut mw=FP::new();

		w1.mul(&mut y.a);  // w1=a*y.a  - this norms w1 and y.a, NOT a
		w2.mul(&mut y.b);  // w2=b*y.b  - this norms w2 and y.b, NOT b
		w5.add(&self.b);    // w5=a+b
		self.b.copy(&y.a); self.b.add(&y.b); // b=y.a+y.b

		self.b.mul(&mut w5);
		mw.copy(&w1); mw.add(&w2); mw.neg();

		self.b.add(&mw); mw.add(&w1);
		self.a.copy(&w1); self.a.add(&mw);

		self.norm();
	}

/* sqrt(a+ib) = sqrt(a+sqrt(a*a-n*b*b)/2)+ib/(2*sqrt(a+sqrt(a*a-n*b*b)/2)) */
/* returns true if this is QR */
	pub fn sqrt(&mut self) -> bool {
		if self.iszilch() {return true}
		let mut w1=FP::new_copy(&self.b);
		let mut w2=FP::new_copy(&self.a);
		w1.sqr(); w2.sqr(); w1.add(&w2);
		if w1.jacobi()!=1 { self.zero(); return false }
		w2.copy(&w1.sqrt()); w1.copy(&w2);
		w2.copy(&self.a); w2.add(&w1); w2.div2();
		if w2.jacobi()!=1 {
			w2.copy(&self.a); w2.sub(&w1); w2.div2();
			if w2.jacobi()!=1 { self.zero(); return false }
		}
		w1.copy(&w2.sqrt());
		self.a.copy(&w1);
		w1.dbl();
		w1.inverse();
		self.b.mul(&mut w1);
		return true;
	}

/* output to hex string */
	pub fn tostring(&mut self) -> String {
		return format!("[{},{}]",self.a.tostring(),self.b.tostring());		
	}

	pub fn to_hex(&self) -> String {
		let mut ret: String = String::with_capacity(2 * BIG_HEX_STRING_LEN);
		ret.push_str(&format!("{} {}", self.a.to_hex(), self.b.to_hex()));
		return ret;
	}

	pub fn from_hex_iter(iter: &mut SplitWhitespace) -> FP2 {
		let mut ret:FP2 = FP2::new();
		ret.a = FP::from_hex_iter(iter);
		ret.b = FP::from_hex_iter(iter);
		return ret;
	}

	pub fn from_hex(val: String) -> FP2 {
		let mut iter = val.split_whitespace();
		return FP2::from_hex_iter(&mut iter);
	}

/* self=1/self */
	pub fn inverse(&mut self) {
		self.norm();
		let mut w1=FP::new_copy(&self.a);
		let mut w2=FP::new_copy(&self.b);

		w1.sqr();
		w2.sqr();
		w1.add(&w2);
		w1.inverse();
		self.a.mul(&mut w1);
		w1.neg();
		self.b.mul(&mut w1);
	}

/* self/=2 */
	pub fn div2(&mut self) {
		self.a.div2();
		self.b.div2();
	}

/* self*=sqrt(-1) */
	pub fn times_i(&mut self) {
	//	a.norm();
		let z=FP::new_copy(&self.a);
		self.a.copy(&self.b); self.a.neg();
		self.b.copy(&z);
	}

/* w*=(1+sqrt(-1)) */
/* where X*2-(1+sqrt(-1)) is irreducible for FP4, assumes p=3 mod 8 */
	pub fn mul_ip(&mut self) {
		self.norm();
		let t=FP2::new_copy(self);
		let z=FP::new_copy(&self.a);
		self.a.copy(&self.b);
		self.a.neg();
		self.b.copy(&z);
		self.add(&t);
		self.norm();
	}

/* w/=(1+sqrt(-1)) */
	pub fn div_ip(&mut self) {
		let mut t=FP2::new();
		self.norm();
		t.a.copy(&self.a); t.a.add(&self.b);
		t.b.copy(&self.b); t.b.sub(&self.a);
		self.copy(&t);
		self.div2();
	}

}
/*
fn main()
{
	let mut x=FP2::new();
}
*/
