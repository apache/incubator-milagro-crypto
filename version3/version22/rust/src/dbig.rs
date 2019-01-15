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

use rom;

use rom::Chunk;

//#[derive(Copy, Clone)]
pub struct DBIG {
 	pub w: [Chunk; rom::DNLEN]
}

//mod big;
use big::BIG;

impl DBIG {
   pub fn new() -> DBIG {
        DBIG {
        	w: [0; rom::DNLEN as usize]
         }
    }	

    pub fn new_copy(y:&DBIG) -> DBIG {
    	let mut s= DBIG::new();   
    	for i in 0..rom::NLEN {s.w[i]=y.w[i]}
    	return s;	
    }

    pub fn new_scopy(x:&BIG) -> DBIG {
    	let mut b= DBIG::new();   
		for i in 0 ..rom::NLEN {
			b.w[i]=x.w[i];
		}
		b.w[rom::NLEN-1]=x.get(rom::NLEN-1)&rom::BMASK; /* top word normalized */
		b.w[rom::NLEN]=x.get(rom::NLEN-1)>>rom::BASEBITS;

		for i in rom::NLEN+1 ..rom::DNLEN {b.w[i]=0}
    	return b; 	
    }

/* split DBIG at position n, return higher half, keep lower half */
    pub fn split(&mut self,n: usize) -> BIG
    {
        let mut t=BIG::new();
        let m=n%rom::BASEBITS;
        let mut carry=self.w[rom::DNLEN-1]<<(rom::BASEBITS-m);
    
        for i in (rom::NLEN-1..rom::DNLEN-1).rev() {
            let nw=(self.w[i]>>m)|carry;
            carry= (self.w[i]<<(rom::BASEBITS-m))&rom::BMASK;
            t.set(i-rom::NLEN+1,nw);
        }
        self.w[rom::NLEN-1]&=((1 as Chunk)<<m)-1;
        return t;
    }

/* general shift left */
    pub fn shl(&mut self,k: usize)
    {
        let n=k%rom::BASEBITS; 
        let m=k/rom::BASEBITS; 
        self.w[rom::DNLEN-1]=((self.w[rom::DNLEN-1-m]<<n))|(self.w[rom::DNLEN-m-2]>>(rom::BASEBITS-n));
        for i in (m+1..rom::DNLEN-1).rev() {
            self.w[i]=((self.w[i-m]<<n)&rom::BMASK)|(self.w[i-m-1]>>(rom::BASEBITS-n));
        }
  
        self.w[m]=(self.w[0]<<n)&rom::BMASK;
        for i in 0 ..m {self.w[i]=0}
    }

/* general shift right */
    pub fn shr(&mut self,k: usize) {
		let n=k%rom::BASEBITS;
		let m=k/rom::BASEBITS;
        for i in 0 ..rom::DNLEN-m-1 {
            self.w[i]=(self.w[m+i]>>n)|((self.w[m+i+1]<<(rom::BASEBITS-n))&rom::BMASK);
        }
        self.w[rom::DNLEN-m-1]=self.w[rom::DNLEN-1]>>n;
        for i in rom::DNLEN - m ..rom::DNLEN {self.w[i]=0}
    }

/* Copy from another DBIG */
	pub fn copy(&mut self,x: &DBIG) {
		for i in 0 ..rom::DNLEN {
			self.w[i]=x.w[i]
		}
	}

	pub fn cmove(&mut self,g:&DBIG,d: isize) {
		let b=-d as Chunk;
		for i in 0 ..rom::DNLEN {
			self.w[i]^=(self.w[i]^g.w[i])&b;
		}
	}

/* self-=x */
	pub fn sub(&mut self,x:&DBIG) {
		for i in 0 ..rom::DNLEN {
			self.w[i]-=x.w[i]; 
		}
	} 

/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
    pub fn comp(a: &DBIG,b: &DBIG) -> isize {
        for i in (0 ..rom::DNLEN).rev() {
            if a.w[i]==b.w[i] {continue}
            if a.w[i]>b.w[i] {return 1}
            else  {return -1}
        }
        return 0;
    }

/* normalise BIG - force all digits < 2^rom::BASEBITS */
    pub fn norm(&mut self) {
        let mut carry=0 as Chunk;
        for i in 0 ..rom::DNLEN-1 {
            let d=self.w[i]+carry;
            self.w[i]=d&rom::BMASK;
            carry=d>>rom::BASEBITS;
        }
        self.w[rom::DNLEN-1]+=carry
    }

/* reduces self DBIG mod a BIG, and returns the BIG */
    pub fn dmod(&mut self,c: &BIG) -> BIG {
        let mut k=0;
        self.norm();
        let mut m=DBIG::new_scopy(c);
        let mut dr=DBIG::new();
    
        if DBIG::comp(self,&m)<0 {
        	let r=BIG::new_dcopy(self);
        	return r;
        }
    
        loop {
            m.shl(1);
            k += 1;
            if DBIG::comp(self,&m)<0 {break;}
        }
    
        while k>0 {
            m.shr(1);

		dr.copy(self);
		dr.sub(&m);
		dr.norm();
		self.cmove(&dr,(1-((dr.w[rom::DNLEN-1]>>(rom::CHUNK-1))&1)) as isize);
/*
            if DBIG::comp(self,&m)>=0 {
				self.sub(&m);
				self.norm();
            } */
            k -= 1;
        }
        let r=BIG::new_dcopy(self);
        return r;
    }

/* return this/c */
    pub fn div(&mut self,c: &BIG) -> BIG {
        let mut k=0;
        let mut m=DBIG::new_scopy(c);
        let mut a=BIG::new();
        let mut e=BIG::new_int(1);
	let mut dr=DBIG::new();
	let mut r=BIG::new();
        self.norm();

        while DBIG::comp(self,&m)>=0 {
            e.fshl(1);
            m.shl(1);
            k+=1;
        }

        while k>0 {
            m.shr(1);
            e.shr(1);

		dr.copy(self);
		dr.sub(&m);
		dr.norm();
		let d=(1-((dr.w[rom::DNLEN-1]>>(rom::CHUNK-1))&1)) as isize;
		self.cmove(&dr,d);
		r.copy(&a);
		r.add(&e);
		r.norm();
		a.cmove(&r,d);
/*
            if DBIG::comp(self,&m)>0 {
                a.add(&e);
                a.norm();
                self.sub(&m);
                self.norm();
            } */
            k-=1;
        }
        return a;
    }

/* return number of bits */
	pub fn nbits(&mut self) -> usize {
		let mut k=rom::DNLEN-1;
		self.norm();
		while (k as isize)>=0 && self.w[k]==0 {k=k-1}
		if (k as isize) <0 {return 0}
		let mut bts=(rom::BASEBITS as usize)*k;
		let mut c=self.w[k];
		while c!=0 {c/=2; bts+=1;}
		return bts;
	}

/* Convert to Hex String */
	pub fn to_string(&mut self) -> String {
		let mut s = String::new();
		let mut len=self.nbits();

		if len%4==0 {
			len/=4;
		} else {
			len/=4;
			len+=1;
		}

		for i in (0 ..len).rev() {
			let mut b=DBIG::new_copy(&self);
			b.shr(i*4);
			s=s + &format!("{:X}", b.w[0]&15);
		}
		return s;
	}	

}
