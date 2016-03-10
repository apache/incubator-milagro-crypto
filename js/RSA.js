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

/* RSA API Functions */

var rsa_private_key=function(n)
{
	this.p=new FF(n);
	this.q=new FF(n);
	this.dp=new FF(n);
	this.dq=new FF(n);
	this.c=new FF(n);
};

var rsa_public_key=function(m)
{
	this.e=0;
	this.n=new FF(m);
};



RSA= {
	RFS: ROM.MODBYTES*ROM.FFLEN,

	bytestohex: function(b)
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

	bytestostring: function(b)
	{
		var s="";
		for (var i=0;i<b.length;i++)
		{
			s+=String.fromCharCode(b[i]);
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


	KEY_PAIR: function(rng,e,PRIV,PUB)
	{ /* IEEE1363 A16.11/A16.12 more or less */

	//	var m,r,bytes,hbytes,words,err,res=0;
		var n=PUB.n.length>>1;
		var t = new FF(n);
		var p1=new FF(n);
		var q1=new FF(n);

		for (;;)
		{

			PRIV.p.random(rng);
			while (PRIV.p.lastbits(2)!=3) PRIV.p.inc(1);
			while (!FF.prime(PRIV.p,rng)) PRIV.p.inc(4);

			p1.copy(PRIV.p);
			p1.dec(1);

			if (p1.cfactor(e)) continue;
			break;
		}

		for (;;)
		{
			PRIV.q.random(rng);
			while (PRIV.q.lastbits(2)!=3) PRIV.q.inc(1);
			while (!FF.prime(PRIV.q,rng)) PRIV.q.inc(4);

			q1.copy(PRIV.q);
			q1.dec(1);

			if (q1.cfactor(e)) continue;
			break;
		}

		PUB.n=FF.mul(PRIV.p,PRIV.q);
		PUB.e=e;

		t.copy(p1);
		t.shr();
		PRIV.dp.set(e);
		PRIV.dp.invmodp(t);
		if (PRIV.dp.parity()===0) PRIV.dp.add(t);
		PRIV.dp.norm();

		t.copy(q1);
		t.shr();
		PRIV.dq.set(e);
		PRIV.dq.invmodp(t);
		if (PRIV.dq.parity()===0) PRIV.dq.add(t);
		PRIV.dq.norm();

		PRIV.c.copy(PRIV.p);
		PRIV.c.invmodp(PRIV.q);

		return;
	},

/* Mask Generation Function */
	MGF1: function(Z,olen,K)
	{
		var H=new HASH();
		var i,hlen=H.len;
		var B=[];

		var counter,cthreshold,k=0;
		for (i=0;i<K.length;i++) K[i]=0;

		cthreshold=Math.floor(olen/hlen); if (olen%hlen!==0) cthreshold++;
		for (counter=0;counter<cthreshold;counter++)
		{
			H.process_array(Z); H.process_num(counter);
			B=H.hash();

			if (k+hlen>olen) for (i=0;i<olen%hlen;i++) K[k++]=B[i];
			else for (i=0;i<hlen;i++) K[k++]=B[i];
		}
	},

	/* OAEP Message Encoding for Encryption */
	OAEP_ENCODE: function(m,rng,p)
	{
		var i,slen,olen=RSA.RFS-1;
		var mlen=m.length;
		var hlen,seedlen;
		var f=[];

		var H=new HASH();
		hlen=H.len;
		var SEED=[];
		seedlen=hlen;
		if (mlen>olen-hlen-seedlen-1) return null;

		var DBMASK=[];

		if (p!==null) H.process_array(p);
		var h=H.hash();
		for (i=0;i<hlen;i++) f[i]=h[i];

		slen=olen-mlen-hlen-seedlen-1;

		for (i=0;i<slen;i++) f[hlen+i]=0;
		f[hlen+slen]=1;
		for (i=0;i<mlen;i++) f[hlen+slen+1+i]=m[i];

		for (i=0;i<seedlen;i++) SEED[i]=rng.getByte();
		this.MGF1(SEED,olen-seedlen,DBMASK);

		for (i=0;i<olen-seedlen;i++) DBMASK[i]^=f[i];
		this.MGF1(DBMASK,seedlen,f);

		for (i=0;i<seedlen;i++) f[i]^=SEED[i];

		for (i=0;i<olen-seedlen;i++) f[i+seedlen]=DBMASK[i];

		/* pad to length RFS */
		var d=1;
		for (i=RSA.RFS-1;i>=d;i--)
			f[i]=f[i-d];
		for (i=d-1;i>=0;i--)
			f[i]=0;

		return f;
	},

	/* OAEP Message Decoding for Decryption */
	OAEP_DECODE: function(p,f)
	{
		var x,t;
		var comp;
		var i,k,olen=RSA.RFS-1;
		var hlen,seedlen;

		var H=new HASH();
		hlen=H.len;
		var SEED=[];
		seedlen=hlen;
		var CHASH=[];
		seedlen=hlen=32;
		if (olen<seedlen+hlen+1) return null;
		var DBMASK=[];
		for (i=0;i<olen-seedlen;i++) DBMASK[i]=0;

		if (f.length<RSA.RFS)
		{
			var d=RSA.RFS-f.length;
			for (i=RFS-1;i>=d;i--)
				f[i]=f[i-d];
			for (i=d-1;i>=0;i--)
				f[i]=0;

		}

		if (p!==null) H.process_array(p);
		var h=H.hash();
		for (i=0;i<hlen;i++) CHASH[i]=h[i];

		x=f[0];

		for (i=seedlen;i<olen;i++)
			DBMASK[i-seedlen]=f[i+1];

		this.MGF1(DBMASK,seedlen,SEED);
		for (i=0;i<seedlen;i++) SEED[i]^=f[i+1];
		this.MGF1(SEED,olen-seedlen,f);
		for (i=0;i<olen-seedlen;i++) DBMASK[i]^=f[i];

		comp=true;
		for (i=0;i<hlen;i++)
		{
			if (CHASH[i]!=DBMASK[i]) comp=false;
		}

		for (i=0;i<olen-seedlen-hlen;i++)
			DBMASK[i]=DBMASK[i+hlen];

		for (i=0;i<hlen;i++)
			SEED[i]=CHASH[i]=0;

		for (k=0;;k++)
		{
			if (k>=olen-seedlen-hlen) return null;
			if (DBMASK[k]!==0) break;
		}

		t=DBMASK[k];
		if (!comp || x!==0 || t!=0x01)
		{
			for (i=0;i<olen-seedlen;i++) DBMASK[i]=0;
			return null;
		}

		var r=[];

		for (i=0;i<olen-seedlen-hlen-k-1;i++)
			r[i]=DBMASK[i+k+1];

		for (i=0;i<olen-seedlen;i++) DBMASK[i]=0;

		return r;
	},

	/* destroy the Private Key structure */
	PRIVATE_KEY_KILL: function(PRIV)
	{
		PRIV.p.zero();
		PRIV.q.zero();
		PRIV.dp.zero();
		PRIV.dq.zero();
		PRIV.c.zero();
	},

	/* RSA encryption with the public key */
	ENCRYPT: function(PUB,F,G)
	{
		var n=PUB.n.getlen();
		var f=new FF(n);

		FF.fromBytes(f,F);
		f.power(PUB.e,PUB.n);
		f.toBytes(G);
	},

	/* RSA decryption with the private key */
	DECRYPT: function(PRIV,G,F)
	{
		var n=PRIV.p.getlen();
		var g=new FF(2*n);

		FF.fromBytes(g,G);
		var jp=g.dmod(PRIV.p);
		var jq=g.dmod(PRIV.q);

		jp.skpow(PRIV.dp,PRIV.p);
		jq.skpow(PRIV.dq,PRIV.q);

		g.zero();
		g.dscopy(jp);
		jp.mod(PRIV.q);
		if (FF.comp(jp,jq)>0) jq.add(PRIV.q);
		jq.sub(jp);
		jq.norm();

		var t=FF.mul(PRIV.c,jq);
		jq=t.dmod(PRIV.q);

		t=FF.mul(jq,PRIV.p);
		g.add(t);
		g.norm();

		g.toBytes(F);
	}

};
