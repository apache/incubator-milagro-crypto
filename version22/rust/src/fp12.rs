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
pub struct FP12 {
	a:FP4,
	b:FP4,
	c:FP4
}

use rom::BIG_HEX_STRING_LEN;

//mod fp;
//use fp::FP;
//mod fp2;
use fp2::FP2;
//mod fp4;
use fp4::FP4;
//mod big;
use big::BIG;
//mod dbig;
//use dbig::DBIG;
//mod rand;
//mod hash256;
//mod rom;
use rom;

impl PartialEq for FP12 {
	fn eq(&self, other: &FP12) -> bool {
		return (self.a == other.a) &&
			(self.b == other.b) &&
			(self.c == other.c);
	}
}

impl fmt::Display for FP12 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "FP12: [ {}, {}, {} ]", self.a, self.b, self.c)
	}
}

impl fmt::Debug for FP12 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "FP12: [ {}, {}, {} ]", self.a, self.b, self.c)
	}
}

impl FP12 {

	pub fn new() -> FP12 {
		FP12 {
				a: FP4::new(),
				b: FP4::new(),
				c: FP4::new()
		}
	}

	pub fn new_int(a: isize) -> FP12 {
		let mut f=FP12::new();
		f.a.copy(&FP4::new_int(a));
		f.b.zero();
		f.c.zero();
		return f;
	}	

	pub fn new_copy(x: &FP12) -> FP12 {
		let mut f=FP12::new();
		f.a.copy(&x.a);
		f.b.copy(&x.b);
		f.c.copy(&x.c);
		return f;
	}

	pub fn new_fp4s(d: &FP4,e: &FP4,f: &FP4) -> FP12 {
		let mut g=FP12::new();
		g.a.copy(d);
		g.b.copy(e);
		g.c.copy(f);
		return g;
	}	

	pub fn new_fp4(d: &FP4) -> FP12 {
		let mut g=FP12::new();
		g.a.copy(d);
		g.b.zero();
		g.c.zero();
		return g;
	}

/* reduce components mod Modulus */
	pub fn reduce(&mut self) {
		self.a.reduce();
		self.b.reduce();
		self.c.reduce();
	}

/* normalise components of w */
	pub fn norm(&mut self) {
		self.a.norm();
		self.b.norm();
		self.c.norm();
	}	

/* test self=0 ? */
	pub fn iszilch(&mut self) -> bool {
		self.reduce();
		return self.a.iszilch() && self.b.iszilch() && self.c.iszilch();
	}	

/* test self=1 ? */
	pub fn isunity(&mut self) -> bool {
		let mut one=FP4::new_int(1);
		return self.a.equals(&mut one) && self.b.iszilch() && self.c.iszilch();
	}

/* test self=x */
	pub fn equals(&mut self,x:&mut FP12) -> bool {
		return self.a.equals(&mut x.a) && self.b.equals(&mut x.b) && self.c.equals(&mut x.c);
	}

	pub fn geta(&mut self) -> FP4 {
		let f=FP4::new_copy(&self.a);
		return f;
	}

	pub fn getb(&mut self) -> FP4 {
		let f=FP4::new_copy(&self.b);
		return f;
	}

	pub fn getc(&mut self) -> FP4 {
		let f=FP4::new_copy(&self.c);
		return f;
	}	

/* copy self=x */
	pub fn copy(&mut self,x :&FP12) {
		self.a.copy(&x.a);
		self.b.copy(&x.b);
		self.c.copy(&x.c);
	}

/* set self=1 */
	pub fn one(&mut self) {
		self.a.one();
		self.b.zero();
		self.c.zero();
	}

/* this=conj(this) */
	pub fn conj(&mut self) {
		self.a.conj();
		self.b.nconj();
		self.c.conj();
	}

/* Granger-Scott Unitary Squaring */
	pub fn usqr(&mut self) {
		let mut a=FP4::new_copy(&self.a);
		let mut b=FP4::new_copy(&self.c);
		let mut c=FP4::new_copy(&self.b);
		let mut d=FP4::new();

		self.a.sqr();
		d.copy(&self.a); d.add(&self.a);
		self.a.add(&d);

		self.a.norm();
		a.nconj();

		a.dbl();
		self.a.add(&a);
		b.sqr();
		b.times_i();

		d.copy(&b); d.add(&b);
		b.add(&d);
		b.norm();

		c.sqr();
		d.copy(&c); d.add(&c);
		c.add(&d);
		c.norm();

		self.b.conj();
		self.b.dbl();
		self.c.nconj();

		self.c.dbl();
		self.b.add(&b);
		self.c.add(&c);
		self.reduce();

	}

/* Chung-Hasan SQR2 method from http://cacr.uwaterloo.ca/techreports/2006/cacr2006-24.pdf */
	pub fn sqr(&mut self) {
		let mut a=FP4::new_copy(&self.a);
		let mut b=FP4::new_copy(&self.b);
		let mut c=FP4::new_copy(&self.c);
		let mut d=FP4::new_copy(&self.a);

		a.sqr();
		b.mul(&mut self.c);
		b.dbl();
		c.sqr();
		d.mul(&mut self.b);
		d.dbl();

		self.c.add(&self.a);
		self.c.add(&self.b);
		self.c.sqr();

		self.a.copy(&a);
		a.add(&b);
		a.norm();
		a.add(&c);
		a.add(&d);
		a.norm();

		a.neg();
		b.times_i();
		c.times_i();

		self.a.add(&b);

		self.b.copy(&c); self.b.add(&d);
		self.c.add(&a);
		self.norm();
	}


/* FP12 full multiplication self=self*y */
	pub fn mul(&mut self,y: &mut FP12) {
		let mut z0=FP4::new_copy(&self.a);
		let mut z1=FP4::new();
		let mut z2=FP4::new_copy(&mut self.b);
		let mut z3=FP4::new();
		let mut t0=FP4::new_copy(&self.a);
		let mut t1=FP4::new_copy(&y.a);

		z0.mul(&mut y.a);
		z2.mul(&mut y.b);

		t0.add(&self.b);
		t1.add(&y.b);

		z1.copy(&t0); z1.mul(&mut t1);
		t0.copy(&self.b); t0.add(&self.c);

		t1.copy(&y.b); t1.add(&y.c);
		z3.copy(&t0); z3.mul(&mut t1);

		t0.copy(&z0); t0.neg();
		t1.copy(&z2); t1.neg();

		z1.add(&t0);
		z1.norm();
		self.b.copy(&z1); self.b.add(&t1);

		z3.add(&t1);
		z2.add(&t0);

		t0.copy(&self.a); t0.add(&self.c);
		t1.copy(&y.a); t1.add(&y.c);
		t0.mul(&mut t1);
		z2.add(&t0);

		t0.copy(&self.c); t0.mul(&mut y.c);
		t1.copy(&t0); t1.neg();

		z2.norm();
		z3.norm();
		self.b.norm();

		self.c.copy(&z2); self.c.add(&t1);
		z3.add(&t1);
		t0.times_i();
		self.b.add(&t0);

		z3.times_i();
		self.a.copy(&z0); self.a.add(&z3);
		self.norm();
	}

/* Special case of multiplication arises from special form of ATE pairing line function */
	pub fn smul(&mut self,y: &mut FP12) {
		let mut z0=FP4::new_copy(&self.a);
		let mut z2=FP4::new_copy(&self.b);
		let mut z3=FP4::new_copy(&self.b);
		let mut t0=FP4::new();
		let mut t1=FP4::new_copy(&y.a);
		
		z0.mul(&mut y.a);
		z2.pmul(&mut y.b.real());
		self.b.add(&self.a);
		t1.padd(&y.b.real());

		self.b.mul(&mut t1);
		z3.add(&self.c);
		z3.pmul(&mut y.b.real());

		t0.copy(&z0); t0.neg();
		t1.copy(&z2); t1.neg();

		self.b.add(&t0);
		self.b.norm();

		self.b.add(&t1);
		z3.add(&t1);
		z2.add(&t0);

		t0.copy(&self.a); t0.add(&self.c);
		t0.mul(&mut y.a);
		self.c.copy(&z2); self.c.add(&t0);

		z3.times_i();
		self.a.copy(&z0); self.a.add(&z3);

		self.norm();
	}

/* self=1/self */
	pub fn inverse(&mut self) {
		let mut f0=FP4::new_copy(&self.a);
		let mut f1=FP4::new_copy(&self.b);
		let mut f2=FP4::new_copy(&self.a);
		let mut f3=FP4::new();

		self.norm();
		f0.sqr();
		f1.mul(&mut self.c);
		f1.times_i();
		f0.sub(&f1);

		f1.copy(&self.c); f1.sqr();
		f1.times_i();
		f2.mul(&mut self.b);
		f1.sub(&f2);

		f2.copy(&self.b); f2.sqr();
		f3.copy(&self.a); f3.mul(&mut self.c);
		f2.sub(&f3);

		f3.copy(&self.b); f3.mul(&mut f2);
		f3.times_i();
		self.a.mul(&mut f0);
		f3.add(&self.a);
		self.c.mul(&mut f1);
		self.c.times_i();

		f3.add(&self.c);
		f3.inverse();
		self.a.copy(&f0); self.a.mul(&mut f3);
		self.b.copy(&f1); self.b.mul(&mut f3);
		self.c.copy(&f2); self.c.mul(&mut f3);
	}

/* self=self^p using Frobenius */
	pub fn frob(&mut self,f: &mut FP2) {
		let mut f2=FP2::new_copy(f);
		let mut f3=FP2::new_copy(f);

		f2.sqr();
		f3.mul(&mut f2);

		self.a.frob(&mut f3);
		self.b.frob(&mut f3);
		self.c.frob(&mut f3);

		self.b.pmul(f);
		self.c.pmul(&mut f2);
	}

/* trace function */
	pub fn trace(&mut self) -> FP4 {
		let mut t=FP4::new();
		t.copy(&self.a);
		t.imul(3);
		t.reduce();
		return t;
	}

/* convert from byte array to FP12 */
	pub fn frombytes(w: &[u8]) -> FP12 {
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		let mb=rom::MODBYTES as usize;

		for i in 0..mb {t[i]=w[i]}
		let mut a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+mb]}
		let mut b=BIG::frombytes(&t);
		let mut c=FP2::new_bigs(&a,&b);

		for i in 0..mb {t[i]=w[i+2*mb]}
		a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+3*mb]}
		b=BIG::frombytes(&t);
		let mut d=FP2::new_bigs(&a,&b);

		let e=FP4::new_fp2s(&c,&d);


		for i in 0..mb {t[i]=w[i+4*mb]}
		a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+5*mb]}
		b=BIG::frombytes(&t);
		c=FP2::new_bigs(&a,&b);

		for i in 0..mb {t[i]=w[i+6*mb]}
		a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+7*mb]}
		b=BIG::frombytes(&t);
		d=FP2::new_bigs(&a,&b);

		let f=FP4::new_fp2s(&c,&d);


		for i in 0..mb {t[i]=w[i+8*mb]}
		a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+9*mb]}
		b=BIG::frombytes(&t);
		
		c=FP2::new_bigs(&a,&b);

		for i in 0..mb {t[i]=w[i+10*mb]}
		a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+11*mb]}
		b=BIG::frombytes(&t);
		d=FP2::new_bigs(&a,&b);

		let g=FP4::new_fp2s(&c,&d);

		return FP12::new_fp4s(&e,&f,&g);
	}

/* convert this to byte array */
	pub fn tobytes(&mut self,w: &mut [u8]) {
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		let mb=rom::MODBYTES as usize;

		self.a.geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i]=t[i]}
		self.a.geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+mb]=t[i]}
		self.a.getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+2*mb]=t[i]}
		self.a.getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+3*mb]=t[i]}

		self.b.geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+4*mb]=t[i]}
		self.b.geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+5*mb]=t[i]}
		self.b.getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+6*mb]=t[i]}
		self.b.getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+7*mb]=t[i]}

		self.c.geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+8*mb]=t[i]}
		self.c.geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+9*mb]=t[i]}
		self.c.getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+10*mb]=t[i]}
		self.c.getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+11*mb]=t[i]}
	}

/* output to hex string */
	pub fn tostring(&mut self) -> String {
		return format!("[{},{},{}]",self.a.tostring(),self.b.tostring(),self.c.tostring());		
	}

	pub fn to_hex(&self) -> String {
		let mut ret: String = String::with_capacity(12 * BIG_HEX_STRING_LEN);
		ret.push_str(&format!("{} {} {}", self.a.to_hex(), self.b.to_hex(), self.c.to_hex()));
		return ret;
	}

	pub fn from_hex_iter(iter: &mut SplitWhitespace) -> FP12 {
		let mut ret:FP12 = FP12::new();
		ret.a = FP4::from_hex_iter(iter);
		ret.b = FP4::from_hex_iter(iter);
		ret.c = FP4::from_hex_iter(iter);
		return ret;
	}

	pub fn from_hex(val: String) -> FP12 {
		let mut iter = val.split_whitespace();
		return FP12::from_hex_iter(&mut iter);
	}

/* self=self^e */
	pub fn pow(&mut self,e: &mut BIG) -> FP12 {
		self.norm();
		e.norm();
		let mut w=FP12::new_copy(self);
		let mut z=BIG::new_copy(&e);
		let mut r=FP12::new_int(1);
		loop {
			let bt=z.parity();
			z.fshr(1);
			if bt==1 {r.mul(&mut w)};
			if z.iszilch() {break}
			w.usqr();
		}
		r.reduce();
		return r;
	}	

/* constant time powering by small integer of max length bts */
	pub fn pinpow(&mut self,e: i32,bts: i32) {
		let mut r:[FP12;2]=[FP12::new_int(1),FP12::new_copy(self)];
		let mut t=FP12::new();

		for i in (0..bts).rev() {
			let b:usize=((e>>i)&1) as usize;
			t.copy(&r[b]);
			r[1-b].mul(&mut t);
			r[b].usqr();
		}
		self.copy(&r[0]);
	}

/* p=q0^u0.q1^u1.q2^u2.q3^u3 */
/* Timing attack secure, but not cache attack secure */

 	pub fn pow4(q:&mut [FP12],u:&[BIG]) -> FP12 {
		let mut a:[i8;4]=[0;4];
		let mut s:[FP12;2]=[FP12::new(),FP12::new()];
		let mut g:[FP12;8]=[FP12::new(),FP12::new(),FP12::new(),FP12::new(),FP12::new(),FP12::new(),FP12::new(),FP12::new()];

		let mut c=FP12::new_int(1);
		let mut p=FP12::new();
		const CT:usize=1+rom::NLEN*(rom::BASEBITS as usize);		
		let mut w:[i8;CT]=[0;CT];

		let mut mt=BIG::new();
		let mut t:[BIG;4]=[BIG::new_copy(&u[0]),BIG::new_copy(&u[1]),BIG::new_copy(&u[2]),BIG::new_copy(&u[3])];

		g[0].copy(&q[0]); s[0].copy(&q[1]); s[0].conj(); g[0].mul(&mut s[0]);
		p.copy(&g[0]);
		g[1].copy(&p);
		g[2].copy(&p);
		g[3].copy(&p);
		g[4].copy(&q[0]); g[4].mul(&mut q[1]);
		p.copy(&g[4]);
		g[5].copy(&p);
		g[6].copy(&p);
		g[7].copy(&p);


		s[1].copy(&q[2]); s[0].copy(&q[3]); s[0].conj(); p.copy(&s[0]); s[1].mul(&mut p);
		p.copy(&s[1]); s[0].copy(&p); s[0].conj(); g[1].mul(&mut s[0]);
		g[2].mul(&mut s[1]);
		g[5].mul(&mut s[0]);
		g[6].mul(&mut s[1]);
		s[1].copy(&q[2]); s[1].mul(&mut q[3]);
		p.copy(&s[1]); s[0].copy(&p); s[0].conj(); g[0].mul(&mut s[0]);
		g[3].mul(&mut s[1]);
		g[4].mul(&mut s[0]);
		g[7].mul(&mut s[1]);

/* if power is even add 1 to power, and add q to correction */

		for i in 0..4 {
			if t[i].parity()==0 {
				t[i].inc(1); t[i].norm();
				c.mul(&mut q[i]);
			}
			mt.add(&t[i]); mt.norm();
		}
		c.conj();
		let nb=1+mt.nbits();

/* convert exponent to signed 1-bit window */
		for j in 0..nb {
			for i in 0..4 {
				a[i]=(t[i].lastbits(2)-2) as i8;
				t[i].dec(a[i] as isize); t[i].norm();
				t[i].fshr(1);
			}
			w[j]=8*a[0]+4*a[1]+2*a[2]+a[3];
		}
		w[nb]=(8*t[0].lastbits(2)+4*t[1].lastbits(2)+2*t[2].lastbits(2)+t[3].lastbits(2)) as i8;
		p.copy(&g[((w[nb] as usize)-1)/2]);

		for i in (0..nb).rev() {
			let m=w[i]>>7;
			let mut j=((w[i]^m)-m) as usize;  /* j=abs(w[i]) */
			j=(j-1)/2;
			s[0].copy(&g[j]); s[1].copy(&g[j]); s[1].conj();
			p.usqr();
			p.mul(&mut s[(m&1) as usize]);
		}
		p.mul(&mut c);  /* apply correction */
		p.reduce();
		return p;
	}


}
/*
fn main()
{
	let mut w=FP12::new();
}
*/
