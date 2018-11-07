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

use xxx::big;
use xxx::ecp;
use xxx::fp2::FP2;
use xxx::fp4::FP4;
use xxx::fp8::FP8;
use xxx::big::BIG;
use xxx::rom;

#[derive(Copy, Clone)]
pub struct FP24 {
	a:FP8,
	b:FP8,
	c:FP8
}

impl FP24 {

	pub fn new() -> FP24 {
		FP24 {
				a: FP8::new(),
				b: FP8::new(),
				c: FP8::new()
		}
	}

	pub fn new_int(a: isize) -> FP24 {
		let mut f=FP24::new();
		f.a.copy(&FP8::new_int(a));
		f.b.zero();
		f.c.zero();
		return f;
	}	

	pub fn new_copy(x: &FP24) -> FP24 {
		let mut f=FP24::new();
		f.a.copy(&x.a);
		f.b.copy(&x.b);
		f.c.copy(&x.c);
		return f;
	}

	pub fn new_fp8s(d: &FP8,e: &FP8,f: &FP8) -> FP24 {
		let mut g=FP24::new();
		g.a.copy(d);
		g.b.copy(e);
		g.c.copy(f);
		return g;
	}	

	pub fn new_fp8(d: &FP8) -> FP24 {
		let mut g=FP24::new();
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
	pub fn iszilch(&self) -> bool {
		//self.reduce();
		return self.a.iszilch() && self.b.iszilch() && self.c.iszilch();
	}	

/* Conditional move of g to self dependant on d */
	pub fn cmove(&mut self,g:&FP24,d: isize) {
		self.a.cmove(&g.a,d);
		self.b.cmove(&g.b,d);
		self.c.cmove(&g.c,d);
	}	

/* return 1 if b==c, no branching */
	fn teq(b: i32,c: i32) -> isize {
		let mut x=b^c;
		x-=1;  // if x=0, x now -1
		return ((x>>31)&1) as isize;
	}

/* Constant time select from pre-computed table */
	pub fn selector(&mut self,g: &[FP24],b: i32) {
		let m=b>>31;
		let mut babs=(b^m)-m;

		babs=(babs-1)/2;

		self.cmove(&g[0],FP24::teq(babs,0));  // conditional move
		self.cmove(&g[1],FP24::teq(babs,1));
		self.cmove(&g[2],FP24::teq(babs,2));
		self.cmove(&g[3],FP24::teq(babs,3));
		self.cmove(&g[4],FP24::teq(babs,4));
		self.cmove(&g[5],FP24::teq(babs,5));
		self.cmove(&g[6],FP24::teq(babs,6));
		self.cmove(&g[7],FP24::teq(babs,7));
 
 		let mut invf=FP24::new_copy(self);
		invf.conj();
		self.cmove(&invf,(m&1) as isize);
	}		


/* test self=1 ? */
	pub fn isunity(&self) -> bool {
		let one=FP8::new_int(1);
		return self.a.equals(&one) && self.b.iszilch() && self.c.iszilch();
	}

/* test self=x */
	pub fn equals(&self,x:&FP24) -> bool {
		return self.a.equals(&x.a) && self.b.equals(&x.b) && self.c.equals(&x.c);
	}

	pub fn geta(&mut self) -> FP8 {
		let f=FP8::new_copy(&self.a);
		return f;
	}

	pub fn getb(&mut self) -> FP8 {
		let f=FP8::new_copy(&self.b);
		return f;
	}

	pub fn getc(&mut self) -> FP8 {
		let f=FP8::new_copy(&self.c);
		return f;
	}	

/* copy self=x */
	pub fn copy(&mut self,x :&FP24) {
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
		let mut a=FP8::new_copy(&self.a);
		let mut b=FP8::new_copy(&self.c);
		let mut c=FP8::new_copy(&self.b);
		let mut d=FP8::new();

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
		let mut a=FP8::new_copy(&self.a);
		let mut b=FP8::new_copy(&self.b);
		let mut c=FP8::new_copy(&self.c);
		let mut d=FP8::new_copy(&self.a);

		a.sqr();
		b.mul(&self.c);
		b.dbl(); b.norm();
		c.sqr();
		d.mul(&self.b);
		d.dbl();

		self.c.add(&self.a);
		self.c.add(&self.b); self.c.norm();
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

/* FP24 full multiplication self=self*y */
	pub fn mul(&mut self,y: &FP24) {
		let mut z0=FP8::new_copy(&self.a);
		let mut z1=FP8::new();
		let mut z2=FP8::new_copy(&mut self.b);
		let mut z3=FP8::new();
		let mut t0=FP8::new_copy(&self.a);
		let mut t1=FP8::new_copy(&y.a);

		z0.mul(&y.a);
		z2.mul(&y.b);

		t0.add(&self.b);
		t1.add(&y.b);

		t0.norm(); t1.norm();

		z1.copy(&t0); z1.mul(&t1);
		t0.copy(&self.b); t0.add(&self.c);
		t1.copy(&y.b); t1.add(&y.c);

		t0.norm(); t1.norm();	

		z3.copy(&t0); z3.mul(&t1);

		t0.copy(&z0); t0.neg();
		t1.copy(&z2); t1.neg();

		z1.add(&t0);
		//z1.norm();
		self.b.copy(&z1); self.b.add(&t1);

		z3.add(&t1);
		z2.add(&t0);

		t0.copy(&self.a); t0.add(&self.c); t0.norm();
		t1.copy(&y.a); t1.add(&y.c); t1.norm();
		t0.mul(&t1);
		z2.add(&t0);

		t0.copy(&self.c); t0.mul(&y.c);
		t1.copy(&t0); t1.neg();


		self.c.copy(&z2); self.c.add(&t1);
		z3.add(&t1);
		t0.times_i();
		self.b.add(&t0);
		z3.norm();

		z3.times_i();
		self.a.copy(&z0); self.a.add(&z3);
		self.norm();
	}

/* Special case of multiplication arises from special form of ATE pairing line function */
	pub fn smul(&mut self,y: &FP24,twist: usize) {

		if twist==ecp::D_TYPE {
			let mut z0=FP8::new_copy(&self.a);
			let mut z2=FP8::new_copy(&self.b);
			let mut z3=FP8::new_copy(&self.b);
			let mut t0=FP8::new();
			let mut t1=FP8::new_copy(&y.a);
		
			z0.mul(&y.a);
			z2.pmul(&y.b.real());
			self.b.add(&self.a);
			t1.padd(&y.b.real());

			self.b.norm(); t1.norm();

			self.b.mul(&t1);
			z3.add(&self.c); z3.norm();
			z3.pmul(&y.b.real());

			t0.copy(&z0); t0.neg();
			t1.copy(&z2); t1.neg();
	
			self.b.add(&t0);
		//self.b.norm();

			self.b.add(&t1);
			z3.add(&t1);
			z2.add(&t0);

			t0.copy(&self.a); t0.add(&self.c);
			t0.norm(); z3.norm();
			
			t0.mul(&y.a);
			self.c.copy(&z2); self.c.add(&t0);

			z3.times_i();
			self.a.copy(&z0); self.a.add(&z3);
		}
		if twist==ecp::M_TYPE {
			let mut z0=FP8::new_copy(&self.a);
			let mut z1=FP8::new();
			let mut z2=FP8::new();
			let mut z3=FP8::new();
			let mut t0=FP8::new_copy(&self.a);
			let mut t1=FP8::new();
		
			z0.mul(&y.a);
			t0.add(&self.b);
			t0.norm();

			z1.copy(&t0); z1.mul(&y.a);
			t0.copy(&self.b); t0.add(&self.c);
			t0.norm();

			z3.copy(&t0); //z3.mul(y.c);
			z3.pmul(&y.c.getb());
			z3.times_i();

			t0.copy(&z0); t0.neg();

			z1.add(&t0);
			self.b.copy(&z1); 
			z2.copy(&t0);

			t0.copy(&self.a); t0.add(&self.c);
			t1.copy(&y.a); t1.add(&y.c);

			t0.norm();
			t1.norm();
	
			t0.mul(&t1);
			z2.add(&t0);

			t0.copy(&self.c);
			
			t0.pmul(&y.c.getb());
			t0.times_i();

			t1.copy(&t0); t1.neg();

			self.c.copy(&z2); self.c.add(&t1);
			z3.add(&t1);
			t0.times_i();
			self.b.add(&t0);
			z3.norm();
			z3.times_i();
			self.a.copy(&z0); self.a.add(&z3);
		}
		self.norm();		
	}

/* self=1/self */
	pub fn inverse(&mut self) {
		let mut f0=FP8::new_copy(&self.a);
		let mut f1=FP8::new_copy(&self.b);
		let mut f2=FP8::new_copy(&self.a);
		let mut f3=FP8::new();

		self.norm();
		f0.sqr();
		f1.mul(&self.c);
		f1.times_i();
		f0.sub(&f1); f0.norm();

		f1.copy(&self.c); f1.sqr();
		f1.times_i();
		f2.mul(&self.b);
		f1.sub(&f2); f1.norm();

		f2.copy(&self.b); f2.sqr();
		f3.copy(&self.a); f3.mul(&self.c);
		f2.sub(&f3); f2.norm();

		f3.copy(&self.b); f3.mul(&f2);
		f3.times_i();
		self.a.mul(&f0);
		f3.add(&self.a);
		self.c.mul(&f1);
		self.c.times_i();

		f3.add(&self.c); f3.norm();
		f3.inverse();
		self.a.copy(&f0); self.a.mul(&f3);
		self.b.copy(&f1); self.b.mul(&f3);
		self.c.copy(&f2); self.c.mul(&f3);
	}

	/* self=self^p using Frobenius */
	pub fn frob(&mut self,f: &FP2,n:isize) {
		let mut f2=FP2::new_copy(f);
		let mut f3=FP2::new_copy(f);

		f2.sqr();
		f3.mul(&f2);

		f3.mul_ip(); f3.norm();

		for _i in 0..n {
			self.a.frob(&f3);
			self.b.frob(&f3);
			self.c.frob(&f3);

			self.b.qmul(f); self.b.times_i2();
			self.c.qmul(&f2); self.c.times_i2(); self.c.times_i2();
		}
	}

/* trace function */
	pub fn trace(&mut self) -> FP8 {
		let mut t=FP8::new();
		t.copy(&self.a);
		t.imul(3);
		t.reduce();
		return t;
	}

/* convert from byte array to FP24 */
	pub fn frombytes(w: &[u8]) -> FP24 {
		let mut t:[u8;big::MODBYTES as usize]=[0;big::MODBYTES as usize];
		let mb=big::MODBYTES as usize;

		for i in 0..mb {t[i]=w[i]}
		let mut a=BIG::frombytes(&t);
		for i in 0..mb {t[i]=w[i+mb]}
		let mut b=BIG::frombytes(&t);
		let mut c=FP2::new_bigs(&a,&b);

		for i in 0..mb {t[i]=w[i+2*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+3*mb]}
		b.copy(&BIG::frombytes(&t));
		let mut d=FP2::new_bigs(&a,&b);

		let mut ea=FP4::new_fp2s(&c,&d);

		for i in 0..mb {t[i]=w[i+4*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+5*mb]}
		b.copy(&BIG::frombytes(&t));
		c.copy(&FP2::new_bigs(&a,&b));

		for i in 0..mb {t[i]=w[i+6*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+7*mb]}
		b.copy(&BIG::frombytes(&t));
		d.copy(&FP2::new_bigs(&a,&b));

		let mut eb=FP4::new_fp2s(&c,&d);


		let e=FP8::new_fp4s(&ea,&eb);


		for i in 0..mb {t[i]=w[i+8*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+9*mb]}
		b.copy(&BIG::frombytes(&t));
		c.copy(&FP2::new_bigs(&a,&b));

		for i in 0..mb {t[i]=w[i+10*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+11*mb]}
		b.copy(&BIG::frombytes(&t));
		d.copy(&FP2::new_bigs(&a,&b));

		ea.copy(&FP4::new_fp2s(&c,&d));

		for i in 0..mb {t[i]=w[i+12*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+13*mb]}
		b.copy(&BIG::frombytes(&t));
		c.copy(&FP2::new_bigs(&a,&b));

		for i in 0..mb {t[i]=w[i+14*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+15*mb]}
		b.copy(&BIG::frombytes(&t));
		d.copy(&FP2::new_bigs(&a,&b));

		eb.copy(&FP4::new_fp2s(&c,&d));


		let f=FP8::new_fp4s(&ea,&eb);


		for i in 0..mb {t[i]=w[i+16*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+17*mb]}
		b.copy(&BIG::frombytes(&t));
		
		c.copy(&FP2::new_bigs(&a,&b));

		for i in 0..mb {t[i]=w[i+18*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+19*mb]}
		b.copy(&BIG::frombytes(&t));
		d.copy(&FP2::new_bigs(&a,&b));

		ea.copy(&FP4::new_fp2s(&c,&d));

		for i in 0..mb {t[i]=w[i+20*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+21*mb]}
		b.copy(&BIG::frombytes(&t));
		
		c.copy(&FP2::new_bigs(&a,&b));

		for i in 0..mb {t[i]=w[i+22*mb]}
		a.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=w[i+23*mb]}
		b.copy(&BIG::frombytes(&t));
		d.copy(&FP2::new_bigs(&a,&b));

		eb.copy(&FP4::new_fp2s(&c,&d));


		let g=FP8::new_fp4s(&ea,&eb);

		return FP24::new_fp8s(&e,&f,&g);
	}

/* convert this to byte array */
	pub fn tobytes(&mut self,w: &mut [u8]) {
		let mut t:[u8;big::MODBYTES as usize]=[0;big::MODBYTES as usize];
		let mb=big::MODBYTES as usize;

		self.a.geta().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i]=t[i]}
		self.a.geta().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+mb]=t[i]}
		self.a.geta().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+2*mb]=t[i]}
		self.a.geta().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+3*mb]=t[i]}

		self.a.getb().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+4*mb]=t[i]}
		self.a.getb().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+5*mb]=t[i]}
		self.a.getb().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+6*mb]=t[i]}
		self.a.getb().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+7*mb]=t[i]}



		self.b.geta().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+8*mb]=t[i]}
		self.b.geta().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+9*mb]=t[i]}
		self.b.geta().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+10*mb]=t[i]}
		self.b.geta().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+11*mb]=t[i]}

		self.b.getb().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+12*mb]=t[i]}
		self.b.getb().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+13*mb]=t[i]}
		self.b.getb().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+14*mb]=t[i]}
		self.b.getb().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+15*mb]=t[i]}



		self.c.geta().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+16*mb]=t[i]}
		self.c.geta().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+17*mb]=t[i]}
		self.c.geta().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+18*mb]=t[i]}
		self.c.geta().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+19*mb]=t[i]}

		self.c.getb().geta().geta().tobytes(&mut t);
		for i in 0..mb {w[i+20*mb]=t[i]}
		self.c.getb().geta().getb().tobytes(&mut t);
		for i in 0..mb {w[i+21*mb]=t[i]}
		self.c.getb().getb().geta().tobytes(&mut t);
		for i in 0..mb {w[i+22*mb]=t[i]}
		self.c.getb().getb().getb().tobytes(&mut t);
		for i in 0..mb {w[i+23*mb]=t[i]}		
	}

/* output to hex string */
	pub fn tostring(&mut self) -> String {
		return format!("[{},{},{}]",self.a.tostring(),self.b.tostring(),self.c.tostring());		
	}

/* self=self^e */
	pub fn pow(&self,e: &BIG) -> FP24 {
		let mut r=FP24::new_copy(self);	
		r.norm();
		let mut e1=BIG::new_copy(e);
		e1.norm();
		let mut e3=BIG::new_copy(&e1);
		e3.pmul(3);
		e3.norm();
		let mut w=FP24::new_copy(&r);

		let nb=e3.nbits();
		for i in (1..nb-1).rev() {
			w.usqr();
			let bt=e3.bit(i)-e1.bit(i);
			if bt==1 {
				w.mul(&r);
			}
			if bt == -1 {
				r.conj(); w.mul(&r); r.conj();
			}
		}

		w.reduce();
		return w;
	}	

/* constant time powering by small integer of max length bts */
	pub fn pinpow(&mut self,e: i32,bts: i32) {
		let mut r:[FP24;2]=[FP24::new_int(1),FP24::new_copy(self)];
		let mut t=FP24::new();

		for i in (0..bts).rev() {
			let b:usize=((e>>i)&1) as usize;
			t.copy(&r[b]);
			r[1-b].mul(&t);
			r[b].usqr();
		}
		self.copy(&r[0]);
	}

	pub fn compow(&mut self,e: &BIG, r: &BIG) -> FP8 {
		let f=FP2::new_bigs(&BIG::new_ints(&rom::FRA),&BIG::new_ints(&rom::FRB));
		let q=BIG::new_ints(&rom::MODULUS);

		let mut g1=FP24::new_copy(self);
		let mut g2=FP24::new_copy(self);


		let mut m=BIG::new_copy(&q);
		m.rmod(&r);

		let mut a=BIG::new_copy(&e);
		a.rmod(&mut m);

		let mut b=BIG::new_copy(&e);
		b.div(&mut m);

		let mut c=g1.trace();

		if b.iszilch() {
			c=c.xtr_pow(&mut a);
			return c;
		}

		g2.frob(&f,1);
		let cp=g2.trace();
		g1.conj();
		g2.mul(&g1);
		let cpm1=g2.trace();
		g2.mul(&g1);
		let cpm2=g2.trace();

		c=c.xtr_pow2(&cp,&cpm1,&cpm2,&mut a,&mut b);

		return c;
	}

/* p=q0^u0.q1^u1.q2^u2.q3^u3... */
// Bos & Costello https://eprint.iacr.org/2013/458.pdf
// Faz-Hernandez & Longa & Sanchez  https://eprint.iacr.org/2013/158.pdf
// Side channel attack secure 
 	pub fn pow8(q:&[FP24],u:&[BIG]) -> FP24 {
		let mut g1:[FP24;8]=[FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new()];
		let mut g2:[FP24;8]=[FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new(),FP24::new()];

		let mut r=FP24::new();
		let mut p=FP24::new();
		const CT:usize=1+big::NLEN*(big::BASEBITS as usize);		
		let mut w1:[i8;CT]=[0;CT];
		let mut s1:[i8;CT]=[0;CT];
		let mut w2:[i8;CT]=[0;CT];
		let mut s2:[i8;CT]=[0;CT];


		let mut mt=BIG::new();
		let mut t:[BIG;8]=[BIG::new_copy(&u[0]),BIG::new_copy(&u[1]),BIG::new_copy(&u[2]),BIG::new_copy(&u[3]),BIG::new_copy(&u[4]),BIG::new_copy(&u[5]),BIG::new_copy(&u[6]),BIG::new_copy(&u[7])];

		for i in 0..8 {
			t[i].norm();
		}

// precomputation
		g1[0].copy(&q[0]); r.copy(&g1[0]);
		g1[1].copy(&r); g1[1].mul(&q[1]);  // q[0].q[1]
		g1[2].copy(&r); g1[2].mul(&q[2]); r.copy(&g1[1]); // q[0].q[2]
		g1[3].copy(&r); g1[3].mul(&q[2]); r.copy(&g1[0]); // q[0].q[1].q[2]
		g1[4].copy(&r); g1[4].mul(&q[3]); r.copy(&g1[1]); // q[0].q[3]
		g1[5].copy(&r); g1[5].mul(&q[3]); r.copy(&g1[2]); // q[0].q[1].q[3]
		g1[6].copy(&r); g1[6].mul(&q[3]); r.copy(&g1[3]); // q[0].q[2].q[3]
		g1[7].copy(&r); g1[7].mul(&q[3]); // q[0].q[1].q[2].q[3]

// Use Frobenius
		let f=FP2::new_bigs(&BIG::new_ints(&rom::FRA),&BIG::new_ints(&rom::FRB));
		for i in 0..8 {
			g2[i].copy(&g1[i]); g2[i].frob(&f,4);
		}


// Make it odd
		let pb1=1-t[0].parity();
		t[0].inc(pb1);
		t[0].norm();	

		let pb2=1-t[4].parity();
		t[4].inc(pb2);
		t[4].norm();		

// Number of bits
		mt.zero();
		for i in 0..8 {
			mt.or(&t[i]);
		}

		let nb=1+mt.nbits();

// Sign pivot 

		s1[nb-1]=1;
		s2[nb-1]=1;
		for i in 0..nb-1 {
			t[0].fshr(1);
			s1[i]=(2*t[0].parity()-1) as i8;
			t[4].fshr(1);
			s2[i]=(2*t[4].parity()-1) as i8;

		}

// Recoded exponent
		for i in 0..nb {
			w1[i]=0;
			let mut k=1;
			for j in 1..4 {
				let bt=s1[i]*(t[j].parity() as i8);
				t[j].fshr(1);
				t[j].dec((bt>>1) as isize);
				t[j].norm();
				w1[i]+=bt*(k as i8);
				k=2*k;
			}

			w2[i]=0;
			k=1;
			for j in 5..8 {
				let bt=s2[i]*(t[j].parity() as i8);
				t[j].fshr(1);
				t[j].dec((bt>>1) as isize);
				t[j].norm();
				w2[i]+=bt*(k as i8);
				k=2*k;
			}			
		}	


// Main loop
		p.selector(&g1,(2*w1[nb-1]+1) as i32);
		r.selector(&g2,(2*w2[nb-1]+1) as i32);
		p.mul(&r);
		for i in (0..nb-1).rev() {
			p.usqr();
			r.selector(&g1,(2*w1[i]+s1[i]) as i32);
			p.mul(&r);
			r.selector(&g2,(2*w2[i]+s2[i]) as i32);
			p.mul(&r);			
		}

// apply correction
		r.copy(&q[0]); r.conj();   
		r.mul(&p);
		p.cmove(&r,pb1);

		r.copy(&q[4]); r.conj();   
		r.mul(&p);
		p.cmove(&r,pb2);

		p.reduce();
		return p;
	}
}
