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

/* AMCL Weierstrass elliptic curve functions over FP2 */

/* Constructor, set this=O */
var ECP2=function()
{
	this.x=new FP2(0);
	this.y=new FP2(1);
	this.z=new FP2(1);
	this.INF=true;
};

ECP2.prototype={
/* Test this=O? */
	is_infinity: function() 
	{
		return this.INF;
	},
/* copy this=P */
	copy: function(P)
	{
		this.x.copy(P.x);
		this.y.copy(P.y);
		this.z.copy(P.z);
		this.INF=P.INF;
	},
/* set this=O */
	inf: function() 
	{
		this.INF=true;
		this.x.zero();
		this.y.zero();
		this.z.zero();
	},

/* conditional move of Q to P dependant on d */
	cmove: function(Q,d)
	{
		this.x.cmove(Q.x,d);
		this.y.cmove(Q.y,d);
		this.z.cmove(Q.z,d);

		var bd=(d!==0)?true:false;
		this.INF^=(this.INF^Q.INF)&bd;
	},

/* Constant time select from pre-computed table */
	select: function(W,b)
	{
		var MP=new ECP2(); 
		var m=b>>31;
		var babs=(b^m)-m;

		babs=(babs-1)/2;

		this.cmove(W[0],ECP2.teq(babs,0));  // conditional move
		this.cmove(W[1],ECP2.teq(babs,1));
		this.cmove(W[2],ECP2.teq(babs,2));
		this.cmove(W[3],ECP2.teq(babs,3));
		this.cmove(W[4],ECP2.teq(babs,4));
		this.cmove(W[5],ECP2.teq(babs,5));
		this.cmove(W[6],ECP2.teq(babs,6));
		this.cmove(W[7],ECP2.teq(babs,7));

		MP.copy(this);
		MP.neg();
		this.cmove(MP,(m&1));
	},

/* Test P == Q */

	equals: function(Q) {
		if (this.is_infinity() && Q.is_infinity()) return true;
		if (this.is_infinity() || Q.is_infinity()) return false;

		var zs2=new FP2(this.z); /*zs2.copy(this.z);*/ zs2.sqr();
		var zo2=new FP2(Q.z); /*zo2.copy(Q.z);*/  zo2.sqr();
		var zs3=new FP2(zs2); /*zs3.copy(zs2);*/ zs3.mul(this.z);
		var zo3=new FP2(zo2); /*zo3.copy(zo2);*/  zo3.mul(Q.z);
		zs2.mul(Q.x);
		zo2.mul(this.x);
		if (!zs2.equals(zo2)) return false;
		zs3.mul(Q.y);
		zo3.mul(this.y);
		if (!zs3.equals(zo3)) return false;

		return true;
	},
/* set this=-this */
	neg: function() 
	{
		if (this.is_infinity()) return;
		this.y.neg(); this.y.norm();
		return;
	},
/* convert this to affine, from (x,y,z) to (x,y) */
	affine: function() 
	{
		if (this.is_infinity()) return;
		var one=new FP2(1);
		if (this.z.equals(one)) return;
		this.z.inverse();

		var z2=new FP2(this.z); //z2.copy(this.z);
		z2.sqr();
		this.x.mul(z2); this.x.reduce();
		this.y.mul(z2); 
		this.y.mul(this.z);  this.y.reduce();
		this.z=one;
	},
/* extract affine x as FP2 */
	getX: function()
	{
		this.affine();
		return this.x;
	},
/* extract affine y as FP2 */
	getY: function()
	{
		this.affine();
		return this.y;
	},
/* extract projective x */
	getx: function()
	{
		return this.x;
	},
/* extract projective y */
	gety: function()
	{
		return this.y;
	},
/* extract projective z */
	getz: function()
	{
		return this.z;
	},
/* convert this to byte array */
	toBytes: function(b)
	{
		var i,t=[];
		this.affine();
		this.x.getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++)
			b[i]=t[i];
		this.x.getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++)
			b[i+ROM.MODBYTES]=t[i];

		this.y.getA().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++)
			b[i+2*ROM.MODBYTES]=t[i];
		this.y.getB().toBytes(t);
		for (i=0;i<ROM.MODBYTES;i++)
			b[i+3*ROM.MODBYTES]=t[i];
	},
/* convert this to hex string */
	toString: function() 
	{
		if (this.is_infinity()) return "infinity";
		this.affine();
		return "("+this.x.toString()+","+this.y.toString()+")";
	},
/* set this=(x,y) */
	setxy: function(ix,iy) 
	{
		this.x.copy(ix);
		this.y.copy(iy);
		this.z.one();

		var rhs=ECP2.RHS(this.x);

		var y2=new FP2(this.y); //y2.copy(this.y);
		y2.sqr();
		if (y2.equals(rhs)) this.INF=false;
		else this.inf();
	},

/* set this=(x,.) */
	setx: function(ix) 
	{
		this.x.copy(ix);
		this.z.one();

		var rhs=ECP2.RHS(this.x);

		if (rhs.sqrt())
		{
			this.y.copy(rhs);
			this.INF=false;
		}
		else this.inf();
	},

/* set this*=q, where q is Modulus, using Frobenius */
	frob: function(X)
	{
		if (this.INF) return;
		var X2=new FP2(X); //X2.copy(X);
		X2.sqr();
		this.x.conj();
		this.y.conj();
		this.z.conj();
		this.z.reduce();
		this.x.mul(X2);
		this.y.mul(X2);
		this.y.mul(X);
	},
/* this+=this */
	dbl: function() 
	{
		if (this.INF) return -1;
		if (this.y.iszilch())
		{
			this.inf();
			return -1;
		}

		var w1=new FP2(this.x); //w1.copy(this.x);
		var w2=new FP2(0); 
		var w3=new FP2(this.x); //w3.copy(this.x);
		var w8=new FP2(this.x); //w8.copy(this.x);

		w1.sqr();
		w8.copy(w1);
		w8.imul(3);

		w2.copy(this.y); w2.sqr();
		w3.copy(this.x); w3.mul(w2);
		w3.imul(4);
		w1.copy(w3); w1.neg();


		this.x.copy(w8); this.x.sqr();
		this.x.add(w1);
		this.x.add(w1);
		this.x.norm();

		this.z.mul(this.y);
		this.z.add(this.z);

		w2.add(w2);
		w2.sqr();
		w2.add(w2);
		w3.sub(this.x);
		this.y.copy(w8); this.y.mul(w3);
		this.y.sub(w2);
		this.y.norm();
		this.z.norm();

		return 1;
	},
/* this+=Q - return 0 for add, 1 for double, -1 for O */
/* this+=Q */
	add: function(Q) 
	{
		if (this.INF)
		{
			this.copy(Q);
			return -1;
		}
		if (Q.INF) return -1;

		var aff=false;

		if (Q.z.isunity()) aff=true;

		var A,C;
		var B=new FP2(this.z);
		var D=new FP2(this.z);
		if (!aff)
		{
			A=new FP2(Q.z);
			C=new FP2(Q.z);

			A.sqr(); B.sqr();
			C.mul(A); D.mul(B);

			A.mul(this.x);
			C.mul(this.y);
		}
		else
		{
			A=new FP2(this.x);
			C=new FP2(this.y);
	
			B.sqr();
			D.mul(B);
		}

		B.mul(Q.x); B.sub(A);
		D.mul(Q.y); D.sub(C);
			
		if (B.iszilch())
		{
			if (D.iszilch())
			{
				this.dbl();
				return 1;
			}
			else
			{
				this.INF=true;
				return -1;
			}
		}

		if (!aff) this.z.mul(Q.z);
		this.z.mul(B);

		var e=new FP2(B); e.sqr();
		B.mul(e);
		A.mul(e);

		e.copy(A);
		e.add(A); e.add(B);
		this.x.copy(D); this.x.sqr(); this.x.sub(e);

		A.sub(this.x);
		this.y.copy(A); this.y.mul(D); 
		C.mul(B); this.y.sub(C);

		this.x.norm();
		this.y.norm();
		this.z.norm();
		return 0;
	},
/* this-=Q */
	sub: function(Q) 
	{
		Q.neg();
		var D=this.add(Q);
		Q.neg();
		return D;
	},

/* P*=e */
	mul: function(e)
	{
/* fixed size windows */
		var i,b,nb,m,s,ns;
		var mt=new BIG();
		var t=new BIG();
		var C=new ECP2();
		var P=new ECP2();
		var Q=new ECP2();
		var W=[];
		var w=[];

		if (this.is_infinity()) return new ECP2();

		this.affine();

// precompute table 
		Q.copy(this);
		Q.dbl();
		W[0]=new ECP2();
		W[0].copy(this);

		for (i=1;i<8;i++)
		{
			W[i]=new ECP2();
			W[i].copy(W[i-1]);
			W[i].add(Q);
		}

// convert the table to affine 

		ECP2.multiaffine(8,W);

// make exponent odd - add 2P if even, P if odd 
		t.copy(e);
		s=t.parity();
		t.inc(1); t.norm(); ns=t.parity(); mt.copy(t); mt.inc(1); mt.norm();
		t.cmove(mt,s);
		Q.cmove(this,ns);
		C.copy(Q);

		nb=1+Math.floor((t.nbits()+3)/4);

// convert exponent to signed 4-bit window 
		for (i=0;i<nb;i++)
		{
			w[i]=(t.lastbits(5)-16);
			t.dec(w[i]); t.norm();
			t.fshr(4);	
		}
		w[nb]=t.lastbits(5);
	
		P.copy(W[Math.floor((w[nb]-1)/2)]);  
		for (i=nb-1;i>=0;i--)
		{
			Q.select(W,w[i]);
			P.dbl();
			P.dbl();
			P.dbl();
			P.dbl();
			P.add(Q);
		}
		P.sub(C);
		P.affine();
		return P;
	}
};

/* convert from byte array to point */
ECP2.fromBytes=function(b)
{
	var i,t=[];
	var ra,rb;

	for (i=0;i<ROM.MODBYTES;i++) t[i]=b[i];
	ra=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=b[i+ROM.MODBYTES];
	rb=BIG.fromBytes(t);

	var rx=new FP2(ra,rb); //rx.bset(ra,rb);

	for (i=0;i<ROM.MODBYTES;i++) t[i]=b[i+2*ROM.MODBYTES];
	ra=BIG.fromBytes(t);
	for (i=0;i<ROM.MODBYTES;i++) t[i]=b[i+3*ROM.MODBYTES];
	rb=BIG.fromBytes(t);

	var ry=new FP2(ra,rb); //ry.bset(ra,rb);

	var P=new ECP2();
	P.setxy(rx,ry);
	return P;
};

/* Calculate RHS of curve equation x^3+B */
ECP2.RHS=function(x) 
{
	x.norm();
	var r=new FP2(x); //r.copy(x);
	r.sqr();

	var c=new BIG(0); c.rcopy(ROM.CURVE_B);
	var b=new FP2(c); //b.bseta(c);
	b.div_ip();
	r.mul(x);
	r.add(b);

	r.reduce();
	return r;
};

/* normalises m-array of ECP2 points. Requires work vector of m FP2s */

ECP2.multiaffine=function(m,P)
{
	var i;
	var t1=new FP2(0);
	var t2=new FP2(0);
	var work=[];

	work[0]=new FP2(1);
	work[1]=new FP2(P[0].z);
	for (i=2;i<m;i++)
	{
		work[i]=new FP2(work[i-1]);
		work[i].mul(P[i-1].z);
	}

	t1.copy(work[m-1]); t1.mul(P[m-1].z);

	t1.inverse();

	t2.copy(P[m-1].z);
	work[m-1].mul(t1);

	for (i=m-2;;i--)
	{
		if (i==0)
		{
			work[0].copy(t1);
			work[0].mul(t2);
			break;
		}
		work[i].mul(t2);
		work[i].mul(t1);
		t2.mul(P[i].z);
	}
/* now work[] contains inverses of all Z coordinates */

	for (i=0;i<m;i++)
	{
		P[i].z.one();
		t1.copy(work[i]); t1.sqr();
		P[i].x.mul(t1);
		t1.mul(work[i]);
		P[i].y.mul(t1);
	}    
};

/* P=u0.Q0+u1*Q1+u2*Q2+u3*Q3 */
ECP2.mul4=function(Q,u)
{
	var i,j,nb;
	var a=[];
	var T=new ECP2();
	var C=new ECP2();
	var P=new ECP2();
	var W=[];
	var mt=new BIG();
	var t=[];
	var w=[];

	for (i=0;i<4;i++)
	{
		t[i]=new BIG(u[i]);
		Q[i].affine();
	}

/* precompute table */

	W[0]=new ECP2(); W[0].copy(Q[0]); W[0].sub(Q[1]);
	W[1]=new ECP2(); W[1].copy(W[0]);
	W[2]=new ECP2(); W[2].copy(W[0]);
	W[3]=new ECP2(); W[3].copy(W[0]);
	W[4]=new ECP2(); W[4].copy(Q[0]); W[4].add(Q[1]);
	W[5]=new ECP2(); W[5].copy(W[4]);
	W[6]=new ECP2(); W[6].copy(W[4]);
	W[7]=new ECP2(); W[7].copy(W[4]);
	T.copy(Q[2]); T.sub(Q[3]);
	W[1].sub(T);
	W[2].add(T);
	W[5].sub(T);
	W[6].add(T);
	T.copy(Q[2]); T.add(Q[3]);
	W[0].sub(T);
	W[3].add(T);
	W[4].sub(T);
	W[7].add(T);

	ECP2.multiaffine(8,W);

/* if multiplier is even add 1 to multiplier, and add P to correction */
	mt.zero(); C.inf();
	for (i=0;i<4;i++)
	{
		if (t[i].parity()==0)
		{
			t[i].inc(1); t[i].norm();
			C.add(Q[i]);
		}
		mt.add(t[i]); mt.norm();
	}

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

	P.copy(W[Math.floor((w[nb]-1)/2)]);  

	for (i=nb-1;i>=0;i--)
	{
		T.select(W,w[i]);
		P.dbl();
		P.add(T);
	}
	P.sub(C); /* apply correction */

	P.affine();
	return P;
};

/* return 1 if b==c, no branching */
ECP2.teq=function(b,c)
{
	var x=b^c;
	x-=1;  // if x=0, x now -1
	return ((x>>31)&1);
};

