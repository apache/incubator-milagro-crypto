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
   Generates a set of test vectors for testing the JavaScript. The output file is
   testVectors.json. This script uses the AMCL library.

   usage: genVectors.py [success authentication] [failed authentication] [epoch days test] [DEBUG}
"""

import sys
import json
import os
import datetime
import json
import random
import mpin

if len(sys.argv) == 5:
    nPos = int(sys.argv[1])
    nNeg = int(sys.argv[2])
    nEpoch = int(sys.argv[3])
    if (sys.argv[4] == "DEBUG"):
        DEBUG = True
elif len(sys.argv) == 4:
    nPos = int(sys.argv[1])
    nNeg = int(sys.argv[2])
    nEpoch = int(sys.argv[3])
    DEBUG = False
else:
    print "Usage: genVectors.py [success authentication] [failed authentication] [epoch days test] [DEBUG]"
    sys.exit(1)
print "Generate nPos = %s nNeg = %s nEpoch = %s" % (nPos, nNeg, nEpoch)

# Seed
seed = os.urandom(32)

# Assign a seed value
RAW = mpin.ffi.new("octet*")
RAWval = mpin.ffi.new("char [%s]" % len(seed), seed)
RAW[0].val = RAWval
RAW[0].len = len(seed)
RAW[0].max = len(seed)

# random number generator
RNG = mpin.ffi.new("csprng*")
mpin.libmpin.CREATE_CSPRNG(RNG,RAW)

# Master Secret Shares
MS1 = mpin.ffi.new("octet*")
MS1val = mpin.ffi.new("char []", mpin.PGS)
MS1[0].val = MS1val
MS1[0].max = mpin.PGS
MS1[0].len = mpin.PGS

MS2 = mpin.ffi.new("octet*")
MS2val = mpin.ffi.new("char []", mpin.PGS)
MS2[0].val = MS2val
MS2[0].max = mpin.PGS
MS2[0].len = mpin.PGS

# Hash value of MPIN_ID
HASH_MPIN_ID = mpin.ffi.new("octet*")
HASH_MPIN_IDval = mpin.ffi.new("char []", mpin.HASH_BYTES)
HASH_MPIN_ID[0].val = HASH_MPIN_IDval
HASH_MPIN_ID[0].max = mpin.HASH_BYTES
HASH_MPIN_ID[0].len = mpin.HASH_BYTES

# Server secret and shares
SS1 = mpin.ffi.new("octet*")
SS1val = mpin.ffi.new("char []", mpin.G2)
SS1[0].val = SS1val
SS1[0].max = mpin.G2
SS1[0].len = mpin.G2

SS2 = mpin.ffi.new("octet*")
SS2val = mpin.ffi.new("char []", mpin.G2)
SS2[0].val = SS2val
SS2[0].max = mpin.G2
SS2[0].len = mpin.G2

SERVER_SECRET = mpin.ffi.new("octet*")
SERVER_SECRETval = mpin.ffi.new("char []",  mpin.G2)
SERVER_SECRET[0].val = SERVER_SECRETval
SERVER_SECRET[0].max = mpin.G2
SERVER_SECRET[0].len = mpin.G2

# Time Permit and shares
TP1 = mpin.ffi.new("octet*")
TP1val = mpin.ffi.new("char []", mpin.G1)
TP1[0].val = TP1val
TP1[0].max = mpin.G1
TP1[0].len = mpin.G1

TP2 = mpin.ffi.new("octet*")
TP2val = mpin.ffi.new("char []", mpin.G1)
TP2[0].val = TP2val
TP2[0].max = mpin.G1
TP2[0].len = mpin.G1

TIME_PERMIT = mpin.ffi.new("octet*")
TIME_PERMITval = mpin.ffi.new("char []", mpin.G1)
TIME_PERMIT[0].val = TIME_PERMITval
TIME_PERMIT[0].max = mpin.G1
TIME_PERMIT[0].len = mpin.G1

# Client Secret
CS1 = mpin.ffi.new("octet*")
CS1val = mpin.ffi.new("char []", mpin.G1)
CS1[0].val = CS1val
CS1[0].max = mpin.G1
CS1[0].len = mpin.G1

CS2 = mpin.ffi.new("octet*")
CS2val = mpin.ffi.new("char []", mpin.G1)
CS2[0].val = CS2val
CS2[0].max = mpin.G1
CS2[0].len = mpin.G1

SEC = mpin.ffi.new("octet*")
SECval = mpin.ffi.new("char []",  mpin.G1)
SEC[0].val = SECval
SEC[0].max = mpin.G1
SEC[0].len = mpin.G1

# Token stored on computer
TOKEN = mpin.ffi.new("octet*")
TOKEN[0].val = mpin.ffi.new("char []",  mpin.G1)
TOKEN[0].max = mpin.G1
TOKEN[0].len = mpin.G1

UT = mpin.ffi.new("octet*")
UTval = mpin.ffi.new("char []",  mpin.G1)
UT[0].val = UTval
UT[0].max = mpin.G1
UT[0].len = mpin.G1

U = mpin.ffi.new("octet*")
Uval = mpin.ffi.new("char []",  mpin.G1)
U[0].val = Uval
U[0].max = mpin.G1
U[0].len = mpin.G1

X = mpin.ffi.new("octet*")
Xval = mpin.ffi.new("char []",  mpin.PGS)
X[0].val = Xval
X[0].max = mpin.PGS
X[0].len = mpin.PGS

Y = mpin.ffi.new("octet*")
Yval = mpin.ffi.new("char []",  mpin.PGS)
Y[0].val = Yval
Y[0].max = mpin.PGS
Y[0].len = mpin.PGS

lenEF = 12 * mpin.PFS
E = mpin.ffi.new("octet*")
Eval = mpin.ffi.new("char []",  lenEF)
E[0].val = Eval
E[0].max = lenEF
E[0].len = lenEF

F = mpin.ffi.new("octet*")
Fval = mpin.ffi.new("char []",  lenEF)
F[0].val = Fval
F[0].max = lenEF
F[0].len = lenEF

# H(ID)
HID = mpin.ffi.new("octet*")
HIDval = mpin.ffi.new("char []", mpin.G1)
HID[0].val = HIDval
HID[0].max = mpin.G1
HID[0].len = mpin.G1

# H(T|H(ID))
HTID = mpin.ffi.new("octet*")
HTIDval = mpin.ffi.new("char []", mpin.G1)
HTID[0].val = HTIDval
HTID[0].max = mpin.G1
HTID[0].len = mpin.G1

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

    if DEBUG:
        print test_no

    vector['test_no'] = test_no
    vector['mpin_id'] = mpin_id

    # Generate master secret shares
    rtn = mpin.libmpin.MPIN_RANDOM_GENERATE(RNG,MS1)
    assert rtn is 0, "MS1"
    vector['MS1'] = mpin.toHex(MS1)
    rtn = mpin.libmpin.MPIN_RANDOM_GENERATE(RNG,MS2)
    assert rtn is 0, "MS2"
    vector['MS2'] = mpin.toHex(MS2)

    # Generate server secret shares
    rtn = mpin.libmpin.MPIN_GET_SERVER_SECRET(MS1,SS1)
    assert rtn is 0, "SS1"
    vector['SS1'] = mpin.toHex(SS1)
    rtn = mpin.libmpin.MPIN_GET_SERVER_SECRET(MS2,SS2)
    assert rtn is 0, "SS2"
    vector['SS2'] = mpin.toHex(SS2)

    # Combine server secret shares
    rtn = mpin.libmpin.MPIN_RECOMBINE_G2(SS1, SS2, SERVER_SECRET)
    assert rtn is 0, "SERVER_SECRET"
    vector['SERVER_SECRET'] = mpin.toHex(SERVER_SECRET)

    # Identity
    MPIN_ID = mpin.ffi.new("octet*")
    MPIN_IDval =  mpin.ffi.new("char [%s]" % len(mpin_id), mpin_id)
    MPIN_ID[0].val = MPIN_IDval
    MPIN_ID[0].max = len(mpin_id)
    MPIN_ID[0].len = len(mpin_id)
    vector['MPIN_ID_HEX'] = mpin.toHex(MPIN_ID)

    # Hash MPIN_ID
    mpin.libmpin.MPIN_HASH_ID(MPIN_ID, HASH_MPIN_ID)
    vector['HASH_MPIN_ID_HEX'] = mpin.toHex(HASH_MPIN_ID)

    # Generate client secret shares
    rtn = mpin.libmpin.MPIN_GET_CLIENT_SECRET(MS1,HASH_MPIN_ID,CS1)
    assert rtn is 0, "CS1"
    vector['CS1'] = mpin.toHex(CS1)
    rtn = mpin.libmpin.MPIN_GET_CLIENT_SECRET(MS2,HASH_MPIN_ID,CS2)
    assert rtn is 0, "CS2"
    vector['CS2'] = mpin.toHex(CS2)

    # Combine client secret shares : TOKEN is the full client secret
    rtn = mpin.libmpin.MPIN_RECOMBINE_G1(CS1, CS2, TOKEN)
    assert rtn is 0, "CS1+CS2"
    vector['CLIENT_SECRET'] = mpin.toHex(TOKEN)

    # Generate Time Permit shares
    rtn = mpin.libmpin.MPIN_GET_CLIENT_PERMIT(date,MS1,HASH_MPIN_ID,TP1)
    assert rtn is 0, "TP1"
    vector['TP1'] = mpin.toHex(TP1)
    vector['DATE'] = date
    rtn = mpin.libmpin.MPIN_GET_CLIENT_PERMIT(date,MS2,HASH_MPIN_ID,TP2)
    assert rtn is 0, "TP2"
    vector['TP2'] = mpin.toHex(TP2)

    # Combine Time Permit shares
    rtn = mpin.libmpin.MPIN_RECOMBINE_G1(TP1, TP2, TIME_PERMIT)
    assert rtn is 0, "TP1+TP2"
    vector['TIME_PERMIT'] = mpin.toHex(TIME_PERMIT)

    # Client extracts PIN from secret to create Token
    rtn = mpin.libmpin.MPIN_EXTRACT_PIN(MPIN_ID, PIN1, TOKEN)
    assert rtn is 0, "TOKEN"
    vector['PIN1'] = PIN1
    vector['TOKEN'] = mpin.toHex(TOKEN)

    # Client first pass
    rtn = mpin.libmpin.MPIN_CLIENT_1(date, MPIN_ID, RNG, X, PIN2, TOKEN, SEC, U, UT, TIME_PERMIT)
    assert rtn is 0, "MPIN_CLIENT_1"
    vector['PIN2'] = PIN2
    vector['X'] = mpin.toHex(X)
    vector['U'] = mpin.toHex(U)
    vector['UT'] = mpin.toHex(UT)
    vector['SEC'] = mpin.toHex(SEC)

    # Server calculates H(ID) and H(T|H(ID)) (if time permits enabled),
    # and maps them to points on the curve HID and HTID resp.
    mpin.libmpin.MPIN_SERVER_1(date, MPIN_ID, HID, HTID)

    # Server generates Random number Y and sends it to Client
    rtn = mpin.libmpin.MPIN_RANDOM_GENERATE(RNG,Y)
    assert rtn is 0, "MPIN_RANDOM_GENERATE"
    vector['Y'] = mpin.toHex(Y)

    # Client second pass
    rtn = mpin.libmpin.MPIN_CLIENT_2(X,Y,SEC)
    assert rtn is 0, "MPIN_CLIENT_2"
    vector['V'] = mpin.toHex(SEC)

    # Server second pass
    rtn = mpin.libmpin.MPIN_SERVER_2(date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F)
    vector['SERVER_OUTPUT'] = rtn
    if PIN1 == PIN2:
        assert rtn == 0, "successful authentication"
    else:
        assert rtn == -19, "failed authentication"
    return vector

if __name__ == '__main__':
    # List of test vectors
    vectors = []

    # Today's date in epoch days
    date = mpin.libmpin.today()

    # Generate test vectors for successful authentication
    for i in range(0,nPos):
        # Assign the User an ID
        name = os.urandom(16).encode("hex")
        userID = name + "@miracl.com"
        issued = datetime.datetime.utcnow().isoformat("T").split(".")[0] + "Z"
        # userID = "testUser@miracl.com"
        # issued = "2014-01-30T19:17:48Z"
        mobile = 1
        salt = os.urandom(16).encode("hex")

        # Form MPin ID
        endUserdata = {
          "issued": issued,
          "userID": userID,
          "mobile": mobile,
          "salt": salt
        }
        mpin_id = json.dumps(endUserdata)

        PIN1 = random.randint(0,10000)
        PIN2 = PIN1
        vector = genVector(mpin_id, date, PIN1, PIN2, i)
        vectors.append(vector)
        # print i

    # Generate test vectors for failed authentication
    for i in range(0,nNeg):
        # Assign the User an ID
        name = os.urandom(16).encode("hex")
        userID = name + "@miracl.com"
        issued = datetime.datetime.utcnow().isoformat("T").split(".")[0] + "Z"
        # userID = "testUser@miracl.com"
        # issued = "2014-01-30T19:17:48Z"
        mobile = 1
        salt = os.urandom(8).encode("hex")

        # Form MPin ID
        endUserdata = {
          "issued": issued,
          "userID": userID,
          "mobile": mobile,
          "salt": salt
        }
        mpin_id = json.dumps(endUserdata)

        PIN1 = random.randint(0,10000)
        PIN2 = PIN1 - 1
        test_no = nPos + i
        vector = genVector(mpin_id, date, PIN1, PIN2, test_no)
        vectors.append(vector)
        # print i

    # Generate test vectors for days in future
    # Assign the User an ID
    name = os.urandom(16).encode("hex")
    userID = name + "@miracl.com"
    issued = datetime.datetime.utcnow().isoformat("T").split(".")[0] + "Z"
    # userID = "testUser@miracl.com"
    # issued = "2014-01-30T19:17:48Z"
    mobile = 1
    salt = os.urandom(8).encode("hex")

    # Form MPin ID
    endUserdata = {
      "issued": issued,
      "userID": userID,
      "mobile": mobile,
      "salt": salt
    }
    mpin_id = json.dumps(endUserdata)
    PIN1 = random.randint(0,10000)
    PIN2 = PIN1
    for i in range(0,nEpoch):
        test_no = nPos + nNeg + i
        vector = genVector(mpin_id, date, PIN1, PIN2, test_no)
        vectors.append(vector)
        date = date + 1

    # Write to JSON file
    json.dump(vectors, open("testVectors.json", "w"))
