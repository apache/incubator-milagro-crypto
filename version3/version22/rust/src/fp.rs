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

#[derive(Copy, Clone)]
pub struct FP {
 	x:BIG
}

use big::BIG;
use dbig::DBIG;
use rom;
use rom::{Chunk, BIG_HEX_STRING_LEN};

impl fmt::Display for FP {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "FP: [ {} ]", self.x)
    }
}

impl fmt::Debug for FP {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "FP: [ {} ]", self.x)
    }
}

impl PartialEq for FP {
    fn eq(&self, other: &FP) -> bool {
        return self.x == other.x;
    }
}

impl FP {

/* Constructors */
	pub fn new() -> FP {
		FP {
				x: BIG::new()
		}
	}

	pub fn new_int(a:isize) -> FP {
		let mut f=FP::new(); 
		f.x.inc(a);
		f.nres();
		return f;		
	}

	pub fn new_copy(y:&FP) -> FP {
		let mut f=FP::new(); 
		f.x.copy(&(y.x));
		return f;
	}

	pub fn new_big(y:&BIG) -> FP {
		let mut f=FP::new(); 
		f.x.copy(y);
        f.nres();
		return f;		
	}

    pub fn nres(&mut self) {
        if rom::MODTYPE != rom::PSEUDO_MERSENNE && rom::MODTYPE != rom::GENERALISED_MERSENNE {
   			let p = BIG::new_ints(&rom::MODULUS);        	
            let mut d=DBIG::new_scopy(&(self.x));
            d.shl(rom::NLEN*(rom::BASEBITS as usize));
            self.x.copy(&d.dmod(&p));
        }
    }

/* convert back to regular form */
    pub fn redc(&mut self) -> BIG {
        if rom::MODTYPE != rom::PSEUDO_MERSENNE && rom::MODTYPE != rom::GENERALISED_MERSENNE {
            let mut d=DBIG::new_scopy(&(self.x));
            return BIG::modulo(&mut d);
        } else {
            let r=BIG::new_copy(&(self.x));
            return r;
        }
    }

   /* convert to string */
	pub fn tostring(&mut self) -> String {
        let s=self.redc().tostring();
        return s;
    }

    pub fn to_hex(&self) -> String {
        let mut ret: String = String::with_capacity(2 * BIG_HEX_STRING_LEN);
        let mut x = self.x;
        ret.push_str(&format!("{}", x.to_hex()));
        return ret;
    }

    pub fn from_hex(val: String) -> FP {
        return FP {
            x: BIG::from_hex(val)
        }
    }

/* reduce this mod Modulus */
    pub fn reduce(&mut self) {
  		let p = BIG::new_ints(&rom::MODULUS);      	
        self.x.rmod(&p)
    }
    
/* test this=0? */
    pub fn iszilch(&mut self) -> bool {
        self.reduce();
        return self.x.iszilch();
    }
    
/* copy from FP b */
    pub fn copy(&mut self,b: &FP) {
        self.x.copy(&(b.x));
    }
    
/* copy from BIG b */
    pub fn bcopy(&mut self,b: &BIG) {
        self.x.copy(&b);
        self.nres();
    }

/* set this=0 */
    pub fn zero(&mut self) {
        self.x.zero();
    }
    
/* set this=1 */
    pub fn one(&mut self) {
        self.x.one(); self.nres()
    }
    
/* normalise this */
    pub fn norm(&mut self) {
        self.x.norm();
    }
/* swap FPs depending on d */
    pub fn cswap(&mut self,b: &mut FP,d: isize) {
        self.x.cswap(&mut (b.x),d);
    }
    
/* copy FPs depending on d */
    pub fn cmove(&mut self,b: &FP,d: isize) {
        self.x.cmove(&(b.x),d);
    }

/* this*=b mod Modulus */
    pub fn mul(&mut self,b: &mut FP)
    {
        self.norm();
        b.norm();
        if BIG::pexceed(&(self.x),&(b.x)) {self.reduce()}

        let mut d=BIG::mul(&(self.x),&(b.x));
        self.x.copy(&BIG::modulo(&mut d))
    }

    fn logb2(w: u32) -> usize {
        let mut v=w;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;

        v = v - ((v >> 1) & 0x55555555);                 
        v = (v & 0x33333333) + ((v >> 2) & 0x33333333);  
        let r= ((   ((v + (v >> 4)) & 0xF0F0F0F).wrapping_mul(0x1010101)) >> 24) as usize;
        return r+1;    
    }

/* this = -this mod Modulus */
    pub fn neg(&mut self) {
  		let mut p = BIG::new_ints(&rom::MODULUS);   
    
        self.norm();

        let sb=FP::logb2(BIG::excess(&(self.x)) as u32);

    //    let mut ov=BIG::excess(&(self.x));
    //    let mut sb=1; while ov != 0 {sb += 1;ov>>=1}
    
        p.fshl(sb);
        self.x.rsub(&p);
    
        if BIG::excess(&(self.x))>=rom::FEXCESS {self.reduce()}
    }

    /* this*=c mod Modulus, where c is a small int */
    pub fn imul(&mut self,c: isize) {
        let mut cc=c;
        self.norm();
        let mut s=false;
        if cc<0 {
            cc = -cc;
            s=true;
        }
        let afx=(BIG::excess(&(self.x))+1)*((cc as Chunk)+1)+1;
        if cc<rom::NEXCESS && afx<rom::FEXCESS {
            self.x.imul(cc);
        } else {
            if afx<rom::FEXCESS {
            	self.x.pmul(cc);
            } else {
  				let p = BIG::new_ints(&rom::MODULUS);               	
				let mut d=self.x.pxmul(cc);
				self.x.copy(&d.dmod(&p));
            }
        }
        if s {self.neg()}
        self.norm();
    }

/* self*=self mod Modulus */
    pub fn sqr(&mut self) {
        self.norm();
        if BIG::sexceed(&(self.x)) {self.reduce()}

        let mut d=BIG::sqr(&(self.x));
        self.x.copy(&BIG::modulo(&mut d))
    }

/* self+=b */
    pub fn add(&mut self,b: &FP) {
        self.x.add(&(b.x));
        if BIG::excess(&(self.x))+2>=rom::FEXCESS {self.reduce()}
    }

/* self+=self */
    pub fn dbl(&mut self) {
        self.x.dbl();
        if BIG::excess(&(self.x))+2>=rom::FEXCESS {self.reduce()}
    }
    
/* self-=b */
    pub fn sub(&mut self,b: &FP)
    {
        let mut n=FP::new_copy(b);
        n.neg();
        self.add(&n);
    }    

/* self/=2 mod Modulus */
    pub fn div2(&mut self) {
        self.x.norm();
        if self.x.parity()==0 {
        	self.x.fshr(1);
        } else {
  			let p = BIG::new_ints(&rom::MODULUS);           	
            self.x.add(&p);
            self.x.norm();
            self.x.fshr(1);
        }
    }
/* self=1/self mod Modulus */
    pub fn inverse(&mut self) {
  		let mut p = BIG::new_ints(&rom::MODULUS);      	
        let mut r=self.redc();
        r.invmodp(&mut p);
        self.x.copy(&r);
        self.nres();
    }

/* return TRUE if self==a */
    pub fn equals(&mut self,a: &mut FP) -> bool {
        a.reduce();
        self.reduce();
        if BIG::comp(&(a.x),(&self.x))==0 {return true}
        return false;
    }   

/* return self^e mod Modulus */
    pub fn pow(&mut self,e: &mut BIG) -> FP {
      	let p = BIG::new_ints(&rom::MODULUS);   	
        let mut r=FP::new_int(1);
        e.norm();
        self.x.norm();
		let mut m=FP::new_copy(self);
        loop {
            let bt=e.parity();
            e.fshr(1);
            if bt==1 {r.mul(&mut m)}
            if e.iszilch() {break}
            m.sqr();
        }
        r.x.rmod(&p);
        return r;
    }

/* return sqrt(this) mod Modulus */
    pub fn sqrt(&mut self) -> FP {
        self.reduce();
      	let mut p = BIG::new_ints(&rom::MODULUS);  
        if rom::MOD8==5 {
            p.dec(5); p.norm(); p.shr(3);
            let mut i=FP::new_copy(self); i.x.shl(1);
            let mut v=i.pow(&mut p);
            i.mul(&mut v); i.mul(&mut v);
            i.x.dec(1);
            let mut r=FP::new_copy(self);
            r.mul(&mut v); r.mul(&mut i);
            r.reduce();
            return r;
        }
        else
        {
            p.inc(1); p.norm(); p.shr(2);
            return self.pow(&mut p);
        }
    }
/* return jacobi symbol (this/Modulus) */
    pub fn jacobi(&mut self) -> isize
    {
     	let mut p = BIG::new_ints(&rom::MODULUS);      	
        let mut w=self.redc();
        return w.jacobi(&mut p);
    }

}
/*
fn main() {
    let p = BIG::new_ints(&rom::MODULUS);  
	let mut e = BIG::new_copy(&p);
	e.dec(1);

    let mut x = FP::new_int(3);
    let mut s=x.pow(&mut e);

    println!("s= {}",s.tostring());
}
*/
