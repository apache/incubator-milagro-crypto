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

/* AMCL Fp^12 functions */

/* FP12 elements are of the form a+i.b+i^2.c */

/* general purpose constructor */
var FP12= function(d,e,f) 
{
	if (d instanceof FP12)
	{
		this.a=new FP4(d.a);
		this.b=new FP4(d.b);
		this.c=new FP4(d.c);
	}
	else
	{
		this.a=new FP4(d);
		this.b=new FP4(e);
		this.c=new FP4(f);
	}
};

FP12.prototype={
/* reduce all components of this mod Modulus */
	reduce: function()
	{
		this.a.reduce();
		this.b.reduce();
		this.c.reduce();
	},
/* normalize all components of this mod Modulus */
	norm: function()
	{
		this.a.norm();
		this.b.norm();
		this.c.norm();
	},
/* test x==0 ? */
	iszilch: function() 
	{
		this.reduce();
		return (this.a.iszilch() && this.b.iszilch() && this.c.iszilch());
	},
/* test x==1 ? */
	isunity: function() 
	{
		var one=new FP4(1);
		return (this.a.equals(one) && this.b.iszilch() && this.b.iszilch());
	},
/* extract a from this */
	geta: function()
	{
		return this.a;
	},
/* extract b */
	getb: function()
	{
		return this.b;
	},
/* extract c */
	getc: function()
	{
		return this.c;
	},
/* return 1 if x==y, else 0 */
	equals: function(x)
	{
		return (this.a.equals(x.a) && this.b.equals(x.b)&& this.c.equals(x.c));
	},
/* copy this=x */
	copy: function(x)
	{
		this.a.copy(x.a);
		this.b.copy(x.b);
		this.c.copy(x.c);
	},
/* set this=1 */
	one: function()
	{
		this.a.one();
		this.b.zero();
		this.c.zero();
	},
/* this=conj(this) */
	conj: function()
	{
		this.a.conj();
		this.b.nconj();
		this.c.conj();
	},

/* set this from 3 FP4s */
	set: function(d,e,f)
	{
		this.a.copy(d);
		this.b.copy(e);
		this.c.copy(f);
	},
/* set this from one FP4 */
	seta: function(d)
	{
		this.a.copy(d);
		this.b.zero();
		this.c.zero();
	},

/* Granger-Scott Unitary Squaring */
	usqr: function()
	{
		var A=new FP4(this.a); //A.copy(this.a);
		var B=new FP4(this.c); //B.copy(this.c);
		var C=new FP4(this.b); //C.copy(this.b);
		var D=new FP4(0);

		this.a.sqr();
		D.copy(this.a); D.add(this.a);
		this.a.add(D);

		A.nconj();

		A.add(A);
		this.a.add(A);
		B.sqr();
		B.times_i();

		D.copy(B); D.add(B);
		B.add(D);

		C.sqr();
		D.copy(C); D.add(C);
		C.add(D);

		this.b.conj();
		this.b.add(this.b);
		this.c.nconj();

		this.c.add(this.c);
		this.b.add(B);
		this.c.add(C);
		this.reduce();
	},

/* Chung-Hasan SQR2 method from http://cacr.uwaterloo.ca/techreports/2006/cacr2006-24.pdf */
	sqr: function()
	{
		var A=new FP4(this.a); //A.copy(this.a);
		var B=new FP4(this.b); //B.copy(this.b);
		var C=new FP4(this.c); //C.copy(this.c);
		var D=new FP4(this.a); //D.copy(this.a);

		A.sqr();
		B.mul(this.c);
		B.add(B);
		C.sqr();
		D.mul(this.b);
		D.add(D);

		this.c.add(this.a);
		this.c.add(this.b);
		this.c.sqr();

		this.a.copy(A);

		A.add(B);
		A.add(C);
		A.add(D);
		A.neg();
		B.times_i();
		C.times_i();

		this.a.add(B);
		this.b.copy(C); this.b.add(D);
		this.c.add(A);

		this.norm();
	},

/* FP12 full multiplication this=this*y */
	mul: function(y)
	{
		var z0=new FP4(this.a); //z0.copy(this.a);
		var z1=new FP4(0);
		var z2=new FP4(this.b); //z2.copy(this.b);
		var z3=new FP4(0);
		var t0=new FP4(this.a); //t0.copy(this.a);
		var t1=new FP4(y.a); //t1.copy(y.a);

		z0.mul(y.a);
		z2.mul(y.b);

		t0.add(this.b);
		t1.add(y.b);

		z1.copy(t0); z1.mul(t1);
		t0.copy(this.b); t0.add(this.c);

		t1.copy(y.b); t1.add(y.c);
		z3.copy(t0); z3.mul(t1);

		t0.copy(z0); t0.neg();
		t1.copy(z2); t1.neg();

		z1.add(t0);
		this.b.copy(z1); this.b.add(t1);

		z3.add(t1);
		z2.add(t0);

		t0.copy(this.a); t0.add(this.c);
		t1.copy(y.a); t1.add(y.c);
		t0.mul(t1);
		z2.add(t0);

		t0.copy(this.c); t0.mul(y.c);
		t1.copy(t0); t1.neg();

		this.c.copy(z2); this.c.add(t1);
		z3.add(t1);
		t0.times_i();
		this.b.add(t0);

		z3.times_i();
		this.a.copy(z0); this.a.add(z3);

		this.norm();
	},

/* Special case this*=y that arises from special form of ATE pairing line function */
	smul: function(y)
	{
		var z0=new FP4(this.a); //z0.copy(this.a);
		var z2=new FP4(this.b); //z2.copy(this.b);
		var z3=new FP4(this.b); //z3.copy(this.b);
		var t0=new FP4(0); 
		var t1=new FP4(y.a); //t1.copy(y.a);
		
		z0.mul(y.a);
		z2.pmul(y.b.real());
		this.b.add(this.a);
		t1.real().add(y.b.real());

		this.b.mul(t1);
		z3.add(this.c);
		z3.pmul(y.b.real());

		t0.copy(z0); t0.neg();
		t1.copy(z2); t1.neg();

		this.b.add(t0);

		this.b.add(t1);
		z3.add(t1);
		z2.add(t0);

		t0.copy(this.a); t0.add(this.c);
		t0.mul(y.a);
		this.c.copy(z2); this.c.add(t0);

		z3.times_i();
		this.a.copy(z0); this.a.add(z3);

		this.norm();
	},

/* this=1/this */
	inverse: function()
	{
		var f0=new FP4(this.a); //f0.copy(this.a);
		var f1=new FP4(this.b); //f1.copy(this.b);
		var f2=new FP4(this.a); //f2.copy(this.a);
		var f3=new FP4(0);

		f0.sqr();
		f1.mul(this.c);
		f1.times_i();
		f0.sub(f1);

		f1.copy(this.c); f1.sqr();
		f1.times_i();
		f2.mul(this.b);
		f1.sub(f2);

		f2.copy(this.b); f2.sqr();
		f3.copy(this.a); f3.mul(this.c);
		f2.sub(f3);

		f3.copy(this.b); f3.mul(f2);
		f3.times_i();
		this.a.mul(f0);
		f3.add(this.a);
		this.c.mul(f1);
		this.c.times_i();

		f3.add(this.c);
		f3.inverse();
		this.a.copy(f0); this.a.mul(f3);
		this.b.copy(f1); this.b.mul(f3);
		this.c.copy(f2); this.c.mul(f3);
	},

/* this=this^p, where p=Modulus, using Frobenius */
	frob: function(f)
	{
		var f2=new FP2(f); 
		var f3=new FP2(f); 

		f2.sqr();
		f3.mul(f2);

		this.a.frob(f3);
		this.b.frob(f3);
		this.c.frob(f3);

		this.b.pmul(f);
		this.c.pmul(f2);
	},

/* trace function */
	trace: function()
	{
		var t=new FP4(0);
		t.copy(this.a);
		t.imul(3);
		t.reduce();
		return t;
	},
/* convert this to hex string */
	toString: function() 
	{
		return ("["+this.a.toString()+","+this.b.toString()+","+this.c.toString()+"]");
	},
/* convert this to byte array */
	toBytes: function(w)
	{
		var i;
		var t=[];
		this.a.geta().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i]=t[i];
		this.a.geta().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+ROM.MODBYTES]=t[i];
		this.a.getb().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+2*ROM.MODBYTES]=t[i];
		this.a.getb().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+3*ROM.MODBYTES]=t[i];

		this.b.geta().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+4*ROM.MODBYTES]=t[i];
		this.b.geta().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+5*ROM.MODBYTES]=t[i];
		this.b.getb().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+6*ROM.MODBYTES]=t[i];
		this.b.getb().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+7*ROM.MODBYTES]=t[i];

		this.c.geta().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+8*ROM.MODBYTES]=t[i];
		this.c.geta().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+9*ROM.MODBYTES]=t[i];
		this.c.getb().getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+10*ROM.MODBYTES]=t[i];
		this.c.getb().getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++) w[i+11*ROM.MODBYTES]=t[i];
	},

/* set this=this^e */
	pow: function(e)
	{
		this.norm();
		e.norm();
		var w=new FP12(this); //w.copy(this);
		var z=new BIG(e); //z.copy(e);
		var r=new FP12(1);

		while (true)
		{
			var bt=z.parity();
			z.fshr(1);
			if (bt==1) r.mul(w);
			if (z.iszilch()) break;
			w.usqr();
		}
		r.reduce();
		return r;
	},

/* constant time powering by small integer of max length bts */
	pinpow: function(e,bts)
	{
		var i,b;
		var R=[];
		R[0]=new FP12(1);
		R[1]=new FP12(this);
		for (i=bts-1;i>=0;i--)
		{
			b=(e>>i)&1;
			R[1-b].mul(R[b]);
			R[b].usqr();
		}
		this.copy(R[0]);
	}
};

/* convert from byte array to FP12 */
FP12.fromBytes= function(w)
{
	var i,a,b,c,d,e,f,g;
	var t=[];

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+ROM.MODBYTES];
	b=BIG.fromBytes(t);
	c=new FP2(a,b); //c.bset(a,b);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+2*ROM.MODBYTES];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+3*ROM.MODBYTES];
	b=BIG.fromBytes(t);
	d=new FP2(a,b); //d.bset(a,b);

	e=new FP4(c,d); //e.set(c,d);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+4*ROM.MODBYTES];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+5*ROM.MODBYTES];
	b=BIG.fromBytes(t);
	c=new FP2(a,b); //c.bset(a,b);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+6*ROM.MODBYTES];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+7*ROM.MODBYTES];
	b=BIG.fromBytes(t);
	d=new FP2(a,b);

	f=new FP4(c,d); //f.set(c,d);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+8*ROM.MODBYTES];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+9*ROM.MODBYTES];
	b=BIG.fromBytes(t);
	c=new FP2(a,b); //c.bset(a,b);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+10*ROM.MODBYTES];
	a=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=w[i+11*ROM.MODBYTES];
	b=BIG.fromBytes(t);
	d=new FP2(a,b); //d.bset(a,b);

	g=new FP4(c,d); //g.set(c,d);

	var r=new FP12(e,f,g); //r.set(e,f,g);

	return r;
};

/* p=q0^u0.q1^u1.q2^u2.q3^u3 */
/* Timing attack secure, but not cache attack secure */

FP12.pow4= function(q,u)
{
	var i,j,nb,m;
	var a=[];
	var g=[];
	var s=[];

	var c=new FP12(1);
	var p=new FP12(0);
	var t=[];

	var mt=new BIG(0);
	var w=[];

	for (i=0;i<4;i++)
		t[i]=new BIG(u[i]);

	s[0]=new FP12(0);
	s[1]=new FP12(0);

	g[0]=new FP12(q[0]); s[0].copy(q[1]); s[0].conj(); g[0].mul(s[0]);
	g[1]=new FP12(g[0]);
	g[2]=new FP12(g[0]);
	g[3]=new FP12(g[0]);
	g[4]=new FP12(q[0]); g[4].mul(q[1]);
	g[5]=new FP12(g[4]);
	g[6]=new FP12(g[4]);
	g[7]=new FP12(g[4]);

	s[1].copy(q[2]); s[0].copy(q[3]); s[0].conj(); s[1].mul(s[0]);
	s[0].copy(s[1]); s[0].conj(); g[1].mul(s[0]);
	g[2].mul(s[1]);
	g[5].mul(s[0]);
	g[6].mul(s[1]);
	s[1].copy(q[2]); s[1].mul(q[3]);
	s[0].copy(s[1]); s[0].conj(); g[0].mul(s[0]);
	g[3].mul(s[1]);
	g[4].mul(s[0]);
	g[7].mul(s[1]);

/* if power is even add 1 to power, and add q to correction */

	for (i=0;i<4;i++)
	{
		if (t[i].parity()==0)
		{
			t[i].inc(1); t[i].norm();
			c.mul(q[i]);
		}
		mt.add(t[i]); mt.norm();
	}
	c.conj();
	nb=1+mt.nbits();

/* convert exponent to signed 1-bit window */
	for (j=0;j<nb;j++)
	{
		for (i=0;i<4;i++)
		{
			a[i]=(t[i].lastbits(2)-2);
			t[i].dec(a[i]); t[i].norm(); 
			t[i].fshr(1);
		}
		w[j]=(8*a[0]+4*a[1]+2*a[2]+a[3]);
	}
	w[nb]=(8*t[0].lastbits(2)+4*t[1].lastbits(2)+2*t[2].lastbits(2)+t[3].lastbits(2));
	p.copy(g[Math.floor((w[nb]-1)/2)]);  

	for (i=nb-1;i>=0;i--)
	{
		m=w[i]>>31;
		j=(w[i]^m)-m;  /* j=abs(w[i]) */
		j=(j-1)/2;
		s[0].copy(g[j]); s[1].copy(g[j]); s[1].conj();
		p.usqr();
		p.mul(s[m&1]);
	}
	p.mul(c);  /* apply correction */
	p.reduce();
	return p;
};

