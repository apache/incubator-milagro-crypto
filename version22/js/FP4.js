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

/* Finite Field arithmetic  Fp^4 functions */

/* FP4 elements are of the form a+ib, where i is sqrt(-1+sqrt(-1))  */

/* general purpose constructor */
var FP4=function(c,d) 
{
	if (c instanceof FP4)
	{
		this.a=new FP2(c.a);
		this.b=new FP2(c.b);
	}
	else
	{
		this.a=new FP2(c);
		this.b=new FP2(d);
	}
};

FP4.prototype={
/* reduce all components of this mod Modulus */
	reduce: function()
	{
		this.a.reduce();
		this.b.reduce();
	},
/* normalise all components of this mod Modulus */
	norm: function()
	{
		this.a.norm();
		this.b.norm();
	},
/* test this==0 ? */
	iszilch: function() 
	{
		this.reduce();
		return (this.a.iszilch() && this.b.iszilch());
	},
/* test this==1 ? */
	isunity: function() 
	{
		var one=new FP2(1);
		return (this.a.equals(one) && this.b.iszilch());
	},
/* test is w real? That is in a+ib test b is zero */
	isreal: function()
	{
		return this.b.iszilch();
	},
/* extract real part a */
	real: function()
	{
		return this.a;
	},

	geta: function()
	{
		return this.a;
	},
/* extract imaginary part b */
	getb: function()
	{
		return this.b;
	},
/* test this=x? */
	equals: function(x)
	{
		return (this.a.equals(x.a) && this.b.equals(x.b));
	},
/* copy this=x */
	copy: function(x)
	{
		this.a.copy(x.a);
		this.b.copy(x.b);
	},
/* this=0 */
	zero: function()
	{
		this.a.zero();
		this.b.zero();
	},
/* this=1 */
	one: function()
	{
		this.a.one();
		this.b.zero();
	},

/* set from two FP2s */
	set: function(c,d)
	{
		this.a.copy(c);
		this.b.copy(d);
	},
/* set a */
	seta: function(c)
	{
		this.a.copy(c);
		this.b.zero();
	},
/* this=-this */
	neg: function()
	{
		var m=new FP2(this.a); //m.copy(this.a);
		var t=new FP2(0);
		m.add(this.b);
		m.neg();
		m.norm();
		t.copy(m); t.add(this.b);
		this.b.copy(m);
		this.b.add(this.a);
		this.a.copy(t);
	},
/* this=conjugate(this) */
	conj: function()
	{
		this.b.neg(); this.b.norm();
	},
/* this=-conjugate(this) */
	nconj: function()
	{
		this.a.neg(); this.a.norm();
	},
/* this+=x */
	add: function(x)
	{
		this.a.add(x.a);
		this.b.add(x.b);
	},
/* this-=x */
	sub: function(x)
	{
		var m=new FP4(x); // m.copy(x); 
		m.neg();
		this.add(m);
	},
/* this*=s where s is FP2 */
	pmul: function(s)
	{
		this.a.mul(s);
		this.b.mul(s);
	},
/* this*=c where s is int */
	imul: function(c)
	{
		this.a.imul(c);
		this.b.imul(c);
	},
/* this*=this */
	sqr: function()
	{
		this.norm();

		var t1=new FP2(this.a); //t1.copy(this.a);
		var t2=new FP2(this.b); //t2.copy(this.b);
		var t3=new FP2(this.a); //t3.copy(this.a);

		t3.mul(this.b);
		t1.add(this.b);
		t2.mul_ip();

		t2.add(this.a);
		this.a.copy(t1);

		this.a.mul(t2);

		t2.copy(t3);
		t2.mul_ip();
		t2.add(t3);

		t2.neg();

		this.a.add(t2);

		this.b.copy(t3);
		this.b.add(t3);

		this.norm();
	},
/* this*=y */
	mul: function(y)
	{
		this.norm();

		var t1=new FP2(this.a); //t1.copy(this.a);
		var t2=new FP2(this.b); //t2.copy(this.b);
		var t3=new FP2(0);
		var t4=new FP2(this.b); //t4.copy(this.b);

		t1.mul(y.a);
		t2.mul(y.b);
		t3.copy(y.b);
		t3.add(y.a);
		t4.add(this.a);

		t4.mul(t3);
		t4.sub(t1);

		this.b.copy(t4);
		this.b.sub(t2);
		t2.mul_ip();
		this.a.copy(t2);
		this.a.add(t1);

		this.norm();
	},
/* convert to hex string */
	toString: function() 
	{
		return ("["+this.a.toString()+","+this.b.toString()+"]");
	},
/* this=1/this */
	inverse: function()
	{
		this.norm();

		var t1=new FP2(this.a); //t1.copy(this.a);
		var t2=new FP2(this.b);// t2.copy(this.b);

		t1.sqr();
		t2.sqr();
		t2.mul_ip();
		t1.sub(t2);
		t1.inverse();
		this.a.mul(t1);
		t1.neg();
		this.b.mul(t1);
	},

/* this*=i where i = sqrt(-1+sqrt(-1)) */
	times_i: function()
	{
		var s=new FP2(this.b); //s.copy(this.b);
		var t=new FP2(this.b); //t.copy(this.b);
		s.times_i();
		t.add(s);
		this.b.copy(this.a);
		this.a.copy(t);
	},

/* this=this^q using Frobenius, where q is Modulus */
	frob: function(f)
	{
		this.a.conj();
		this.b.conj();
		this.b.mul(f);
	},

/* this=this^e */
	pow: function(e)
	{
		this.norm();
		e.norm();
		var w=new FP4(this); //w.copy(this);
		var z=new BIG(e); //z.copy(e);
		var r=new FP4(1);
		while (true)
		{
			var bt=z.parity();
			z.fshr(1);
			if (bt==1) r.mul(w);
			if (z.iszilch()) break;
			w.sqr();
		}
		r.reduce();
		return r;
	},

/* XTR xtr_a function */
	xtr_A: function(w,y,z) 
	{
		var r=new FP4(w); //r.copy(w);
		var t=new FP4(w); //t.copy(w);
		r.sub(y);
		r.pmul(this.a);
		t.add(y);
		t.pmul(this.b);
		t.times_i();

		this.copy(r);
		this.add(t);
		this.add(z);

		this.norm();
	},
/* XTR xtr_d function */
	xtr_D: function() 
	{
		var w=new FP4(this); //w.copy(this);
		this.sqr(); w.conj();
		w.add(w);
		this.sub(w);
		this.reduce();
	},
/* r=x^n using XTR method on traces of FP12s */
	xtr_pow: function(n) 
	{
		var a=new FP4(3);
		var b=new FP4(this);  
		var c=new FP4(b); 
		c.xtr_D();
		var t=new FP4(0);
		var r=new FP4(0);

		n.norm();
		var par=n.parity();
		var v=new BIG(n); v.fshr(1);
		if (par===0) {v.dec(1); v.norm();}

		var nb=v.nbits();
		for (var i=nb-1;i>=0;i--)
		{
			if (v.bit(i)!=1)
			{
				t.copy(b);
				this.conj();
				c.conj();
				b.xtr_A(a,this,c);
				this.conj();
				c.copy(t);
				c.xtr_D();
				a.xtr_D();
			}
			else
			{
				t.copy(a); t.conj();
				a.copy(b);
				a.xtr_D();
				b.xtr_A(c,this,t);
				c.xtr_D();
			}
		}
		if (par===0) r.copy(c);
		else r.copy(b);
		r.reduce();
		return r;
	},

/* r=ck^a.cl^n using XTR double exponentiation method on traces of FP12s. See Stam thesis. */
	xtr_pow2: function(ck,ckml,ckm2l,a,b)
	{
		a.norm(); b.norm();
		var e=new BIG(a); //e.copy(a);
		var d=new BIG(b); //d.copy(b);
		var w=new BIG(0);

		var cu=new FP4(ck); //cu.copy(ck); // can probably be passed in w/o copying
		var cv=new FP4(this); //cv.copy(this);
		var cumv=new FP4(ckml); //cumv.copy(ckml);
		var cum2v=new FP4(ckm2l); //cum2v.copy(ckm2l);
		var r=new FP4(0);
		var t=new FP4(0);

		var f2=0;
		while (d.parity()===0 && e.parity()===0)
		{
			d.fshr(1);
			e.fshr(1);
			f2++;
		}

		while (BIG.comp(d,e)!==0)
		{
			if (BIG.comp(d,e)>0)
			{
				w.copy(e); w.imul(4); w.norm();
				if (BIG.comp(d,w)<=0)
				{
					w.copy(d); d.copy(e);
					e.rsub(w); e.norm();

					t.copy(cv); 
					t.xtr_A(cu,cumv,cum2v);
					cum2v.copy(cumv); 
					cum2v.conj();
					cumv.copy(cv);
					cv.copy(cu);
					cu.copy(t);

				}
				else if (d.parity()===0)
				{
					d.fshr(1);
					r.copy(cum2v); r.conj();
					t.copy(cumv);
					t.xtr_A(cu,cv,r);
					cum2v.copy(cumv);
					cum2v.xtr_D();
					cumv.copy(t);
					cu.xtr_D();
				}
				else if (e.parity()==1)
				{
					d.sub(e); d.norm();
					d.fshr(1);
					t.copy(cv);
					t.xtr_A(cu,cumv,cum2v);
					cu.xtr_D();
					cum2v.copy(cv);
					cum2v.xtr_D();
					cum2v.conj();
					cv.copy(t);
				}
				else
				{
					w.copy(d);
					d.copy(e); d.fshr(1);
					e.copy(w);
					t.copy(cumv);
					t.xtr_D();
					cumv.copy(cum2v); cumv.conj();
					cum2v.copy(t); cum2v.conj();
					t.copy(cv);
					t.xtr_D();
					cv.copy(cu);
					cu.copy(t);
				}
			}
			if (BIG.comp(d,e)<0)
			{
				w.copy(d); w.imul(4); w.norm();
				if (BIG.comp(e,w)<=0)
				{
					e.sub(d); e.norm();
					t.copy(cv);
					t.xtr_A(cu,cumv,cum2v);
					cum2v.copy(cumv);
					cumv.copy(cu);
					cu.copy(t);
				}
				else if (e.parity()===0)
				{
					w.copy(d);
					d.copy(e); d.fshr(1);
					e.copy(w);
					t.copy(cumv);
					t.xtr_D();
					cumv.copy(cum2v); cumv.conj();
					cum2v.copy(t); cum2v.conj();
					t.copy(cv);
					t.xtr_D();
					cv.copy(cu);
					cu.copy(t);
				}
				else if (d.parity()==1)
				{
					w.copy(e);
					e.copy(d);
					w.sub(d); w.norm();
					d.copy(w); d.fshr(1);
					t.copy(cv);
					t.xtr_A(cu,cumv,cum2v);
					cumv.conj();
					cum2v.copy(cu);
					cum2v.xtr_D();
					cum2v.conj();
					cu.copy(cv);
					cu.xtr_D();
					cv.copy(t);
				}
				else
				{
					d.fshr(1);
					r.copy(cum2v); r.conj();
					t.copy(cumv);
					t.xtr_A(cu,cv,r);
					cum2v.copy(cumv);
					cum2v.xtr_D();
					cumv.copy(t);
					cu.xtr_D();
				}
			}
		}
		r.copy(cv);
		r.xtr_A(cu,cumv,cum2v);
		for (var i=0;i<f2;i++)
			r.xtr_D();
		r=r.xtr_pow(d);
		return r;
	}

};
