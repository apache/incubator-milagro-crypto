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

/*
#cgo CFLAGS:  -std=c99 -O3 -I/usr/local/include/amcl
#cgo LDFLAGS: -L/usr/local/lib -lmpin  -lamcl -lm
#include <stdio.h>
#include <stdlib.h>
#include "amcl.h"
#include "mpin.h"
#include "utils.h"
*/
import "C"
import (
	"encoding/hex"
	"fmt"
	"unsafe"

	amcl "github.com/miracl/amcl-go"
)

const EAS int = int(C.PAS)
const EGS int = int(C.PGS)
const EFS int = int(C.PFS)
const HASH_BYTES int = int(C.HASH_BYTES)
const IVS int = 12
const G1S = 2*EFS + 1
const G2S = 4 * EFS
const GTS = 12 * EFS

var RNG C.csprng

func OCT_free(valOctet *C.octet) {
	C.free(unsafe.Pointer(valOctet.val))
}

func GetOctetZero(lenStr int) C.octet {
	valBytes := make([]byte, lenStr)
	val := string(valBytes)
	valCS := C.CString(val)
	lenCS := C.int(lenStr)
	octetVal := C.octet{lenCS, lenCS, valCS}
	return octetVal
}

func GetOctet(valStr string) C.octet {
	valCS := C.CString(valStr)
	lenCS := C.int(len(valStr))
	octetVal := C.octet{lenCS, lenCS, valCS}
	return octetVal
}

func GetOctetHex(valHex string) C.octet {
	valBytes, err := hex.DecodeString(valHex)
	if err != nil {
		octetVal := GetOctetZero(0)
		return octetVal
	}
	valStr := string(valBytes)
	octetVal := GetOctet(valStr)
	return octetVal
}

func OCT_len(valOctet *C.octet) int {
	return int(valOctet.len)
}

// Convert an octet to a string
func OCT_toStr(valOct *C.octet) string {
	dstLen := OCT_len(valOct)
	dstBytes := make([]byte, dstLen)
	dstStr := string(dstBytes)
	dst := C.CString(dstStr)
	C.OCT_toStr(valOct, dst)
	dstStr = C.GoStringN(dst, valOct.len)
	C.free(unsafe.Pointer(dst))
	return dstStr
}

// Convert an octet to bytes
func OCT_toBytes(valOct *C.octet) []byte {
	dstLen := OCT_len(valOct)
	dstBytes := make([]byte, dstLen)
	dstStr := string(dstBytes)
	dst := C.CString(dstStr)
	C.OCT_toStr(valOct, dst)
	dstStr = C.GoStringN(dst, valOct.len)
	C.free(unsafe.Pointer(dst))
	dstBytes = []byte(dstStr)
	return dstBytes
}

// Convert an octet to a hex string
func OCT_toHex(valOctet *C.octet) string {
	dstLen := OCT_len(valOctet)
	dstBytes := make([]byte, hex.EncodedLen(dstLen))
	dstStr := string(dstBytes)
	dst := C.CString(dstStr)
	C.OCT_toHex(valOctet, dst)
	dstStr = C.GoString(dst)
	C.free(unsafe.Pointer(dst))
	return dstStr
}

/* return time in slots since epoch */
func MPIN_today() int {
	date := C.today()
	return int(date)
}

/* return time since epoch */
func MPIN_GET_TIME() int {
	timeValue := C.MPIN_GET_TIME()
	return int(timeValue)
}

func CREATE_CSPRNG(RNG *C.csprng, SEED []byte) {
	// Form Octet
	SEEDStr := string(SEED)
	SEEDOct := GetOctet(SEEDStr)
	defer OCT_free(&SEEDOct)
	C.CREATE_CSPRNG(RNG, &SEEDOct)
}

func MPIN_HASH_ID(ID []byte) (HASHID []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	HASHIDOct := GetOctetZero(HASH_BYTES)
	defer OCT_free(&HASHIDOct)

	// Hash MPIN_ID
	C.MPIN_HASH_ID(&IDOct, &HASHIDOct)

	// Convert octet to bytes
	HASHID = OCT_toBytes(&HASHIDOct)

	return
}

/* create random secret S. Use GO RNG */
func MPIN_RANDOM_GENERATE_WRAP(RNG *amcl.RAND) (int, []byte) {
	var S [EGS]byte
	errorCode := amcl.MPIN_RANDOM_GENERATE(RNG, S[:])
	return errorCode, S[:]
}

/* create random secret S. Use C RNG */
func MPIN_RANDOM_GENERATE_C(RNG *C.csprng) (errorCode int, S []byte) {
	// Form Octet
	SOct := GetOctetZero(EGS)
	defer OCT_free(&SOct)

	rtn := C.MPIN_RANDOM_GENERATE(RNG, &SOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	S = OCT_toBytes(&SOct)

	return
}

/* Extract Server Secret SS=S*Q where Q is fixed generator in G2 and S is master secret */
func MPIN_GET_SERVER_SECRET_WRAP(S []byte) (errorCode int, SS []byte) {
	// Form Octets
	SStr := string(S)
	SOct := GetOctet(SStr)
	defer OCT_free(&SOct)
	SSOct := GetOctetZero(G2S)
	defer OCT_free(&SSOct)

	rtn := C.MPIN_GET_SERVER_SECRET(&SOct, &SSOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	SS = OCT_toBytes(&SSOct)

	return
}

/* R=R1+R2 in group G1 */
func MPIN_RECOMBINE_G1_WRAP(R1 []byte, R2 []byte) (errorCode int, R []byte) {
	// Form Octets
	R1Str := string(R1)
	R1Oct := GetOctet(R1Str)
	defer OCT_free(&R1Oct)
	R2Str := string(R2)
	R2Oct := GetOctet(R2Str)
	defer OCT_free(&R2Oct)
	ROct := GetOctetZero(G1S)
	defer OCT_free(&ROct)

	rtn := C.MPIN_RECOMBINE_G1(&R1Oct, &R2Oct, &ROct)
	errorCode = int(rtn)

	// Convert octet to bytes
	R = OCT_toBytes(&ROct)

	return
}

/* W=W1+W2 in group G2 */
func MPIN_RECOMBINE_G2_WRAP(W1 []byte, W2 []byte) (errorCode int, W []byte) {
	// Form Octets
	W1Str := string(W1)
	W1Oct := GetOctet(W1Str)
	defer OCT_free(&W1Oct)
	W2Str := string(W2)
	W2Oct := GetOctet(W2Str)
	defer OCT_free(&W2Oct)
	WOct := GetOctetZero(G2S)
	defer OCT_free(&WOct)

	rtn := C.MPIN_RECOMBINE_G2(&W1Oct, &W2Oct, &WOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	W = OCT_toBytes(&WOct)

	return
}

/* Client secret CS=S*H(ID) where ID is client ID and S is master secret */
/* CID is hashed externally */
func MPIN_GET_CLIENT_SECRET_WRAP(S []byte, ID []byte) (errorCode int, CS []byte) {
	// Form Octets
	SStr := string(S)
	SOct := GetOctet(SStr)
	defer OCT_free(&SOct)
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	CSOct := GetOctetZero(G1S)
	defer OCT_free(&CSOct)

	rtn := C.MPIN_GET_CLIENT_SECRET(&SOct, &IDOct, &CSOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	CS = OCT_toBytes(&CSOct)

	return
}

/* Time Permit TP=S*(date|H(ID)) where S is master secret */
func MPIN_GET_CLIENT_PERMIT_WRAP(date int, S, ID []byte) (errorCode int, TP []byte) {
	// Form Octets
	SStr := string(S)
	SOct := GetOctet(SStr)
	defer OCT_free(&SOct)
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	TPOct := GetOctetZero(G1S)
	defer OCT_free(&TPOct)

	rtn := C.MPIN_GET_CLIENT_PERMIT(C.int(date), &SOct, &IDOct, &TPOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	TP = OCT_toBytes(&TPOct)

	return
}

/* Extract PIN from CS for identity CID to form TOKEN */
func MPIN_EXTRACT_PIN_WRAP(ID []byte, PIN int, CS []byte) (errorCode int, TOKEN []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	CSStr := string(CS)
	CSOct := GetOctet(CSStr)
	defer OCT_free(&CSOct)

	rtn := C.MPIN_EXTRACT_PIN(&IDOct, C.int(PIN), &CSOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	TOKEN = OCT_toBytes(&CSOct)

	return
}

/* One pass MPIN Client. Using GO RNG */
func MPIN_CLIENT_WRAP(date, TimeValue, PIN int, RNG *amcl.RAND, ID, X, TOKEN, TP, MESSAGE []byte) (errorCode int, XOut, Y, SEC, U, UT []byte) {
	amcl.MPIN_RANDOM_GENERATE(RNG, X[:])
	errorCode, XOut, Y, SEC, U, UT = MPIN_CLIENT_C(date, ID[:], nil, X[:], PIN, TOKEN[:], TP[:], MESSAGE[:], TimeValue)
	return
}

/* One pass MPIN Client. Using C RNG */
func MPIN_CLIENT_C(date int, ID []byte, RNG *C.csprng, X []byte, PIN int, TOKEN []byte, TP []byte, MESSAGE []byte, TimeValue int) (errorCode int, XOut, Y, SEC, U, UT []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	XStr := string(X)
	XOct := GetOctet(XStr)
	defer OCT_free(&XOct)
	TOKENStr := string(TOKEN)
	TOKENOct := GetOctet(TOKENStr)
	defer OCT_free(&TOKENOct)
	TPStr := string(TP)
	TPOct := GetOctet(TPStr)
	defer OCT_free(&TPOct)
	MESSAGEStr := string(MESSAGE)
	MESSAGEOct := GetOctet(MESSAGEStr)
	defer OCT_free(&MESSAGEOct)

	SECOct := GetOctetZero(G1S)
	defer OCT_free(&SECOct)
	UOct := GetOctetZero(G1S)
	defer OCT_free(&UOct)
	UTOct := GetOctetZero(G1S)
	defer OCT_free(&UTOct)
	YOct := GetOctetZero(EGS)
	defer OCT_free(&YOct)

	rtn := C.MPIN_CLIENT(C.int(date), &IDOct, RNG, &XOct, C.int(PIN), &TOKENOct, &SECOct, &UOct, &UTOct, &TPOct, &MESSAGEOct, C.int(TimeValue), &YOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	XOut = OCT_toBytes(&XOct)
	SEC = OCT_toBytes(&SECOct)
	U = OCT_toBytes(&UOct)
	UT = OCT_toBytes(&UTOct)
	Y = OCT_toBytes(&YOct)

	return
}

// Precompute values for use by the client side of M-Pin Full
func MPIN_PRECOMPUTE_WRAP(TOKEN []byte, ID []byte) (errorCode int, GT1 []byte, GT2 []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	TOKENStr := string(TOKEN)
	TOKENOct := GetOctet(TOKENStr)
	defer OCT_free(&TOKENOct)

	GT1Oct := GetOctetZero(GTS)
	defer OCT_free(&GT1Oct)
	GT2Oct := GetOctetZero(GTS)
	defer OCT_free(&GT2Oct)

	rtn := C.MPIN_PRECOMPUTE(&TOKENOct, &IDOct, &GT1Oct, &GT2Oct)
	errorCode = int(rtn)

	// Convert octet to bytes
	GT1 = OCT_toBytes(&GT1Oct)
	GT2 = OCT_toBytes(&GT2Oct)

	return
}

/*
 W=x*H(G);
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 if typ=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
 Use GO RNG
*/
func MPIN_GET_G1_MULTIPLE_WRAP(RNG *amcl.RAND, typ int, X, G []byte) (errorCode int, XOut, W []byte) {
	amcl.MPIN_RANDOM_GENERATE(RNG, X[:])
	errorCode, XOut, W = MPIN_GET_G1_MULTIPLE_C(nil, typ, X[:], G[:])
	return
}

/*
 W=x*H(G);
 if RNG == NULL then X is passed in
 if RNG != NULL the X is passed out
 if typ=0 W=x*G where G is point on the curve, else W=x*M(G), where M(G) is mapping of octet G to point on the curve
 Use C RNG
*/
func MPIN_GET_G1_MULTIPLE_C(RNG *C.csprng, typ int, X []byte, G []byte) (errorCode int, XOut, W []byte) {
	XStr := string(X)
	XOct := GetOctet(XStr)
	defer OCT_free(&XOct)
	GStr := string(G)
	GOct := GetOctet(GStr)
	defer OCT_free(&GOct)

	WOct := GetOctetZero(G1S)
	defer OCT_free(&WOct)

	rtn := C.MPIN_GET_G1_MULTIPLE(RNG, C.int(typ), &XOct, &GOct, &WOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	XOut = OCT_toBytes(&XOct)
	W = OCT_toBytes(&WOct)

	return
}

/* One pass MPIN Server */
func MPIN_SERVER_WRAP(date, TimeValue int, SS, U, UT, V, ID, MESSAGE []byte) (errorCode int, HID, HTID, Y, E, F []byte) {
	SSStr := string(SS)
	SSOct := GetOctet(SSStr)
	defer OCT_free(&SSOct)
	UStr := string(U)
	UOct := GetOctet(UStr)
	defer OCT_free(&UOct)
	UTStr := string(UT)
	UTOct := GetOctet(UTStr)
	defer OCT_free(&UTOct)
	VStr := string(V)
	VOct := GetOctet(VStr)
	defer OCT_free(&VOct)
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)
	MESSAGEStr := string(MESSAGE)
	MESSAGEOct := GetOctet(MESSAGEStr)
	defer OCT_free(&MESSAGEOct)

	HIDOct := GetOctetZero(G1S)
	defer OCT_free(&HIDOct)
	HTIDOct := GetOctetZero(G1S)
	defer OCT_free(&HTIDOct)
	YOct := GetOctetZero(EGS)
	defer OCT_free(&YOct)
	EOct := GetOctetZero(GTS)
	defer OCT_free(&EOct)
	FOct := GetOctetZero(GTS)
	defer OCT_free(&FOct)

	rtn := C.MPIN_SERVER(C.int(date), &HIDOct, &HTIDOct, &YOct, &SSOct, &UOct, &UTOct, &VOct, &EOct, &FOct, &IDOct, &MESSAGEOct, C.int(TimeValue))
	errorCode = int(rtn)

	// Convert octet to bytes
	HID = OCT_toBytes(&HIDOct)
	HTID = OCT_toBytes(&HTIDOct)
	Y = OCT_toBytes(&YOct)
	E = OCT_toBytes(&EOct)
	F = OCT_toBytes(&FOct)

	return
}

/* Pollards kangaroos used to return PIN error */
func MPIN_KANGAROO(E []byte, F []byte) (PINError int) {
	EStr := string(E)
	EOct := GetOctet(EStr)
	defer OCT_free(&EOct)
	FStr := string(F)
	FOct := GetOctet(FStr)
	defer OCT_free(&FOct)

	rtn := C.MPIN_KANGAROO(&EOct, &FOct)
	PINError = int(rtn)
	return
}

/* calculate common key on server side */
/* Z=r.A - no time permits involved */
func MPIN_SERVER_KEY_WRAP(Z, SS, W, U, UT []byte) (errorCode int, SK []byte) {
	ZStr := string(Z)
	ZOct := GetOctet(ZStr)
	defer OCT_free(&ZOct)
	SSStr := string(SS)
	SSOct := GetOctet(SSStr)
	defer OCT_free(&SSOct)
	WStr := string(W)
	WOct := GetOctet(WStr)
	defer OCT_free(&WOct)
	UStr := string(U)
	UOct := GetOctet(UStr)
	defer OCT_free(&UOct)
	UTStr := string(UT)
	UTOct := GetOctet(UTStr)
	defer OCT_free(&UTOct)

	SKOct := GetOctetZero(EAS)
	defer OCT_free(&SKOct)

	rtn := C.MPIN_SERVER_KEY(&ZOct, &SSOct, &WOct, &UOct, &UTOct, &SKOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	SK = OCT_toBytes(&SKOct)

	return
}

/* calculate common key on client side */
/* wCID = w.(A+AT) */
func MPIN_CLIENT_KEY_WRAP(PIN int, GT1, GT2, R, X, T []byte) (errorCode int, CK []byte) {
	GT1Str := string(GT1)
	GT1Oct := GetOctet(GT1Str)
	defer OCT_free(&GT1Oct)
	GT2Str := string(GT2)
	GT2Oct := GetOctet(GT2Str)
	defer OCT_free(&GT2Oct)
	RStr := string(R)
	ROct := GetOctet(RStr)
	defer OCT_free(&ROct)
	XStr := string(X)
	XOct := GetOctet(XStr)
	defer OCT_free(&XOct)
	TStr := string(T)
	TOct := GetOctet(TStr)
	defer OCT_free(&TOct)

	CKOct := GetOctetZero(EAS)
	defer OCT_free(&CKOct)

	rtn := C.MPIN_CLIENT_KEY(&GT1Oct, &GT2Oct, C.int(PIN), &ROct, &XOct, &TOct, &CKOct)
	errorCode = int(rtn)

	// Convert octet to bytes
	CK = OCT_toBytes(&CKOct)

	return
}

// Generate a random byte array
func GENERATE_RANDOM_C(RNG *C.csprng, randomLen int) (random []byte) {
	randomOct := GetOctetZero(randomLen)
	defer OCT_free(&randomOct)

	C.generateRandom(RNG, &randomOct)

	// Convert octet to bytes
	random = OCT_toBytes(&randomOct)

	return
}

// Generate random six digit value
func GENERATE_OTP_C(RNG *C.csprng) int {
	rtn := C.generateOTP(RNG)
	return int(rtn)
}

/* AES-GCM Encryption:
   K is key, H is header, IV is initialization vector and P is plaintext.
   Returns cipthertext and tag (MAC) */
func AES_GCM_ENCRYPT(K, IV, H, P []byte) ([]byte, []byte) {
	KStr := string(K)
	KOct := GetOctet(KStr)
	defer OCT_free(&KOct)
	IVStr := string(IV)
	IVOct := GetOctet(IVStr)
	defer OCT_free(&IVOct)
	HStr := string(H)
	HOct := GetOctet(HStr)
	defer OCT_free(&HOct)
	PStr := string(P)
	POct := GetOctet(PStr)
	defer OCT_free(&POct)

	TOct := GetOctetZero(16)
	defer OCT_free(&TOct)
	lenC := len(PStr)
	COct := GetOctetZero(lenC)
	defer OCT_free(&COct)

	C.AES_GCM_ENCRYPT(&KOct, &IVOct, &HOct, &POct, &COct, &TOct)

	// Convert octet to bytes
	C := OCT_toBytes(&COct)
	T := OCT_toBytes(&TOct)

	return C, T[:]
}

/* AES-GCM Deryption:
   K is key, H is header, IV is initialization vector and P is plaintext.
   Returns cipthertext and tag (MAC) */
func AES_GCM_DECRYPT(K, IV, H, C []byte) ([]byte, []byte) {
	KStr := string(K)
	KOct := GetOctet(KStr)
	defer OCT_free(&KOct)
	IVStr := string(IV)
	IVOct := GetOctet(IVStr)
	defer OCT_free(&IVOct)
	HStr := string(H)
	HOct := GetOctet(HStr)
	defer OCT_free(&HOct)
	CStr := string(C)
	COct := GetOctet(CStr)
	defer OCT_free(&COct)

	TOct := GetOctetZero(16)
	defer OCT_free(&TOct)
	lenP := len(CStr)
	POct := GetOctetZero(lenP)
	defer OCT_free(&POct)

	C.AES_GCM_DECRYPT(&KOct, &IVOct, &HOct, &COct, &POct, &TOct)

	// Convert octet to bytes
	P := OCT_toBytes(&POct)
	T := OCT_toBytes(&TOct)

	return P, T[:]
}

/* Password based Key Derivation Function */
/* Input password p, salt s, and repeat count */
/* Output key of length olen */
func PBKDF2(Pass, Salt []byte, rep, olen int) (Key []byte) {
	PassStr := string(Pass)
	PassOct := GetOctet(PassStr)
	defer OCT_free(&PassOct)
	SaltStr := string(Salt)
	SaltOct := GetOctet(SaltStr)
	defer OCT_free(&SaltOct)

	KeyOct := GetOctetZero(olen)
	defer OCT_free(&KeyOct)

	C.PBKDF2(&PassOct, &SaltOct, C.int(rep), C.int(olen), &KeyOct)

	// Convert octet to bytes
	Key = OCT_toBytes(&KeyOct)

	return
}

func MPIN_printBinary(array []byte) {
	for i := 0; i < len(array); i++ {
		fmt.Printf("%02x", array[i])
	}
	fmt.Printf("\n")
}

/* Outputs H(CID) and H(T|H(CID)) for time permits. If no time permits set HID=HTID */
func MPIN_SERVER_1_WRAP(date int, ID []byte) (HID, HTID []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)

	HIDOct := GetOctetZero(G1S)
	defer OCT_free(&HIDOct)
	HTIDOct := GetOctetZero(G1S)
	defer OCT_free(&HTIDOct)

	C.MPIN_SERVER_1(C.int(date), &IDOct, &HIDOct, &HTIDOct)

	// Convert octet to bytes
	HID = OCT_toBytes(&HIDOct)
	HTID = OCT_toBytes(&HTIDOct)

	return
}

/* Implement step 2 of MPin protocol on server side */
func MPIN_SERVER_2_WRAP(date int, HID []byte, HTID []byte, Y []byte, SS []byte, U []byte, UT []byte, V []byte) (errorCode int, E, F []byte) {
	// Form Octets
	HIDStr := string(HID)
	HIDOct := GetOctet(HIDStr)
	defer OCT_free(&HIDOct)
	HTIDStr := string(HTID)
	HTIDOct := GetOctet(HTIDStr)
	defer OCT_free(&HTIDOct)
	YStr := string(Y)
	YOct := GetOctet(YStr)
	defer OCT_free(&YOct)
	SSStr := string(SS)
	SSOct := GetOctet(SSStr)
	defer OCT_free(&SSOct)
	UStr := string(U)
	UOct := GetOctet(UStr)
	defer OCT_free(&UOct)
	UTStr := string(UT)
	UTOct := GetOctet(UTStr)
	defer OCT_free(&UTOct)
	VStr := string(V)
	VOct := GetOctet(VStr)
	defer OCT_free(&VOct)

	EOct := GetOctetZero(GTS)
	defer OCT_free(&EOct)
	FOct := GetOctetZero(GTS)
	defer OCT_free(&FOct)
	rtn := C.MPIN_SERVER_2(C.int(date), &HIDOct, &HTIDOct, &YOct, &SSOct, &UOct, &UTOct, &VOct, &EOct, &FOct)

	errorCode = int(rtn)
	E = OCT_toBytes(&EOct)
	F = OCT_toBytes(&FOct)

	return
}

/* Implement step 1 on client side of MPin protocol. Use GO code to generate random X */
func MPIN_CLIENT_1_WRAP(date int, ID []byte, RNG *amcl.RAND, X []byte, PIN int, TOKEN []byte, TP []byte) (errorCode int, XOut, SEC, U, UT []byte) {
	amcl.MPIN_RANDOM_GENERATE(RNG, X[:])
	errorCode, XOut, SEC, U, UT = MPIN_CLIENT_1_C(date, ID[:], nil, X[:], PIN, TOKEN[:], TP[:])
	return
}

/* Implement step 1 on client side of MPin protocol
   When rng=nil the X value is externally generated
*/
func MPIN_CLIENT_1_C(date int, ID []byte, rng *C.csprng, X []byte, PIN int, TOKEN []byte, TP []byte) (errorCode int, XOut, SEC, U, UT []byte) {
	// Form Octets
	IDStr := string(ID)
	IDOct := GetOctet(IDStr)
	defer OCT_free(&IDOct)

	XStr := string(X)
	XOct := GetOctet(XStr)
	defer OCT_free(&XOct)

	TOKENStr := string(TOKEN)
	TOKENOct := GetOctet(TOKENStr)
	defer OCT_free(&TOKENOct)

	TPStr := string(TP)
	TPOct := GetOctet(TPStr)
	defer OCT_free(&TPOct)

	SECOct := GetOctetZero(G1S)
	defer OCT_free(&SECOct)
	UOct := GetOctetZero(G1S)
	defer OCT_free(&UOct)
	UTOct := GetOctetZero(G1S)
	defer OCT_free(&UTOct)

	rtn := C.MPIN_CLIENT_1(C.int(date), &IDOct, rng, &XOct, C.int(PIN), &TOKENOct, &SECOct, &UOct, &UTOct, &TPOct)

	errorCode = int(rtn)
	// Convert octet to bytes
	XOut = OCT_toBytes(&XOct)
	SEC = OCT_toBytes(&SECOct)
	U = OCT_toBytes(&UOct)
	UT = OCT_toBytes(&UTOct)

	return
}

/* Implement step 2 on client side of MPin protocol */
func MPIN_CLIENT_2_WRAP(X []byte, Y []byte, SEC []byte) (errorCode int, V []byte) {
	// Form Octets
	XStr := string(X)
	XOct := GetOctet(XStr)
	defer OCT_free(&XOct)
	YStr := string(Y)
	YOct := GetOctet(YStr)
	defer OCT_free(&YOct)
	SECStr := string(SEC)
	SECOct := GetOctet(SECStr)
	defer OCT_free(&SECOct)

	rtn := C.MPIN_CLIENT_2(&XOct, &YOct, &SECOct)

	errorCode = int(rtn)
	// Convert octet to bytes
	V = OCT_toBytes(&SECOct)

	return
}
