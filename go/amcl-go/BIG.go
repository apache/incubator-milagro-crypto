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

/* AMCL BIG number class */

package amcl

import "strconv"

//import "fmt"

type BIG struct {
	w [NLEN]int64
}

func NewBIG() *BIG {
	b := new(BIG)
	for i := 0; i < NLEN; i++ {
		b.w[i] = 0
	}
	return b
}

func NewBIGint(x int) *BIG {
	b := new(BIG)
	b.w[0] = int64(x)
	for i := 1; i < NLEN; i++ {
		b.w[i] = 0
	}
	return b
}

func NewBIGcopy(x *BIG) *BIG {
	b := new(BIG)
	for i := 0; i < NLEN; i++ {
		b.w[i] = x.w[i]
	}
	return b
}

func NewBIGdcopy(x *DBIG) *BIG {
	b := new(BIG)
	for i := 0; i < NLEN; i++ {
		b.w[i] = x.w[i]
	}
	return b
}

func NewBIGints(x [NLEN]int64) *BIG {
	b := new(BIG)
	for i := 0; i < NLEN; i++ {
		b.w[i] = x[i]
	}
	return b
}

func (r *BIG) get(i int) int64 {
	return r.w[i]
}

func (r *BIG) set(i int, x int64) {
	r.w[i] = x
}

func (r *BIG) xortop(x int64) {
	r.w[NLEN-1] ^= x
}

func (r *BIG) ortop(x int64) {
	r.w[NLEN-1] |= x
}

/* test for zero */
func (r *BIG) iszilch() bool {
	for i := 0; i < NLEN; i++ {
		if r.w[i] != 0 {
			return false
		}
	}
	return true
}

/* set to zero */
func (r *BIG) zero() {
	for i := 0; i < NLEN; i++ {
		r.w[i] = 0
	}
}

/* Test for equal to one */
func (r *BIG) isunity() bool {
	for i := 1; i < NLEN; i++ {
		if r.w[i] != 0 {
			return false
		}
	}
	if r.w[0] != 1 {
		return false
	}
	return true
}

/* set to one */
func (r *BIG) one() {
	r.w[0] = 1
	for i := 1; i < NLEN; i++ {
		r.w[i] = 0
	}
}

/* Copy from another BIG */
func (r *BIG) copy(x *BIG) {
	for i := 0; i < NLEN; i++ {
		r.w[i] = x.w[i]
	}
}

/* Copy from another DBIG */
func (r *BIG) dcopy(x *DBIG) {
	for i := 0; i < NLEN; i++ {
		r.w[i] = x.w[i]
	}
}

/* calculate Field Excess */
func EXCESS(a *BIG) int64 {
	return ((a.w[NLEN-1] & OMASK) >> (MODBITS % BASEBITS))
}

/* normalise BIG - force all digits < 2^BASEBITS */
func (r *BIG) norm() int64 {
	var carry int64 = 0
	for i := 0; i < NLEN-1; i++ {
		d := r.w[i] + carry
		r.w[i] = d & MASK
		carry = d >> BASEBITS
	}
	r.w[NLEN-1] = (r.w[NLEN-1] + carry)

	return (r.w[NLEN-1] >> ((8 * MODBYTES) % BASEBITS))
}

/* Conditional swap of two bigs depending on d using XOR - no branches */
func (r *BIG) cswap(b *BIG, d int32) {
	var c = int64(d)
	c = ^(c - 1)

	for i := 0; i < NLEN; i++ {
		t := c & (r.w[i] ^ b.w[i])
		r.w[i] ^= t
		b.w[i] ^= t
	}
}

func (r *BIG) cmove(g *BIG, d int32) {
	var b = int64(-d)

	for i := 0; i < NLEN; i++ {
		r.w[i] ^= (r.w[i] ^ g.w[i]) & b
	}
}

/* Shift right by less than a word */
func (r *BIG) fshr(k uint) int64 {
	w := r.w[0] & ((int64(1) << k) - 1) /* shifted out part */
	for i := 0; i < NLEN-1; i++ {
		r.w[i] = (r.w[i] >> k) | ((r.w[i+1] << (BASEBITS - k)) & MASK)
	}
	r.w[NLEN-1] = r.w[NLEN-1] >> k
	return w
}

/* general shift right */
func (r *BIG) shr(k uint) {
	n := (k % BASEBITS)
	m := int(k / BASEBITS)
	for i := 0; i < NLEN-m-1; i++ {
		r.w[i] = (r.w[m+i] >> n) | ((r.w[m+i+1] << (BASEBITS - n)) & MASK)
	}
	r.w[NLEN-m-1] = r.w[NLEN-1] >> n
	for i := NLEN - m; i < NLEN; i++ {
		r.w[i] = 0
	}
}

/* Shift right by less than a word */
func (r *BIG) fshl(k uint) int64 {
	r.w[NLEN-1] = (r.w[NLEN-1] << k) | (r.w[NLEN-2] >> (BASEBITS - k))
	for i := NLEN - 2; i > 0; i-- {
		r.w[i] = ((r.w[i] << k) & MASK) | (r.w[i-1] >> (BASEBITS - k))
	}
	r.w[0] = (r.w[0] << k) & MASK
	return (r.w[NLEN-1] >> ((8 * MODBYTES) % BASEBITS)) /* return excess - only used in ff.c */
}

/* general shift left */
func (r *BIG) shl(k uint) {
	n := k % BASEBITS
	m := int(k / BASEBITS)

	r.w[NLEN-1] = (r.w[NLEN-1-m] << n) | (r.w[NLEN-m-2] >> (BASEBITS - n))
	for i := NLEN - 2; i > m; i-- {
		r.w[i] = ((r.w[i-m] << n) & MASK) | (r.w[i-m-1] >> (BASEBITS - n))
	}
	r.w[m] = (r.w[0] << n) & MASK
	for i := 0; i < m; i++ {
		r.w[i] = 0
	}
}

/* return number of bits */
func (r *BIG) nbits() int {
	k := NLEN - 1
	r.norm()
	for k >= 0 && r.w[k] == 0 {
		k--
	}
	if k < 0 {
		return 0
	}
	bts := int(BASEBITS) * k
	c := r.w[k]
	for c != 0 {
		c /= 2
		bts++
	}
	return bts
}

/* Convert to Hex String */
func (r *BIG) toString() string {
	s := ""
	len := r.nbits()

	if len%4 == 0 {
		len /= 4
	} else {
		len /= 4
		len++

	}
	MB := int(MODBYTES * 2)
	if len < MB {
		len = MB
	}

	for i := len - 1; i >= 0; i-- {
		b := NewBIGcopy(r)

		b.shr(uint(i * 4))
		s += strconv.FormatInt(b.w[0]&15, 16)
	}
	return s
}

func (r *BIG) add(x *BIG) {
	for i := 0; i < NLEN; i++ {
		r.w[i] = r.w[i] + x.w[i]
	}
}

/* return this+x */
func (r *BIG) plus(x *BIG) *BIG {
	s := new(BIG)
	for i := 0; i < NLEN; i++ {
		s.w[i] = r.w[i] + x.w[i]
	}
	return s
}

/* this+=x, where x is int */
func (r *BIG) inc(x int) {
	r.norm()
	r.w[0] += int64(x)
}

/* return this-x */
func (r *BIG) minus(x *BIG) *BIG {
	d := new(BIG)
	for i := 0; i < NLEN; i++ {
		d.w[i] = r.w[i] - x.w[i]
	}
	return d
}

/* this-=x */
func (r *BIG) sub(x *BIG) {
	for i := 0; i < NLEN; i++ {
		r.w[i] = r.w[i] - x.w[i]
	}
}

/* reverse subtract this=x-this */
func (r *BIG) rsub(x *BIG) {
	for i := 0; i < NLEN; i++ {
		r.w[i] = x.w[i] - r.w[i]
	}
}

/* this-=x, where x is int */
func (r *BIG) dec(x int) {
	r.norm()
	r.w[0] -= int64(x)
}

/* this*=x, where x is small int<NEXCESS */
func (r *BIG) imul(c int) {
	for i := 0; i < NLEN; i++ {
		r.w[i] *= int64(c)
	}
}

/* convert this BIG to byte array */
func (r *BIG) tobytearray(b []byte, n int) {
	r.norm()
	c := NewBIGcopy(r)

	for i := int(MODBYTES) - 1; i >= 0; i-- {
		b[i+n] = byte(c.w[0])
		c.fshr(8)
	}
}

/* convert from byte array to BIG */
func frombytearray(b []byte, n int) *BIG {
	m := NewBIG()
	for i := 0; i < int(MODBYTES); i++ {
		m.fshl(8)
		m.w[0] += int64(b[i+n] & 0xff)
	}
	return m
}

func (r *BIG) toBytes(b []byte) {
	r.tobytearray(b, 0)
}

func fromBytes(b []byte) *BIG {
	return frombytearray(b, 0)
}

/* set this[i]+=x*y+c, and return high part */

func (r *BIG) muladd(a int64, b int64, c int64, i int) int64 {
	x0 := a & HMASK
	x1 := (a >> HBITS)
	y0 := b & HMASK
	y1 := (b >> HBITS)
	bot := x0 * y0
	top := x1 * y1
	mid := x0*y1 + x1*y0
	x0 = mid & HMASK
	x1 = (mid >> HBITS)
	bot += x0 << HBITS
	bot += c
	bot += r.w[i]
	top += x1
	carry := bot >> BASEBITS
	bot &= MASK
	top += carry
	r.w[i] = bot
	return top
}

/* this*=x, where x is >NEXCESS */
func (r *BIG) pmul(c int) int64 {
	var carry int64 = 0
	r.norm()
	for i := 0; i < NLEN; i++ {
		ak := r.w[i]
		r.w[i] = 0
		carry = r.muladd(ak, int64(c), carry, i)
	}
	return carry
}

/* this*=c and catch overflow in DBIG */
func (r *BIG) pxmul(c int) *DBIG {
	m := NewDBIG()
	var carry int64 = 0
	for j := 0; j < NLEN; j++ {
		carry = m.muladd(r.w[j], int64(c), carry, j)
	}
	m.w[NLEN] = carry
	return m
}

/* divide by 3 */
func (r *BIG) div3() int {
	var carry int64 = 0
	r.norm()
	base := (int64(1) << BASEBITS)
	for i := NLEN - 1; i >= 0; i-- {
		ak := (carry*base + r.w[i])
		r.w[i] = ak / 3
		carry = ak % 3
	}
	return int(carry)
}

/* return a*b where result fits in a BIG */
func smul(a *BIG, b *BIG) *BIG {
	var carry int64
	c := NewBIG()
	for i := 0; i < NLEN; i++ {
		carry = 0
		for j := 0; j < NLEN; j++ {
			if i+j < NLEN {
				carry = c.muladd(a.w[i], b.w[j], carry, i+j)
			}
		}
	}
	return c
}

/* Compare a and b, return 0 if a==b, -1 if a<b, +1 if a>b. Inputs must be normalised */
func comp(a *BIG, b *BIG) int {
	for i := NLEN - 1; i >= 0; i-- {
		if a.w[i] == b.w[i] {
			continue
		}
		if a.w[i] > b.w[i] {
			return 1
		} else {
			return -1
		}
	}
	return 0
}

/* return parity */
func (r *BIG) parity() int {
	return int(r.w[0] % 2)
}

/* return n-th bit */
func (r *BIG) bit(n int) int {
	if (r.w[n/int(BASEBITS)] & (int64(1) << (uint(n) % BASEBITS))) > 0 {
		return 1
	}
	return 0
}

/* return n last bits */
func (r *BIG) lastbits(n int) int {
	msk := (1 << uint(n)) - 1
	r.norm()
	return (int(r.w[0])) & msk
}

/* set x = x mod 2^m */
func (r *BIG) mod2m(m uint) {
	wd := int(m / BASEBITS)
	bt := m % BASEBITS
	msk := (int64(1) << bt) - 1
	r.w[wd] &= msk
	for i := wd + 1; i < NLEN; i++ {
		r.w[i] = 0
	}
}

/* Arazi and Qi inversion mod 256 */
func invmod256(a int) int {
	var t1 int = 0
	c := (a >> 1) & 1
	t1 += c
	t1 &= 1
	t1 = 2 - t1
	t1 <<= 1
	U := t1 + 1

	// i=2
	b := a & 3
	t1 = U * b
	t1 >>= 2
	c = (a >> 2) & 3
	t2 := (U * c) & 3
	t1 += t2
	t1 *= U
	t1 &= 3
	t1 = 4 - t1
	t1 <<= 2
	U += t1

	// i=4
	b = a & 15
	t1 = U * b
	t1 >>= 4
	c = (a >> 4) & 15
	t2 = (U * c) & 15
	t1 += t2
	t1 *= U
	t1 &= 15
	t1 = 16 - t1
	t1 <<= 4
	U += t1

	return U
}

/* a=1/a mod 2^256. This is very fast! */
func (r *BIG) invmod2m() {
	U := NewBIG()
	b := NewBIG()
	c := NewBIG()

	U.inc(invmod256(r.lastbits(8)))

	for i := 8; i < 256; i <<= 1 {
		ui := uint(i)
		b.copy(r)
		b.mod2m(ui)
		t1 := smul(U, b)
		t1.shr(ui)
		c.copy(r)
		c.shr(ui)
		c.mod2m(ui)

		t2 := smul(U, c)
		t2.mod2m(ui)
		t1.add(t2)
		b = smul(t1, U)
		t1.copy(b)
		t1.mod2m(ui)

		t2.one()
		t2.shl(ui)
		t1.rsub(t2)
		t1.norm()
		t1.shl(ui)
		U.add(t1)
	}
	r.copy(U)
}

/* reduce this mod m */
func (r *BIG) mod(m *BIG) {
	r.norm()
	if comp(r, m) < 0 {
		return
	}

	m.fshl(1)
	k := 1

	for comp(r, m) >= 0 {
		m.fshl(1)
		k++
	}

	for k > 0 {
		m.fshr(1)
		if comp(r, m) >= 0 {
			r.sub(m)
			r.norm()
		}
		k--
	}
}

/* divide this by m */
func (r *BIG) div(m *BIG) {
	k := 0
	r.norm()
	e := NewBIGint(1)
	b := NewBIGcopy(r)
	r.zero()

	for comp(b, m) >= 0 {
		e.fshl(1)
		m.fshl(1)
		k++
	}

	for k > 0 {
		m.fshr(1)
		e.fshr(1)
		if comp(b, m) >= 0 {
			r.add(e)
			r.norm()
			b.sub(m)
			b.norm()
		}
		k--
	}
}

/* get 8*MODBYTES size random number */
func random(rng *RAND) *BIG {
	m := NewBIG()
	var j int = 0
	var r byte = 0
	/* generate random BIG */
	for i := 0; i < 8*int(MODBYTES); i++ {
		if j == 0 {
			r = rng.GetByte()
		} else {
			r >>= 1
		}

		b := int64(r & 1)
		m.shl(1)
		m.w[0] += b // m.inc(b)
		j++
		j &= 7
	}
	return m
}

/* Create random BIG in portable way, one bit at a time */
func randomnum(q *BIG, rng *RAND) *BIG {
	d := NewDBIG()
	var j int = 0
	var r byte = 0
	for i := 0; i < 2*int(MODBITS); i++ {
		if j == 0 {
			r = rng.GetByte()
		} else {
			r >>= 1
		}

		b := int64(r & 1)
		d.shl(1)
		d.w[0] += b // m.inc(b);
		j++
		j &= 7
	}
	m := d.mod(q)
	return m
}

/* return NAF value as +/- 1, 3 or 5. x and x3 should be normed.
nbs is number of bits processed, and nzs is number of trailing 0s detected */
func nafbits(x *BIG, x3 *BIG, i int) [3]int {
	var n [3]int
	var j int
	nb := x3.bit(i) - x.bit(i)

	n[1] = 1
	n[0] = 0
	if nb == 0 {
		n[0] = 0
		return n
	}
	if i == 0 {
		n[0] = nb
		return n
	}
	if nb > 0 {
		n[0] = 1
	} else {
		n[0] = (-1)
	}

	for j = i - 1; j > 0; j-- {
		n[1]++
		n[0] *= 2
		nb = x3.bit(j) - x.bit(j)
		if nb > 0 {
			n[0] += 1
		}
		if nb < 0 {
			n[0] -= 1
		}
		if n[0] > 5 || n[0] < -5 {
			break
		}
	}

	if n[0]%2 != 0 && j != 0 { /* backtrack */
		if nb > 0 {
			n[0] = (n[0] - 1) / 2
		}
		if nb < 0 {
			n[0] = (n[0] + 1) / 2
		}
		n[1]--
	}
	for n[0]%2 == 0 { /* remove trailing zeros */
		n[0] /= 2
		n[2]++
		n[1]--
	}
	return n
}

/* return a*b as DBIG */
func mul(a *BIG, b *BIG) *DBIG {
	c := NewDBIG()
	var carry int64
	a.norm()
	b.norm()

	for i := 0; i < NLEN; i++ {
		carry = 0
		for j := 0; j < NLEN; j++ {
			carry = c.muladd(a.w[i], b.w[j], carry, i+j)
		}
		c.w[NLEN+i] = carry
	}

	return c
}

/* return a^2 as DBIG */
func sqr(a *BIG) *DBIG {
	c := NewDBIG()
	var carry int64
	a.norm()
	for i := 0; i < NLEN; i++ {
		carry = 0
		for j := i + 1; j < NLEN; j++ {
			carry = c.muladd(2*a.w[i], a.w[j], carry, i+j)
		}
		c.w[NLEN+i] = carry
	}

	for i := 0; i < NLEN; i++ {
		c.w[2*i+1] += c.muladd(a.w[i], a.w[i], 0, 2*i)
	}
	c.norm()
	return c
}

/* reduce a DBIG to a BIG using the appropriate form of the modulus */
func mod(d *DBIG) *BIG {
	var b *BIG
	if MODTYPE == PSEUDO_MERSENNE {
		t := d.split(MODBITS)
		b = NewBIGdcopy(d)

		v := t.pmul(int(MConst))
		tw := t.w[NLEN-1]
		t.w[NLEN-1] &= TMASK
		t.w[0] += (MConst * ((tw >> TBITS) + (v << (BASEBITS - TBITS))))

		b.add(t)
	}
	if MODTYPE == MONTGOMERY_FRIENDLY {
		for i := 0; i < NLEN; i++ {
			d.w[NLEN+i] += d.muladd(d.w[i], MConst-1, d.w[i], NLEN+i-1)
		}
		b = NewBIG()

		for i := 0; i < NLEN; i++ {
			b.w[i] = d.w[NLEN+i]
		}
	}

	if MODTYPE == NOT_SPECIAL {
		md := NewBIGints(Modulus)
		var carry, m int64
		for i := 0; i < NLEN; i++ {
			if MConst == -1 {
				m = (-d.w[i]) & MASK
			} else {
				if MConst == 1 {
					m = d.w[i]
				} else {
					m = (MConst * d.w[i]) & MASK
				}
			}

			carry = 0
			for j := 0; j < NLEN; j++ {
				carry = d.muladd(m, md.w[j], carry, i+j)
			}
			d.w[NLEN+i] += carry
		}

		b = NewBIG()
		for i := 0; i < NLEN; i++ {
			b.w[i] = d.w[NLEN+i]
		}

	}
	b.norm()
	return b
}

/* return a*b mod m */
func modmul(a, b, m *BIG) *BIG {
	a.mod(m)
	b.mod(m)
	d := mul(a, b)
	return d.mod(m)
}

/* return a^2 mod m */
func modsqr(a, m *BIG) *BIG {
	a.mod(m)
	d := sqr(a)
	return d.mod(m)
}

/* return -a mod m */
func modneg(a, m *BIG) *BIG {
	a.mod(m)
	return m.minus(a)
}

/* return this^e mod m */
func (r *BIG) powmod(e *BIG, m *BIG) *BIG {
	r.norm()
	e.norm()
	a := NewBIGint(1)
	z := NewBIGcopy(e)
	s := NewBIGcopy(r)
	for true {
		bt := z.parity()
		z.fshr(1)
		if bt == 1 {
			a = modmul(a, s, m)
		}
		if z.iszilch() {
			break
		}
		s = modsqr(s, m)
	}
	return a
}

/* Jacobi Symbol (this/p). Returns 0, 1 or -1 */
func (r *BIG) jacobi(p *BIG) int {
	m := 0
	t := NewBIGint(0)
	x := NewBIGint(0)
	n := NewBIGint(0)
	zilch := NewBIGint(0)
	one := NewBIGint(1)
	if p.parity() == 0 || comp(r, zilch) == 0 || comp(p, one) <= 0 {
		return 0
	}
	r.norm()
	x.copy(r)
	n.copy(p)
	x.mod(p)

	for comp(n, one) > 0 {
		if comp(x, zilch) == 0 {
			return 0
		}
		n8 := n.lastbits(3)
		k := 0
		for x.parity() == 0 {
			k++
			x.shr(1)
		}
		if k%2 == 1 {
			m += (n8*n8 - 1) / 8
		}
		m += (n8 - 1) * (x.lastbits(2) - 1) / 4
		t.copy(n)
		t.mod(x)
		n.copy(x)
		x.copy(t)
		m %= 2

	}
	if m == 0 {
		return 1
	}
	return -1
}

/* this=1/this mod p. Binary method */
func (r *BIG) invmodp(p *BIG) {
	r.mod(p)
	u := NewBIGcopy(r)

	v := NewBIGcopy(p)
	x1 := NewBIGint(1)
	x2 := NewBIGint(0)
	t := NewBIGint(0)
	one := NewBIGint(1)
	for comp(u, one) != 0 && comp(v, one) != 0 {
		for u.parity() == 0 {
			u.shr(1)
			if x1.parity() != 0 {
				x1.add(p)
				x1.norm()
			}
			x1.shr(1)
		}
		for v.parity() == 0 {
			v.shr(1)
			if x2.parity() != 0 {
				x2.add(p)
				x2.norm()
			}
			x2.shr(1)
		}
		if comp(u, v) >= 0 {
			u.sub(v)
			u.norm()
			if comp(x1, x2) >= 0 {
				x1.sub(x2)
			} else {
				t.copy(p)
				t.sub(x2)
				x1.add(t)
			}
			x1.norm()
		} else {
			v.sub(u)
			v.norm()
			if comp(x2, x1) >= 0 {
				x2.sub(x1)
			} else {
				t.copy(p)
				t.sub(x1)
				x2.add(t)
			}
			x2.norm()
		}
	}
	if comp(u, one) == 0 {
		r.copy(x1)
	} else {
		r.copy(x2)
	}
}

/*
func main() {
	a := NewBIGint(3)
	m := NewBIGints(Modulus)

	fmt.Printf("Modulus= "+m.toString())
	fmt.Printf("\n")


	e := NewBIGcopy(m);
	e.dec(7); e.norm();
	fmt.Printf("Exponent= "+e.toString())
	fmt.Printf("\n")
	a=a.powmod(e,m);
	fmt.Printf("Result= "+a.toString())
}
*/
