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

/* MiotCL BN Curve Pairing functions */

package XXX

//import "fmt"

/* Line function */
func line(A *ECP4, B *ECP4, Qx *FP, Qy *FP) *FP24 {
	var a *FP8
	var b *FP8
	var c *FP8

	if A == B { /* Doubling */
		XX := NewFP4copy(A.getx()) //X
		YY := NewFP4copy(A.gety()) //Y
		ZZ := NewFP4copy(A.getz()) //Z
		YZ := NewFP4copy(YY)       //Y
		YZ.mul(ZZ)                 //YZ
		XX.sqr()                   //X^2
		YY.sqr()                   //Y^2
		ZZ.sqr()                   //Z^2

		YZ.imul(4)
		YZ.neg()
		YZ.norm()   //-2YZ
		YZ.qmul(Qy) //-2YZ.Ys

		XX.imul(6)  //3X^2
		XX.qmul(Qx) //3X^2.Xs

		sb := 3 * CURVE_B_I
		ZZ.imul(sb)
		if SEXTIC_TWIST == D_TYPE {
			ZZ.div_2i()
		}
		if SEXTIC_TWIST == M_TYPE {
			ZZ.times_i()
			ZZ.add(ZZ)
			YZ.times_i()
			YZ.norm()
		}
		ZZ.norm() // 3b.Z^2

		YY.add(YY)
		ZZ.sub(YY)
		ZZ.norm() // 3b.Z^2-Y^2

		a = NewFP8fp4s(YZ, ZZ) // -2YZ.Ys | 3b.Z^2-Y^2 | 3X^2.Xs
		if SEXTIC_TWIST == D_TYPE {

			b = NewFP8fp4(XX) // L(0,1) | L(0,0) | L(1,0)
			c = NewFP8int(0)
		}
		if SEXTIC_TWIST == M_TYPE {
			b = NewFP8int(0)
			c = NewFP8fp4(XX)
			c.times_i()
		}
		A.dbl()

	} else { /* Addition */

		X1 := NewFP4copy(A.getx()) // X1
		Y1 := NewFP4copy(A.gety()) // Y1
		T1 := NewFP4copy(A.getz()) // Z1
		T2 := NewFP4copy(A.getz()) // Z1

		T1.mul(B.gety()) // T1=Z1.Y2
		T2.mul(B.getx()) // T2=Z1.X2

		X1.sub(T2)
		X1.norm() // X1=X1-Z1.X2
		Y1.sub(T1)
		Y1.norm() // Y1=Y1-Z1.Y2

		T1.copy(X1) // T1=X1-Z1.X2
		X1.qmul(Qy) // X1=(X1-Z1.X2).Ys

		if SEXTIC_TWIST == M_TYPE {
			X1.times_i()
			X1.norm()
		}

		T1.mul(B.gety()) // T1=(X1-Z1.X2).Y2

		T2.copy(Y1)      // T2=Y1-Z1.Y2
		T2.mul(B.getx()) // T2=(Y1-Z1.Y2).X2
		T2.sub(T1)
		T2.norm() // T2=(Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2
		Y1.qmul(Qx)
		Y1.neg()
		Y1.norm() // Y1=-(Y1-Z1.Y2).Xs

		a = NewFP8fp4s(X1, T2) // (X1-Z1.X2).Ys  |  (Y1-Z1.Y2).X2 - (X1-Z1.X2).Y2  | - (Y1-Z1.Y2).Xs
		if SEXTIC_TWIST == D_TYPE {
			b = NewFP8fp4(Y1)
			c = NewFP8int(0)
		}
		if SEXTIC_TWIST == M_TYPE {
			b = NewFP8int(0)
			c = NewFP8fp4(Y1)
			c.times_i()
		}
		A.Add(B)
	}

	return NewFP24fp8s(a, b, c)
}

/* Optimal R-ate pairing */
func Ate(P1 *ECP4, Q1 *ECP) *FP24 {
	x := NewBIGints(CURVE_Bnx)
	n := NewBIGcopy(x)
	var lv *FP24

	n3 := NewBIGcopy(n)
	n3.pmul(3)
	n3.norm()

	P := NewECP4()
	P.Copy(P1)
	P.Affine()
	Q := NewECP()
	Q.Copy(Q1)
	Q.Affine()

	Qx := NewFPcopy(Q.getx())
	Qy := NewFPcopy(Q.gety())

	A := NewECP4()
	r := NewFP24int(1)

	A.Copy(P)
	NP := NewECP4()
	NP.Copy(P)
	NP.neg()

	nb := n3.nbits()

	for i := nb - 2; i >= 1; i-- {
		r.sqr()
		lv = line(A, A, Qx, Qy)
		r.smul(lv, SEXTIC_TWIST)
		bt := n3.bit(i) - n.bit(i)
		if bt == 1 {
			lv = line(A, P, Qx, Qy)
			r.smul(lv, SEXTIC_TWIST)
		}
		if bt == -1 {
			lv = line(A, NP, Qx, Qy)
			r.smul(lv, SEXTIC_TWIST)
		}
	}

	if SIGN_OF_X == NEGATIVEX {
		r.conj()
	}

	return r
}

/* Optimal R-ate double pairing e(P,Q).e(R,S) */
func Ate2(P1 *ECP4, Q1 *ECP, R1 *ECP4, S1 *ECP) *FP24 {
	x := NewBIGints(CURVE_Bnx)
	n := NewBIGcopy(x)
	var lv *FP24

	n3 := NewBIGcopy(n)
	n3.pmul(3)
	n3.norm()

	P := NewECP4()
	P.Copy(P1)
	P.Affine()
	Q := NewECP()
	Q.Copy(Q1)
	Q.Affine()
	R := NewECP4()
	R.Copy(R1)
	R.Affine()
	S := NewECP()
	S.Copy(S1)
	S.Affine()

	Qx := NewFPcopy(Q.getx())
	Qy := NewFPcopy(Q.gety())
	Sx := NewFPcopy(S.getx())
	Sy := NewFPcopy(S.gety())

	A := NewECP4()
	B := NewECP4()
	r := NewFP24int(1)

	A.Copy(P)
	B.Copy(R)
	NP := NewECP4()
	NP.Copy(P)
	NP.neg()
	NR := NewECP4()
	NR.Copy(R)
	NR.neg()

	nb := n3.nbits()

	for i := nb - 2; i >= 1; i-- {
		r.sqr()
		lv = line(A, A, Qx, Qy)
		r.smul(lv, SEXTIC_TWIST)
		lv = line(B, B, Sx, Sy)
		r.smul(lv, SEXTIC_TWIST)
		bt := n3.bit(i) - n.bit(i)
		if bt == 1 {
			lv = line(A, P, Qx, Qy)
			r.smul(lv, SEXTIC_TWIST)
			lv = line(B, R, Sx, Sy)
			r.smul(lv, SEXTIC_TWIST)
		}
		if bt == -1 {
			lv = line(A, NP, Qx, Qy)
			r.smul(lv, SEXTIC_TWIST)
			lv = line(B, NR, Sx, Sy)
			r.smul(lv, SEXTIC_TWIST)
		}
	}

	if SIGN_OF_X == NEGATIVEX {
		r.conj()
	}

	return r
}

/* final exponentiation - keep separate for multi-pairings and to avoid thrashing stack */
func Fexp(m *FP24) *FP24 {
	f := NewFP2bigs(NewBIGints(Fra), NewBIGints(Frb))
	x := NewBIGints(CURVE_Bnx)
	r := NewFP24copy(m)

	/* Easy part of final exp */
	lv := NewFP24copy(r)

	lv.Inverse()
	r.conj()

	r.Mul(lv)
	lv.Copy(r)
	r.frob(f, 4)
	r.Mul(lv)

	/* Hard part of final exp */
	// Ghamman & Fouotsa Method

	t7 := NewFP24copy(r)
	t7.usqr()
	t1 := t7.Pow(x)

	x.fshr(1)
	t2 := t1.Pow(x)
	x.fshl(1)

	if SIGN_OF_X == NEGATIVEX {
		t1.conj()
	}
	t3 := NewFP24copy(t1)
	t3.conj()
	t2.Mul(t3)
	t2.Mul(r)

	t3 = t2.Pow(x)
	t4 := t3.Pow(x)
	t5 := t4.Pow(x)

	if SIGN_OF_X == NEGATIVEX {
		t3.conj()
		t5.conj()
	}

	t3.frob(f, 6)
	t4.frob(f, 5)
	t3.Mul(t4)

	t6 := t5.Pow(x)
	if SIGN_OF_X == NEGATIVEX {
		t6.conj()
	}

	t5.frob(f, 4)
	t3.Mul(t5)

	t0 := NewFP24copy(t2)
	t0.conj()
	t6.Mul(t0)

	t5.Copy(t6)
	t5.frob(f, 3)

	t3.Mul(t5)
	t5 = t6.Pow(x)
	t6 = t5.Pow(x)

	if SIGN_OF_X == NEGATIVEX {
		t5.conj()
	}

	t0.Copy(t5)
	t0.frob(f, 2)
	t3.Mul(t0)
	t0.Copy(t6)
	t0.frob(f, 1)

	t3.Mul(t0)
	t5 = t6.Pow(x)

	if SIGN_OF_X == NEGATIVEX {
		t5.conj()
	}
	t2.frob(f, 7)

	t5.Mul(t7)
	t3.Mul(t2)
	t3.Mul(t5)

	r.Mul(t3)
	r.reduce()

	return r
}

/* GLV method */
func glv(e *BIG) []*BIG {
	var u []*BIG

	q := NewBIGints(CURVE_Order)
	x := NewBIGints(CURVE_Bnx)
	x2 := smul(x, x)
	x = smul(x2, x2)
	u = append(u, NewBIGcopy(e))
	u[0].Mod(x)
	u = append(u, NewBIGcopy(e))
	u[1].div(x)
	u[1].rsub(q)
	return u
}

/* Galbraith & Scott Method */
func gs(e *BIG) []*BIG {
	var u []*BIG

	q := NewBIGints(CURVE_Order)
	x := NewBIGints(CURVE_Bnx)
	w := NewBIGcopy(e)
	for i := 0; i < 7; i++ {
		u = append(u, NewBIGcopy(w))
		u[i].Mod(x)
		w.div(x)
	}
	u = append(u, NewBIGcopy(w))
	if SIGN_OF_X == NEGATIVEX {
		u[1].copy(Modneg(u[1], q))
		u[3].copy(Modneg(u[3], q))
		u[5].copy(Modneg(u[5], q))
		u[7].copy(Modneg(u[7], q))

	}

	return u
}

/* Multiply P by e in group G1 */
func G1mul(P *ECP, e *BIG) *ECP {
	var R *ECP
	if USE_GLV {
		R = NewECP()
		R.Copy(P)
		Q := NewECP()
		Q.Copy(P)
		Q.Affine()
		q := NewBIGints(CURVE_Order)
		cru := NewFPbig(NewBIGints(CURVE_Cru))
		t := NewBIGint(0)
		u := glv(e)
		Q.getx().mul(cru)

		np := u[0].nbits()
		t.copy(Modneg(u[0], q))
		nn := t.nbits()
		if nn < np {
			u[0].copy(t)
			R.neg()
		}

		np = u[1].nbits()
		t.copy(Modneg(u[1], q))
		nn = t.nbits()
		if nn < np {
			u[1].copy(t)
			Q.neg()
		}
		u[0].norm()
		u[1].norm()
		R = R.Mul2(u[0], Q, u[1])

	} else {
		R = P.mul(e)
	}
	return R
}

/* Multiply P by e in group G2 */
func G2mul(P *ECP4, e *BIG) *ECP4 {
	var R *ECP4
	if USE_GS_G2 {
		var Q []*ECP4

		F := ECP4_frob_constants()

		q := NewBIGints(CURVE_Order)
		u := gs(e)

		t := NewBIGint(0)

		Q = append(Q, NewECP4())
		Q[0].Copy(P)
		for i := 1; i < 8; i++ {
			Q = append(Q, NewECP4())
			Q[i].Copy(Q[i-1])
			Q[i].frob(F, 1)
		}
		for i := 0; i < 8; i++ {
			np := u[i].nbits()
			t.copy(Modneg(u[i], q))
			nn := t.nbits()
			if nn < np {
				u[i].copy(t)
				Q[i].neg()
			}
			u[i].norm()
		}

		R = mul8(Q, u)

	} else {
		R = P.mul(e)
	}
	return R
}

/* f=f^e */
/* Note that this method requires a lot of RAM! Better to use compressed XTR method, see FP4.java */
func GTpow(d *FP24, e *BIG) *FP24 {
	var r *FP24
	if USE_GS_GT {
		var g []*FP24
		f := NewFP2bigs(NewBIGints(Fra), NewBIGints(Frb))
		q := NewBIGints(CURVE_Order)
		t := NewBIGint(0)

		u := gs(e)

		g = append(g, NewFP24copy(d))
		for i := 1; i < 8; i++ {
			g = append(g, NewFP24int(0))
			g[i].Copy(g[i-1])
			g[i].frob(f, 1)
		}
		for i := 0; i < 8; i++ {
			np := u[i].nbits()
			t.copy(Modneg(u[i], q))
			nn := t.nbits()
			if nn < np {
				u[i].copy(t)
				g[i].conj()
			}
			u[i].norm()
		}
		r = pow8(g, u)
	} else {
		r = d.Pow(e)
	}
	return r
}
