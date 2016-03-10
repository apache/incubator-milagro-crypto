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

package amcl

const EAS int = 16
const EGS int = int(MODBYTES)
const EFS int = int(MODBYTES)
const HASH_BYTES int = 32
const IVS int = 12
const G1S = 2*EFS + 1
const G2S = 4 * EFS
const GTS = 12 * EFS

/* create random secret S. Use GO RNG */
func MPIN_RANDOM_GENERATE_WRAP(RNG *RAND) (int, []byte) {
	var S [EGS]byte
	errorCode := MPIN_RANDOM_GENERATE(RNG, S[:])
	return errorCode, S[:]
}

/* Extract Server Secret SS=S*Q where Q is fixed generator in G2 and S is master secret */
func MPIN_GET_SERVER_SECRET_WRAP(S []byte) (int, []byte) {
	var SS [G2S]byte
	errorCode := MPIN_GET_SERVER_SECRET(S[:], SS[:])
	return errorCode, SS[:]
}

/* R=R1+R2 in group G1 */
func MPIN_RECOMBINE_G1_WRAP(R1 []byte, R2 []byte) (int, []byte) {
	var R [G1S]byte
	errorCode := MPIN_RECOMBINE_G1(R1[:], R2[:], R[:])
	return errorCode, R[:]
}

/* W=W1+W2 in group G2 */
func MPIN_RECOMBINE_G2_WRAP(W1 []byte, W2 []byte) (int, []byte) {
	var W [G2S]byte
	errorCode := MPIN_RECOMBINE_G2(W1[:], W2[:], W[:])
	return errorCode, W[:]
}

/* Client secret CS=S*H(ID) where ID is client ID and S is master secret */
/* CID is hashed externally */
func MPIN_GET_CLIENT_SECRET_WRAP(S []byte, ID []byte) (int, []byte) {
	var CS [G1S]byte
	errorCode := MPIN_GET_CLIENT_SECRET(S[:], ID[:], CS[:])
	return errorCode, CS[:]
}

/* Time Permit TP=S*(date|H(ID)) where S is master secret */
func MPIN_GET_CLIENT_PERMIT_WRAP(date int, S []byte, ID []byte) (int, []byte) {
	var TP [G1S]byte
	errorCode := MPIN_GET_CLIENT_PERMIT(date, S[:], ID[:], TP[:])
	return errorCode, TP[:]
}

/* Extract PIN from CS for identity CID to form TOKEN */
func MPIN_EXTRACT_PIN_WRAP(ID []byte, PIN int, CS []byte) (int, []byte) {
	CSIn := make([]byte, G1S)
	copy(CSIn, CS)
	errorCode := MPIN_EXTRACT_PIN(ID[:], PIN, CSIn[:])
	return errorCode, CSIn[:]
}

/* One pass MPIN Client. Using GO RNG */
func MPIN_CLIENT_WRAP(date, TimeValue, PIN int, RNG *RAND, ID, X, TOKEN, TP, MESSAGE []byte) (int, []byte, []byte, []byte, []byte, []byte) {
	var Y [EGS]byte
	var SEC [G1S]byte
	var U [G1S]byte
	var UT [G1S]byte
	errorCode := MPIN_CLIENT(date, ID, RNG, X[:], PIN, TOKEN[:], SEC[:], U[:], UT[:], TP[:], MESSAGE, TimeValue, Y[:])
	return errorCode, X[:], Y[:], SEC[:], U[:], UT[:]
}

// Precompute values for use by the client side of M-Pin Full
func MPIN_PRECOMPUTE_WRAP(TOKEN []byte, ID []byte) (int, []byte, []byte) {
	var GT1 [GTS]byte
	var GT2 [GTS]byte
	errorCode := MPIN_PRECOMPUTE(TOKEN[:], ID[:], GT1[:], GT2[:])
	return errorCode, GT1[:], GT2[:]
}

/*
 W=x*H(G);
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 if typ=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
 Use GO RNG
*/
func MPIN_GET_G1_MULTIPLE_WRAP(RNG *RAND, typ int, X, G []byte) (int, []byte, []byte) {
	var Z [G1S]byte
	errorCode := MPIN_GET_G1_MULTIPLE(RNG, typ, X[:], G[:], Z[:])
	return errorCode, X[:], Z[:]
}

/* One pass MPIN Server */
func MPIN_SERVER_WRAP(date int, TimeValue int, SS, U, UT, V, ID, MESSAGE []byte) (int, []byte, []byte, []byte, []byte, []byte) {
	var HID [G1S]byte
	var HTID [G1S]byte
	var Y [EGS]byte
	var E [GTS]byte
	var F [GTS]byte

	errorCode := MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], V[:], E[:], F[:], ID[:], MESSAGE[:], TimeValue)

	return errorCode, HID[:], HTID[:], Y[:], E[:], F[:]
}

/* calculate common key on server side */
/* Z=r.A - no time permits involved */
func MPIN_SERVER_KEY_WRAP(Z, SS, W, U, UT []byte) (int, []byte) {
	var SK [EAS]byte
	errorCode := MPIN_SERVER_KEY(Z[:], SS[:], W[:], U[:], UT[:], SK[:])
	return errorCode, SK[:]
}

/* calculate common key on client side */
/* wCID = w.(A+AT) */
func MPIN_CLIENT_KEY_WRAP(PIN int, GT1, GT2, R, X, T []byte) (int, []byte) {
	var CK [EAS]byte
	errorCode := MPIN_CLIENT_KEY(GT1[:], GT2[:], PIN, R[:], X[:], T[:], CK[:])
	return errorCode, CK[:]
}

/* Extract big type PIN.hash(ID) from CS to form TOKEN */
func MPIN_EXTRACT_BIG_PIN_WRAP(ID, PIN, CS []byte) (int, []byte) {
	TOKEN := make([]byte, G1S)
	pin := fromBytes(PIN)
	P := ECP_fromBytes(CS)
	if P.is_infinity() {
		return MPIN_INVALID_POINT, TOKEN[:]
	}
	h := Hashit(0, ID)
	R := mapit(h)

	R = R.mul(pin)
	P.sub(R)

	P.toBytes(TOKEN)

	return 0, TOKEN[:]
}

/* Add big type PIN.hash(ID) to TOKEN for identity ID to form CS */
func MPIN_ADD_BIG_PIN_WRAP(ID, PIN, TOKEN []byte) (int, []byte) {
	CS := make([]byte, G1S)
	pin := fromBytes(PIN)
	P := ECP_fromBytes(TOKEN)
	if P.is_infinity() {
		return MPIN_INVALID_POINT, CS[:]
	}
	h := Hashit(0, ID)
	R := mapit(h)

	R = R.mul(pin)
	P.add(R)

	P.toBytes(CS)

	return 0, CS[:]
}

/* dst = a ^ b ^ c */
func XORBytes(a, b, c []byte) ([]byte, int) {
	n := len(a)
	dst := make([]byte, n)
	if (len(b) != n) || (len(c) != n) {
		return dst[:], 1
	}
	for i := 0; i < n; i++ {
		dst[i] = a[i] ^ b[i] ^ c[i]
	}
	return dst[:], 0
}
