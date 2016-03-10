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

var AES = function() {
	this.mode=0;
	this.fkey=[];
	this.rkey=[];
	this.f=[];
};

AES.prototype={
/* reset cipher */
	reset:function(m,iv)
	{ /* reset mode, or reset iv */
		var i;
		this.mode=m;
		for (i=0;i<16;i++)
			this.f[i]=0;
		if (this.mode!=this.ECB && iv!==null)
			for (i=0;i<16;i++)
				this.f[i]=iv[i];
	},

	getreg:function()
	{
		var ir=[];
		for (var i=0;i<16;i++) ir[i]=this.f[i];
		return ir;
	},

/* Initialise cipher */
	init: function(m,key,iv)
	{	/* Key=16 bytes */
		/* Key Scheduler. Create expanded encryption key */
		var i,j,k,N,nk;
		var CipherKey= [];
    	var b=[];
		nk=4;
		this.reset(m,iv);
		N=44;

		for (i=j=0;i<nk;i++,j+=4)
		{
			for (k=0;k<4;k++) b[k]=key[j+k];
			CipherKey[i]=AES.pack(b);
		}
		for (i=0;i<nk;i++) this.fkey[i]=CipherKey[i];
		for (j=nk,k=0;j<N;j+=nk,k++)
		{
			this.fkey[j]=this.fkey[j-nk]^AES.SubByte(AES.ROTL24(this.fkey[j-1]))^(ROM.rco[k])&0xff;
			for (i=1;i<nk && (i+j)<N;i++)
				this.fkey[i+j]=this.fkey[i+j-nk]^this.fkey[i+j-1];
		}

 /* now for the expanded decrypt key in reverse order */

		for (j=0;j<4;j++) this.rkey[j+N-4]=this.fkey[j];
		for (i=4;i<N-4;i+=4)
		{
			k=N-4-i;
			for (j=0;j<4;j++) this.rkey[k+j]=AES.InvMixCol(this.fkey[i+j]);
		}
		for (j=N-4;j<N;j++) this.rkey[j-N+4]=this.fkey[j];
	},

/* Encrypt a single block */
	ecb_encrypt: function(buff)
	{
		var i,j,k;
		var t;
    	var b=[];
    	var p=[];
    	var q=[];

		for (i=j=0;i<4;i++,j+=4)
		{
			for (k=0;k<4;k++) b[k]=buff[j+k];
			p[i]=AES.pack(b);
			p[i]^=this.fkey[i];
		}

		k=4;

/* State alternates between p and q */
		for (i=1;i<10;i++)
		{
			q[0]=this.fkey[k]^ROM.ftable[p[0]&0xff]^
				AES.ROTL8(ROM.ftable[(p[1]>>>8)&0xff])^
				AES.ROTL16(ROM.ftable[(p[2]>>>16)&0xff])^
				AES.ROTL24(ROM.ftable[(p[3]>>>24)&0xff]);
			q[1]=this.fkey[k+1]^ROM.ftable[p[1]&0xff]^
				AES.ROTL8(ROM.ftable[(p[2]>>>8)&0xff])^
				AES.ROTL16(ROM.ftable[(p[3]>>>16)&0xff])^
				AES.ROTL24(ROM.ftable[(p[0]>>>24)&0xff]);
			q[2]=this.fkey[k+2]^ROM.ftable[p[2]&0xff]^
				AES.ROTL8(ROM.ftable[(p[3]>>>8)&0xff])^
				AES.ROTL16(ROM.ftable[(p[0]>>>16)&0xff])^
				AES.ROTL24(ROM.ftable[(p[1]>>>24)&0xff]);
			q[3]=this.fkey[k+3]^ROM.ftable[p[3]&0xff]^
				AES.ROTL8(ROM.ftable[(p[0]>>>8)&0xff])^
				AES.ROTL16(ROM.ftable[(p[1]>>>16)&0xff])^
				AES.ROTL24(ROM.ftable[(p[2]>>>24)&0xff]);

			k+=4;
			for (j=0;j<4;j++)
			{
				t=p[j]; p[j]=q[j]; q[j]=t;
			}
		}

/* Last Round */

		q[0]=this.fkey[k]^(ROM.fbsub[p[0]&0xff]&0xff)^
			AES.ROTL8(ROM.fbsub[(p[1]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.fbsub[(p[2]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.fbsub[(p[3]>>>24)&0xff]&0xff);

		q[1]=this.fkey[k+1]^(ROM.fbsub[p[1]&0xff]&0xff)^
			AES.ROTL8(ROM.fbsub[(p[2]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.fbsub[(p[3]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.fbsub[(p[0]>>>24)&0xff]&0xff);

		q[2]=this.fkey[k+2]^(ROM.fbsub[p[2]&0xff]&0xff)^
			AES.ROTL8(ROM.fbsub[(p[3]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.fbsub[(p[0]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.fbsub[(p[1]>>>24)&0xff]&0xff);

		q[3]=this.fkey[k+3]^(ROM.fbsub[(p[3])&0xff]&0xff)^
			AES.ROTL8(ROM.fbsub[(p[0]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.fbsub[(p[1]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.fbsub[(p[2]>>>24)&0xff]&0xff);

		for (i=j=0;i<4;i++,j+=4)
		{
			b=AES.unpack(q[i]);
			for (k=0;k<4;k++) buff[j+k]=b[k];
		}
	},

/* Decrypt a single block */
	ecb_decrypt: function(buff)
	{
		var i,j,k;
		var t;
    	var b=[];
    	var p=[];
    	var q=[];

		for (i=j=0;i<4;i++,j+=4)
		{
			for (k=0;k<4;k++) b[k]=buff[j+k];
			p[i]=AES.pack(b);
			p[i]^=this.rkey[i];
		}

		k=4;

/* State alternates between p and q */
		for (i=1;i<10;i++)
		{
			q[0]=this.rkey[k]^ROM.rtable[p[0]&0xff]^
				AES.ROTL8(ROM.rtable[(p[3]>>>8)&0xff])^
				AES.ROTL16(ROM.rtable[(p[2]>>>16)&0xff])^
				AES.ROTL24(ROM.rtable[(p[1]>>>24)&0xff]);
			q[1]=this.rkey[k+1]^ROM.rtable[p[1]&0xff]^
				AES.ROTL8(ROM.rtable[(p[0]>>>8)&0xff])^
				AES.ROTL16(ROM.rtable[(p[3]>>>16)&0xff])^
				AES.ROTL24(ROM.rtable[(p[2]>>>24)&0xff]);
			q[2]=this.rkey[k+2]^ROM.rtable[p[2]&0xff]^
				AES.ROTL8(ROM.rtable[(p[1]>>>8)&0xff])^
				AES.ROTL16(ROM.rtable[(p[0]>>>16)&0xff])^
				AES.ROTL24(ROM.rtable[(p[3]>>>24)&0xff]);
			q[3]=this.rkey[k+3]^ROM.rtable[p[3]&0xff]^
				AES.ROTL8(ROM.rtable[(p[2]>>>8)&0xff])^
				AES.ROTL16(ROM.rtable[(p[1]>>>16)&0xff])^
				AES.ROTL24(ROM.rtable[(p[0]>>>24)&0xff]);

			k+=4;
			for (j=0;j<4;j++)
			{
				t=p[j]; p[j]=q[j]; q[j]=t;
			}
		}

/* Last Round */

		q[0]=this.rkey[k]^(ROM.rbsub[p[0]&0xff]&0xff)^
			AES.ROTL8(ROM.rbsub[(p[3]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.rbsub[(p[2]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.rbsub[(p[1]>>>24)&0xff]&0xff);
		q[1]=this.rkey[k+1]^(ROM.rbsub[p[1]&0xff]&0xff)^
			AES.ROTL8(ROM.rbsub[(p[0]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.rbsub[(p[3]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.rbsub[(p[2]>>>24)&0xff]&0xff);
		q[2]=this.rkey[k+2]^(ROM.rbsub[p[2]&0xff]&0xff)^
			AES.ROTL8(ROM.rbsub[(p[1]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.rbsub[(p[0]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.rbsub[(p[3]>>>24)&0xff]&0xff);
		q[3]=this.rkey[k+3]^(ROM.rbsub[p[3]&0xff]&0xff)^
			AES.ROTL8(ROM.rbsub[(p[2]>>>8)&0xff]&0xff)^
			AES.ROTL16(ROM.rbsub[(p[1]>>>16)&0xff]&0xff)^
			AES.ROTL24(ROM.rbsub[(p[0]>>>24)&0xff]&0xff);

		for (i=j=0;i<4;i++,j+=4)
		{
			b=AES.unpack(q[i]);
			for (k=0;k<4;k++) buff[j+k]=b[k];
		}

	},

/* Encrypt using selected mode of operation */
	encrypt: function(buff)
	{
		var j,bytes;
		var st=[];
		var fell_off;

// Supported Modes of Operation

		fell_off=0;

		switch (this.mode)
		{
		case ROM.ECB:
			this.ecb_encrypt(buff);
			return 0;
		case ROM.CBC:
			for (j=0;j<16;j++) buff[j]^=this.f[j];
			this.ecb_encrypt(buff);
			for (j=0;j<16;j++) this.f[j]=buff[j];
			return 0;

		case ROM.CFB1:
		case ROM.CFB2:
		case ROM.CFB4:
			bytes=this.mode-ROM.CFB1+1;
			for (j=0;j<bytes;j++) fell_off=(fell_off<<8)|this.f[j];
			for (j=0;j<16;j++) st[j]=this.f[j];
			for (j=bytes;j<16;j++) this.f[j-bytes]=this.f[j];
			this.ecb_encrypt(st);
			for (j=0;j<bytes;j++)
			{
				buff[j]^=st[j];
				this.f[16-bytes+j]=buff[j];
			}
			return fell_off;

		case ROM.OFB1:
		case ROM.OFB2:
		case ROM.OFB4:
		case ROM.OFB8:
		case ROM.OFB16:

			bytes=this.mode-ROM.OFB1+1;
			this.ecb_encrypt(this.f);
			for (j=0;j<bytes;j++) buff[j]^=this.f[j];
			return 0;

    default:
			return 0;
		}
	},

/* Decrypt using selected mode of operation */
	decrypt: function(buff)
	{
		var j,bytes;
		var st=[];
		var fell_off;

   // Supported modes of operation
		fell_off=0;
		switch (this.mode)
		{
		case ROM.ECB:
			this.ecb_decrypt(buff);
			return 0;
		case ROM.CBC:
			for (j=0;j<16;j++)
			{
				st[j]=this.f[j];
				this.f[j]=buff[j];
			}
			this.ecb_decrypt(buff);
			for (j=0;j<16;j++)
			{
				buff[j]^=st[j];
				st[j]=0;
			}
			return 0;
		case ROM.CFB1:
		case ROM.CFB2:
		case ROM.CFB4:
			bytes=this.mode-ROM.CFB1+1;
			for (j=0;j<bytes;j++) fell_off=(fell_off<<8)|this.f[j];
			for (j=0;j<16;j++) st[j]=this.f[j];
			for (j=bytes;j<16;j++) this.f[j-bytes]=this.f[j];
			this.ecb_encrypt(st);
			for (j=0;j<bytes;j++)
			{
				this.f[16-bytes+j]=buff[j];
				buff[j]^=st[j];
			}
			return fell_off;
		case ROM.OFB1:
		case ROM.OFB2:
		case ROM.OFB4:
		case ROM.OFB8:
		case ROM.OFB16:
			bytes=this.mode-ROM.OFB1+1;
			this.ecb_encrypt(this.f);
			for (j=0;j<bytes;j++) buff[j]^=this.f[j];
			return 0;


		default:
			return 0;
		}
	},

/* Clean up and delete left-overs */
	end: function()
	{ // clean up
		var i;
		for (i=0;i<44;i++)
			this.fkey[i]=this.rkey[i]=0;
		for (i=0;i<16;i++)
			this.f[i]=0;
	}

};

AES.ROTL8=function(x)
{
	return (((x)<<8)|((x)>>>24));
};

AES.ROTL16=function(x)
{
	return (((x)<<16)|((x)>>>16));
};

AES.ROTL24=function(x)
{
	return (((x)<<24)|((x)>>>8));
};

AES.pack= function(b)
{ /* pack 4 bytes into a 32-bit Word */
		return (((b[3])&0xff)<<24)|((b[2]&0xff)<<16)|((b[1]&0xff)<<8)|(b[0]&0xff);
};

AES.unpack=function(a)
{ /* unpack bytes from a word */
	var b=[];
	b[0]=(a&0xff);
	b[1]=((a>>>8)&0xff);
	b[2]=((a>>>16)&0xff);
	b[3]=((a>>>24)&0xff);
	return b;
};

AES.bmul=function(x,y)
{ /* x.y= AntiLog(Log(x) + Log(y)) */

	var ix=(x&0xff);
	var iy=(y&0xff);
	var lx=(ROM.ltab[ix])&0xff;
	var ly=(ROM.ltab[iy])&0xff;
	if (x!==0 && y!==0) return ROM.ptab[(lx+ly)%255];
	else return 0;
};

//  if (x && y)

AES.SubByte=function(a)
{
	var b=AES.unpack(a);
	b[0]=ROM.fbsub[b[0]&0xff];
	b[1]=ROM.fbsub[b[1]&0xff];
	b[2]=ROM.fbsub[b[2]&0xff];
	b[3]=ROM.fbsub[b[3]&0xff];
	return AES.pack(b);
};

AES.product=function(x,y)
{ /* dot product of two 4-byte arrays */
	var xb=AES.unpack(x);
	var yb=AES.unpack(y);
	return (AES.bmul(xb[0],yb[0])^AES.bmul(xb[1],yb[1])^AES.bmul(xb[2],yb[2])^AES.bmul(xb[3],yb[3]))&0xff;
};

AES.InvMixCol=function(x)
{ /* matrix Multiplication */
	var y,m;
	var b=[];
	m=AES.pack(ROM.InCo);
	b[3]=AES.product(m,x);
	m=AES.ROTL24(m);
	b[2]=AES.product(m,x);
	m=AES.ROTL24(m);
	b[1]=AES.product(m,x);
	m=AES.ROTL24(m);
	b[0]=AES.product(m,x);
	y=AES.pack(b);
	return y;
};
