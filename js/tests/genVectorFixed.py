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
   Generates the same test vector for today. The output file is
   testVectors.json. This uses a fixed seed and MPIN ID

   usage: genVectorFixed.py
"""

import sys
import json
import os
import datetime
import json
import random
from mpin import *

# Initialize M-Pin Domain parameters
mpdom = ffi.new("mpin_domain*")
rtn = libmpin.MPIN_DOMAIN_INIT_NEW(mpdom)
if rtn != 0:
    print "initialization failed: Error %s" % rtn

# Seed
seed_hex = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f60616263"
seed = seed_hex.decode("hex")

# Assign a seed value
RAW = ffi.new("octet*")
RAWval = ffi.new("char [%s]" % len(seed), seed)
RAW[0].val = RAWval
RAW[0].len = len(seed)
RAW[0].max = len(seed)

# random number generator
RNG = ffi.new("csprng*")
libmpin.CREATE_CSPRNG(RNG,RAW)

# Master Secret Shares
MS1 = ffi.new("octet*")
MS1val = ffi.new("char []", PGS)
MS1[0].val = MS1val
MS1[0].max = PGS
MS1[0].len = PGS

# Generate master secret shares
rtn = libmpin.MPIN_RANDOM_GENERATE(mpdom,RNG,MS1)
if rtn != 0:
  print "libmpin.MPIN_RANDOM_GENERATE(mpdom,RNG,MS1) Error %s", rtn

# Hash value of MPIN_ID
HASH_MPIN_ID = ffi.new("octet*")
HASH_MPIN_IDval = ffi.new("char []",  HASH_BYTES)
HASH_MPIN_ID[0].val = HASH_MPIN_IDval
HASH_MPIN_ID[0].max = HASH_BYTES
HASH_MPIN_ID[0].len = HASH_BYTES

SERVER_SECRET = ffi.new("octet*")
SERVER_SECRETval = ffi.new("char []",  G2)
SERVER_SECRET[0].val = SERVER_SECRETval
SERVER_SECRET[0].max = G2
SERVER_SECRET[0].len = G2


TIME_PERMIT = ffi.new("octet*")
TIME_PERMITval = ffi.new("char []", G1)
TIME_PERMIT[0].val = TIME_PERMITval
TIME_PERMIT[0].max = G1
TIME_PERMIT[0].len = G1

CLIENT_SECRET = ffi.new("octet*")
CLIENT_SECRETval = ffi.new("char []",  G1)
CLIENT_SECRET[0].val = CLIENT_SECRETval
CLIENT_SECRET[0].max = G1
CLIENT_SECRET[0].len = G1

# Token stored on computer
TOKEN = ffi.new("octet*")
TOKEN[0].val = ffi.new("char []",  G1)
TOKEN[0].max = G1
TOKEN[0].len = G1

UT = ffi.new("octet*")
UTval = ffi.new("char []",  G1)
UT[0].val = UTval
UT[0].max = G1
UT[0].len = G1

U = ffi.new("octet*")
Uval = ffi.new("char []",  G1)
U[0].val = Uval
U[0].max = G1
U[0].len = G1

X = ffi.new("octet*")
Xval = ffi.new("char []",  PGS)
X[0].val = Xval
X[0].max = PGS
X[0].len = PGS

Y = ffi.new("octet*")
Yval = ffi.new("char []",  PGS)
Y[0].val = Yval
Y[0].max = PGS
Y[0].len = PGS

lenEF = 12 * PFS
E = ffi.new("octet*")
Eval = ffi.new("char []",  lenEF)
E[0].val = Eval
E[0].max = lenEF
E[0].len = lenEF

F = ffi.new("octet*")
Fval = ffi.new("char []",  lenEF)
F[0].val = Fval
F[0].max = lenEF
F[0].len = lenEF

def genVector(mpin_id, date, PIN1, PIN2, test_no):
    """Generate a single test vector

    Use mpin_id and date to generate a
    valid Client Secret and Time Permit

    Args::

        mpin_id: The M-Pin ID
        date: The date of M-Pin Authentication
        PIN1: PIN for generating token
        PIN2: PIN for authenticating
        test_no: Test vector identifier

    Returns:
        vector: A test vector

    Raises:
        Exception
    """
    vector = {}

    vector['test_no'] = test_no
    vector['mpin_id'] = mpin_id

    # Generate server secret shares
    print "MS1 ", toHex(MS1)
    rtn = libmpin.MPIN_GET_SERVER_SECRET(mpdom,MS1,SERVER_SECRET)
    if rtn != 0:
        print "libmpin.MPIN_GET_SERVER_SECRET(mpdom,MS1,SS1) Error %s" % rtn
    vector['SERVER_SECRET'] = toHex(SERVER_SECRET)
    print "SERVER_SECRET ", toHex(SERVER_SECRET)

    # Identity
    MPIN_ID = ffi.new("octet*")
    MPIN_IDval =  ffi.new("char [%s]" % len(mpin_id), mpin_id)
    MPIN_ID[0].val = MPIN_IDval
    MPIN_ID[0].max = len(mpin_id)
    MPIN_ID[0].len = len(mpin_id)
    vector['MPIN_ID_HEX'] = toHex(MPIN_ID)
    print "mpin_id ", mpin_id
    print "MPIN_ID_HEX ", toHex(MPIN_ID)

    # Hash MPIN_ID
    libmpin.hash(ffi.NULL, -1, MPIN_ID, ffi.NULL, HASH_MPIN_ID);
    vector['HASH_MPIN_ID_HEX'] = toHex(HASH_MPIN_ID)
    print "HASH_MPIN_ID_HEX ", toHex(HASH_MPIN_ID)

    # Generate client secret shares
    rtn = libmpin.MPIN_GET_CLIENT_MULTIPLE(mpdom,MS1,HASH_MPIN_ID,TOKEN)
    assert rtn is 0, "CS1"
    vector['CLIENT_SECRET'] = toHex(TOKEN)
    print "HASH_MPIN_ID ", toHex(HASH_MPIN_ID)
    print "CLIENT_SECRET ", toHex(TOKEN)


    # Generate Time Permit shares
    rtn = libmpin.MPIN_GET_CLIENT_PERMIT(mpdom,date,MS1,HASH_MPIN_ID,TIME_PERMIT)
    assert rtn is 0, "TP1"
    vector['TIME_PERMIT'] = toHex(TIME_PERMIT)
    vector['DATE'] = date
    print "TIME_PERMIT", TIME_PERMIT
    print "DATE", date

    # Client extracts PIN from secret to create Token
    rtn = libmpin.MPIN_EXTRACT_PIN(mpdom, MPIN_ID, PIN1, TOKEN)
    assert rtn is 0, "TOKEN"
    vector['PIN1'] = PIN1
    vector['TOKEN'] = toHex(TOKEN)
    print "TOKEN ", toHex(TOKEN)

    # Client first pass
    rtn = libmpin.MPIN_CLIENT_1(mpdom,date,MPIN_ID,RNG,X,PIN2,TOKEN, CLIENT_SECRET,U,TIME_PERMIT,UT,ffi.NULL,ffi.NULL);
    assert rtn is 0, "MPIN_CLIENT_1"
    vector['PIN2'] = PIN2
    vector['X'] = toHex(X)
    vector['U'] = toHex(U)
    vector['UT'] = toHex(UT)
    vector['SEC'] = toHex(CLIENT_SECRET)
    print 'PIN2 ', PIN2
    print 'X ', toHex(X)
    print 'U ', toHex(U)
    print 'UT ', toHex(UT)
    print 'SEC',  toHex(CLIENT_SECRET)

    # Server generates Random number Y and sends it to Client
    rtn = libmpin.MPIN_RANDOM_GENERATE(mpdom,RNG,Y)
    assert rtn is 0, "MPIN_RANDOM_GENERATE"
    vector['Y'] = toHex(Y)
    print 'Y', toHex(Y)

    # Client second pass
    rtn = libmpin.MPIN_CLIENT_2(mpdom,X,Y,CLIENT_SECRET)
    assert rtn is 0, "MPIN_CLIENT_2"
    vector['V'] = toHex(CLIENT_SECRET)
    print 'V ', toHex(CLIENT_SECRET)

    # Server second pass
    rtn = libmpin.MPIN_MINI_SERVER(mpdom, date, MPIN_ID, Y, SERVER_SECRET, U,UT,CLIENT_SECRET,E,F)
    if PIN1 == PIN2:
        assert rtn == 0, "successful authentication"
    else:
        assert rtn == -19, "failed authentication"
    return vector

if __name__ == '__main__':

    # List of test vectors
    vectors = []

    # Today's date in epoch days
    date = libmpin.today()

    mpin_id = "testUser@miracl.com"
    PIN1 = 1234
    PIN2 = PIN1
    vector = genVector(mpin_id, date, PIN1, PIN2, 0)
    vectors.append(vector)

    # Write to JSON file
    json.dump(vectors, open("testVectors.json", "w"))
