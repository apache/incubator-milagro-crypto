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

public sealed class ECDH
{
	public const int INVALID_PUBLIC_KEY = -2;
	public const int ERROR = -3;
	public const int INVALID = -4;
	public static readonly int EFS = ROM.MODBYTES;
	public static readonly int EGS = ROM.MODBYTES;
	public static readonly int EAS = AES.KS;
	public static readonly int EBS = AES.BS;

/* Convert Integer to n-byte array */
	private static sbyte[] inttoBytes(int n, int len)
	{
		int i;
		sbyte[] b = new sbyte[len];

		for (i = 0;i < len;i++)
		{
			b[i] = 0;
		}
		i = len;
		while (n > 0 && i > 0)
		{
			i--;
			b[i] = unchecked((sbyte)(n & 0xff));
			n /= 256;
		}
		return b;
	}

/* Key Derivation Functions */
/* Input octet Z */
/* Output key of length olen */
	public static sbyte[] KDF1(sbyte[] Z, int olen)
	{
/* NOTE: the parameter olen is the length of the output K in bytes */
		HASH H = new HASH();
		int hlen = HASH.len;
		sbyte[] K = new sbyte[olen];

		sbyte[] B;
		int counter , cthreshold , k = 0;

		for (int i = 0;i < K.Length;i++)
		{
			K[i] = 0;
		}

		cthreshold = olen / hlen;
		if (olen % hlen != 0)
		{
			cthreshold++;
		}

		for (counter = 0;counter < cthreshold;counter++)
		{
			H.process_array(Z);
			if (counter > 0)
			{
				H.process_num(counter);
			}
			B = H.hash();
			if (k + hlen > olen)
			{
				for (int i = 0;i < olen % hlen;i++)
				{
					K[k++] = B[i];
				}
			}
			else
			{
				for (int i = 0;i < hlen;i++)
				{
					K[k++] = B[i];
				}
			}
		}
		return K;
	}

	public static sbyte[] KDF2(sbyte[] Z, sbyte[] P, int olen)
	{
/* NOTE: the parameter olen is the length of the output k in bytes */
		HASH H = new HASH();
		int hlen = HASH.len;
		sbyte[] K = new sbyte[olen];

		sbyte[] B = new sbyte[hlen];
		int counter , cthreshold , k = 0;

		for (int i = 0;i < K.Length;i++)
		{
			K[i] = 0;
		}

		cthreshold = olen / hlen;
		if (olen % hlen != 0)
		{
			cthreshold++;
		}

		for (counter = 1;counter <= cthreshold;counter++)
		{
			H.process_array(Z);
			H.process_num(counter);
			H.process_array(P);
			B = H.hash();
			if (k + hlen > olen)
			{
				for (int i = 0;i < olen % hlen;i++)
				{
					K[k++] = B[i];
				}
			}
			else
			{
				for (int i = 0;i < hlen;i++)
				{
					K[k++] = B[i];
				}
			}
		}
		return K;
	}

/* Password based Key Derivation Function */
/* Input password p, salt s, and repeat count */
/* Output key of length olen */
	public static sbyte[] PBKDF2(sbyte[] Pass, sbyte[] Salt, int rep, int olen)
	{
		int i, j, k, d, opt;
		d = olen / 32;
		if (olen % 32 != 0)
		{
			d++;
		}
		sbyte[] F = new sbyte[EFS];
		sbyte[] U = new sbyte[EFS];
		sbyte[] S = new sbyte[Salt.Length + 4];

		sbyte[] K = new sbyte[d * EFS];
		opt = 0;

		for (i = 1;i <= d;i++)
		{
			for (j = 0;j < Salt.Length;j++)
			{
				S[j] = Salt[j];
			}
			sbyte[] N = inttoBytes(i,4);
			for (j = 0;j < 4;j++)
			{
				S[Salt.Length + j] = N[j];
			}

			HMAC(S,Pass,F);

			for (j = 0;j < EFS;j++)
			{
				U[j] = F[j];
			}
			for (j = 2;j <= rep;j++)
			{
				HMAC(U,Pass,U);
				for (k = 0;k < EFS;k++)
				{
					F[k] ^= U[k];
				}
			}
			for (j = 0;j < EFS;j++)
			{
				K[opt++] = F[j];
			}
		}
		sbyte[] key = new sbyte[olen];
		for (i = 0;i < olen;i++)
		{
			key[i] = K[i];
		}
		return key;
	}

/* Calculate HMAC of m using key k. HMAC is tag of length olen */
	public static int HMAC(sbyte[] M, sbyte[] K, sbyte[] tag)
	{
	/* Input is from an octet m        *
	* olen is requested output length in bytes. k is the key  *
	* The output is the calculated tag */
		int b;
		sbyte[] B;
		sbyte[] K0 = new sbyte[64];
		int olen = tag.Length;

		b = K0.Length;
		if (olen < 4 || olen> HASH.len)
		{
			return 0;
		}

		for (int i = 0;i < b;i++)
		{
			K0[i] = 0;
		}

		HASH H = new HASH();

		if (K.Length > b)
		{
			H.process_array(K);
			B = H.hash();
			for (int i = 0;i < 32;i++)
			{
				K0[i] = B[i];
			}
		}
		else
		{
			for (int i = 0;i < K.Length;i++)
			{
				K0[i] = K[i];
			}
		}

		for (int i = 0;i < b;i++)
		{
			K0[i] ^= 0x36;
		}
		H.process_array(K0);
		H.process_array(M);
		B = H.hash();

		for (int i = 0;i < b;i++)
		{
			K0[i] ^= 0x6a;
		}
		H.process_array(K0);
		H.process_array(B);
		B = H.hash();

		for (int i = 0;i < olen;i++)
		{
			tag[i] = B[i];
		}

		return 1;
	}

/* AES encryption/decryption. Encrypt byte array M using key K and returns ciphertext */
	public static sbyte[] AES_CBC_IV0_ENCRYPT(sbyte[] K, sbyte[] M)
	{ // AES CBC encryption, with Null IV and key K
	/* Input is from an octet string M, output is to an octet string C */
	/* Input is padded as necessary to make up a full final block */
		AES a = new AES();
		bool fin;
		int i, j, ipt, opt;
		sbyte[] buff = new sbyte[16];
		int clen = 16 + (M.Length / 16) * 16;

		sbyte[] C = new sbyte[clen];
		int padlen;

		a.init(AES.CBC,K,null);

		ipt = opt = 0;
		fin = false;
		for (;;)
		{
			for (i = 0;i < 16;i++)
			{
				if (ipt < M.Length)
				{
					buff[i] = M[ipt++];
				}
				else
				{
					fin = true;
					break;
				}
			}
			if (fin)
			{
				break;
			}
			a.encrypt(buff);
			for (i = 0;i < 16;i++)
			{
				C[opt++] = buff[i];
			}
		}

/* last block, filled up to i-th index */

		padlen = 16 - i;
		for (j = i;j < 16;j++)
		{
			buff[j] = (sbyte)padlen;
		}

		a.encrypt(buff);

		for (i = 0;i < 16;i++)
		{
			C[opt++] = buff[i];
		}
		a.end();
		return C;
	}

/* returns plaintext if all consistent, else returns null string */
	public static sbyte[] AES_CBC_IV0_DECRYPT(sbyte[] K, sbyte[] C)
	{ // padding is removed
		AES a = new AES();
		int i, ipt, opt, ch;
		sbyte[] buff = new sbyte[16];
		sbyte[] MM = new sbyte[C.Length];
		bool fin, bad;
		int padlen;
		ipt = opt = 0;

		a.init(AES.CBC,K,null);

		if (C.Length == 0)
		{
			return new sbyte[0];
		}
		ch = C[ipt++];

		fin = false;

		for (;;)
		{
			for (i = 0;i < 16;i++)
			{
				buff[i] = (sbyte)ch;
				if (ipt >= C.Length)
				{
					fin = true;
					break;
				}
				else
				{
					ch = C[ipt++];
				}
			}
			a.decrypt(buff);
			if (fin)
			{
				break;
			}
			for (i = 0;i < 16;i++)
			{
				MM[opt++] = buff[i];
			}
		}

		a.end();
		bad = false;
		padlen = buff[15];
		if (i != 15 || padlen < 1 || padlen>16)
		{
			bad = true;
		}
		if (padlen >= 2 && padlen <= 16)
		{
			for (i = 16 - padlen;i < 16;i++)
			{
				if (buff[i] != padlen)
				{
					bad = true;
				}
			}
		}

		if (!bad)
		{
			for (i = 0;i < 16 - padlen;i++)
			{
					MM[opt++] = buff[i];
			}
		}

		if (bad)
		{
			return new sbyte[0];
		}

		sbyte[] M = new sbyte[opt];
		for (i = 0;i < opt;i++)
		{
			M[i] = MM[i];
		}

		return M;
	}

/* Calculate a public/private EC GF(p) key pair W,S where W=S.G mod EC(p),
 * where S is the secret key and W is the public key
 * and G is fixed generator.
 * If RNG is NULL then the private key is provided externally in S
 * otherwise it is generated randomly internally */
	public static int KEY_PAIR_GENERATE(RAND RNG, sbyte[] S, sbyte[] W)
	{
		BIG r, gx, gy, s;
		ECP G, WP;
		int res = 0;
		sbyte[] T = new sbyte[EFS];

		gx = new BIG(ROM.CURVE_Gx);
		if (ROM.CURVETYPE != ROM.MONTGOMERY)
		{
			gy = new BIG(ROM.CURVE_Gy);
			G = new ECP(gx,gy);
		}
		else
		{
			G = new ECP(gx);
		}

		r = new BIG(ROM.CURVE_Order);

		if (RNG == null)
		{
			s = BIG.fromBytes(S);
		}
		else
		{
			s = BIG.randomnum(r,RNG);

			s.toBytes(T);
			for (int i = 0;i < EGS;i++)
			{
				S[i] = T[i];
			}
		}

		WP = G.mul(s);
		WP.toBytes(W);

		return res;
	}

/* validate public key. Set full=true for fuller check */
	public static int PUBLIC_KEY_VALIDATE(bool full, sbyte[] W)
	{
		BIG r;
		ECP WP = ECP.fromBytes(W);
		int res = 0;

		r = new BIG(ROM.CURVE_Order);

		if (WP.is_infinity())
		{
			res = INVALID_PUBLIC_KEY;
		}
		if (res == 0 && full)
		{
			WP = WP.mul(r);
			if (!WP.is_infinity())
			{
				res = INVALID_PUBLIC_KEY;
			}
		}
		return res;
	}

/* IEEE-1363 Diffie-Hellman online calculation Z=S.WD */
	public static int ECPSVDP_DH(sbyte[] S, sbyte[] WD, sbyte[] Z)
	{
		BIG r, s;
		ECP W;
		int res = 0;
		sbyte[] T = new sbyte[EFS];

		s = BIG.fromBytes(S);

		W = ECP.fromBytes(WD);
		if (W.is_infinity())
		{
			res = ERROR;
		}

		if (res == 0)
		{
			r = new BIG(ROM.CURVE_Order);
			s.mod(r);
			W = W.mul(s);
			if (W.is_infinity())
			{
				res = ERROR;
			}
			else
			{
				W.X.toBytes(T);
				for (int i = 0;i < EFS;i++)
				{
					Z[i] = T[i];
				}
			}
		}
		return res;
	}

/* IEEE ECDSA Signature, C and D are signature on F using private key S */
	public static int ECPSP_DSA(RAND RNG, sbyte[] S, sbyte[] F, sbyte[] C, sbyte[] D)
	{
		sbyte[] T = new sbyte[EFS];
		BIG gx, gy, r, s, f, c, d, u, vx;
		ECP G, V;

		HASH H = new HASH();
		H.process_array(F);
		sbyte[] B = H.hash();

		gx = new BIG(ROM.CURVE_Gx);
		gy = new BIG(ROM.CURVE_Gy);

		G = new ECP(gx,gy);
		r = new BIG(ROM.CURVE_Order);

		s = BIG.fromBytes(S);
		f = BIG.fromBytes(B);

		c = new BIG(0);
		d = new BIG(0);
		V = new ECP();

		do
		{
			u = BIG.randomnum(r,RNG);

			V.copy(G);
			V = V.mul(u);
			vx = V.X;
			c.copy(vx);
			c.mod(r);
			if (c.iszilch())
			{
				continue;
			}
			u.invmodp(r);
			d.copy(BIG.modmul(s,c,r));
			d.add(f);
			d.copy(BIG.modmul(u,d,r));
		} while (d.iszilch());

		c.toBytes(T);
		for (int i = 0;i < EFS;i++)
		{
			C[i] = T[i];
		}
		d.toBytes(T);
		for (int i = 0;i < EFS;i++)
		{
			D[i] = T[i];
		}
		return 0;
	}

/* IEEE1363 ECDSA Signature Verification. Signature C and D on F is verified using public key W */
	public static int ECPVP_DSA(sbyte[] W, sbyte[] F, sbyte[] C, sbyte[] D)
	{
		BIG r, gx, gy, f, c, d, h2;
		int res = 0;
		ECP G, WP, P;

		HASH H = new HASH();
		H.process_array(F);
		sbyte[] B = H.hash();

		gx = new BIG(ROM.CURVE_Gx);
		gy = new BIG(ROM.CURVE_Gy);

		G = new ECP(gx,gy);
		r = new BIG(ROM.CURVE_Order);

		c = BIG.fromBytes(C);
		d = BIG.fromBytes(D);
		f = BIG.fromBytes(B);

		if (c.iszilch() || BIG.comp(c,r) >= 0 || d.iszilch() || BIG.comp(d,r) >= 0)
		{
			res = INVALID;
		}

		if (res == 0)
		{
			d.invmodp(r);
			f.copy(BIG.modmul(f,d,r));
			h2 = BIG.modmul(c,d,r);

			WP = ECP.fromBytes(W);
			if (WP.is_infinity())
			{
				res = ERROR;
			}
			else
			{
				P = new ECP();
				P.copy(WP);
				P = P.mul2(h2,G,f);
				if (P.is_infinity())
				{
					res = INVALID;
				}
				else
				{
					d = P.X;
					d.mod(r);
					if (BIG.comp(d,c) != 0)
					{
						res = INVALID;
					}
				}
			}
		}

		return res;
	}

/* IEEE1363 ECIES encryption. Encryption of plaintext M uses public key W and produces ciphertext V,C,T */
	public static sbyte[] ECIES_ENCRYPT(sbyte[] P1, sbyte[] P2, RAND RNG, sbyte[] W, sbyte[] M, sbyte[] V, sbyte[] T)
	{
		int i;

		sbyte[] Z = new sbyte[EFS];
		sbyte[] VZ = new sbyte[3 * EFS + 1];
		sbyte[] K1 = new sbyte[EAS];
		sbyte[] K2 = new sbyte[EAS];
		sbyte[] U = new sbyte[EGS];

		if (KEY_PAIR_GENERATE(RNG,U,V) != 0)
		{
			return new sbyte[0];
		}
		if (ECPSVDP_DH(U,W,Z) != 0)
		{
			return new sbyte[0];
		}

		for (i = 0;i < 2 * EFS + 1;i++)
		{
			VZ[i] = V[i];
		}
		for (i = 0;i < EFS;i++)
		{
			VZ[2 * EFS + 1 + i] = Z[i];
		}


		sbyte[] K = KDF2(VZ,P1,EFS);

		for (i = 0;i < EAS;i++)
		{
			K1[i] = K[i];
			K2[i] = K[EAS + i];
		}

		sbyte[] C = AES_CBC_IV0_ENCRYPT(K1,M);

		sbyte[] L2 = inttoBytes(P2.Length,8);

		sbyte[] AC = new sbyte[C.Length + P2.Length + 8];
		for (i = 0;i < C.Length;i++)
		{
			AC[i] = C[i];
		}
		for (i = 0;i < P2.Length;i++)
		{
			AC[C.Length + i] = P2[i];
		}
		for (i = 0;i < 8;i++)
		{
			AC[C.Length + P2.Length + i] = L2[i];
		}

		HMAC(AC,K2,T);

		return C;
	}

/* IEEE1363 ECIES decryption. Decryption of ciphertext V,C,T using private key U outputs plaintext M */
	public static sbyte[] ECIES_DECRYPT(sbyte[] P1, sbyte[] P2, sbyte[] V, sbyte[] C, sbyte[] T, sbyte[] U)
	{

		int i;

		sbyte[] Z = new sbyte[EFS];
		sbyte[] VZ = new sbyte[3 * EFS + 1];
		sbyte[] K1 = new sbyte[EAS];
		sbyte[] K2 = new sbyte[EAS];
		sbyte[] TAG = new sbyte[T.Length];

		if (ECPSVDP_DH(U,V,Z) != 0)
		{
			return new sbyte[0];
		}

		for (i = 0;i < 2 * EFS + 1;i++)
		{
			VZ[i] = V[i];
		}
		for (i = 0;i < EFS;i++)
		{
			VZ[2 * EFS + 1 + i] = Z[i];
		}

		sbyte[] K = KDF2(VZ,P1,EFS);

		for (i = 0;i < EAS;i++)
		{
			K1[i] = K[i];
			K2[i] = K[EAS + i];
		}

		sbyte[] M = AES_CBC_IV0_DECRYPT(K1,C);

		if (M.Length == 0)
		{
			return M;
		}

		sbyte[] L2 = inttoBytes(P2.Length,8);

		sbyte[] AC = new sbyte[C.Length + P2.Length + 8];

		for (i = 0;i < C.Length;i++)
		{
			AC[i] = C[i];
		}
		for (i = 0;i < P2.Length;i++)
		{
			AC[C.Length + i] = P2[i];
		}
		for (i = 0;i < 8;i++)
		{
			AC[C.Length + P2.Length + i] = L2[i];
		}

		HMAC(AC,K2,TAG);

		bool same = true;
		for (i = 0;i < T.Length;i++)
		{
			if (T[i] != TAG[i])
			{
				same = false;
			}
		}
		if (!same)
		{
			return new sbyte[0];
		}

		return M;

	}
}
