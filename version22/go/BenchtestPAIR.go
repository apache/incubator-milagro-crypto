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

/* Test and benchmark elliptic curve and RSA functions */

package main

import "fmt"

import "time"

const MIN_TIME int=10
const MIN_ITERS int=10

func main() {
	var RAW [100]byte

	rng:=NewRAND()

	rng.Clean();
	for i:=0;i<100;i++ {RAW[i]=byte(i)}

	rng.Seed(100,RAW[:])

	if CURVE_PAIRING_TYPE==BN_CURVE {
		fmt.Printf("BN Pairing-Friendly Curve\n")
	}
	if CURVE_PAIRING_TYPE==BLS_CURVE {
		fmt.Printf("BLS Pairing-Friendly Curve\n")
	}

	fmt.Printf("Modulus size %d bits\n",MODBITS)
	fmt.Printf("%d bit build\n",CHUNK)

	G:=NewECPbigs(NewBIGints(CURVE_Gx),NewBIGints(CURVE_Gy))
	r:=NewBIGints(CURVE_Order)
	s:=randomnum(r,rng)

	P:=G1mul(G,r)

	if !P.is_infinity() {
		fmt.Printf("FAILURE - rP!=O\n");
		return;
	}

	start := time.Now()
	iterations:=0
	elapsed:=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		P=G1mul(G,s)
		iterations++
		elapsed=time.Since(start)
	} 
	dur:=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("G1 mul              - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	Q:=NewECP2fp2s(NewFP2bigs(NewBIGints(CURVE_Pxa),NewBIGints(CURVE_Pxb)),NewFP2bigs(NewBIGints(CURVE_Pya),NewBIGints(CURVE_Pyb)))
	W:=G2mul(Q,r)

	if !W.is_infinity() {
		fmt.Printf("FAILURE - rQ!=O\n");
		return;
	}

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		W=G2mul(Q,s)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("G2 mul              - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	w:=ate(Q,P)
	w=fexp(w)

	g:=GTpow(w,r)

	if !g.isunity() {
		fmt.Printf("FAILURE - g^r!=1\n");
		return;
	}

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		g=GTpow(w,s)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("GT pow              - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	f:=NewFP2bigs(NewBIGints(CURVE_Fra),NewBIGints(CURVE_Frb))
	q:=NewBIGints(Modulus)

	m:=NewBIGcopy(q)
	m.mod(r)

	a:=NewBIGcopy(s)
	a.mod(m)

	b:=NewBIGcopy(s)
	b.div(m)

	g.copy(w)
	c:=g.trace()

	g.frob(f)
	cp:=g.trace()

	w.conj()
	g.mul(w)
	cpm1:=g.trace()
	g.mul(w)
	cpm2:=g.trace()

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		c=c.xtr_pow2(cp,cpm1,cpm2,a,b)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("GT pow (compressed) - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		w=ate(Q,P)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("PAIRing ATE         - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		g=fexp(w)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("PAIRing FEXP        - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	P.copy(G)
	Q.copy(W)

	P=G1mul(P,s)

	g=ate(Q,P)
	g=fexp(g)

	P.copy(G)
	Q=G2mul(Q,s)

	w=ate(Q,P)
	w=fexp(w)

	if !g.equals(w) {
		fmt.Printf("FAILURE - e(sQ,p)!=e(Q,sP) \n")
		return
	}

	Q.copy(W);
	g=ate(Q,P)
	g=fexp(g)
	g=GTpow(g,s)

	if !g.equals(w) {
		fmt.Printf("FAILURE - e(sQ,p)!=e(Q,P)^s \n")
		return
	}

	fmt.Printf("All tests pass\n") 
}
