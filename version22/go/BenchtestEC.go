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
	var P [RSA_RFS]byte
	var M [RSA_RFS]byte
	var C [RSA_RFS]byte

	rng:=NewRAND()

	rng.Clean();
	for i:=0;i<100;i++ {RAW[i]=byte(i)}

	rng.Seed(100,RAW[:])

	pub:=New_rsa_public_key(FFLEN)
	priv:=New_rsa_private_key(HFLEN)

	if CURVETYPE==WEIERSTRASS {
		fmt.Printf("Weierstrass parameterization\n")
	}		
	if CURVETYPE==EDWARDS {
		fmt.Printf("Edwards parameterization\n")
	}
	if CURVETYPE==MONTGOMERY {
		fmt.Printf("Montgomery parameterization\n")
	}

	if MODTYPE==PSEUDO_MERSENNE {
		fmt.Printf("Pseudo-Mersenne Modulus\n")
	}
	if MODTYPE==MONTGOMERY_FRIENDLY {
		fmt.Printf("Montgomery friendly Modulus\n")
	}
	if MODTYPE==GENERALISED_MERSENNE {
		fmt.Printf("Generalised-Mersenne Modulus\n")
	}
	if MODTYPE==NOT_SPECIAL {
		fmt.Printf("Not special Modulus\n")
	}

	fmt.Printf("Modulus size %d bits\n",MODBITS)
	fmt.Printf("%d bit build\n",CHUNK)

	var s *BIG
	var G *ECP

	gx:=NewBIGints(CURVE_Gx)
	if CURVETYPE!=MONTGOMERY {
		gy:=NewBIGints(CURVE_Gy)
		G=NewECPbigs(gx,gy)
	} else {
		G=NewECPbig(gx)
	}

	r:=NewBIGints(CURVE_Order)
	s=randomnum(r,rng)

	WP:=G.mul(r)
	if !WP.is_infinity() {
		fmt.Printf("FAILURE - rG!=O\n")
		return
	}

	start := time.Now()
	iterations:=0
	elapsed:=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		WP=G.mul(s)
		iterations++
		elapsed=time.Since(start)
	} 
	dur:=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("EC  mul - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	fmt.Printf("Generating %d-bit RSA public/private key pair\n",FFLEN*BIGBITS);

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		RSA_KEY_PAIR(rng,65537,priv,pub)
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("RSA gen - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	for i:=0;i<RSA_RFS;i++ {M[i]=byte(i%128)};

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		RSA_ENCRYPT(pub,M[:],C[:])
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("RSA enc - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	start = time.Now()
	iterations=0
	elapsed=time.Since(start)
	for (int(elapsed/time.Second))<MIN_TIME || iterations<MIN_ITERS {
		RSA_DECRYPT(priv,C[:],P[:])
		iterations++
		elapsed=time.Since(start)
	} 
	dur=float64(elapsed/time.Millisecond)/float64(iterations)
	fmt.Printf("RSA dec - %8d iterations  ",iterations)
	fmt.Printf(" %8.2f ms per iteration\n",dur)

	for i:=0;i<RSA_RFS;i++ {
		if (P[i]!=M[i]) {
			fmt.Printf("FAILURE - RSA decryption\n")
			return
		}
	}

	fmt.Printf("All tests pass\n")
}
