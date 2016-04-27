#!/usr/bin/env python

"""
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
"""


"""
wcc

This module use cffi to access the c functions in the WCC library.

There is also an example usage program in this file.

"""
import cffi
import platform

# WCC Group Size
PGS = 32
# WCC Field Size
PFS = 32
G1 = 2*PFS + 1
G2 = 4*PFS
# Length of hash
HASH_BYTES = 32
# AES-GCM IV length
IVL = 12
# AES Key length
PAS = 16

ffi = cffi.FFI()
ffi.cdef("""
typedef struct {
unsigned int ira[21];  /* random number...   */
int rndptr;   /* ...array & pointer */
unsigned int borrow;
int pool_ptr;
char pool[32];    /* random pool */
} csprng;

typedef struct
{
  int len;
  int max;
  char *val;
} octet;

extern int WCC_RANDOM_GENERATE(csprng *RNG,octet* S);
extern void  WCC_Hq(octet *A,octet *B,octet *C,octet *D,octet *h);
extern int WCC_GET_G2_MULTIPLE(int hashDone,octet *S,octet *ID,octet *VG2);
extern int WCC_GET_G1_MULTIPLE(int hashDone,octet *S,octet *ID,octet *VG1);
extern int WCC_GET_G1_TPMULT(int date, octet *S,octet *ID,octet *VG1);
extern int WCC_GET_G2_TPMULT(int date, octet *S,octet *ID,octet *VG2);
extern int WCC_GET_G1_PERMIT(int date,octet *S,octet *HID,octet *G1TP);
extern int WCC_GET_G2_PERMIT(int date,octet *S,octet *HID,octet *G2TP);
extern int WCC_SENDER_KEY(int date, octet *xOct, octet *piaOct, octet *pibOct, octet *PbG2Oct, octet *PgG1Oct, octet *AKeyG1Oct, octet *ATPG1Oct, octet *IdBOct, octet *AESKeyOct);
extern int WCC_RECEIVER_KEY(int date, octet *yOct, octet *wOct,  octet *piaOct, octet *pibOct,  octet *PaG1Oct, octet *PgG1Oct, octet *BKeyG2Oct,octet *BTPG2Oct,  octet *IdAOct, octet *AESKeyOct);
extern void WCC_AES_GCM_ENCRYPT(octet *K,octet *IV,octet *H,octet *P,octet *C,octet *T);
extern void WCC_AES_GCM_DECRYPT(octet *K,octet *IV,octet *H,octet *C,octet *P,octet *T);
extern void WCC_HASH_ID(octet *,octet *);
extern int WCC_RECOMBINE_G1(octet *,octet *,octet *);
extern int WCC_RECOMBINE_G2(octet *,octet *,octet *);
extern unsigned int WCC_today(void);
extern void WCC_CREATE_CSPRNG(csprng *,octet *);
extern void WCC_KILL_CSPRNG(csprng *RNG);
extern void version(char* info);

""")

if (platform.system() == 'Windows'):
    libwcc = ffi.dlopen("libwcc.dll")
elif (platform.system() == 'Darwin'):
    libwcc = ffi.dlopen("libwcc.dylib")
else:
    libwcc = ffi.dlopen("libwcc.so")


def toHex(octetValue):
    """Converts an octet type into a string

    Add all the values in an octet into an array. This arrays is then
    converted to a string and hex encoded.

    Args::

        octetValue. An octet type

    Returns::

        String

    Raises:
        Exception
    """
    i = 0
    val = []
    while i < octetValue[0].len:
        val.append(octetValue[0].val[i])
        i = i+1
    return ''.join(val).encode("hex")


if __name__ == "__main__":
    # Print hex values
    DEBUG = False

    build_version = ffi.new("char []", 256)
    libwcc.version(build_version)
    print ffi.string(build_version)

    # Seed
    seedHex = "0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20"
    seed = seedHex.decode("hex")

    # Master Secret Shares
    MS1 = ffi.new("octet*")
    MS1val = ffi.new("char []", PGS)
    MS1[0].val = MS1val
    MS1[0].max = PGS
    MS1[0].len = PGS

    MS2 = ffi.new("octet*")
    MS2val = ffi.new("char []", PGS)
    MS2[0].val = MS2val
    MS2[0].max = PGS
    MS2[0].len = PGS

    # Alice Identity
    alice_id = raw_input("Please enter Alice's identity:")
    IdA = ffi.new("octet*")
    IdAval = ffi.new("char [%s]" % len(alice_id), alice_id)
    IdA[0].val = IdAval
    IdA[0].max = len(alice_id)
    IdA[0].len = len(alice_id)

    # Hash value of IdA
    AHV = ffi.new("octet*")
    AHVval = ffi.new("char []",  HASH_BYTES)
    AHV[0].val = AHVval
    AHV[0].max = HASH_BYTES
    AHV[0].len = HASH_BYTES

    # Bob Identity
    bob_id = raw_input("Please enter Bob's identity:")
    IdB = ffi.new("octet*")
    IdBval = ffi.new("char [%s]" % len(bob_id), bob_id)
    IdB[0].val = IdBval
    IdB[0].max = len(bob_id)
    IdB[0].len = len(bob_id)

    # Hash value of IdB
    BHV = ffi.new("octet*")
    BHVval = ffi.new("char []",  HASH_BYTES)
    BHV[0].val = BHVval
    BHV[0].max = HASH_BYTES
    BHV[0].len = HASH_BYTES

    # Sender keys
    A1KeyG1 = ffi.new("octet*")
    A1KeyG1val = ffi.new("char []", G1)
    A1KeyG1[0].val = A1KeyG1val
    A1KeyG1[0].max = G1
    A1KeyG1[0].len = G1

    A2KeyG1 = ffi.new("octet*")
    A2KeyG1val = ffi.new("char []", G1)
    A2KeyG1[0].val = A2KeyG1val
    A2KeyG1[0].max = G1
    A2KeyG1[0].len = G1

    AKeyG1 = ffi.new("octet*")
    AKeyG1val = ffi.new("char []", G1)
    AKeyG1[0].val = AKeyG1val
    AKeyG1[0].max = G1
    AKeyG1[0].len = G1

    # Receiver keys
    B1KeyG2 = ffi.new("octet*")
    B1KeyG2val = ffi.new("char []", G2)
    B1KeyG2[0].val = B1KeyG2val
    B1KeyG2[0].max = G2
    B1KeyG2[0].len = G2

    B2KeyG2 = ffi.new("octet*")
    B2KeyG2val = ffi.new("char []", G2)
    B2KeyG2[0].val = B2KeyG2val
    B2KeyG2[0].max = G2
    B2KeyG2[0].len = G2

    BKeyG2 = ffi.new("octet*")
    BKeyG2val = ffi.new("char []", G2)
    BKeyG2[0].val = BKeyG2val
    BKeyG2[0].max = G2
    BKeyG2[0].len = G2

    # Sender time permits
    A1TPG1 = ffi.new("octet*")
    A1TPG1val = ffi.new("char []", G1)
    A1TPG1[0].val = A1TPG1val
    A1TPG1[0].max = G1
    A1TPG1[0].len = G1

    A2TPG1 = ffi.new("octet*")
    A2TPG1val = ffi.new("char []", G1)
    A2TPG1[0].val = A2TPG1val
    A2TPG1[0].max = G1
    A2TPG1[0].len = G1

    ATPG1 = ffi.new("octet*")
    ATPG1val = ffi.new("char []", G1)
    ATPG1[0].val = ATPG1val
    ATPG1[0].max = G1
    ATPG1[0].len = G1

    # Receiver time permits
    B1TPG2 = ffi.new("octet*")
    B1TPG2val = ffi.new("char []", G2)
    B1TPG2[0].val = B1TPG2val
    B1TPG2[0].max = G2
    B1TPG2[0].len = G2

    B2TPG2 = ffi.new("octet*")
    B2TPG2val = ffi.new("char []", G2)
    B2TPG2[0].val = B2TPG2val
    B2TPG2[0].max = G2
    B2TPG2[0].len = G2

    BTPG2 = ffi.new("octet*")
    BTPG2val = ffi.new("char []", G2)
    BTPG2[0].val = BTPG2val
    BTPG2[0].max = G2
    BTPG2[0].len = G2

    # AES Keys
    KEY1 = ffi.new("octet*")
    KEY1val = ffi.new("char []", PAS)
    KEY1[0].val = KEY1val
    KEY1[0].max = PAS
    KEY1[0].len = PAS

    KEY2 = ffi.new("octet*")
    KEY2val = ffi.new("char []", PAS)
    KEY2[0].val = KEY2val
    KEY2[0].max = PAS
    KEY2[0].len = PAS

    X = ffi.new("octet*")
    Xval = ffi.new("char []", PGS)
    X[0].val = Xval
    X[0].max = PGS
    X[0].len = PGS

    Y = ffi.new("octet*")
    Yval = ffi.new("char []", PGS)
    Y[0].val = Yval
    Y[0].max = PGS
    Y[0].len = PGS

    W = ffi.new("octet*")
    Wval = ffi.new("char []", PGS)
    W[0].val = Wval
    W[0].max = PGS
    W[0].len = PGS

    PIA = ffi.new("octet*")
    PIAval = ffi.new("char []", PGS)
    PIA[0].val = PIAval
    PIA[0].max = PGS
    PIA[0].len = PGS

    PIB = ffi.new("octet*")
    PIBval = ffi.new("char []", PGS)
    PIB[0].val = PIBval
    PIB[0].max = PGS
    PIB[0].len = PGS

    PaG1 = ffi.new("octet*")
    PaG1val = ffi.new("char []", G1)
    PaG1[0].val = PaG1val
    PaG1[0].max = G1
    PaG1[0].len = G1

    PgG1 = ffi.new("octet*")
    PgG1val = ffi.new("char []", G1)
    PgG1[0].val = PgG1val
    PgG1[0].max = G1
    PgG1[0].len = G1

    PbG2 = ffi.new("octet*")
    PbG2val = ffi.new("char []", G2)
    PbG2[0].val = PbG2val
    PbG2[0].max = G2
    PbG2[0].len = G2

    # Assign a seed value
    RAW = ffi.new("octet*")
    RAWval = ffi.new("char [%s]" % len(seed), seed)
    RAW[0].val = RAWval
    RAW[0].len = len(seed)
    RAW[0].max = len(seed)
    if DEBUG:
        print "RAW: %s" % toHex(RAW)

    # random number generator
    RNG = ffi.new("csprng*")
    libwcc.WCC_CREATE_CSPRNG(RNG, RAW)

    # Today's date in epoch days
    date = libwcc.WCC_today()
    if DEBUG:
        print "Date %s" % date

    # Hash IdA
    libwcc.WCC_HASH_ID(IdA, AHV)
    if DEBUG:
        print "IdA: %s" % toHex(IdA)
        print "AHV: %s" % toHex(AHV)

    # Hash IdB
    libwcc.WCC_HASH_ID(IdB, BHV)
    if DEBUG:
        print "IdB: %s" % toHex(IdB)
        print "BHV: %s" % toHex(BHV)

    # Generate master secret for MIRACL and Customer
    rtn = libwcc.WCC_RANDOM_GENERATE(RNG, MS1)
    if rtn != 0:
        print "libwcc.WCC_RANDOM_GENERATE(RNG,MS1) Error %s", rtn
    rtn = libwcc.WCC_RANDOM_GENERATE(RNG, MS2)
    if rtn != 0:
        print "libwcc.WCC_RANDOM_GENERATE(RNG,MS2) Error %s" % rtn
    if DEBUG:
        print "MS1: %s" % toHex(MS1)
        print "MS2: %s" % toHex(MS2)

    # Generate Alice's sender key shares
    rtn = libwcc.WCC_GET_G1_MULTIPLE(1,MS1, AHV, A1KeyG1)
    if rtn != 0:
        print "libwcc.WCC_GET_G1_MULTIPLE(MS1,AHV,A1KeyG1) Error %s" % rtn
    rtn = libwcc.WCC_GET_G1_MULTIPLE(1,MS2, AHV, A2KeyG1)
    if rtn != 0:
        print "libwcc.WCC_GET_G1_MULTIPLE(MS2,AHV,A2KeyG1) Error %s" % rtn
    if DEBUG:
        print "A1KeyG1: %s" % toHex(A1KeyG1)
        print "A2KeyG1: %s" % toHex(A2KeyG1)

    # Combine Alices's sender key shares
    rtn = libwcc.WCC_RECOMBINE_G1(A1KeyG1, A2KeyG1, AKeyG1)
    if rtn != 0:
        print "libwcc.WCC_RECOMBINE_G1(A1KeyG1, A2KeyG1, AKeyG1) Error %s" % rtn
    print "AKeyG1: %s" % toHex(AKeyG1)

    # Generate Alice's sender time permit shares
    rtn = libwcc.WCC_GET_G1_PERMIT(date, MS1, AHV, A1TPG1)
    if rtn != 0:
        print "libwcc.WCC_GET_G1_PERMIT(date,MS1,AHV,A1TPG1) Error %s" % rtn
    rtn = libwcc.WCC_GET_G1_PERMIT(date, MS2, AHV, A2TPG1)
    if rtn != 0:
        print "libwcc.WCC_GET_G1_PERMIT(date,MS2,AHV,A2TPG1) Error %s" % rtn
    if DEBUG:
        print "A1TPG1: %s" % toHex(A1TPG1)
        print "A2TPG1: %s" % toHex(A2TPG1)

    # Combine Alice's sender Time Permit shares
    rtn = libwcc.WCC_RECOMBINE_G1(A1TPG1, A2TPG1, ATPG1)
    if rtn != 0:
        print "libwcc.WCC_RECOMBINE_G1(A1TPG1, A2TPG1, ATPG1) Error %s" % rtn
    print "ATPG1: %s" % toHex(ATPG1)

    # Generate Bob's receiver secret key shares
    rtn = libwcc.WCC_GET_G2_MULTIPLE(1,MS1, BHV, B1KeyG2)
    if rtn != 0:
        print "libwcc.WCC_GET_G2_MULTIPLE(MS1,BHV,B1KeyG2) Error %s" % rtn
    rtn = libwcc.WCC_GET_G2_MULTIPLE(1,MS2, BHV, B2KeyG2)
    if rtn != 0:
        print "libwcc.WCC_GET_G2_MULTIPLE(MS2,BHV,B2KeyG2) Error %s" % rtn
    if DEBUG:
        print "B1KeyG2: %s" % toHex(B1KeyG2)
        print "B2KeyG2: %s" % toHex(B2KeyG2)

    # Combine Bobs's receiver secret key shares
    rtn = libwcc.WCC_RECOMBINE_G2(B1KeyG2, B2KeyG2, BKeyG2)
    if rtn != 0:
        print "libwcc.WCC_RECOMBINE_G2(B1KeyG2, B2KeyG2, BKeyG2) Error %s" % rtn
    print "BKeyG2: %s" % toHex(BKeyG2)

    # Generate Bob's receiver time permit shares
    rtn = libwcc.WCC_GET_G2_PERMIT(date, MS1, BHV, B1TPG2)
    if rtn != 0:
        print "libwcc.WCC_GET_G2_PERMIT(date,MS1,BHV,B1TPG2) Error %s" % rtn
    rtn = libwcc.WCC_GET_G2_PERMIT(date, MS2, BHV, B2TPG2)
    if rtn != 0:
        print "libwcc.WCC_GET_G2_PERMIT(date,MS2,BHV,B2TPG2) Error %s" % rtn
    if DEBUG:
        print "B1TPG2: %s" % toHex(B1TPG2)
        print "B2TPG2: %s" % toHex(B2TPG2)

    # Combine Bob's receiver time permit shares
    rtn = libwcc.WCC_RECOMBINE_G2(B1TPG2, B2TPG2, BTPG2)
    if rtn != 0:
        print "libwcc.WCC_RECOMBINE_G2(B1TPG2, B2TPG2, BTPG2) Error %s" % rtn
    print "BTPG2: %s" % toHex(BTPG2)

    rtn = libwcc.WCC_RANDOM_GENERATE(RNG, X)
    if rtn != 0:
        print "libwcc.WCC_RANDOM_GENERATE(RNG,X) Error %s", rtn
    if DEBUG:
        print "X: %s" % toHex(X)

    rtn = libwcc.WCC_GET_G1_TPMULT(date,X,IdA,PaG1);
    if rtn != 0:
        print "libwcc.WCC_GET_G1_TPMULT(date,X,IdA,PaG1) Error %s", rtn
    if DEBUG:
        print "PaG1: %s" % toHex(PaG1)

    rtn = libwcc.WCC_RANDOM_GENERATE(RNG, W)
    if rtn != 0:
        print "libwcc.WCC_RANDOM_GENERATE(RNG,W) Error %s", rtn
    if DEBUG:
        print "W: %s" % toHex(W)

    rtn = libwcc.WCC_GET_G1_TPMULT(date,W,IdA,PgG1);
    if rtn != 0:
        print "libwcc.WCC_GET_G1_TPMULT(date,W,IdA,PgG1) Error %s", rtn
    if DEBUG:
        print "PgG1: %s" % toHex(PgG1)

    rtn = libwcc.WCC_RANDOM_GENERATE(RNG, Y)
    if rtn != 0:
        print "libwcc.WCC_RANDOM_GENERATE(RNG,Y) Error %s", rtn
    if DEBUG:
        print "Y: %s" % toHex(Y)

    rtn = libwcc.WCC_GET_G2_TPMULT(date,Y,IdB,PbG2);
    if rtn != 0:
        print "libwcc.WCC_GET_G1_TPMULT(date,Y,IdB,PbG2) Error %s", rtn
    if DEBUG:
        print "PbG2: %s" % toHex(PbG2)

    # PIA = Hq(PaG1,PbG2,PgG1,IdB)
    libwcc.WCC_Hq(PaG1,PbG2,PgG1,IdB,PIA);
    if DEBUG:
        print "PIA: %s" % toHex(PIA)

    # PIB = Hq(PbG2,PaG1,PgG1,IdA)
    libwcc.WCC_Hq(PbG2,PaG1,PgG1,IdA,PIB);
    if DEBUG:
        print "PIB: %s" % toHex(PIB)
        
    # Alice calculates AES Key 
    rtn = libwcc.WCC_SENDER_KEY(date, X, PIA, PIB, PbG2, PgG1, AKeyG1, ATPG1, IdB, KEY1)
    if rtn != 0:
        print "libwcc.WCC_SENDER_KEY(date, X, PIA, PIB, PbG2, PgG1, AKeyG1, ATPG1, IdB, KEY1) Error %s" % rtn
    print "{0}'s AES Key: {1}".format(alice_id, toHex(KEY1))

    # Bob calculates AES Key
    rtn = libwcc.WCC_RECEIVER_KEY(date, Y, W, PIA, PIB, PaG1, PgG1, BKeyG2, BTPG2, IdA, KEY2)
    if rtn != 0:
        print "libwcc.WCC_RECEIVER_KEY(date, Y, W, PIA, PIB, PaG1, PgG1, BKeyG2, BTPG2, IdA, KEY2) Error %s" % rtn
    print "{0}'s AES Key: {1}".format(bob_id, toHex(KEY2))

    libwcc.WCC_KILL_CSPRNG(RNG)
