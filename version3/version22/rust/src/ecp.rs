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
pub struct ECP {
	x:FP,
	y:FP,
	z:FP,
	inf: bool
}


//use rom;
//mod fp;
use fp::FP;
//mod big;
use big::BIG;
//mod dbig;
//use dbig::DBIG;
//mod rand;
//mod hash256;
//mod rom;
use rom;
use rom::BIG_HEX_STRING_LEN;

impl fmt::Display for ECP {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "ECP: [ {}, {}, {}, {} ]", self.inf, self.x, self.y, self.z)
	}
}

impl fmt::Debug for ECP {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "ECP: [ {}, {}, {}, {} ]", self.inf, self.x, self.y, self.z)
	}
}

impl PartialEq for ECP {
	fn eq(&self, other: &ECP) -> bool {
		return (self.inf == other.inf) &&
			(self.x == other.x) &&
			(self.y == other.y) &&
			(self.z == other.z);
	}
}

#[allow(non_snake_case)]
impl ECP {

	pub fn new() -> ECP {
		ECP {
				x: FP::new(),
				y: FP::new(),
				z: FP::new(),
				inf: true
		}
	}

/* set (x,y) from two BIGs */
	pub fn new_bigs(ix: &BIG,iy: &BIG) -> ECP {
		let mut E=ECP::new();
		E.x.bcopy(ix); 
		E.y.bcopy(iy); 
		E.z.one(); 
		let mut rhs=ECP::rhs(&mut E.x);
		if rom::CURVETYPE==rom::MONTGOMERY {
			if rhs.jacobi()==1 {
				E.inf=false;
			} else {E.inf()}
		} else {
			let mut y2=FP::new_copy(&E.y);
			y2.sqr();	
			if y2.equals(&mut rhs) {
				E.inf=false
			} else {E.inf()}
		}
		return E;
	}

/* set (x,y) from BIG and a bit */
	pub fn new_bigint(ix: &BIG,s: isize) -> ECP {
		let mut E=ECP::new();
		E.x.bcopy(ix); 
		E.z.one(); 

		let mut rhs=ECP::rhs(&mut E.x);

		if rhs.jacobi()==1 {
			let mut ny=rhs.sqrt();
			if ny.redc().parity()!=s {ny.neg()}
			E.y.copy(&ny);
			E.inf=false;
		} else {E.inf()}
		return E;
	}

#[allow(non_snake_case)]
/* set from x - calculate y from curve equation */
	pub fn new_big(ix: &BIG) -> ECP {
		let mut E=ECP::new();
		E.x.bcopy(ix); 
		E.z.one(); 
		let mut rhs=ECP::rhs(&mut E.x);
		if rhs.jacobi()==1 {
			if rom::CURVETYPE!=rom::MONTGOMERY {E.y.copy(&rhs.sqrt())}
			E.inf=false;
		} else {E.inf=true}
		return E;
	}

/* set this=O */
	pub fn inf(&mut self) {
		self.inf=true;
		self.x.zero();
		self.y.one();
		self.z.one();
	}

/* Calculate RHS of curve equation */
	fn rhs(x: &mut FP) -> FP {
		x.norm();
		let mut r=FP::new_copy(x);
		r.sqr();

		if rom::CURVETYPE==rom::WEIERSTRASS { // x^3+Ax+B
			let b=FP::new_big(&BIG::new_ints(&rom::CURVE_B));
			r.mul(x);
			if rom::CURVE_A==-3 {
				let mut cx=FP::new_copy(x);
				cx.imul(3);
				cx.neg(); cx.norm();
				r.add(&cx);
			}
			r.add(&b);
		}
		if rom::CURVETYPE==rom::EDWARDS { // (Ax^2-1)/(Bx^2-1) 
			let mut b=FP::new_big(&BIG::new_ints(&rom::CURVE_B));
			let one=FP::new_int(1);
			b.mul(&mut r);
			b.sub(&one);
			if rom::CURVE_A==-1 {r.neg()}
			r.sub(&one);
			b.inverse();
			r.mul(&mut b);
		}
		if rom::CURVETYPE==rom::MONTGOMERY { // x^3+Ax^2+x
			let mut x3=FP::new();
			x3.copy(&r);
			x3.mul(x);
			r.imul(rom::CURVE_A);
			r.add(&x3);
			r.add(&x);
		}
		r.reduce();
		return r;
	}

/* test for O point-at-infinity */
	pub fn is_infinity(&mut self) -> bool {
		if rom::CURVETYPE==rom::EDWARDS {
			self.x.reduce(); self.y.reduce(); self.z.reduce();
			return self.x.iszilch() && self.y.equals(&mut self.z);
		} else {return self.inf}
 	}

/* Conditional swap of P and Q dependant on d */
	pub fn cswap(&mut self,Q: &mut ECP,d: isize) {
		self.x.cswap(&mut Q.x,d);
		if rom::CURVETYPE!=rom::MONTGOMERY {self.y.cswap(&mut Q.y,d)}
		self.z.cswap(&mut Q.z,d);
		if rom::CURVETYPE!=rom::EDWARDS {
			let mut bd=true;
			if d==0 {bd=false}
			bd=bd&&(self.inf!=Q.inf);
			self.inf=bd!=self.inf;
			Q.inf=bd!=Q.inf;
		}
	}

/* Conditional move of Q to P dependant on d */
	pub fn cmove(&mut self,Q: &ECP,d: isize) {
		self.x.cmove(&Q.x,d);
		if rom::CURVETYPE!=rom::MONTGOMERY {self.y.cmove(&Q.y,d)}
		self.z.cmove(&Q.z,d);
		if rom::CURVETYPE!=rom::EDWARDS {
			let mut bd=true;
			if d==0 {bd=false}
			self.inf=self.inf!=((self.inf!=Q.inf)&&bd);
		}
	}

/* return 1 if b==c, no branching */
	fn teq(b: i32,c: i32) -> isize {
		let mut x=b^c;
		x-=1;  // if x=0, x now -1
		return ((x>>31)&1) as isize;
	}

/* this=P */
	pub fn copy(&mut self,P: & ECP) {
		self.x.copy(&P.x);
		if rom::CURVETYPE!=rom::MONTGOMERY {self.y.copy(&P.y)}
		self.z.copy(&P.z);
		self.inf=P.inf;
}

/* this=-this */
	pub fn neg(&mut self) {
		if self.is_infinity() {return}
		if rom::CURVETYPE==rom::WEIERSTRASS {
			self.y.neg(); self.y.norm();
		}
		if rom::CURVETYPE==rom::EDWARDS {
			self.x.neg(); self.x.norm();
		}
		return;
	}
/* multiply x coordinate */
	pub fn mulx(&mut self,c: &mut FP) {
		self.x.mul(c);
	}

/* Constant time select from pre-computed table */
	fn selector(&mut self, W: &[ECP],b: i32) {   // unsure about &[& syntax. An array of pointers I hope..
		let mut MP=ECP::new();
		let m=b>>31;
		let mut babs=(b^m)-m;

		babs=(babs-1)/2;

		self.cmove(&W[0],ECP::teq(babs,0));  // conditional move
		self.cmove(&W[1],ECP::teq(babs,1));
		self.cmove(&W[2],ECP::teq(babs,2));
		self.cmove(&W[3],ECP::teq(babs,3));
		self.cmove(&W[4],ECP::teq(babs,4));
		self.cmove(&W[5],ECP::teq(babs,5));
		self.cmove(&W[6],ECP::teq(babs,6));
		self.cmove(&W[7],ECP::teq(babs,7));
 
		MP.copy(self);
		MP.neg();
		self.cmove(&MP,(m&1) as isize);
	}

/* Test P == Q */
	pub fn equals(&mut self,Q: &mut ECP) -> bool {
		if self.is_infinity() && Q.is_infinity() {return true}
		if self.is_infinity() || Q.is_infinity() {return false}
		if rom::CURVETYPE==rom::WEIERSTRASS {
			let mut zs2=FP::new_copy(&self.z); zs2.sqr();
			let mut zo2=FP::new_copy(&Q.z); zo2.sqr();
			let mut zs3=FP::new_copy(&zs2); zs3.mul(&mut self.z);
			let mut zo3=FP::new_copy(&zo2); zo3.mul(&mut Q.z);
			zs2.mul(&mut Q.x);
			zo2.mul(&mut self.x);
			if !zs2.equals(&mut zo2) {return false}
			zs3.mul(&mut Q.y);
			zo3.mul(&mut self.y);
			if !zs3.equals(&mut zo3) {return false}
		} else {
			let mut a=FP::new();
			let mut b=FP::new();
			a.copy(&self.x); a.mul(&mut Q.z); a.reduce();
			b.copy(&Q.x); b.mul(&mut self.z); b.reduce();
			if !a.equals(&mut b) {return false}
			if rom::CURVETYPE==rom::EDWARDS {
				a.copy(&self.y); a.mul(&mut Q.z); a.reduce();
				b.copy(&Q.y); b.mul(&mut self.z); b.reduce();
				if !a.equals(&mut b) {return false}
			}
		}
		return true;
	}

/* set to affine - from (x,y,z) to (x,y) */
	pub fn affine(&mut self) {
		if self.is_infinity() {return}
		let mut one=FP::new_int(1);
		if self.z.equals(&mut one) {return}
		self.z.inverse();
		if rom::CURVETYPE==rom::WEIERSTRASS {
			let mut z2=FP::new_copy(&self.z);
			z2.sqr();
			self.x.mul(&mut z2); self.x.reduce();
			self.y.mul(&mut z2);
			self.y.mul(&mut self.z); self.y.reduce();
		}
		if rom::CURVETYPE==rom::EDWARDS {
			self.x.mul(&mut self.z); self.x.reduce();
			self.y.mul(&mut self.z); self.y.reduce();
		}
		if rom::CURVETYPE==rom::MONTGOMERY {
			self.x.mul(&mut self.z); self.x.reduce();
		}
		self.z.one();
	}

/* extract x as a BIG */
	pub fn getx(&mut self) -> BIG {
		self.affine();
		return self.x.redc();
	}

/* extract y as a BIG */
	pub fn gety(&mut self) -> BIG {
		self.affine();
		return self.y.redc();
	}

/* get sign of Y */
	pub fn gets(&mut self) -> isize {
		self.affine();
		let y=self.gety();
		return y.parity();
	}

/* extract x as an FP */
	pub fn getpx(&self) -> FP {
		let w=FP::new_copy(&self.x);
		return w;
	}
/* extract y as an FP */
	pub fn getpy(&self) -> FP {
		let w=FP::new_copy(&self.y);
		return w;
	}

/* extract z as an FP */
	pub fn getpz(&self) -> FP {
		let w=FP::new_copy(&self.z);
		return w;
	}

/* convert to byte array */
	pub fn tobytes(&mut self,b: &mut [u8]) {
		let mb=rom::MODBYTES as usize;
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		if rom::CURVETYPE!=rom::MONTGOMERY {
			b[0]=0x04;
		} else {b[0]=0x02}
	
		self.affine();
		self.x.redc().tobytes(&mut t);
		for i in 0..mb {b[i+1]=t[i]}
		if rom::CURVETYPE!=rom::MONTGOMERY {
			self.y.redc().tobytes(&mut t);
			for i in 0..mb {b[i+mb+1]=t[i]}
		}
	}

/* convert from byte array to point */
	pub fn frombytes(b: &[u8]) -> ECP {
		let mut t:[u8;rom::MODBYTES as usize]=[0;rom::MODBYTES as usize];
		let mb=rom::MODBYTES as usize;
		let p=BIG::new_ints(&rom::MODULUS);

		for i in 0..mb {t[i]=b[i+1]}
		let px=BIG::frombytes(&t);
		if BIG::comp(&px,&p)>=0 {return ECP::new()}

		if b[0]==0x04 {
			for i in 0..mb {t[i]=b[i+mb+1]}
			let py=BIG::frombytes(&t);
			if BIG::comp(&py,&p)>=0 {return ECP::new()}
			return ECP::new_bigs(&px,&py);
		} else {return ECP::new_big(&px)}
	}

	pub fn to_hex(&self) -> String {
		let mut ret: String = String::with_capacity(4 * BIG_HEX_STRING_LEN);
		ret.push_str(&format!("{} {} {} {}", self.inf, self.x.to_hex(), self.y.to_hex(), self.z.to_hex()));
		return ret;
	}

	pub fn from_hex_iter(iter: &mut SplitWhitespace) -> ECP {
		let mut ret:ECP = ECP::new();
		if let Some(x) = iter.next() {
			ret.inf = x == "true";
			ret.x = FP::from_hex(iter.next().unwrap_or("").to_string());
			ret.y = FP::from_hex(iter.next().unwrap_or("").to_string());
			ret.z = FP::from_hex(iter.next().unwrap_or("").to_string());
		}
		return ret;
	}

	pub fn from_hex(val: String) -> ECP {
		let mut iter = val.split_whitespace();
		return ECP::from_hex_iter(&mut iter);
	}

/* convert to hex string */
	pub fn tostring(&mut self) -> String {
	 	if self.is_infinity() {return String::from("infinity")}
		self.affine();
		if rom::CURVETYPE==rom::MONTGOMERY {
			return format!("({})",self.x.redc().tostring());
		} else {return format!("({},{})",self.x.redc().tostring(),self.y.redc().tostring())} ; 
	}

/* this*=2 */
	pub fn dbl(&mut self) {
		if rom::CURVETYPE==rom::WEIERSTRASS {
			if self.inf {return}
			if self.y.iszilch() {
				self.inf();
				return;
			}

			let mut w1=FP::new_copy(&self.x);
			let mut w6=FP::new_copy(&self.z);
			let mut w2=FP::new();
			let mut w3=FP::new_copy(&self.x);
			let mut w8=FP::new_copy(&self.x);

			if rom::CURVE_A==-3 {
				w6.sqr();
				w1.copy(&w6);
				w1.neg();
				w3.add(&w1);

				w8.add(&w6);

				w3.mul(&mut w8);
				w8.copy(&w3);
				w8.imul(3);
			} else {
				w1.sqr();
				w8.copy(&w1);
				w8.imul(3);
			}

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
            //w2.norm();
            self.y.sub(&w2);
            self.y.norm();
            self.z.norm();
        }
        if rom::CURVETYPE==rom::EDWARDS {
            let mut c=FP::new_copy(&self.x);
            let mut d=FP::new_copy(&self.y);
            let mut h=FP::new_copy(&self.z);
            let mut j=FP::new();
    
            self.x.mul(&mut self.y); self.x.dbl();
            c.sqr();
            d.sqr();
            if rom::CURVE_A == -1 {c.neg()}
            self.y.copy(&c); self.y.add(&d);
            self.y.norm();
            h.sqr(); h.dbl(); 
            self.z.copy(&self.y);
            j.copy(&self.y); j.sub(&h);
            self.x.mul(&mut j);
            c.sub(&d);
            self.y.mul(&mut c);
            self.z.mul(&mut j);
    
            self.x.norm();
            self.y.norm();
            self.z.norm();
        }
        if rom::CURVETYPE==rom::MONTGOMERY {
            let mut a=FP::new_copy(&self.x);
            let mut b=FP::new_copy(&self.x);
            let mut aa=FP::new();
            let mut bb=FP::new();
            let mut c=FP::new();
    
            if self.inf {return}
    
            a.add(&self.z);
            aa.copy(&a); aa.sqr();
            b.sub(&self.z);
            bb.copy(&b); bb.sqr();
            c.copy(&aa); c.sub(&bb);
    
            self.x.copy(&aa); self.x.mul(&mut bb);
    
            a.copy(&c); a.imul((rom::CURVE_A+2)/4);
    
            bb.add(&a);
            self.z.copy(&bb); self.z.mul(&mut c);
            self.x.norm();
            self.z.norm();
        }
        return;
    }

    /* self+=Q */
    pub fn add(&mut self,Q:&mut ECP)
    {
        if rom::CURVETYPE==rom::WEIERSTRASS {
            if self.inf {
				self.copy(&Q);
				return;
            }
            if Q.inf {return}
    
            let mut aff=false;
    
            let mut one=FP::new_int(1);
            if Q.z.equals(&mut one) {aff=true}
    
            let mut a=FP::new();
            let mut c=FP::new();
            let mut b=FP::new_copy(&self.z);
            let mut d=FP::new_copy(&self.z);
            if !aff {
            	a.copy(&Q.z);
            	c.copy(&Q.z);
    
				a.sqr(); b.sqr();
				c.mul(&mut a); d.mul(&mut b);
    
				a.mul(&mut self.x);
				c.mul(&mut self.y);
            }
            else
            {
				a.copy(&self.x);
				c.copy(&self.y);
    
				b.sqr();
				d.mul(&mut b);
            }
    
            b.mul(&mut Q.x); b.sub(&a);
            d.mul(&mut Q.y); d.sub(&c);
    
            if b.iszilch()
            {
				if d.iszilch()
				{
                    self.dbl();
                    return;
				}
				else
				{
                    self.inf=true;
                    return;
				}
            }
    
            if !aff {self.z.mul(&mut Q.z)}
            self.z.mul(&mut b);
    
            let mut e=FP::new_copy(&b); e.sqr();
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
        }
        if rom::CURVETYPE==rom::EDWARDS {
            let mut bb=FP::new_big(&BIG::new_ints(&rom::CURVE_B));
            let mut a=FP::new_copy(&self.z);
            let mut b=FP::new();
            let mut c=FP::new_copy(&self.x);
            let mut d=FP::new_copy(&self.y);
            let mut e=FP::new();
            let mut f=FP::new();
            let mut g=FP::new();
    
            a.mul(&mut Q.z);
            b.copy(&a); b.sqr();
            c.mul(&mut Q.x);
            d.mul(&mut Q.y);
    
            e.copy(&c); e.mul(&mut d); e.mul(&mut bb);
            f.copy(&b); f.sub(&e);
            g.copy(&b); g.add(&e);
    
            if rom::CURVE_A==1 {
				e.copy(&d); e.sub(&c);
            }
            c.add(&d);
    
            b.copy(&self.x); b.add(&self.y);
            d.copy(&Q.x); d.add(&Q.y);
            b.mul(&mut d);
            b.sub(&c);
            b.mul(&mut f);
            self.x.copy(&a); self.x.mul(&mut b);

            if rom::CURVE_A==1 {
				c.copy(&e); c.mul(&mut g);
            }
            if rom::CURVE_A == -1 {
				c.mul(&mut g);
            }
            self.y.copy(&a); self.y.mul(&mut c);
            self.z.copy(&f); self.z.mul(&mut g);
            self.x.norm(); self.y.norm(); self.z.norm();
        }
        return;
    }

/* Differential Add for Montgomery curves. this+=Q where W is this-Q and is affine. */
	pub fn dadd(&mut self,Q: &ECP,W: &ECP) {
		let mut a=FP::new_copy(&self.x);
		let mut b=FP::new_copy(&self.x);
		let mut c=FP::new_copy(&Q.x);
		let mut d=FP::new_copy(&Q.x);
		let mut da=FP::new();
		let mut cb=FP::new();
			
		a.add(&self.z);
		b.sub(&self.z);

		c.add(&Q.z);
		d.sub(&Q.z);

		da.copy(&d); da.mul(&mut a);
		cb.copy(&c); cb.mul(&mut b);

		a.copy(&da); a.add(&cb); a.sqr();
		b.copy(&da); b.sub(&cb); b.sqr();

		self.x.copy(&a);
		self.z.copy(&W.x); self.z.mul(&mut b);

		if self.z.iszilch() {
			self.inf();
		} else {self.inf=false;}

		self.x.norm();
	}

/* self-=Q */
	pub fn sub(&mut self,Q:&mut ECP) {
		Q.neg();
		self.add(Q);
		Q.neg();
	}

	fn multiaffine(P: &mut [ECP]) {
		let mut t1=FP::new();
		let mut t2=FP::new();

		let mut work:[FP;8]=[FP::new(),FP::new(),FP::new(),FP::new(),FP::new(),FP::new(),FP::new(),FP::new()];
		let m=8;

		work[0].one();
		work[1].copy(&P[0].z);

		for i in 2..m {
			t1.copy(&work[i-1]);
			work[i].copy(&t1);
			work[i].mul(&mut P[i-1].z);
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
			t1.copy(&work[i]);
			t1.sqr();
			P[i].x.mul(&mut t1);
			t1.mul(&mut work[i]);
			P[i].y.mul(&mut t1);
		}    
	}

/* constant time multiply by small integer of length bts - use ladder */
	pub fn pinmul(&mut self,e: i32,bts: i32) -> ECP {	
		if rom::CURVETYPE==rom::MONTGOMERY {
			return self.mul(&mut BIG::new_int(e as isize));
		} else {
			let mut P=ECP::new();
			let mut R0=ECP::new();
			let mut R1=ECP::new(); R1.copy(&self);

			for i in (0..bts).rev() {
				let b=((e>>i)&1) as isize;
				P.copy(&R1);
				P.add(&mut R0);
				R0.cswap(&mut R1,b);
				R1.copy(&P);
				R0.dbl();
				R0.cswap(&mut R1,b);
			}
			P.copy(&R0);
			P.affine();
			return P;
		}
	}

/* return e.self */

	pub fn mul(&mut self,e:&mut BIG) -> ECP {
		if e.iszilch() || self.is_infinity() {return ECP::new()}
		let mut P=ECP::new();
		if rom::CURVETYPE==rom::MONTGOMERY {
/* use Ladder */
			let mut D=ECP::new();
			let mut R0=ECP::new(); R0.copy(&self);
			let mut R1=ECP::new(); R1.copy(&self);
			R1.dbl();
			D.copy(&self); D.affine();
			let nb=e.nbits();

			for i in (0..nb-1).rev() {
				let b=e.bit(i);
				P.copy(&R1);
				P.dadd(&mut R0,&D);
				R0.cswap(&mut R1,b);
				R1.copy(&P);
				R0.dbl();
				R0.cswap(&mut R1,b);
			}
			P.copy(&R0)
		} else {
// fixed size windows 
			let mut mt=BIG::new();
			let mut t=BIG::new();
			let mut Q=ECP::new();
			let mut C=ECP::new();

		 	let mut W:[ECP;8]=[ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new()];

		 	const CT:usize=1+(rom::NLEN*(rom::BASEBITS as usize)+3)/4;
			let mut w:[i8;CT]=[0;CT];

			self.affine();

			Q.copy(&self);
			Q.dbl();

			W[0].copy(&self);

			for i in 1..8 {
				C.copy(&W[i-1]);
				W[i].copy(&C);
				W[i].add(&mut Q);
			}

// convert the table to affine 
			if rom::CURVETYPE==rom::WEIERSTRASS {
				ECP::multiaffine(&mut W);
			}

// make exponent odd - add 2P if even, P if odd 
			t.copy(&e);
			let s=t.parity();
			t.inc(1); t.norm(); let ns=t.parity(); mt.copy(&t); mt.inc(1); mt.norm();
			t.cmove(&mt,s);
			Q.cmove(&self,ns);
			C.copy(&Q);

			let nb=1+(t.nbits()+3)/4;

// convert exponent to signed 4-bit window 
			for i in 0..nb {
				w[i]=(t.lastbits(5)-16) as i8;
				t.dec(w[i] as isize); t.norm();
				t.fshr(4);	
			}
			w[nb]=t.lastbits(5) as i8;

			P.copy(&W[((w[nb] as usize)-1)/2]); 
			for i in (0..nb).rev() {
				Q.selector(&W,w[i] as i32);
				P.dbl();
				P.dbl();
				P.dbl();
				P.dbl();
				P.add(&mut Q);
			}
			P.sub(&mut C); /* apply correction */
		}
		P.affine();
		return P;
	}

/* Return e.this+f.Q */

	pub fn mul2(&mut self,e: &BIG,Q: &mut ECP,f: &BIG) -> ECP {
		let mut te=BIG::new();
		let mut tf=BIG::new();
		let mut mt=BIG::new();
		let mut S=ECP::new();
		let mut T=ECP::new();
		let mut C=ECP::new();

		let mut W:[ECP;8]=[ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new(),ECP::new()];

		const CT:usize=1+(rom::NLEN*(rom::BASEBITS as usize)+1)/2;
		let mut w: [i8;CT]=[0;CT];		

		self.affine();
		Q.affine();

		te.copy(e);
		tf.copy(f);

// precompute table 

		W[1].copy(&self); W[1].sub(Q);
		W[2].copy(&self); W[2].add(Q);
		S.copy(&Q); S.dbl();
		C.copy(&W[1]); W[0].copy(&C); W[0].sub(&mut S); // copy to C is stupid Rust thing..
		C.copy(&W[2]); W[3].copy(&C); W[3].add(&mut S);
		T.copy(&self); T.dbl();
		C.copy(&W[1]); W[5].copy(&C); W[5].add(&mut T);
		C.copy(&W[2]); W[6].copy(&C); W[6].add(&mut T);
		C.copy(&W[5]); W[4].copy(&C); W[4].sub(&mut S);
		C.copy(&W[6]); W[7].copy(&C); W[7].add(&mut S);

// convert the table to affine 
		if rom::CURVETYPE==rom::WEIERSTRASS { 
			ECP::multiaffine(&mut W);
		}

// if multiplier is odd, add 2, else add 1 to multiplier, and add 2P or P to correction 

		let mut s=te.parity();
		te.inc(1); te.norm(); let mut ns=te.parity(); mt.copy(&te); mt.inc(1); mt.norm();
		te.cmove(&mt,s);
		T.cmove(&self,ns);
		C.copy(&T);

		s=tf.parity();
		tf.inc(1); tf.norm(); ns=tf.parity(); mt.copy(&tf); mt.inc(1); mt.norm();
		tf.cmove(&mt,s);
		S.cmove(&Q,ns);
		C.add(&mut S);

		mt.copy(&te); mt.add(&tf); mt.norm();
		let nb=1+(mt.nbits()+1)/2;

// convert exponent to signed 2-bit window 
		for i in 0..nb {
			let a=te.lastbits(3)-4;
			te.dec(a); te.norm();
			te.fshr(2);
			let b=tf.lastbits(3)-4;
			tf.dec(b); tf.norm();
			tf.fshr(2);
			w[i]=(4*a+b) as i8;
		}
		w[nb]=(4*te.lastbits(3)+tf.lastbits(3)) as i8;
		S.copy(&W[((w[nb] as usize)-1)/2]);  

		for i in (0..nb).rev() {
			T.selector(&W,w[i] as i32);
			S.dbl();
			S.dbl();
			S.add(&mut T);
		}
		S.sub(&mut C); /* apply correction */
		S.affine();
		return S;
	}


}
/*
fn main()
{
	let mut E=ECP::new();

	let mut W:[&ECP;8]=[&ECP::new(),&ECP::new(),&ECP::new(),&ECP::new(),&ECP::new(),&ECP::new(),&ECP::new(),&ECP::new()];

	let mut gx=BIG::new_ints(&rom::CURVE_GX);
	let mut gy=BIG::new();
	let mut P=ECP::new();

	if rom::CURVETYPE!=rom::MONTGOMERY {gy.copy(&BIG::new_ints(&rom::CURVE_GY))}
	let mut r=BIG::new_ints(&rom::CURVE_ORDER);

	//r.dec(7);
	
	println!("gx= {}",gx.tostring());

	if rom::CURVETYPE!=rom::MONTGOMERY {
		println!("gy= {}",gy.tostring());
	}	

	if rom::CURVETYPE!=rom::MONTGOMERY {
		P.copy(&ECP::new_bigs(&gx,&gy))}
	else  {P.copy(&ECP::new_big(&gx))}

	println!("P= {}",P.tostring());		

	let mut R=P.mul(&mut r);
		//for i in 0..10000	(R=P.mul(r));
	
	println!("R= {}",R.tostring());

}
*/
