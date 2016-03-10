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

/*
 * Implementation of the Secure Hashing Algorithm (SHA-256)
 *
 * Generates a 256 bit message digest. It should be impossible to come
 * come up with two messages that hash to the same value ("collision free").
 *
 * For use with byte-oriented messages only.
 */

public class HASH
{
	private int[] length = new int[2];
	private int[] h = new int[8];
	private int[] w = new int[64];

	public const int H0 = 0x6A09E667;
	public const int H1 = unchecked((int)0xBB67AE85);
	public const int H2 = 0x3C6EF372;
	public const int H3 = unchecked((int)0xA54FF53A);
	public const int H4 = 0x510E527F;
	public const int H5 = unchecked((int)0x9B05688C);
	public const int H6 = 0x1F83D9AB;
	public const int H7 = 0x5BE0CD19;

	public const int len = 32;

	public static readonly int[] K = new int[] {0x428a2f98, 0x71374491, unchecked((int)0xb5c0fbcf), unchecked((int)0xe9b5dba5), 0x3956c25b, 0x59f111f1, unchecked((int)0x923f82a4), unchecked((int)0xab1c5ed5), unchecked((int)0xd807aa98), 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, unchecked((int)0x80deb1fe), unchecked((int)0x9bdc06a7), unchecked((int)0xc19bf174), unchecked((int)0xe49b69c1), unchecked((int)0xefbe4786), 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, unchecked((int)0x983e5152), unchecked((int)0xa831c66d), unchecked((int)0xb00327c8), unchecked((int)0xbf597fc7), unchecked((int)0xc6e00bf3), unchecked((int)0xd5a79147), 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, unchecked((int)0x81c2c92e), unchecked((int)0x92722c85), unchecked((int)0xa2bfe8a1), unchecked((int)0xa81a664b), unchecked((int)0xc24b8b70), unchecked((int)0xc76c51a3), unchecked((int)0xd192e819), unchecked((int)0xd6990624), unchecked((int)0xf40e3585), 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, unchecked((int)0x84c87814), unchecked((int)0x8cc70208), unchecked((int)0x90befffa), unchecked((int)0xa4506ceb), unchecked((int)0xbef9a3f7), unchecked((int)0xc67178f2)};


/* functions */
	private static int S(int n, int x)
	{
		return (((int)((uint)(x) >> n)) | ((x) << (32 - n)));
	}

	private static int R(int n, int x)
	{
		return ((int)((uint)(x) >> n));
	}

	private static int Ch(int x, int y, int z)
	{
		return ((x & y) ^ (~(x) & z));
	}

	private static int Maj(int x, int y, int z)
	{
		return ((x & y) ^ (x & z) ^ (y & z));
	}

	private static int Sig0(int x)
	{
		return (S(2,x) ^ S(13,x) ^ S(22,x));
	}

	private static int Sig1(int x)
	{
		return (S(6,x) ^ S(11,x) ^ S(25,x));
	}

	private static int theta0(int x)
	{
		return (S(7,x) ^ S(18,x) ^ R(3,x));
	}

	private static int theta1(int x)
	{
		return (S(17,x) ^ S(19,x) ^ R(10,x));
	}


	private void transform()
	{ // basic transformation step
		int a, b, c, d, e, f, g, hh, t1, t2;
		int j;
		for (j = 16;j < 64;j++)
		{
			w[j] = theta1(w[j - 2]) + w[j - 7] + theta0(w[j - 15]) + w[j - 16];
		}
		a = h[0];
		b = h[1];
		c = h[2];
		d = h[3];
		e = h[4];
		f = h[5];
		g = h[6];
		hh = h[7];

		for (j = 0;j < 64;j++)
		{ // 64 times - mush it up
			t1 = hh + Sig1(e) + Ch(e,f,g) + K[j] + w[j];
			t2 = Sig0(a) + Maj(a,b,c);
			hh = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;

		}
		h[0] += a;
		h[1] += b;
		h[2] += c;
		h[3] += d;
		h[4] += e;
		h[5] += f;
		h[6] += g;
		h[7] += hh;
	}

/* Initialise Hash function */
	public virtual void init()
	{ // initialise
		int i;
		for (i = 0;i < 64;i++)
		{
			w[i] = 0;
		}
		length[0] = length[1] = 0;
		h[0] = H0;
		h[1] = H1;
		h[2] = H2;
		h[3] = H3;
		h[4] = H4;
		h[5] = H5;
		h[6] = H6;
		h[7] = H7;
	}

/* Constructor */
	public HASH()
	{
		init();
	}

/* process a single byte */
	public virtual void process(int byt)
	{ // process the next message byte
		int cnt;
		cnt = (length[0] / 32) % 16;

		w[cnt] <<= 8;
		w[cnt] |= (byt & 0xFF);
		length[0] += 8;
		if (length[0] == 0)
		{
			length[1]++;
			length[0] = 0;
		}
		if ((length[0] % 512) == 0)
		{
			transform();
		}
	}

/* process an array of bytes */
	public virtual void process_array(sbyte[] b)
	{
		for (int i = 0;i < b.Length;i++)
		{
			process((int)b[i]);
		}
	}

/* process a 32-bit integer */
	public virtual void process_num(int n)
	{
		process((n >> 24) & 0xff);
		process((n >> 16) & 0xff);
		process((n >> 8) & 0xff);
		process(n & 0xff);
	}

/* Generate 32-byte Hash */
	public virtual sbyte[] hash()
	{ // pad message and finish - supply digest
		int i;
		sbyte[] digest = new sbyte[32];
		int len0, len1;
		len0 = length[0];
		len1 = length[1];
		process(0x80);
		while ((length[0] % 512) != 448)
		{
			process(0);
		}
		w[14] = len1;
		w[15] = len0;
		transform();
		for (i = 0;i < len;i++)
		{ // convert to bytes
			digest[i] = unchecked((sbyte)((h[i / 4] >> (8 * (3 - i % 4))) & 0xff));
		}
		init();
		return digest;
	}

/* test program: should produce digest */

//248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1
/*
	public static void main(String[] args) {
		byte[] test="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq".getBytes();
		byte[] digest;
		int i;
		HASH sh=new HASH();

		for (i=0;i<test.length;i++)
			sh.process(test[i]);

		digest=sh.hash();
		for (i=0;i<32;i++) System.out.format("%02x",digest[i]);

	//	for (i=0;i<32;i++) System.out.format("%d ",digest[i]);

		System.out.println("");
	} */
}

