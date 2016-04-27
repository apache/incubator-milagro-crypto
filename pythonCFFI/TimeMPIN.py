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
import sys
import timeit
import warnings

from mpin import ffi, G1, G2, HASH_BYTES, libmpin, PGS, toHex

warnings.filterwarnings("ignore")


def time_func(stmt, n=10, setup='from __main__ import *'):
    t = timeit.Timer(stmt, setup)
    total_time = t.timeit(n)
    iter_time = total_time / n
    iter_per_sec = n / total_time
    print "func:%s nIter:%s total_time:%s iter_time:%s iter_per_sec: %s" % (stmt, n, total_time, iter_time, iter_per_sec)

if (len(sys.argv) == 2) and (sys.argv[1] == "DEBUG"):
    DEBUG = True
else:
    DEBUG = False

nIter = 1000

if __name__ == "__main__":
    # Print hex values
    DEBUG = False
    MULTI_PASS = False

    # Seed
    seedHex = "79dd3f23c70bb529a8e3b221cf62da0dd4bd3ca35bd0c515cd9cde5ffa6a5c4d"
    seed = seedHex.decode("hex")

    # Identity
    identity = "alice@miracl.com"
    MPIN_ID = ffi.new("octet*")
    MPIN_IDval = ffi.new("char [%s]" % len(identity), identity)
    MPIN_ID[0].val = MPIN_IDval
    MPIN_ID[0].max = len(identity)
    MPIN_ID[0].len = len(identity)

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

    # Hash value of MPIN_ID
    HASH_MPIN_ID = ffi.new("octet*")
    HASH_MPIN_IDval = ffi.new("char []",  HASH_BYTES)
    HASH_MPIN_ID[0].val = HASH_MPIN_IDval
    HASH_MPIN_ID[0].max = HASH_BYTES
    HASH_MPIN_ID[0].len = HASH_BYTES

    # Client secret and shares
    CS1 = ffi.new("octet*")
    CS1val = ffi.new("char []", G1)
    CS1[0].val = CS1val
    CS1[0].max = G1
    CS1[0].len = G1

    CS2 = ffi.new("octet*")
    CS2val = ffi.new("char []", G1)
    CS2[0].val = CS2val
    CS2[0].max = G1
    CS2[0].len = G1

    SEC = ffi.new("octet*")
    SECval = ffi.new("char []", G1)
    SEC[0].val = SECval
    SEC[0].max = G1
    SEC[0].len = G1

    # Server secret and shares
    SS1 = ffi.new("octet*")
    SS1val = ffi.new("char []", G2)
    SS1[0].val = SS1val
    SS1[0].max = G2
    SS1[0].len = G2

    SS2 = ffi.new("octet*")
    SS2val = ffi.new("char []", G2)
    SS2[0].val = SS2val
    SS2[0].max = G2
    SS2[0].len = G2

    SERVER_SECRET = ffi.new("octet*")
    SERVER_SECRETval = ffi.new("char []", G2)
    SERVER_SECRET[0].val = SERVER_SECRETval
    SERVER_SECRET[0].max = G2
    SERVER_SECRET[0].len = G2

    # Time Permit and shares
    TP1 = ffi.new("octet*")
    TP1val = ffi.new("char []", G1)
    TP1[0].val = TP1val
    TP1[0].max = G1
    TP1[0].len = G1

    TP2 = ffi.new("octet*")
    TP2val = ffi.new("char []", G1)
    TP2[0].val = TP2val
    TP2[0].max = G1
    TP2[0].len = G1

    TIME_PERMIT = ffi.new("octet*")
    TIME_PERMITval = ffi.new("char []", G1)
    TIME_PERMIT[0].val = TIME_PERMITval
    TIME_PERMIT[0].max = G1
    TIME_PERMIT[0].len = G1

    # Token stored on computer
    TOKEN = ffi.new("octet*")
    TOKENval = ffi.new("char []", G1)
    TOKEN[0].val = TOKENval
    TOKEN[0].max = G1
    TOKEN[0].len = G1

    # H(ID)
    HID = ffi.new("octet*")
    HIDval = ffi.new("char []", G1)
    HID[0].val = HIDval
    HID[0].max = G1
    HID[0].len = G1

    # H(T|H(ID))
    HTID = ffi.new("octet*")
    HTIDval = ffi.new("char []", G1)
    HTID[0].val = HTIDval
    HTID[0].max = G1
    HTID[0].len = G1

    UT = ffi.new("octet*")
    UTval = ffi.new("char []", G1)
    UT[0].val = UTval
    UT[0].max = G1
    UT[0].len = G1

    U = ffi.new("octet*")
    Uval = ffi.new("char []", G1)
    U[0].val = Uval
    U[0].max = G1
    U[0].len = G1

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

    E = ffi.NULL
    F = ffi.NULL

    date = libmpin.MPIN_today()
    if date:
        HID = ffi.NULL
        U = ffi.NULL
    else:
        HTID = ffi.NULL
        UT = ffi.NULL

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
    libmpin.MPIN_CREATE_CSPRNG(RNG, RAW)

    # Hash MPIN_ID
    libmpin.MPIN_HASH_ID(MPIN_ID, HASH_MPIN_ID)
    if DEBUG:
        print "MPIN_ID: %s" % toHex(MPIN_ID)
        print "HASH_MPIN_ID: %s" % toHex(HASH_MPIN_ID)

    # Generate master secret for MIRACL and Customer
    rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS1)
    if rtn != 0:
        print "libmpin.MPIN_RANDOM_GENERATE(RNG,MS1) Error %s", rtn
    rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS2)
    if rtn != 0:
        print "libmpin.MPIN_RANDOM_GENERATE(RNG,MS2) Error %s" % rtn
    if DEBUG:
        print "MS1: %s" % toHex(MS1)
        print "MS2: %s" % toHex(MS2)

    # Generate server secret shares
    rtn = libmpin.MPIN_GET_SERVER_SECRET(MS1, SS1)
    if rtn != 0:
        print "libmpin.MPIN_GET_SERVER_SECRET(MS1,SS1) Error %s" % rtn
    rtn = libmpin.MPIN_GET_SERVER_SECRET(MS2, SS2)
    if rtn != 0:
        print "libmpin.MPIN_GET_SERVER_SECRET(MS2,SS2) Error %s" % rtn
    if DEBUG:
        print "SS1: %s" % toHex(SS1)
        print "SS2: %s" % toHex(SS2)

    # Combine server secret shares
    rtn = libmpin.MPIN_RECOMBINE_G2(SS1, SS2, SERVER_SECRET)
    if rtn != 0:
        print "libmpin.MPIN_RECOMBINE_G2( SS1, SS2, SERVER_SECRET) Error %s" % rtn
    if DEBUG:
        print "SERVER_SECRET: %s" % toHex(SERVER_SECRET)

    # Generate client secret shares
    rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS1, HASH_MPIN_ID, CS1)
    if rtn != 0:
        print "libmpin.MPIN_GET_CLIENT_SECRET(MS1,HASH_MPIN_ID,CS1) Error %s" % rtn
    rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS2, HASH_MPIN_ID, CS2)
    if rtn != 0:
        print "libmpin.MPIN_GET_CLIENT_SECRET(MS2,HASH_MPIN_ID,CS2) Error %s" % rtn
    if DEBUG:
        print "CS1: %s" % toHex(CS1)
        print "CS2: %s" % toHex(CS2)

    # Combine client secret shares : TOKEN is the full client secret
    rtn = libmpin.MPIN_RECOMBINE_G1(CS1, CS2, TOKEN)
    if rtn != 0:
        print "libmpin.MPIN_RECOMBINE_G1( CS1, CS2, TOKEN) Error %s" % rtn
    print "Client Secret: %s" % toHex(TOKEN)

    # Generate Time Permit shares
    if DEBUG:
        print "Date %s" % date
    rtn = libmpin.MPIN_GET_CLIENT_PERMIT(date, MS1, HASH_MPIN_ID, TP1)
    if rtn != 0:
        print "libmpin.MPIN_GET_CLIENT_PERMIT(date,MS1,HASH_MPIN_ID,TP1) Error %s" % rtn
    rtn = libmpin.MPIN_GET_CLIENT_PERMIT(date, MS2, HASH_MPIN_ID, TP2)
    if rtn != 0:
        print "libmpin.MPIN_GET_CLIENT_PERMIT(date,MS2,HASH_MPIN_ID,TP2) Error %s" % rtn
    if DEBUG:
        print "TP1: %s" % toHex(TP1)
        print "TP2: %s" % toHex(TP2)

    # Combine Time Permit shares
    rtn = libmpin.MPIN_RECOMBINE_G1(TP1, TP2, TIME_PERMIT)
    if rtn != 0:
        print "libmpin.MPIN_RECOMBINE_G1(TP1, TP2, TIME_PERMIT) Error %s" % rtn
    if DEBUG:
        print "TIME_PERMIT: %s" % toHex(TIME_PERMIT)

    # Client extracts PIN from secret to create Token
    PIN = 1234
    rtn = libmpin.MPIN_EXTRACT_PIN(MPIN_ID, PIN, TOKEN)
    if rtn != 0:
        print "libmpin.MPIN_EXTRACT_PIN( MPIN_ID, PIN, TOKEN) Error %s" % rtn
    print "Token: %s" % toHex(TOKEN)

    if MULTI_PASS:
        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(date, MPIN_ID, RNG, X, PIN, TOKEN, SEC, U, UT, TIME_PERMIT)
        if rtn != 0:
            print "MPIN_CLIENT_1  ERROR %s" % rtn
        if DEBUG:
            print "X: %s" % toHex(X)

        # Server calculates H(ID) and H(T|H(ID)) (if time permits enabled),
        # and maps them to points on the curve HID and HTID resp.
        libmpin.MPIN_SERVER_1(date, MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        if rtn != 0:
            print "libmpin.MPIN_RANDOM_GENERATE(RNG,Y) Error %s" % rtn
        if DEBUG:
            print "Y: %s" % toHex(Y)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        if rtn != 0:
            print "libmpin.MPIN_CLIENT_2(X,Y,SEC) Error %s" % rtn
        if DEBUG:
            print "V: %s" % toHex(SEC)

        # Server second pass
        rtn = libmpin.MPIN_SERVER_2(date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F)
        if rtn != 0:
            print "ERROR: Multi Pass %s is not authenticated" % identity
        else:
            print "SUCCESS: Multi Pass %s is authenticated" % identity
    else:
        # Client
        TimeValue = libmpin.MPIN_GET_TIME()
        time_func('libmpin.MPIN_CLIENT(date, MPIN_ID, RNG, X, PIN, TOKEN, SEC, U, UT, TIME_PERMIT, ffi.NULL, TimeValue, Y)', nIter)
        if DEBUG:
            print "X: %s" % toHex(X)

        # Server
        time_func('libmpin.MPIN_SERVER(date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F, MPIN_ID, ffi.NULL, TimeValue)', nIter)
        rtn = libmpin.MPIN_SERVER(date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F, MPIN_ID, ffi.NULL, TimeValue)
        if rtn != 0:
            print "ERROR: Single Pass %s is not authenticated" % identity
        else:
            print "SUCCESS: Single Pass %s is authenticated" % identity
