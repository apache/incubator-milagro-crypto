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

/* MPIN API Functions */

var MPIN = {
	BAD_PARAMS:-11,
	INVALID_POINT:-14,
	WRONG_ORDER:-18,
	BAD_PIN:-19,
/* configure PIN here */
	MAXPIN:10000,  /* max PIN */
	PBLEN:14,     /* MAXPIN length in bits */
	TS:10,        /* 10 for 4 digit PIN, 14 for 6-digit PIN - 2^TS/TS approx = sqrt(MAXPIN) */
	TRAP:200,     /* 200 for 4 digit PIN, 2000 for 6-digit PIN  - approx 2*sqrt(MAXPIN) */
	EFS:ROM.MODBYTES,
	EGS:ROM.MODBYTES,
	PAS:16,

	SHA256 : 32,
	SHA384 : 48,
	SHA512 : 64,

	HASH_TYPE : 32,


/* return time in slots since epoch */
	today: function() {
		var now=new Date();
		return Math.floor(now.getTime()/(60000*1440));  // for daily tokens
	},

	bytestostring: function(b)
	{
		var s="";
		var len=b.length;
		var ch;

		for (var i=0;i<len;i++)
		{
			ch=b[i];
			s+=((ch>>>4)&15).toString(16);
			s+=(ch&15).toString(16);

		}
		return s;
	},

	stringtobytes: function(s)
	{
		var b=[];
		for (var i=0;i<s.length;i++)
			b.push(s.charCodeAt(i));
		return b;
	},

	comparebytes: function(a,b)
	{
		if (a.length!=b.length) return false;
		for (var i=0;i<a.length;i++)
		{
			if (a[i]!=b[i]) return false;
		}
		return true;
	},

	mpin_hash: function(sha,c,U)
	{
		var t=[];
		var w=[];
		var h=[];

		c.geta().getA().toBytes(w); for (var i=0;i<this.EFS;i++) t[i]=w[i];
		c.geta().getB().toBytes(w); for (var i=this.EFS;i<2*this.EFS;i++) t[i]=w[i-this.EFS];
		c.getb().getA().toBytes(w); for (var i=2*this.EFS;i<3*this.EFS;i++) t[i]=w[i-2*this.EFS];
		c.getb().getB().toBytes(w); for (var i=3*this.EFS;i<4*this.EFS;i++) t[i]=w[i-3*this.EFS];
		
		U.getX().toBytes(w); for (var i=4*this.EFS;i<5*this.EFS;i++) t[i]=w[i-4*this.EFS];
		U.getY().toBytes(w); for (var i=5*this.EFS;i<6*this.EFS;i++) t[i]=w[i-5*this.EFS];
	
		if (sha==this.SHA256)
		{
			var H=new HASH256();
			H.process_array(t);
			h=H.hash();
		}
		if (sha==this.SHA384)
		{
			var H=new HASH384();
			H.process_array(t);
			h=H.hash();
		}
		if (sha==this.SHA512)
		{
			var H=new HASH512();
			H.process_array(t);
			h=H.hash();
		}
		if (h.length==0) return null;
		var R=[];
		for (var i=0;i<this.PAS;i++) R[i]=h[i];
		return R;
	},
/* Hash number (optional) and string to point on curve */

	hashit: function(sha,n,B)
	{
		var R=[];

		if (sha==this.SHA256)
		{
			var H=new HASH256();
			if (n>0) H.process_num(n);
			H.process_array(B);
			R=H.hash();
		}
		if (sha==this.SHA384)
		{
			var H=new HASH384();
			if (n>0) H.process_num(n);
			H.process_array(B);
			R=H.hash();
		}
		if (sha==this.SHA512)
		{
			var H=new HASH512();
			if (n>0) H.process_num(n);
			H.process_array(B);
			R=H.hash();
		}
		if (R.length==0) return null;
		var W=[];

		if (sha>=ROM.MODBYTES)
			for (var i=0;i<ROM.MODBYTES;i++) W[i]=R[i];
		else
		{
			for (var i=0;i<sha;i++) W[i]=R[i];
			for (var i=sha;i<ROM.MODBYTES;i++) W[i]=0;
		}
		return W;
	},

	mapit: function(h)
	{
		var q=new BIG(0); q.rcopy(ROM.Modulus);
		var x=BIG.fromBytes(h);
		x.mod(q);
		var P=new ECP();
		while (true)
		{
			P.setxi(x,0);
			if (!P.is_infinity()) break;
			x.inc(1); x.norm();
		}
		if (ROM.CURVE_PAIRING_TYPE!=ROM.BN_CURVE)
		{
			var c=new BIG(0); c.rcopy(ROM.CURVE_Cof);
			P=P.mul(c);
		}
		return P;
	},

/* needed for SOK */
	mapit2: function(h)
	{
		var q=new BIG(0); q.rcopy(ROM.Modulus);
		var x=BIG.fromBytes(h);
		var one=new BIG(1);
		x.mod(q);
		var Q,T,K,X;
		while (true)
		{
			X=new FP2(one,x);
			Q=new ECP2(); Q.setx(X);
			if (!Q.is_infinity()) break;
			x.inc(1); x.norm();
		}
/* Fast Hashing to G2 - Fuentes-Castaneda, Knapp and Rodriguez-Henriquez */

		var Fa=new BIG(0); Fa.rcopy(ROM.CURVE_Fra);
		var Fb=new BIG(0); Fb.rcopy(ROM.CURVE_Frb);
		X=new FP2(Fa,Fb); 
		x=new BIG(0); x.rcopy(ROM.CURVE_Bnx);

		T=new ECP2(); T.copy(Q);
		T.mul(x); T.neg();
		K=new ECP2(); K.copy(T);
		K.dbl(); K.add(T); K.affine();

		K.frob(X);
		Q.frob(X); Q.frob(X); Q.frob(X);
		Q.add(T); Q.add(K);
		T.frob(X); T.frob(X);
		Q.add(T);
		Q.affine();
		return Q;

	},

/* these next two functions help to implement elligator squared - http://eprint.iacr.org/2014/043 */
/* maps a random u to a point on the curve */
	map: function(u,cb)
	{
		var P=new ECP();
		var x=new BIG(u);
		var p=new BIG(0); p.rcopy(ROM.Modulus);
		x.mod(p);
		while (true)
		{
			P.setxi(x,cb);
			if (!P.is_infinity()) break;
			x.inc(1);  x.norm();
		}
		return P;
	},

/* returns u derived from P. Random value in range 1 to return value should then be added to u */
	unmap: function(u,P)
	{
		var s=P.getS();
		var R=new ECP();
		var r=0;
		var x=P.getX();
		u.copy(x);
		while (true)
		{
			u.dec(1); u.norm();
			r++;
			R.setxi(u,s); //=new ECP(u,s);
			if (!R.is_infinity()) break;
		}
		return r;
	},

/* these next two functions implement elligator squared - http://eprint.iacr.org/2014/043 */
/* Elliptic curve point E in format (0x04,x,y} is converted to form {0x0-,u,v} */
/* Note that u and v are indistinguisible from random strings */
	ENCODING: function(rng,E)
	{
		var i,rn,m,su,sv;
		var T=[];

		for (i=0;i<this.EFS;i++) T[i]=E[i+1];
		var u=BIG.fromBytes(T);
		for (i=0;i<this.EFS;i++) T[i]=E[i+this.EFS+1];
		var v=BIG.fromBytes(T);
		
		var P=new ECP(0); P.setxy(u,v);
		if (P.is_infinity()) return this.INVALID_POINT;

		var p=new BIG(0); p.rcopy(ROM.Modulus);
		u=BIG.randomnum(p,rng);

		su=rng.getByte(); if (su<0) su=-su; su%=2;
		
		var W=this.map(u,su);
		P.sub(W);
		sv=P.getS();
		rn=this.unmap(v,P);
		m=rng.getByte(); if (m<0) m=-m; m%=rn;
		v.inc(m+1);
		E[0]=(su+2*sv);
		u.toBytes(T);
		for (i=0;i<this.EFS;i++) E[i+1]=T[i];
		v.toBytes(T);
		for (i=0;i<this.EFS;i++) E[i+this.EFS+1]=T[i];		
		
		return 0;
	},

	DECODING: function(D)
	{
		var i,su,sv;
		var T=[];

		if ((D[0]&0x04)!==0) return this.INVALID_POINT;

		for (i=0;i<this.EFS;i++) T[i]=D[i+1];
		var u=BIG.fromBytes(T);
		for (i=0;i<this.EFS;i++) T[i]=D[i+this.EFS+1];
		var v=BIG.fromBytes(T);

		su=D[0]&1;
		sv=(D[0]>>1)&1;
		var W=this.map(u,su);
		var P=this.map(v,sv);
		P.add(W);
		u=P.getX();
		v=P.getY();
		D[0]=0x04;
		u.toBytes(T);
		for (i=0;i<this.EFS;i++) D[i+1]=T[i];
		v.toBytes(T);
		for (i=0;i<this.EFS;i++) D[i+this.EFS+1]=T[i];		
		
		return 0;
	},

/* R=R1+R2 in group G1 */
	RECOMBINE_G1: function(R1,R2,R)
	{
		var P=ECP.fromBytes(R1);
		var Q=ECP.fromBytes(R2);

		if (P.is_infinity() || Q.is_infinity()) return this.INVALID_POINT;

		P.add(Q);

		P.toBytes(R);
		return 0;
	},

/* W=W1+W2 in group G2 */
	RECOMBINE_G2: function(W1,W2,W)
	{
		var P=ECP2.fromBytes(W1);
		var Q=ECP2.fromBytes(W2);

		if (P.is_infinity() || Q.is_infinity()) return this.INVALID_POINT;

		P.add(Q);
	
		P.toBytes(W);
		return 0;
	},

	HASH_ID: function(sha,ID)
	{
		return this.hashit(sha,0,ID);
	},

/* create random secret S */
	RANDOM_GENERATE: function(rng,S)
	{
		var r=new BIG(0); r.rcopy(ROM.CURVE_Order);
		var s=BIG.randomnum(r,rng);
		if (ROM.AES_S>0)
		{
			s.mod2m(2*ROM.AES_S);
		}		
		s.toBytes(S);
		return 0;
	},

/* Extract PIN from TOKEN for identity CID */
	EXTRACT_PIN: function(sha,CID,pin,TOKEN)
	{
		var P=ECP.fromBytes(TOKEN);
		if (P.is_infinity()) return this.INVALID_POINT;
		var h=this.hashit(sha,0,CID);
		var R=this.mapit(h);

		pin%=this.MAXPIN;

		R=R.pinmul(pin,this.PBLEN);
		P.sub(R);

		P.toBytes(TOKEN);

		return 0;
	},

/* Extract Server Secret SST=S*Q where Q is fixed generator in G2 and S is master secret */
	GET_SERVER_SECRET: function(S,SST)
	{

		var A=new BIG(0);
		var B=new BIG(0);
		A.rcopy(ROM.CURVE_Pxa); B.rcopy(ROM.CURVE_Pxb);
		var QX=new FP2(0); QX.bset(A,B);
		A.rcopy(ROM.CURVE_Pya); B.rcopy(ROM.CURVE_Pyb);
		var QY=new FP2(0); QY.bset(A,B);

		var Q=new ECP2();
		Q.setxy(QX,QY);

		var s=BIG.fromBytes(S);
		Q=PAIR.G2mul(Q,s);
		Q.toBytes(SST);
		return 0;
	},

	TEST_PAIR: function(PR)
	{
		var G=new ECP(0);
		var A=new BIG(0);
		var B=new BIG(0);
		A.rcopy(ROM.CURVE_Pxa); B.rcopy(ROM.CURVE_Pxb);
		var QX=new FP2(0); QX.bset(A,B);
		A.rcopy(ROM.CURVE_Pya); B.rcopy(ROM.CURVE_Pyb);
		var QY=new FP2(0); QY.bset(A,B);

		var Q=new ECP2();
		Q.setxy(QX,QY);		

		var gx=new BIG(0); gx.rcopy(ROM.CURVE_Gx);	
		var gy=new BIG(0); gy.rcopy(ROM.CURVE_Gy);
		G.setxy(gx,gy);			

		var g=PAIR.ate(Q,G);
		g=PAIR.fexp(g);	
		g.toBytes(PR);
	},

/*
 W=x*H(G);
 if RNG == NULL then X is passed in 
 if RNG != NULL the X is passed out 
 if type=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
*/
	GET_G1_MULTIPLE: function(rng,type,X,G,W)
	{
		var x;
		var r=new BIG(0); r.rcopy(ROM.CURVE_Order);

		if (rng!=null)
		{
			x=BIG.randomnum(r,rng);
			if (ROM.AES_S>0)
			{
				x.mod2m(2*ROM.AES_S);
			}
			x.toBytes(X);
		}
		else
		{
			x=BIG.fromBytes(X);
		}
		var P;
		if (type==0)
		{
			P=ECP.fromBytes(G);
			if (P.is_infinity()) return INVALID_POINT;
		}
		else
			P=this.mapit(G);

		PAIR.G1mul(P,x).toBytes(W);
		return 0;
	},


/* Client secret CST=S*H(CID) where CID is client ID and S is master secret */
	GET_CLIENT_SECRET: function(S,CID,CST)
	{
		return this.GET_G1_MULTIPLE(null,1,S,CID,CST);
	},

/* Time Permit CTT=S*(date|H(CID)) where S is master secret */
	GET_CLIENT_PERMIT: function(sha,date,S,CID,CTT)
	{
		var h=this.hashit(sha,date,CID);
		var P=this.mapit(h);

		var s=BIG.fromBytes(S);
		P=PAIR.G1mul(P,s);
		P.toBytes(CTT);
		return 0;
	},

/* Implement step 1 on client side of MPin protocol */
	CLIENT_1: function(sha,date,CLIENT_ID,rng,X,pin,TOKEN,SEC,xID,xCID,PERMIT)
	{
		var r=new BIG(0); r.rcopy(ROM.CURVE_Order);
	//	var q=new BIG(0); q.rcopy(ROM.Modulus);
		var x;
		if (rng!==null)
		{
			x=BIG.randomnum(r,rng);
			if (ROM.AES_S>0)
			{
				x.mod2m(2*ROM.AES_S);
			}
			x.toBytes(X);
		}
		else
		{
			x=BIG.fromBytes(X);
		}
		var P,T,W;

		var h=this.hashit(sha,0,CLIENT_ID);
		P=this.mapit(h);
		T=ECP.fromBytes(TOKEN);
		if (T.is_infinity()) return this.INVALID_POINT;

		pin%=this.MAXPIN;
		W=P.pinmul(pin,this.PBLEN);
		T.add(W);

		if (date!=0)
		{
			W=ECP.fromBytes(PERMIT);
			if (W.is_infinity()) return this.INVALID_POINT;
			T.add(W);
			h=this.hashit(sha,date,h);
			W=this.mapit(h);
			if (xID!=null)
			{
				P=PAIR.G1mul(P,x);
				P.toBytes(xID);
				W=PAIR.G1mul(W,x);
				P.add(W);
			}
			else
			{
				P.add(W);
				P=PAIR.G1mul(P,x);
			}
			if (xCID!=null) P.toBytes(xCID);
		}
		else
		{
			if (xID!=null)
			{
				P=PAIR.G1mul(P,x);
				P.toBytes(xID);
			}
		}		
		
		T.toBytes(SEC);
		return 0;
	},

/* Implement step 2 on client side of MPin protocol */
	CLIENT_2: function(X,Y,SEC)
	{
		var r=new BIG(0); r.rcopy(ROM.CURVE_Order);
		var P=ECP.fromBytes(SEC);
		if (P.is_infinity()) return this.INVALID_POINT;

		var px=BIG.fromBytes(X);
		var py=BIG.fromBytes(Y);
		px.add(py);
		px.mod(r);
	//	px.rsub(r);

		P=PAIR.G1mul(P,px);
		P.neg();
		P.toBytes(SEC);
		//PAIR.G1mul(P,px).toBytes(SEC);
		return 0;
	},

/* Outputs H(CID) and H(T|H(CID)) for time permits. If no time permits set HID=HTID */
	SERVER_1: function(sha,date,CID,HID,HTID)
	{
		var h=this.hashit(sha,0,CID);
		var R,P=this.mapit(h);

		P.toBytes(HID);
		if (date!==0)
		{
			//if (HID!=null) P.toBytes(HID);
			h=this.hashit(sha,date,h);
			R=this.mapit(h);
			P.add(R);
			P.toBytes(HTID);
		}
		//else P.toBytes(HID);
	},

/* Implement step 1 of MPin protocol on server side */
	SERVER_2: function(date,HID,HTID,Y,SST,xID,xCID,mSEC,E,F)
	{
		var A=new BIG(0);
		var B=new BIG(0);
		A.rcopy(ROM.CURVE_Pxa); B.rcopy(ROM.CURVE_Pxb);
		var QX=new FP2(0); QX.bset(A,B);
		A.rcopy(ROM.CURVE_Pya); B.rcopy(ROM.CURVE_Pyb);
		var QY=new FP2(0); QY.bset(A,B);

		var Q=new ECP2();
		Q.setxy(QX,QY);

		var sQ=ECP2.fromBytes(SST);
		if (sQ.is_infinity()) return this.INVALID_POINT;	

		var R;
		if (date!==0)
			R=ECP.fromBytes(xCID);
		else 
		{
			if (xID==null) return this.BAD_PARAMS;
			R=ECP.fromBytes(xID);
		}
		if (R.is_infinity()) return this.INVALID_POINT;

		var y=BIG.fromBytes(Y);
		var P;

		if (date!=0) P=ECP.fromBytes(HTID);
		else
		{
			if (HID==null) return this.BAD_PARAMS;
			P=ECP.fromBytes(HID);
		}
		if (P.is_infinity()) return this.INVALID_POINT;

		P=PAIR.G1mul(P,y);
		P.add(R);
		R=ECP.fromBytes(mSEC);
		if (R.is_infinity()) return this.INVALID_POINT;

		var g=PAIR.ate2(Q,R,sQ,P);
		g=PAIR.fexp(g);

		if (!g.isunity())
		{
			if (HID!=null && xID!=null && E!=null && F!=null)
			{
				g.toBytes(E);
				if (date!==0)
				{
					P=ECP.fromBytes(HID);
					if (P.is_infinity()) return this.INVALID_POINT;
					R=ECP.fromBytes(xID);
					if (R.is_infinity()) return this.INVALID_POINT;

					P=PAIR.G1mul(P,y);
					P.add(R);
				}
				g=PAIR.ate(Q,P);
				g=PAIR.fexp(g);

				g.toBytes(F);
			}
			return this.BAD_PIN;
		}
		return 0;
	},

/* Pollards kangaroos used to return PIN error */
	KANGAROO: function(E,F)
	{
		var ge=FP12.fromBytes(E);
		var gf=FP12.fromBytes(F);
		var distance = [];
		var t=new FP12(gf);
		var table=[];
		var i,j,m,s,dn,dm,res,steps;

		s=1;
		for (m=0;m<this.TS;m++)
		{
			distance[m]=s;
			table[m]=new FP12(t);
			s*=2;
			t.usqr();
		}
		t.one();
		dn=0;
		for (j=0;j<this.TRAP;j++)
		{
			i=t.geta().geta().getA().lastbits(20)%this.TS;
			t.mul(table[i]);
			dn+=distance[i];
		}
		gf.copy(t); gf.conj();
		steps=0; dm=0;
		res=0;
		while (dm-dn<this.MAXPIN)
		{
			steps++;
			if (steps>4*this.TRAP) break;
			i=ge.geta().geta().getA().lastbits(20)%this.TS;
			ge.mul(table[i]);
			dm+=distance[i];
			if (ge.equals(t))
			{
				res=dm-dn;
				break;
			}
			if (ge.equals(gf))
			{
				res=dn-dm;
				break;
			}

		}
		if (steps>4*this.TRAP || dm-dn>=this.MAXPIN) {res=0; }    // Trap Failed  - probable invalid token
		return res;
	},

        /* return time  since epoch */
	GET_TIME: function() {
		var now=new Date();
		return Math.floor(now.getTime()/(1000));
	},

        /* y = H(time,xCID) */
	GET_Y: function(sha,TimeValue,xCID,Y)
	{
		var q=new BIG(0); 
		q.rcopy(ROM.CURVE_Order);
		var h=this.hashit(sha,TimeValue,xCID);
		var y=BIG.fromBytes(h);
		y.mod(q);
		if (ROM.AES_S>0)
		{
			y.mod2m(2*ROM.AES_S);
		}
		y.toBytes(Y);
		return 0;
	},

        /* One pass MPIN Client */
	CLIENT: function(sha,date,CLIENT_ID,rng,X,pin,TOKEN,SEC,xID,xCID,PERMIT,TimeValue,Y)
	{

                var rtn=0;
                var pID;
                if (date == 0) {
                  pID = xID;
		} else {
                  pID = xCID;
                  xID = null;
		}

                rtn = this.CLIENT_1(sha,date,CLIENT_ID,rng,X,pin,TOKEN,SEC,xID,xCID,PERMIT);
                if (rtn != 0)
                  return rtn;

                this.GET_Y(sha,TimeValue,pID,Y);

                rtn = this.CLIENT_2(X,Y,SEC);
                if (rtn != 0)
                  return rtn;

                return 0;
        },

        /* One pass MPIN Server */
	SERVER: function(sha,date,HID,HTID,Y,SST,xID,xCID,mSEC,E,F,CID,TimeValue)
        {
                var rtn=0;
                var pID;
                if (date == 0) {
                  pID = xID;
		} else {
                  pID = xCID;
		}

                this.SERVER_1(sha,date,CID,HID,HTID);

                this.GET_Y(sha,TimeValue,pID,Y);
  
                rtn = this.SERVER_2(date,HID,HTID,Y,SST,xID,xCID,mSEC,E,F);
                if (rtn != 0)
                  return rtn;

                return 0;
        },

/* Functions to support M-Pin Full */

	PRECOMPUTE: function(TOKEN,CID,G1,G2)
	{
		var P,T;
		var g;
		
		T=ECP.fromBytes(TOKEN);
		if (T.is_infinity()) return INVALID_POINT; 

		P=this.mapit(CID);

		var A=new BIG(0);
		var B=new BIG(0);
		A.rcopy(ROM.CURVE_Pxa); B.rcopy(ROM.CURVE_Pxb);
		var QX=new FP2(0); QX.bset(A,B);
		A.rcopy(ROM.CURVE_Pya); B.rcopy(ROM.CURVE_Pyb);
		var QY=new FP2(0); QY.bset(A,B);

		var Q=new ECP2();
		Q.setxy(QX,QY);

		g=PAIR.ate(Q,T);
		g=PAIR.fexp(g);
		g.toBytes(G1);

		g=PAIR.ate(Q,P);
		g=PAIR.fexp(g);
		g.toBytes(G2);

		return 0;
	},

/* Hash the M-Pin transcript - new */

	HASH_ALL: function(sha,HID,xID,xCID,SEC,Y,R,W)
	{
		var tlen=0;
		var T=[];


		for (var i=0;i<HID.length;i++) T[i]=HID[i];
		tlen+=HID.length;
		if (xCID!=null)
		{
			for (var i=0;i<xCID.length;i++) T[i+tlen]=xCID[i];
			tlen+=xCID.length;
		}	
		else
		{
			for (i=0;i<xID.length;i++) T[i+tlen]=xID[i];
			tlen+=xID.length;
		}	
		for (var i=0;i<SEC.length;i++) T[i+tlen]=SEC[i];
		tlen+=SEC.length;		
		for (var i=0;i<Y.length;i++) T[i+tlen]=Y[i];
		tlen+=Y.length;	
		for (var i=0;i<R.length;i++) T[i+tlen]=R[i];
		tlen+=R.length;		
		for (var i=0;i<W.length;i++) T[i+tlen]=W[i];
		tlen+=W.length;		

		return this.hashit(sha,0,T);
	},

/* calculate common key on client side */
/* wCID = w.(A+AT) */
	CLIENT_KEY: function(sha,G1,G2,pin,R,X,H,wCID,CK)
	{
		var t=[];

		var g1=FP12.fromBytes(G1);
		var g2=FP12.fromBytes(G2);
		var z=BIG.fromBytes(R);
		var x=BIG.fromBytes(X);
		var h=BIG.fromBytes(H);

		var W=ECP.fromBytes(wCID);
		if (W.is_infinity()) return this.INVALID_POINT; 

		W=PAIR.G1mul(W,x);

		var fa=new BIG(0); fa.rcopy(ROM.CURVE_Fra);
		var fb=new BIG(0); fb.rcopy(ROM.CURVE_Frb);
		var f=new FP2(fa,fb); //f.bset(fa,fb);

		var r=new BIG(0); r.rcopy(ROM.CURVE_Order);
		var q=new BIG(0); q.rcopy(ROM.Modulus);

		z.add(h);
		z.mod(r);

		var m=new BIG(q);
		m.mod(r);

		var a=new BIG(z);
		a.mod(m);

		var b=new BIG(z);
		b.div(m);

		g2.pinpow(pin,this.PBLEN);
		g1.mul(g2);

		var c=g1.trace();
		g2.copy(g1);
		g2.frob(f);
		var cp=g2.trace();
		g1.conj();
		g2.mul(g1);
		var cpm1=g2.trace();
		g2.mul(g1);
		var cpm2=g2.trace();

		c=c.xtr_pow2(cp,cpm1,cpm2,a,b);

		t=this.mpin_hash(sha,c,W);

		for (var i=0;i<this.PAS;i++) CK[i]=t[i];

		return 0;
	},

/* calculate common key on server side */
/* Z=r.A - no time permits involved */

	SERVER_KEY: function(sha,Z,SST,W,H,HID,xID,xCID,SK)
	{
		var t=[];

		var sQ=ECP2.fromBytes(SST);
		if (sQ.is_infinity()) return this.INVALID_POINT; 
		var R=ECP.fromBytes(Z);
		if (R.is_infinity()) return this.INVALID_POINT; 
		var A=ECP.fromBytes(HID);
		if (A.is_infinity()) return this.INVALID_POINT; 

		var U;
		if (xCID!=null)
			U=ECP.fromBytes(xCID);
		else
			U=ECP.fromBytes(xID);
		if (U.is_infinity()) return this.INVALID_POINT; 

		var w=BIG.fromBytes(W);
		var h=BIG.fromBytes(H);
		A=PAIR.G1mul(A,h);
		R.add(A);

		U=PAIR.G1mul(U,w);
		var g=PAIR.ate(sQ,R);
		g=PAIR.fexp(g);

		var c=g.trace();

		t=this.mpin_hash(sha,c,U);

		for (var i=0;i<this.PAS;i++) SK[i]=t[i];

		return 0;
	}
};
