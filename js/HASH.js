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

var HASH = function() {
	this.length=[];
	this.h=[];
	this.w=[];
	this.init();
};

HASH.prototype={

	len: 32,
	/* functions */
	S: function(n,x)
	{
		return (((x)>>>n) | ((x)<<(32-n)));
	},

	R: function(n,x)
	{
		return ((x)>>>n);
	},

	Ch: function(x,y,z)
	{
		return ((x&y)^(~(x)&z));
	},

	Maj: function(x,y,z)
	{
		return ((x&y)^(x&z)^(y&z));
	},

	Sig0: function(x)
	{
		return (this.S(2,x)^this.S(13,x)^this.S(22,x));
	},

	Sig1: function(x)
	{
		return (this.S(6,x)^this.S(11,x)^this.S(25,x));
	},

	theta0: function(x)
	{
		return (this.S(7,x)^this.S(18,x)^this.R(3,x));
	},

	theta1: function(x)
	{
		return (this.S(17,x)^this.S(19,x)^this.R(10,x));
	},

	transform: function()
	{ /* basic transformation step */
		var a,b,c,d,e,f,g,hh,t1,t2;
		var j;
		for (j=16;j<64;j++)
			this.w[j]=(this.theta1(this.w[j-2])+this.w[j-7]+this.theta0(this.w[j-15])+this.w[j-16])|0;

		a=this.h[0]; b=this.h[1]; c=this.h[2]; d=this.h[3];
		e=this.h[4]; f=this.h[5]; g=this.h[6]; hh=this.h[7];

		for (j=0;j<64;j++)
		{ /* 64 times - mush it up */
			t1=(hh+this.Sig1(e)+this.Ch(e,f,g)+ROM.HK[j]+this.w[j])|0;
			t2=(this.Sig0(a)+this.Maj(a,b,c))|0;
			hh=g; g=f; f=e;
			e=(d+t1)|0; // Need to knock these back down to prevent 52-bit overflow
			d=c;
			c=b;
			b=a;
			a=(t1+t2)|0;

		}
		this.h[0]+=a; this.h[1]+=b; this.h[2]+=c; this.h[3]+=d;
		this.h[4]+=e; this.h[5]+=f; this.h[6]+=g; this.h[7]+=hh;

		this.h[0]|=0;
		this.h[1]|=0;
		this.h[2]|=0;
		this.h[3]|=0;
		this.h[4]|=0;
		this.h[5]|=0;
		this.h[6]|=0;
		this.h[7]|=0;
	},

/* Initialise Hash function */
	init: function()
	{ /* initialise */
		var i;
		for (i=0;i<64;i++) this.w[i]=0;
		this.length[0]=this.length[1]=0;
		this.h[0]=ROM.H0;
		this.h[1]=ROM.H1;
		this.h[2]=ROM.H2;
		this.h[3]=ROM.H3;
		this.h[4]=ROM.H4;
		this.h[5]=ROM.H5;
		this.h[6]=ROM.H6;
		this.h[7]=ROM.H7;
	},

/* process a single byte */
	process: function(byt)
	{ /* process the next message byte */
		var cnt;

		cnt=(this.length[0]>>>5)%16;
		this.w[cnt]<<=8;
		this.w[cnt]|=(byt&0xFF);
		this.length[0]+=8;
		if ((this.length[0]&0xffffffff)===0) { this.length[1]++; this.length[0]=0; }
		if ((this.length[0]%512)===0) this.transform();
	},

/* process an array of bytes */
	process_array: function(b)
	{
		for (var i=0;i<b.length;i++) this.process(b[i]);
	},

/* process a 32-bit integer */
	process_num: function(n)
	{
		this.process((n>>24)&0xff);
		this.process((n>>16)&0xff);
		this.process((n>>8)&0xff);
		this.process(n&0xff);
	},

	hash: function()
	{ /* pad message and finish - supply digest */
		var i;
		var digest=[];
		var len0,len1;
		len0=this.length[0];
		len1=this.length[1];
		this.process(0x80);
		while ((this.length[0]%512)!=448) this.process(0);

		this.w[14]=len1;
		this.w[15]=len0;
		this.transform();

		for (i=0;i<32;i++)
		{ /* convert to bytes */
			digest[i]=((this.h[i>>>2]>>(8*(3-i%4))) & 0xff);
		}
		this.init();
		return digest;
	}

};


