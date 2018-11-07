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

/* Elliptic Curve API high-level functions  */

public final class ECDH {
	public static final int INVALID_PUBLIC_KEY=-2;
	public static final int ERROR=-3;
	public static final int INVALID=-4;
	public static final int EFS=ROM.MODBYTES;
	public static final int EGS=ROM.MODBYTES;
	public static final int EAS=16;
	public static final int EBS=16;
	public static final int SHA256=32;
	public static final int SHA384=48;
	public static final int SHA512=64;

	public static final int HASH_TYPE=SHA512;

/* Convert Integer to n-byte array */
	private static byte[] inttoBytes(int n,int len)
	{
		int i;
		byte[] b=new byte[len];

		for (i=0;i<len;i++) b[i]=0;
		i=len; 
		while (n>0 && i>0)
		{
			i--;
			b[i]=(byte)(n&0xff);
			n/=256;
		}	
		return b;
	}

	private static byte[] hashit(int sha,byte[] A,int n,byte[] B,int pad)
	{
		byte[] R=null;

		if (sha==SHA256)
		{
			HASH256 H=new HASH256();
			H.process_array(A); if (n>0) H.process_num(n);
			if (B!=null) H.process_array(B);
			R=H.hash();
		}
		if (sha==SHA384)
		{
			HASH384 H=new HASH384();
			H.process_array(A); if (n>0) H.process_num(n);
			if (B!=null) H.process_array(B);
			R=H.hash();
		}
		if (sha==SHA512)
		{
			HASH512 H=new HASH512();
			H.process_array(A); if (n>0) H.process_num(n);
			if (B!=null) H.process_array(B);
			R=H.hash();
		}
		if (R==null) return null;

		if (pad==0) return R;
/* If pad>0 output is truncated or padded to pad bytes */
		byte[] W=new byte[pad];
		if (pad<=sha) 
		{
			for (int i=0;i<pad;i++) W[i]=R[i];
		}
		else
		{
			for (int i=0;i<sha;i++) W[i]=R[i];
			for (int i=sha;i<pad;i++) W[i]=0;
		}
		return W;
	}

/* Key Derivation Functions */
/* Input octet Z */
/* Output key of length olen */
	public static byte[] KDF1(int sha,byte[] Z,int olen)
	{
/* NOTE: the parameter olen is the length of the output K in bytes */
		int hlen=sha;
		byte[] K=new byte[olen];
		byte[] B;
		int counter,cthreshold,k=0;
    
		for (int i=0;i<K.length;i++) K[i]=0;

		cthreshold=olen/hlen; if (olen%hlen!=0) cthreshold++;

		for (counter=0;counter<cthreshold;counter++)
		{
			B=hashit(sha,Z,counter,null,0);
			if (k+hlen>olen) for (int i=0;i<olen%hlen;i++) K[k++]=B[i];
			else for (int i=0;i<hlen;i++) K[k++]=B[i];
		}
		return K;
	}

	public static byte[] KDF2(int sha,byte[] Z,byte[] P,int olen)
	{
/* NOTE: the parameter olen is the length of the output k in bytes */
		int hlen=sha;
		byte[] K=new byte[olen];
		byte[] B;
		int counter,cthreshold,k=0;
    
		for (int i=0;i<K.length;i++) K[i]=0;

		cthreshold=olen/hlen; if (olen%hlen!=0) cthreshold++;

		for (counter=1;counter<=cthreshold;counter++)
		{
			B=hashit(sha,Z,counter,P,0);
			if (k+hlen>olen) for (int i=0;i<olen%hlen;i++) K[k++]=B[i];
			else for (int i=0;i<hlen;i++) K[k++]=B[i];
		}

		return K;
	}

/* Password based Key Derivation Function */
/* Input password p, salt s, and repeat count */
/* Output key of length olen */
	public static byte[] PBKDF2(int sha,byte[] Pass,byte[] Salt,int rep,int olen)
	{
		int i,j,k,len,d,opt;
		d=olen/sha; if (olen%sha!=0) d++;
		byte[] F=new byte[sha];
		byte[] U=new byte[sha];
		byte[] S=new byte[Salt.length+4];

		byte[] K=new byte[d*sha];
		opt=0;

		for (i=1;i<=d;i++)
		{
			for (j=0;j<Salt.length;j++) S[j]=Salt[j];
			byte[] N=inttoBytes(i,4);
			for (j=0;j<4;j++) S[Salt.length+j]=N[j];

			HMAC(sha,S,Pass,F);

			for (j=0;j<sha;j++) U[j]=F[j];
			for (j=2;j<=rep;j++)
			{
				HMAC(sha,U,Pass,U);
				for (k=0;k<sha;k++) F[k]^=U[k];
			}
			for (j=0;j<sha;j++) K[opt++]=F[j];
		}
		byte[] key=new byte[olen];
		for (i=0;i<olen;i++) key[i]=K[i];
		return key;
	}

/* Calculate HMAC of m using key k. HMAC is tag of length olen */
	public static int HMAC(int sha,byte[] M,byte[] K,byte[] tag)
	{
	/* Input is from an octet m        *
	* olen is requested output length in bytes. k is the key  *
	* The output is the calculated tag */
		int b=64;
		if (sha>32) b=128;
		byte[] B;
		byte[] K0=new byte[b];
		int olen=tag.length;

		//b=K0.length;
		if (olen<4 /*|| olen>sha*/) return 0;

		for (int i=0;i<b;i++) K0[i]=0;

		if (K.length > b) 
		{
			B=hashit(sha,K,0,null,0);
			for (int i=0;i<sha;i++) K0[i]=B[i];
		}
		else
			for (int i=0;i<K.length;i++ ) K0[i]=K[i];
		
		for (int i=0;i<b;i++) K0[i]^=0x36;
		B=hashit(sha,K0,0,M,0);

		for (int i=0;i<b;i++) K0[i]^=0x6a;
		B=hashit(sha,K0,0,B,olen);

		for (int i=0;i<olen;i++) tag[i]=B[i];

		return 1;
	}

/* AES encryption/decryption. Encrypt byte array M using key K and returns ciphertext */
	public static byte[] AES_CBC_IV0_ENCRYPT(byte[] K,byte[] M)
	{ /* AES CBC encryption, with Null IV and key K */
	/* Input is from an octet string M, output is to an octet string C */
	/* Input is padded as necessary to make up a full final block */
		AES a=new AES();
		boolean fin;
		int i,j,ipt,opt;
		byte[] buff=new byte[16];
		int clen=16+(M.length/16)*16;

		byte[] C=new byte[clen];
		int padlen;

		a.init(AES.CBC,K.length,K,null);

		ipt=opt=0;
		fin=false;
		for(;;)
		{
			for (i=0;i<16;i++)
			{
				if (ipt<M.length) buff[i]=M[ipt++];
				else {fin=true; break;}
			}
			if (fin) break;
			a.encrypt(buff);
			for (i=0;i<16;i++)
				C[opt++]=buff[i];
		}    

/* last block, filled up to i-th index */

		padlen=16-i;
		for (j=i;j<16;j++) buff[j]=(byte)padlen;

		a.encrypt(buff);

		for (i=0;i<16;i++)
			C[opt++]=buff[i];
		a.end();    
		return C;
	}

/* returns plaintext if all consistent, else returns null string */
	public static byte[] AES_CBC_IV0_DECRYPT(byte[] K,byte[] C)
	{ /* padding is removed */
		AES a=new AES();
		int i,ipt,opt,ch;
		byte[] buff=new byte[16];
		byte[] MM=new byte[C.length];
		boolean fin,bad;
		int padlen;
		ipt=opt=0;

		a.init(AES.CBC,K.length,K,null);

		if (C.length==0) return new byte[0];
		ch=C[ipt++]; 
  
		fin=false;

		for(;;)
		{
			for (i=0;i<16;i++)
			{
				buff[i]=(byte)ch;      
				if (ipt>=C.length) {fin=true; break;}  
				else ch=C[ipt++];  
			}
			a.decrypt(buff);
			if (fin) break;
			for (i=0;i<16;i++)
				MM[opt++]=buff[i];
		}    

		a.end();
		bad=false;
		padlen=buff[15];
		if (i!=15 || padlen<1 || padlen>16) bad=true;
		if (padlen>=2 && padlen<=16)
			for (i=16-padlen;i<16;i++) if (buff[i]!=padlen) bad=true;
    
		if (!bad) for (i=0;i<16-padlen;i++)
					MM[opt++]=buff[i];

		if (bad) return new byte[0];

		byte[] M=new byte[opt];
		for (i=0;i<opt;i++) M[i]=MM[i];

		return M;
	}

/* Calculate a public/private EC GF(p) key pair W,S where W=S.G mod EC(p),
 * where S is the secret key and W is the public key
 * and G is fixed generator.
 * If RNG is NULL then the private key is provided externally in S
 * otherwise it is generated randomly internally */
	public static int KEY_PAIR_GENERATE(RAND RNG,byte[] S,byte[] W)
	{
		BIG r,gx,gy,s,wx,wy;
		ECP G,WP;
		int res=0;
	//	byte[] T=new byte[EFS];

		gx=new BIG(ROM.CURVE_Gx);

		if (ROM.CURVETYPE!=ROM.MONTGOMERY)
		{
			gy=new BIG(ROM.CURVE_Gy);
			G=new ECP(gx,gy);
		}
		else
			G=new ECP(gx);

		r=new BIG(ROM.CURVE_Order);

		if (RNG==null)
		{
			s=BIG.fromBytes(S);
			s.mod(r);
		}
		else
		{
			s=BIG.randomnum(r,RNG);
		}

		if (ROM.AES_S>0)
		{
			s.mod2m(2*ROM.AES_S);
		}
		s.toBytes(S);

		WP=G.mul(s);
		WP.toBytes(W);

		return res;
	}

/* validate public key. Set full=true for fuller check */
	public static int PUBLIC_KEY_VALIDATE(boolean full,byte[] W)
	{
		BIG r;
		ECP WP=ECP.fromBytes(W);
		int res=0;

		r=new BIG(ROM.CURVE_Order);

		if (WP.is_infinity()) res=INVALID_PUBLIC_KEY;

		if (res==0 && full)
		{
			WP=WP.mul(r);
			if (!WP.is_infinity()) res=INVALID_PUBLIC_KEY; 
		}
		return res;
	}

/* IEEE-1363 Diffie-Hellman online calculation Z=S.WD */
	public static int ECPSVDP_DH(byte[] S,byte[] WD,byte[] Z)    
	{
		BIG r,s,wx,wy,z;
		int valid;
		ECP W;
		int res=0;
		byte[] T=new byte[EFS];

		s=BIG.fromBytes(S);

		W=ECP.fromBytes(WD);
		if (W.is_infinity()) res=ERROR;

		if (res==0)
		{
			r=new BIG(ROM.CURVE_Order);
			s.mod(r);

			W=W.mul(s);
			if (W.is_infinity()) res=ERROR; 
			else 
			{
				W.getX().toBytes(T);
				for (int i=0;i<EFS;i++) Z[i]=T[i];
			}
		}
		return res;
	}

/* IEEE ECDSA Signature, C and D are signature on F using private key S */
	public static int ECPSP_DSA(int sha,RAND RNG,byte[] S,byte[] F,byte[] C,byte[] D)
	{
		byte[] T=new byte[EFS];
		BIG gx,gy,r,s,f,c,d,u,vx,w;
		ECP G,V;
		byte[] B=hashit(sha,F,0,null,ROM.MODBYTES);

		gx=new BIG(ROM.CURVE_Gx);
		gy=new BIG(ROM.CURVE_Gy);

		G=new ECP(gx,gy);
		r=new BIG(ROM.CURVE_Order);

		s=BIG.fromBytes(S);
		f=BIG.fromBytes(B);

		c=new BIG(0);
		d=new BIG(0);
		V=new ECP();

		do {
			u=BIG.randomnum(r,RNG);
			w=BIG.randomnum(r,RNG);
			if (ROM.AES_S>0)
			{
				u.mod2m(2*ROM.AES_S);
			}			
			V.copy(G);
			V=V.mul(u);   		
			vx=V.getX();
			c.copy(vx);
			c.mod(r);
			if (c.iszilch()) continue;

			u.copy(BIG.modmul(u,w,r));

			u.invmodp(r);
			d.copy(BIG.modmul(s,c,r));
			d.add(f);

			d.copy(BIG.modmul(d,w,r));

			d.copy(BIG.modmul(u,d,r));
		} while (d.iszilch());
       
		c.toBytes(T);
		for (int i=0;i<EFS;i++) C[i]=T[i];
		d.toBytes(T);
		for (int i=0;i<EFS;i++) D[i]=T[i];
		return 0;
	}

/* IEEE1363 ECDSA Signature Verification. Signature C and D on F is verified using public key W */
	public static int ECPVP_DSA(int sha,byte[] W,byte[] F, byte[] C,byte[] D)
	{
		BIG r,gx,gy,f,c,d,h2;
		int res=0;
		ECP G,WP,P;
		int valid; 

		byte[] B=hashit(sha,F,0,null,ROM.MODBYTES);

		gx=new BIG(ROM.CURVE_Gx);
		gy=new BIG(ROM.CURVE_Gy);

		G=new ECP(gx,gy);
		r=new BIG(ROM.CURVE_Order);

		c=BIG.fromBytes(C);
		d=BIG.fromBytes(D);
		f=BIG.fromBytes(B);
     
		if (c.iszilch() || BIG.comp(c,r)>=0 || d.iszilch() || BIG.comp(d,r)>=0) 
            res=INVALID;

		if (res==0)
		{
			d.invmodp(r);
			f.copy(BIG.modmul(f,d,r));
			h2=BIG.modmul(c,d,r);

			WP=ECP.fromBytes(W);
			if (WP.is_infinity()) res=ERROR;
			else
			{
				P=new ECP();
				P.copy(WP);
				P=P.mul2(h2,G,f);
				if (P.is_infinity()) res=INVALID;
				else
				{
					d=P.getX();
					d.mod(r);
					if (BIG.comp(d,c)!=0) res=INVALID;
				}
			}
		}

		return res;
	}

/* IEEE1363 ECIES encryption. Encryption of plaintext M uses public key W and produces ciphertext V,C,T */
	public static byte[] ECIES_ENCRYPT(int sha,byte[] P1,byte[] P2,RAND RNG,byte[] W,byte[] M,byte[] V,byte[] T)
	{ 
		int i,len;

		byte[] Z=new byte[EFS];
		byte[] VZ=new byte[3*EFS+1];
		byte[] K1=new byte[EAS];
		byte[] K2=new byte[EAS];
		byte[] U=new byte[EGS];

		if (KEY_PAIR_GENERATE(RNG,U,V)!=0) return new byte[0];  
		if (ECPSVDP_DH(U,W,Z)!=0) return new byte[0];     

		for (i=0;i<2*EFS+1;i++) VZ[i]=V[i];
		for (i=0;i<EFS;i++) VZ[2*EFS+1+i]=Z[i];


		byte[] K=KDF2(sha,VZ,P1,EFS);

		for (i=0;i<EAS;i++) {K1[i]=K[i]; K2[i]=K[EAS+i];} 

		byte[] C=AES_CBC_IV0_ENCRYPT(K1,M);

		byte[] L2=inttoBytes(P2.length,8);	
	
		byte[] AC=new byte[C.length+P2.length+8];
		for (i=0;i<C.length;i++) AC[i]=C[i];
		for (i=0;i<P2.length;i++) AC[C.length+i]=P2[i];
		for (i=0;i<8;i++) AC[C.length+P2.length+i]=L2[i];
	
		HMAC(sha,AC,K2,T);

		return C;
	}

/* IEEE1363 ECIES decryption. Decryption of ciphertext V,C,T using private key U outputs plaintext M */
	public static byte[] ECIES_DECRYPT(int sha,byte[] P1,byte[] P2,byte[] V,byte[] C,byte[] T,byte[] U)
	{ 

		int i,len;

		byte[] Z=new byte[EFS];
		byte[] VZ=new byte[3*EFS+1];
		byte[] K1=new byte[EAS];
		byte[] K2=new byte[EAS];
		byte[] TAG=new byte[T.length];

		if (ECPSVDP_DH(U,V,Z)!=0) return new byte[0];  

		for (i=0;i<2*EFS+1;i++) VZ[i]=V[i];
		for (i=0;i<EFS;i++) VZ[2*EFS+1+i]=Z[i];

		byte[] K=KDF2(sha,VZ,P1,EFS);

		for (i=0;i<EAS;i++) {K1[i]=K[i]; K2[i]=K[EAS+i];} 

		byte[] M=AES_CBC_IV0_DECRYPT(K1,C); 

		if (M.length==0) return M;

		byte[] L2=inttoBytes(P2.length,8);	
	
		byte[] AC=new byte[C.length+P2.length+8];

		for (i=0;i<C.length;i++) AC[i]=C[i];
		for (i=0;i<P2.length;i++) AC[C.length+i]=P2[i];
		for (i=0;i<8;i++) AC[C.length+P2.length+i]=L2[i];
	
		HMAC(sha,AC,K2,TAG);

		boolean same=true;
		for (i=0;i<T.length;i++) if (T[i]!=TAG[i]) same=false;
		if (!same) return new byte[0];
	
		return M;

	}
}
