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
pub struct ECP2 {
	x:FP2,
	y:FP2,
	z:FP2,
	inf: bool
}


use rom;
use rom::BIG_HEX_STRING_LEN;
//mod fp2;
use fp2::FP2;
//mod fp;
//use fp::FP;
//mod big;
use big::BIG;
//mod dbig;
//use dbig::DBIG;
//mod rand;
//mod hash256;
//mod rom;

impl fmt::Display for ECP2 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "ECP2: [ {}, {}, {}, {} ]", self.inf, self.x, self.y, self.z)
	}
}

impl fmt::Debug for ECP2 {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "ECP2: [ {}, {}, {}, {} ]", self.inf, self.x, self.y, self.z)
	}
}

impl PartialEq for ECP2 {
	fn eq(&self, other: &ECP2) -> bool {
		return (self.inf == other.inf) &&
			(self.x == other.x) &&
			(self.y == other.y) &&
			(self.z == other.z);
	}
}

#[allow(non_snake_case)]
impl ECP2 {

	pub fn new() -> ECP2 {
		ECP2 {
				x: FP2::new(),
				y: FP2::new(),
				z: FP2::new(),
				inf: true
		}
	}
#[allow(non_snake_case)]
/* construct this from (x,y) - but set to O if not on curve */
	pub fn new_fp2s(ix:&FP2,iy:&FP2) -> ECP2 {
		let mut E=ECP2::new();
		E.x.copy(&ix);
		E.y.copy(&iy);
		E.z.one();

		let mut rhs=ECP2::rhs(&mut E.x);
		let mut y2=FP2::new_copy(&E.y);
		y2.sqr();
		if y2.equals(&mut rhs) {
			E.inf=false;
		} else {E.x.zero();E.inf=true}
		return E;
}

/* construct this from x - but set to O if not on curve */
	pub fn new_fp2(ix:&FP2) -> ECP2 {	
		let mut E=ECP2::new();
		E.x.copy(&ix);
		E.y.one();
		E.z.one();

		let mut rhs=ECP2::rhs(&mut E.x);
		if rhs.sqrt() {
			E.y.copy(&rhs);
			E.inf=false;
		} else {E.x.zero();E.inf=true}
		return E;
	}

/* Test this=O? */
	pub fn is_infinity(&mut self) -> bool {
		return self.inf;
	}

/* copy self=P */
	pub fn copy(&mut self,P: &ECP2) {
		self.x.copy(&P.x);
		self.y.copy(&P.y);
		self.z.copy(&P.z);
		self.inf=P.inf;
	}

/* set self=O */
	pub fn inf(&mut self) {
		self.inf=true;
		self.x.zero();
		self.y.zero();
		self.z.zero();
	}

/* set self=-self */
	pub fn neg(&mut self) {
		if self.is_infinity() {return}
		self.y.neg(); self.y.reduce();
	}	

/* Conditional move of Q to self dependant on d */
	pub fn cmove(&mut self,Q: &ECP2,d: isize) {
		self.x.cmove(&Q.x,d);
		self.y.cmove(&Q.y,d);
		self.z.cmove(&Q.z,d);

		let bd:bool;
		if d==0 {bd=false}
		else {bd=true}

		self.inf=self.inf!=(self.inf!=Q.inf)&&bd;
	}

/* return 1 if b==c, no branching */
	fn teq(b: i32,c: i32) -> isize {
		let mut x=b^c;
		x-=1;  // if x=0, x now -1
		return ((x>>31)&1) as isize;
	}

/* Constant time select from pre-computed table */
	pub fn selector(&mut self,W: &[ECP2],b: i32) {
		let mut MP=ECP2::new(); 
		let m=b>>31;
		let mut babs=(b^m)-m;

		babs=(babs-1)/2;

		self.cmove(&W[0],ECP2::teq(babs,0));  // conditional move
		self.cmove(&W[1],ECP2::teq(babs,1));
		self.cmove(&W[2],ECP2::teq(babs,2));
		self.cmove(&W[3],ECP2::teq(babs,3));
		self.cmove(&W[4],ECP2::teq(babs,4));
		self.cmove(&W[5],ECP2::teq(babs,5));
		self.cmove(&W[6],ECP2::teq(babs,6));
		self.cmove(&W[7],ECP2::teq(babs,7));
 
		MP.copy(self);
		MP.neg();
		self.cmove(&MP,(m&1) as isize);
	}	

/* Test if P == Q */
	pub fn equals(&mut self,Q :&mut ECP2) -> bool {
		if self.is_infinity() && Q.is_infinity() {return true}
		if self.is_infinity() || Q.is_infinity() {return false}

		let mut zs2=FP2::new_copy(&self.z); zs2.sqr();
		let mut zo2=FP2::new_copy(&Q.z); zo2.sqr();
		let mut zs3=FP2::new_copy(&zs2); zs3.mul(&mut self.z);
		let mut zo3=FP2::new_copy(&zo2); zo3.mul(&mut Q.z);
		zs2.mul(&mut Q.x);
		zo2.mul(&mut self.x);
		if !zs2.equals(&mut zo2) {return false}
		zs3.mul(&mut Q.y);
		zo3.mul(&mut self.y);
		if !zs3.equals(&mut zo3) {return false}

		return true;
	}

/* set to Affine - (x,y,z) to (x,y) */
	pub fn affine(&mut self) {
		if self.is_infinity() {return}
		let mut one=FP2::new_int(1);
		if self.z.equals(&mut one) {return}
		self.z.inverse();

		let mut z2=FP2::new_copy(&self.z);
		z2.sqr();
		self.x.mul(&mut z2); self.x.reduce();
		self.y.mul(&mut z2); 
		self.y.mul(&mut self.z); self.y.reduce();
		self.z.copy(&one);
	}

/* extract affine x as FP2 */
	pub fn getx(&mut self) -> FP2 {
		self.affine();
		return FP2::new_copy(&self.x);
	}

/* extract affine y as FP2 */
	pub fn gety(&mut self) -> FP2 {
		self.affine();
		return FP2::new_copy(&self.y);
	}

/* extract projective x */
	pub fn getpx(&mut self) -> FP2 {
		return FP2::new_copy(&self.x);
	}
/* extract projective y */
	pub fn getpy(&mut self) -> FP2 {
		return FP2::new_copy(&self.y);
	}
/* extract projective z */
	pub fn getpz(&mut self) -> FP2 {
		return FP2::new_copy(&self.z);
	}

/* convert to byte array */
	pub fn tobytes(&mut self,b: &mut [u8]) {
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		let mb=rom::MODBYTES as usize;

		self.affine();
		self.x.geta().tobytes(&mut t);
		for i in 0..mb { b[i]=t[i]}
		self.x.getb().tobytes(&mut t);
		for i in 0..mb { b[i+mb]=t[i]}

		self.y.geta().tobytes(&mut t);
		for i in 0..mb {b[i+2*mb]=t[i]}
		self.y.getb().tobytes(&mut t);
		for i in 0..mb {b[i+3*mb]=t[i]}
	}

/* convert from byte array to point */
	pub fn frombytes(b: &[u8]) -> ECP2 {
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		let mb=rom::MODBYTES as usize;

		for i in 0..mb {t[i]=b[i]}
		let mut ra=BIG::frombytes(&t);
		for i in 0..mb {t[i]=b[i+mb]}
		let mut rb=BIG::frombytes(&t);
		let rx=FP2::new_bigs(&ra,&rb);

		for i in 0..mb {t[i]=b[i+2*mb]}
		ra.copy(&BIG::frombytes(&t));
		for i in 0..mb {t[i]=b[i+3*mb]}
		rb.copy(&BIG::frombytes(&t));
		let ry=FP2::new_bigs(&ra,&rb);

		return ECP2::new_fp2s(&rx,&ry);
	}

/* convert this to hex string */
	pub fn tostring(&mut  self) -> String {
		if self.is_infinity() {return String::from("infinity")}
		self.affine();
		return format!("({},{})",self.x.tostring(),self.y.tostring());
}

	pub fn to_hex(&self) -> String {
		let mut ret: String = String::with_capacity(7 * BIG_HEX_STRING_LEN);
		ret.push_str(&format!("{} {} {} {}", self.inf, self.x.to_hex(), self.y.to_hex(), self.z.to_hex()));
		return ret;
	}

	pub fn from_hex_iter(iter: &mut SplitWhitespace) -> ECP2 {
		let mut ret:ECP2 = ECP2::new();
		if let Some(x) = iter.next() {
			ret.inf = x == "true";
			ret.x = FP2::from_hex_iter(iter);
			ret.y = FP2::from_hex_iter(iter);
			ret.z = FP2::from_hex_iter(iter);
		}
		return ret;
	}

	pub fn from_hex(val: String) -> ECP2 {
		let mut iter = val.split_whitespace();
		return ECP2::from_hex_iter(&mut iter);
	}

/* Calculate RHS of twisted curve equation x^3+B/i */
	pub fn rhs(x:&mut FP2) -> FP2 {
		x.norm();
		let mut r=FP2::new_copy(x);
		r.sqr();
		let mut b=FP2::new_big(&BIG::new_ints(&rom::CURVE_B));
		b.div_ip();
		r.mul(x);
		r.add(&b);

		r.reduce();
		return r;
	}

/* self+=self */
	pub fn dbl(&mut self) -> isize {
		if self.inf {return -1}
		if self.y.iszilch() {
			self.inf();
			return -1
		}

		let mut w1=FP2::new_copy(&self.x);
		let mut w2=FP2::new();
		let mut w3=FP2::new_copy(&self.x);
		let mut w8=FP2::new_copy(&self.x);

		w1.sqr();
		w8.copy(&w1);
		w8.imul(3);

		w2.copy(&self.y); w2.sqr();
		w3.copy(&self.x); w3.mul(&mut w2);
		w3.imul(4);
		w1.copy(&w3); w1.neg();
		w1.norm();

		self.x.copy(&w8); self.x.sqr();
		self.x.add(&w1);
		self.x.add(&w1);
		self.x.norm();

		self.z.mul(&mut self.y);
		self.z.dbl();

		w2.dbl();
		w2.sqr();
		w2.dbl();
		w3.sub(&self.x);
		self.y.copy(&w8); self.y.mul(&mut w3);
		w2.norm();
		self.y.sub(&w2);

		self.y.norm();
		self.z.norm();

		return 1;
	}

/* self+=Q - return 0 for add, 1 for double, -1 for O */
	pub fn add(&mut self,Q:&mut ECP2) -> isize {
		if self.inf {
			self.copy(Q);
			return -1;
		}
		if Q.inf {return -1}

		let mut aff=false;

		if Q.z.isunity() {aff=true}

		let mut a=FP2::new();
		let mut c=FP2::new();
		let mut b=FP2::new_copy(&self.z);
		let mut d=FP2::new_copy(&self.z);

		if !aff {
			a.copy(&Q.z);
			c.copy(&Q.z);

			a.sqr(); b.sqr();
			c.mul(&mut a); d.mul(&mut b);

			a.mul(&mut self.x);
			c.mul(&mut self.y);
		} else {
			a.copy(&self.x);
			c.copy(&self.y);
	
			b.sqr();
			d.mul(&mut b);
		}

		b.mul(&mut Q.x); b.sub(&a);
		d.mul(&mut Q.y); d.sub(&c);

		if b.iszilch() {
			if d.iszilch() {
				self.dbl();
				return 1;
			} else	{
				self.inf=true;
				return -1;
			}
		}

		if !aff {self.z.mul(&mut Q.z)}
		self.z.mul(&mut b);

		let mut e=FP2::new_copy(&b); e.sqr();
		b.mul(&mut e);
		a.mul(&mut e);

		e.copy(&a);
		e.add(&a); e.add(&b);
		self.x.copy(&d); self.x.sqr(); self.x.sub(&e);

		a.sub(&self.x);
		self.y.copy(&a); self.y.mul(&mut d);
		c.mul(&mut b); self.y.sub(&c);

		self.x.norm();
		self.y.norm();
		self.z.norm();

		return 0;
	}

/* set this-=Q */
	pub fn sub(&mut self,Q :&mut ECP2) -> isize {
		Q.neg();
		let d=self.add(Q);
		Q.neg();
		return d;
	}

/* set this*=q, where q is Modulus, using Frobenius */
	pub fn frob(&mut self,x:&mut FP2) {
	 	if self.inf {return}
		let mut x2=FP2::new_copy(x);
		x2.sqr();
		self.x.conj();
		self.y.conj();
		self.z.conj();
		self.z.reduce();
		self.x.mul(&mut x2);
		self.y.mul(&mut x2);
		self.y.mul(x);
	}

/* normalises m-array of ECP2 points. Requires work vector of m FP2s */

	pub fn multiaffine(P: &mut [ECP2]) {
		let mut t1=FP2::new();
		let mut t2=FP2::new();

		let mut work:[FP2;8]=[FP2::new(),FP2::new(),FP2::new(),FP2::new(),FP2::new(),FP2::new(),FP2::new(),FP2::new()];
		let m=8;

		work[0].one();
		work[1].copy(&P[0].z);

		for i in 2..m {
			t1.copy(&work[i-1]);
			work[i].copy(&t1);
			work[i].mul(&mut P[i-1].z)
		}

		t1.copy(&work[m-1]); 
		t1.mul(&mut P[m-1].z);
		t1.inverse();
		t2.copy(&P[m-1].z);
		work[m-1].mul(&mut t1);

		let mut i=m-2;

		loop {
			if i==0 {
				work[0].copy(&t1);
				work[0].mul(&mut t2);
				break;
			}
			work[i].mul(&mut t2);
			work[i].mul(&mut t1);
			t2.mul(&mut P[i].z);
			i-=1;
		}
/* now work[] contains inverses of all Z coordinates */

		for i in 0..m {
			P[i].z.one();
			t1.copy(&work[i]); t1.sqr();
			P[i].x.mul(&mut t1);
			t1.mul(&mut work[i]);
			P[i].y.mul(&mut t1);
		}    
	}

/* self*=e */
	pub fn mul(&mut self,e: &BIG) -> ECP2 {
/* fixed size windows */
		let mut mt=BIG::new();
		let mut t=BIG::new();
		let mut P=ECP2::new();
		let mut Q=ECP2::new();
		let mut C=ECP2::new();

		if self.is_infinity() {return P}

		let mut W:[ECP2;8]=[ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new()];

		const CT:usize=1+(rom::NLEN*(rom::BASEBITS as usize)+3)/4;
		let mut w:[i8;CT]=[0;CT]; 

		self.affine();

/* precompute table */
		Q.copy(&self);
		Q.dbl();
		
		W[0].copy(&self);

		for i in 1..8 {
			C.copy(&W[i-1]);
			W[i].copy(&C);
			W[i].add(&mut Q);
		}

/* convert the table to affine */

		ECP2::multiaffine(&mut W);

/* make exponent odd - add 2P if even, P if odd */
		t.copy(&e);
		let s=t.parity();
		t.inc(1); t.norm(); let ns=t.parity(); mt.copy(&t); mt.inc(1); mt.norm();
		t.cmove(&mt,s);
		Q.cmove(&self,ns);
		C.copy(&Q);

		let nb=1+(t.nbits()+3)/4;

/* convert exponent to signed 4-bit window */
		for i in 0..nb {
			w[i]=(t.lastbits(5)-16) as i8;
			t.dec(w[i] as isize); t.norm();
			t.fshr(4);	
		}
		w[nb]=(t.lastbits(5)) as i8;
		
		P.copy(&W[((w[nb] as usize) -1)/2]);
		for i in (0..nb).rev() {
			Q.selector(&W,w[i] as i32);
			P.dbl();
			P.dbl();
			P.dbl();
			P.dbl();
			P.add(&mut Q);
		}
		P.sub(&mut C);
		P.affine();
		return P;
	}

/* P=u0.Q0+u1*Q1+u2*Q2+u3*Q3 */
	pub fn mul4(Q: &mut [ECP2],u: &[BIG]) -> ECP2 {
		let mut a:[i8;4]=[0;4];
		let mut T=ECP2::new();
		let mut C=ECP2::new();
		let mut P=ECP2::new();

		let mut W:[ECP2;8]=[ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new(),ECP2::new()];

		let mut mt=BIG::new();

		let mut t:[BIG;4]=[BIG::new_copy(&u[0]),BIG::new_copy(&u[1]),BIG::new_copy(&u[2]),BIG::new_copy(&u[3])];

		const CT:usize=1+rom::NLEN*(rom::BASEBITS as usize);
		let mut w:[i8;CT]=[0;CT];

		for i in 0..4 {
			Q[i].affine();
		}

/* precompute table */

		W[0].copy(&Q[0]); W[0].sub(&mut Q[1]);
		C.copy(&W[0]); W[1].copy(&C);
		W[2].copy(&C);
		W[3].copy(&C);
		W[4].copy(&Q[0]); W[4].add(&mut Q[1]);
		C.copy(&W[4]); W[5].copy(&C);
		W[6].copy(&C);
		W[7].copy(&C);

		T.copy(&Q[2]); T.sub(&mut Q[3]);
		W[1].sub(&mut T);
		W[2].add(&mut T);
		W[5].sub(&mut T);
		W[6].add(&mut T);
		T.copy(&Q[2]); T.add(&mut Q[3]);
		W[0].sub(&mut T);
		W[3].add(&mut T);
		W[4].sub(&mut T);
		W[7].add(&mut T);

		ECP2::multiaffine(&mut W);

/* if multiplier is even add 1 to multiplier, and add P to correction */
		mt.zero(); C.inf();
		for i in 0..4 {
			if t[i].parity()==0 {
				t[i].inc(1); t[i].norm();
				C.add(&mut Q[i]);
			}
			mt.add(&t[i]); mt.norm();
		}

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

		P.copy(&W[((w[nb] as usize)-1)/2]);  
		for i in (0..nb).rev() {
			T.selector(&W,w[i] as i32);
			P.dbl();
			P.add(&mut T);
		}
		P.sub(&mut C); /* apply correction */

		P.affine();
		return P;
	}

}
/*
fn main()
{
	let mut r=BIG::new_ints(&rom::MODULUS);

	let pxa=BIG::new_ints(&rom::CURVE_PXA);
	let pxb=BIG::new_ints(&rom::CURVE_PXB);
	let pya=BIG::new_ints(&rom::CURVE_PYA);
	let pyb=BIG::new_ints(&rom::CURVE_PYB);

	let fra=BIG::new_ints(&rom::CURVE_FRA);
	let frb=BIG::new_ints(&rom::CURVE_FRB);

	let mut f=FP2::new_bigs(&fra,&frb);

	let px=FP2::new_bigs(&pxa,&pxb);
	let py=FP2::new_bigs(&pya,&pyb);

	let mut P=ECP2::new_fp2s(&px,&py);

	println!("P= {}",P.tostring());

	P=P.mul(&mut r);
	println!("P= {}",P.tostring());

	let mut  Q=ECP2::new_fp2s(&px,&py);
	Q.frob(&mut f);
	println!("Q= {}",Q.tostring());
}
*/
