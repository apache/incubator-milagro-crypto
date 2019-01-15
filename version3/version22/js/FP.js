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

/* Finite Field arithmetic */
/* AMCL mod p functions */

/* General purpose COnstructor */
var FP = function(x) {
	if (x instanceof FP)
	{
		this.f=new BIG(x.f);
	}
	else
	{
		this.f=new BIG(x);
		this.nres();
	}
};

FP.prototype={
/* set this=0 */
	zero: function()
	{
		return this.f.zero();
	},

/* copy from a BIG in ROM */
	rcopy: function(y)
	{
		this.f.rcopy(y);
		this.nres();
	},

/* copy from another BIG */
	bcopy: function(y)
	{
		this.f.copy(y);
		this.nres();
//alert("4. f= "+this.f.toString());
	},

/* copy from another FP */
	copy: function(y)
	{
		return this.f.copy(y.f);
	},

/* conditional swap of a and b depending on d */
	cswap: function(b,d)
	{
		this.f.cswap(b.f,d);
	},

/* conditional copy of b to a depending on d */
	cmove: function(b,d)
	{
		this.f.cmove(b.f,d);
	},

/* convert to Montgomery n-residue form */
	nres: function()
	{
		if (ROM.MODTYPE!=ROM.PSEUDO_MERSENNE && ROM.MODTYPE!=ROM.GENERALISED_MERSENNE)
		{
			var p=new BIG();
			p.rcopy(ROM.Modulus);
			var d=new DBIG(0);
//alert("f= "+this.f.toString());
			d.hcopy(this.f);
			d.norm();
//alert("1. d= "+d.toString());
			d.shl(ROM.NLEN*ROM.BASEBITS);
//alert("2. d= "+d.toString());
			this.f.copy(d.mod(p));
//alert("3. f= "+this.f.toString());

		}
		return this;
	},
	
/* convert back to regular form */
	redc: function()
	{
		var r=new BIG(0);
		r.copy(this.f);
		if (ROM.MODTYPE!=ROM.PSEUDO_MERSENNE && ROM.MODTYPE!=ROM.GENERALISED_MERSENNE)
		{
			var d=new DBIG(0);
			d.hcopy(this.f);
//alert("rd= "+d.toString());
			var w=BIG.mod(d);
//alert("w= "+w.toString());
			r.copy(w);
		}

		return r;
	},	

/* convert this to string */
	toString: function() 
	{
		var s=this.redc().toString();
		return s;
	},

/* test this=0 */
	iszilch: function() 
	{
		this.reduce();
		return this.f.iszilch();
	},

/* reduce this mod Modulus */
	reduce: function()
	{
		var p=new BIG(0);
		p.rcopy(ROM.Modulus);
		return this.f.mod(p);
	},

/* set this=1 */
	one: function()
	{
		this.f.one(); 
		return this.nres();
	},

/* normalise this */
	norm: function()
	{
		return this.f.norm();
	},

/* this*=b mod Modulus */
	mul: function(b)
	{
		this.norm();
		b.norm();
		var ea=BIG.EXCESS(this.f);
		var eb=BIG.EXCESS(b.f);

		if ((ea+1)*(eb+1)>ROM.FEXCESS) this.reduce();
		//if ((ea+1) >= Math.floor((ROM.FEXCESS-1)/(eb+1))) this.reduce();

		var d=BIG.mul(this.f,b.f);
		this.f.copy(BIG.mod(d));
		return this;
	},

/* this*=c mod Modulus where c is an int */
	imul: function(c)
	{
		var s=false;
		this.norm();
		if (c<0)
		{
			c=-c;
			s=true;
		}

		var afx=(BIG.EXCESS(this.f)+1)*(c+1)+1;
		if (c<ROM.NEXCESS && afx<ROM.FEXCESS)
		{
			this.f.imul(c);
		}
		else
		{
			if (afx<ROM.FEXCESS) this.f.pmul(c);
			else
			{
				var p=new BIG(0);
				p.rcopy(ROM.Modulus);
				var d=this.f.pxmul(c);
				this.f.copy(d.mod(p));
			}
		}
		if (s) this.neg();
		return this.norm();
	},

/* this*=this mod Modulus */
	sqr: function()
	{
		var d;
		this.norm();
		var ea=BIG.EXCESS(this.f);

		if ((ea+1)*(ea+1)>ROM.FEXCESS) this.reduce();
		//if ((ea+1)>= Math.floor((ROM.FEXCESS-1)/(ea+1))) this.reduce();

		d=BIG.sqr(this.f);
		var t=BIG.mod(d); 
		this.f.copy(t);
		return this;
	},

/* this+=b */
	add: function(b) 
	{
		this.f.add(b.f);
		if (BIG.EXCESS(this.f)+2>=ROM.FEXCESS) this.reduce();
		return this;
	},
/* this=-this mod Modulus */
	neg: function()
	{
		var sb,ov;
		var m=new BIG(0);
		m.rcopy(ROM.Modulus);

		this.norm();
		sb=FP.logb2(BIG.EXCESS(this.f));

//		ov=BIG.EXCESS(this.f); 
//		sb=1; while(ov!==0) {sb++;ov>>=1;} 

		m.fshl(sb);
		this.f.rsub(m);	
		if (BIG.EXCESS(this.f)>=ROM.FEXCESS) this.reduce();
		return this;
	},

/* this-=b */
	sub: function(b)
	{
		var n=new FP(0);
		n.copy(b);
		n.neg();
		this.add(n);
		return this;
	},

/* this/=2 mod Modulus */
	div2: function()
	{
		this.norm();
		if (this.f.parity()===0)
			this.f.fshr(1);
		else
		{
			var p=new BIG(0);
			p.rcopy(ROM.Modulus);

			this.f.add(p);
			this.f.norm();
			this.f.fshr(1);
		}
		return this;
	},

/* this=1/this mod Modulus */
	inverse: function()
	{
		var p=new BIG(0);
		p.rcopy(ROM.Modulus);
		var r=this.redc();
		r.invmodp(p);
		this.f.copy(r);
		return this.nres();
	},

/* return TRUE if this==a */
	equals: function(a)
	{
		a.reduce();
		this.reduce();
		if (BIG.comp(a.f,this.f)===0) return true;
		return false;
	},

/* return this^e mod Modulus */
	pow: function(e)
	{
		var bt;
		var r=new FP(1);
		e.norm();
		this.norm();
		var m=new FP(0);
		m.copy(this);
		while (true)
		{
			bt=e.parity();
			e.fshr(1);
			if (bt==1) r.mul(m);
			if (e.iszilch()) break;
			m.sqr();
		}

		r.reduce();
		return r;
	},

/* return jacobi symbol (this/Modulus) */
	jacobi: function()
	{
		var p=new BIG(0);
		p.rcopy(ROM.Modulus);
		var w=this.redc();
		return w.jacobi(p);
	},

/* return sqrt(this) mod Modulus */
	sqrt: function()
	{
		this.reduce();
		var b=new BIG(0);
		b.rcopy(ROM.Modulus);
		if (ROM.MOD8==5)
		{
			b.dec(5); b.norm(); b.shr(3);
			var i=new FP(0); 
			i.copy(this);
			i.f.shl(1);
			var v=i.pow(b);
			i.mul(v); i.mul(v);
			i.f.dec(1);
			var r=new FP(0);
			r.copy(this);
			r.mul(v); r.mul(i); 
			r.reduce();
			return r;
		}
		else
		{
			b.inc(1); b.norm(); b.shr(2);
			return this.pow(b);
		}
	}

};

FP.logb2=function(v)
{
		v |= v >>> 1;
		v |= v >>> 2;
		v |= v >>> 4;
		v |= v >>> 8;
		v |= v >>> 16;

		v = v - ((v >>> 1) & 0x55555555);                  
		v = (v & 0x33333333) + ((v >>> 2) & 0x33333333);  
		var r = ((v + (v >>> 4) & 0xF0F0F0F) * 0x1010101) >>> 24; 
		return r+1;
};
