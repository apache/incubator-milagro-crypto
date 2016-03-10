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

/* RSA API high-level functions  */

package amcl

import "fmt"

const RSA_RFS int = int(MODBYTES) * FFLEN

type rsa_private_key struct {
	p, q, dp, dq, c *FF
}

func New_rsa_private_key(n int) *rsa_private_key {
	SK := new(rsa_private_key)
	SK.p = NewFFint(n)
	SK.q = NewFFint(n)
	SK.dp = NewFFint(n)
	SK.dq = NewFFint(n)
	SK.c = NewFFint(n)
	return SK
}

type rsa_public_key struct {
	e int
	n *FF
}

func New_rsa_public_key(m int) *rsa_public_key {
	PK := new(rsa_public_key)
	PK.e = 0
	PK.n = NewFFint(m)
	return PK
}

func RSA_KEY_PAIR(rng *RAND, e int, PRIV *rsa_private_key, PUB *rsa_public_key) { /* IEEE1363 A16.11/A16.12 more or less */
	n := PUB.n.getlen() / 2
	t := NewFFint(n)
	p1 := NewFFint(n)
	q1 := NewFFint(n)

	for true {
		PRIV.p.random(rng)
		for PRIV.p.lastbits(2) != 3 {
			PRIV.p.inc(1)
		}
		for !prime(PRIV.p, rng) {
			PRIV.p.inc(4)
		}

		p1.copy(PRIV.p)
		p1.dec(1)

		if p1.cfactor(e) {
			continue
		}
		break
	}

	for true {
		PRIV.q.random(rng)
		for PRIV.q.lastbits(2) != 3 {
			PRIV.q.inc(1)
		}
		for !prime(PRIV.q, rng) {
			PRIV.q.inc(4)
		}

		q1.copy(PRIV.q)
		q1.dec(1)

		if q1.cfactor(e) {
			continue
		}

		break
	}

	PUB.n = ff_mul(PRIV.p, PRIV.q)
	PUB.e = e

	t.copy(p1)
	t.shr()
	PRIV.dp.set(e)
	PRIV.dp.invmodp(t)
	if PRIV.dp.parity() == 0 {
		PRIV.dp.add(t)
	}
	PRIV.dp.norm()

	t.copy(q1)
	t.shr()
	PRIV.dq.set(e)
	PRIV.dq.invmodp(t)
	if PRIV.dq.parity() == 0 {
		PRIV.dq.add(t)
	}
	PRIV.dq.norm()

	PRIV.c.copy(PRIV.p)
	PRIV.c.invmodp(PRIV.q)

}

/* Mask Generation Function */

func RSA_MGF1(Z []byte, olen int, K []byte) {
	H := NewHASH()
	hlen := 32

	var k int = 0
	for i := 0; i < len(K); i++ {
		K[i] = 0
	}

	cthreshold := olen / hlen
	if olen%hlen != 0 {
		cthreshold++
	}
	for counter := 0; counter < cthreshold; counter++ {
		H.Process_array(Z)
		H.Process_num(int32(counter))
		B := H.Hash()

		if k+hlen > olen {
			for i := 0; i < olen%hlen; i++ {
				K[k] = B[i]
				k++
			}
		} else {
			for i := 0; i < hlen; i++ {
				K[k] = B[i]
				k++
			}
		}
	}
}

func RSA_printBinary(array []byte) {
	for i := 0; i < len(array); i++ {
		fmt.Printf("%02x", array[i])
	}
	fmt.Printf("\n")
}

/* OAEP Message Encoding for Encryption */
func RSA_OAEP_ENCODE(m []byte, rng *RAND, p []byte) []byte {
	olen := RSA_RFS - 1
	mlen := len(m)
	var f [RSA_RFS]byte

	H := NewHASH()
	hlen := 32
	var SEED [32]byte
	seedlen := hlen
	if mlen > olen-hlen-seedlen-1 {
		return nil
	}

	var DBMASK [RSA_RFS - 1 - 32]byte

	if p != nil {
		H.Process_array(p)
	}
	h := H.Hash()
	for i := 0; i < hlen; i++ {
		f[i] = h[i]
	}

	slen := olen - mlen - hlen - seedlen - 1

	for i := 0; i < slen; i++ {
		f[hlen+i] = 0
	}
	f[hlen+slen] = 1
	for i := 0; i < mlen; i++ {
		f[hlen+slen+1+i] = m[i]
	}

	for i := 0; i < seedlen; i++ {
		SEED[i] = rng.GetByte()
	}
	RSA_MGF1(SEED[:], olen-seedlen, DBMASK[:])

	for i := 0; i < olen-seedlen; i++ {
		DBMASK[i] ^= f[i]
	}
	RSA_MGF1(DBMASK[:], seedlen, f[:])

	for i := 0; i < seedlen; i++ {
		f[i] ^= SEED[i]
	}

	for i := 0; i < olen-seedlen; i++ {
		f[i+seedlen] = DBMASK[i]
	}

	/* pad to length RFS */
	d := 1
	for i := RSA_RFS - 1; i >= d; i-- {
		f[i] = f[i-d]
	}
	for i := d - 1; i >= 0; i-- {
		f[i] = 0
	}
	return f[:]
}

/* OAEP Message Decoding for Decryption */
func RSA_OAEP_DECODE(p []byte, f []byte) []byte {
	olen := RSA_RFS - 1

	H := NewHASH()
	hlen := 32
	var SEED [32]byte
	seedlen := hlen
	var CHASH [32]byte

	if olen < seedlen+hlen+1 {
		return nil
	}
	var DBMASK [RSA_RFS - 1 - 32]byte
	for i := 0; i < olen-seedlen; i++ {
		DBMASK[i] = 0
	}

	if len(f) < RSA_RFS {
		d := RSA_RFS - len(f)
		for i := RSA_RFS - 1; i >= d; i-- {
			f[i] = f[i-d]
		}
		for i := d - 1; i >= 0; i-- {
			f[i] = 0
		}
	}

	if p != nil {
		H.Process_array(p)
	}
	h := H.Hash()
	for i := 0; i < hlen; i++ {
		CHASH[i] = h[i]
	}

	x := f[0]

	for i := seedlen; i < olen; i++ {
		DBMASK[i-seedlen] = f[i+1]
	}

	RSA_MGF1(DBMASK[:], seedlen, SEED[:])
	for i := 0; i < seedlen; i++ {
		SEED[i] ^= f[i+1]
	}
	RSA_MGF1(SEED[:], olen-seedlen, f)
	for i := 0; i < olen-seedlen; i++ {
		DBMASK[i] ^= f[i]
	}

	comp := true
	for i := 0; i < hlen; i++ {
		if CHASH[i] != DBMASK[i] {
			comp = false
		}
	}

	for i := 0; i < olen-seedlen-hlen; i++ {
		DBMASK[i] = DBMASK[i+hlen]
	}

	for i := 0; i < hlen; i++ {
		SEED[i] = 0
		CHASH[i] = 0
	}

	var k int
	for k = 0; ; k++ {
		if k >= olen-seedlen-hlen {
			return nil
		}
		if DBMASK[k] != 0 {
			break
		}
	}

	t := DBMASK[k]
	if !comp || x != 0 || t != 0x01 {
		for i := 0; i < olen-seedlen; i++ {
			DBMASK[i] = 0
		}
		return nil
	}

	var r = make([]byte, olen-seedlen-hlen-k-1)

	for i := 0; i < olen-seedlen-hlen-k-1; i++ {
		r[i] = DBMASK[i+k+1]
	}

	for i := 0; i < olen-seedlen; i++ {
		DBMASK[i] = 0
	}

	return r
}

/* destroy the Private Key structure */
func RSA_PRIVATE_KEY_KILL(PRIV *rsa_private_key) {
	PRIV.p.zero()
	PRIV.q.zero()
	PRIV.dp.zero()
	PRIV.dq.zero()
	PRIV.c.zero()
}

/* RSA encryption with the public key */
func RSA_ENCRYPT(PUB *rsa_public_key, F []byte, G []byte) {
	n := PUB.n.getlen()
	f := NewFFint(n)

	ff_fromBytes(f, F)
	f.power(PUB.e, PUB.n)
	f.toBytes(G)
}

/* RSA decryption with the private key */
func RSA_DECRYPT(PRIV *rsa_private_key, G []byte, F []byte) {
	n := PRIV.p.getlen()
	g := NewFFint(2 * n)

	ff_fromBytes(g, G)
	jp := g.dmod(PRIV.p)
	jq := g.dmod(PRIV.q)

	jp.skpow(PRIV.dp, PRIV.p)
	jq.skpow(PRIV.dq, PRIV.q)

	g.zero()
	g.dscopy(jp)
	jp.mod(PRIV.q)
	if ff_comp(jp, jq) > 0 {
		jq.add(PRIV.q)
	}
	jq.sub(jp)
	jq.norm()

	t := ff_mul(PRIV.c, jq)
	jq = t.dmod(PRIV.q)

	t = ff_mul(jq, PRIV.p)
	g.add(t)
	g.norm()

	g.toBytes(F)
}
