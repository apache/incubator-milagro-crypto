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
import os
import unittest
import json
import hashlib

from mpin import ffi, G1, G2, HASH_BYTES, libmpin, PFS, PGS, toHex

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

lenEF = 12 * PFS
E = ffi.new("octet*")
Eval = ffi.new("char []", lenEF)
E[0].val = Eval
E[0].max = lenEF
E[0].len = lenEF

F = ffi.new("octet*")
Fval = ffi.new("char []", lenEF)
F[0].val = Fval
F[0].max = lenEF
F[0].len = lenEF

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


class TestMPIN(unittest.TestCase):
    """Tests M-Pin crypto code"""

    def setUp(self):

        # Form MPin ID
        endUserData = {
            "issued": "2013-10-19T06:12:28Z",
            "userID": "testUser@miracl.com",
            "mobile": 1,
            "salt": "e985da112a378c222cfc2f7226097b0c"
        }
        mpin_id = json.dumps(endUserData)

        self.MPIN_ID = ffi.new("octet*")
        self.MPIN_IDval = ffi.new("char [%s]" % len(mpin_id), mpin_id)
        self.MPIN_ID[0].val = self.MPIN_IDval
        self.MPIN_ID[0].max = len(mpin_id)
        self.MPIN_ID[0].len = len(mpin_id)

        # Hash value of MPIN_ID
        self.HASH_MPIN_ID = ffi.new("octet*")
        self.HASH_MPIN_IDval = ffi.new("char []",  HASH_BYTES)
        self.HASH_MPIN_ID[0].val = self.HASH_MPIN_IDval
        self.HASH_MPIN_ID[0].max = HASH_BYTES
        self.HASH_MPIN_ID[0].len = HASH_BYTES
        libmpin.MPIN_HASH_ID(self.MPIN_ID, self.HASH_MPIN_ID)

        # Assign a seed value
        seedHex = "3ade3d4a5c698e8910bf92f25d97ceeb7c25ed838901a5cb5db2cf25434c1fe76c7f79b7af2e5e1e4988e4294dbd9bd9fa3960197fb7aec373609fb890d74b16a4b14b2ae7e23b75f15d36c21791272372863c4f8af39980283ae69a79cf4e48e908f9e0"
        self.seed = seedHex.decode("hex")
        self.RAW = ffi.new("octet*")
        self.RAWval = ffi.new("char [%s]" % len(self.seed), self.seed)
        self.RAW[0].val = self.RAWval
        self.RAW[0].len = len(self.seed)
        self.RAW[0].max = len(self.seed)

        self.date = 16238
        self.ms1Hex = "035fb3138d1dfa87cd7aab637faa7400f0c8416ab7e922ddea8dc0462bc0123a"
        self.ms2Hex = "1c9c8ad28f36fd10f5f33ea3b9db0a1eaf8ef7fc13cd55d1b47d7e607e3ca619"
        self.ss1Hex = "114658f8ccb93969fc7ec23257397dbe820a329016182d3168cb2d9b8ce9cbcc0d580feb5b621d6c00ed12e70688ec130a138a48b2c2fd57695bad71290c365d1d621117cbcd2061357965ca2f81468979ca527c9888ae87d84d8df0589e6814041a1ca3163dd77d9b0c29c0a6aadca1cbaa7c2905d8eb8ab12051ea06f12439"
        self.ss2Hex = "239b55f12040f70b6a5e53c45985aa4bfa368a3d26a000775f7960a2716023350b08851b5753c33bddba07c172f4745abb70e587902dca52e49199ee4c2562130a910fb6402c417e753a873b907153820ae5854a4c27948806000e82452d77f3127e66573cae233c07137e63fbac8f09ef39718c421f71f1b9be5f150a17bd12"
        self.serverSecretHex = "09aa0e36fa30c9b0cc9d5fa7b2be33595542f408a49ad0c91c4f7bba3d993cfc23eca9811699525fa8523069b6d6e1540deeb94eaa26c1e15776b4ab65e4603c0a161baa02d75fb1e966863b51ac5ee67872f663926b8537292dd170e83a136e098e2da97161db6c75a182088af1da6557d06a7816ec6ac7882262ce183fc605"
        self.cs1Hex = "04050de8b61fa91b4747017a7ed72ceb4b26056382435413eab2b9259a4c99f9161eb63db017296616b59b861f2433f44cc56a3d8d5ed600cf338de0f7ce62b900"
        self.cs2Hex = "0420b7333796a1d7a5abc553dcbacff281f5a4b8dea937da548ee6103c0089b6a218e0d1d617a55f9b789c4ebd25f5cf8aafd0f666306031cf5b6faa9e50d88f50"
        self.clientSecretHex = "040b98a2b31786a50f548bd692171f99d1d1be26129725519624d3cc90b98bd17321a24c5b9511ff887672e460d6157b1c5461114e4e4f62d6207f5e8d48919fad"
        self.tp1Hex = "04123c806b8366721e3718a731fa109c144561f033ff5fe44356091a3c2365c9182218b86526247eb7fd552575800ef3cc9da22c6655d7795377b216030c9f1f62"
        self.tp2Hex = "041cf3f5604e5f8ecb44bc3e36f5fa1fe68e873b0822646a3a8f0fcf0477f561241a6bda4a9e93a455a6b940126feae5959ca855870cd8275d7dd1a4df4ddb6211"
        self.timePermitHex = "0405198cbd152cfc860cc22d6c16d92ff1a1d5d6d9b268588c3a74c4e6fc056a0002d35c769458aaaf01ecc34cbaf7b6d8154deb2d26db332c5a421557f4fe1ba3"
        self.tokenHex = "04213945622f5c6638c13692d77f12983cb0664fb51f7f36a5664872894b87dc051786a8747cf01b749f78fb75aa64bc932e71c9a6e1ee261aa687564bf38be329"

        # Token
        self.token = self.tokenHex.decode("hex")
        self.TOKEN = ffi.new("octet*")
        self.TOKENval = ffi.new("char [%s]" % len(self.token), self.token)
        self.TOKEN[0].val = self.TOKENval
        self.TOKEN[0].len = len(self.token)
        self.TOKEN[0].max = len(self.token)

        # TIME_PERMIT
        self.timePermit = self.timePermitHex.decode("hex")
        self.TIMEPERMIT = ffi.new("octet*")
        self.TIMEPERMITval = ffi.new("char [%s]" % len(self.timePermit), self.timePermit)
        self.TIMEPERMIT[0].val = self.TIMEPERMITval
        self.TIMEPERMIT[0].len = len(self.timePermit)
        self.TIMEPERMIT[0].max = len(self.timePermit)

        # SERVER_SECRET
        self.serverSecret = self.serverSecretHex.decode("hex")
        self.SERVER_SECRET = ffi.new("octet*")
        self.SERVER_SECRETval = ffi.new("char [%s]" % len(self.serverSecret), self.serverSecret)
        self.SERVER_SECRET[0].val = self.SERVER_SECRETval
        self.SERVER_SECRET[0].len = len(self.serverSecret)
        self.SERVER_SECRET[0].max = len(self.serverSecret)

        self.ms1 = self.ms1Hex.decode("hex")
        self.clientSecret = self.clientSecretHex.decode("hex")
        self.timePermit = self.timePermitHex.decode("hex")

    def test_1(self):
        """test_1 Good PIN and good token"""
        PIN = 1234

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN, self.TOKEN, SEC, U, UT, self.TIMEPERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, self.SERVER_SECRET, U, UT, SEC, E, F)
        self.assertEqual(rtn, 0)

    def test_2(self):
        """test_2 Bad PIN and good token"""
        PIN = 2000

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN, self.TOKEN, SEC, U, UT, self.TIMEPERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, self.SERVER_SECRET, U, UT, SEC, E, F)
        self.assertEqual(rtn, -19)

    def test_3(self):
        """test_3 Good PIN and bad token"""
        PIN = 1234

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN, self.TOKEN, SEC, U, UT, self.TIMEPERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        # clientSecret aka V is equal to UT to model a bad token
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, self.SERVER_SECRET, U, UT, UT, E, F)
        self.assertEqual(rtn, -19)

    def test_4(self):
        """test_4 Test hash function"""
        HASH_MPIN_ID = ffi.new("octet*")
        HASH_MPIN_IDval = ffi.new("char []",  HASH_BYTES)
        HASH_MPIN_ID[0].val = HASH_MPIN_IDval
        HASH_MPIN_ID[0].max = HASH_BYTES
        HASH_MPIN_ID[0].len = HASH_BYTES

        for i in range(1, 10000):
            bytesStr = os.urandom(128)
            hash_object2 = hashlib.sha256(bytesStr)
            digest = hash_object2.hexdigest()
            MPIN_ID = ffi.new("octet*")
            MPIN_IDval = ffi.new("char [%s]" % len(bytesStr), bytesStr)
            MPIN_ID[0].val = MPIN_IDval
            MPIN_ID[0].max = len(bytesStr)
            MPIN_ID[0].len = len(bytesStr)
            libmpin.MPIN_HASH_ID(MPIN_ID, HASH_MPIN_ID)
            self.assertEqual(digest, toHex(HASH_MPIN_ID))

    def test_5(self):
        """test_5 Make sure all client secret are unique"""
        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Generate master secret share
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS1)
        self.assertEqual(rtn, 0)

        s = set()
        match = 0
        for i in range(1, 1000):
            rand_val = os.urandom(32)
            HASH_MPIN_ID = ffi.new("octet*")
            HASH_MPIN_IDval = ffi.new("char [%s]" % HASH_BYTES, rand_val)
            HASH_MPIN_ID[0].val = HASH_MPIN_IDval
            HASH_MPIN_ID[0].max = HASH_BYTES
            HASH_MPIN_ID[0].len = HASH_BYTES

            # Generate client secret shares
            rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS1, HASH_MPIN_ID, CS1)
            self.assertEqual(rtn, 0)
            cs1Hex = toHex(CS1)
            if cs1Hex in s:
                match = 1
            self.assertEqual(match, 0)
            s.add(cs1Hex)

    def test_6(self):
        """test_6 Make sure all one time passwords are random i.e. they should collide"""
        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        s = set()
        match = 0
        for i in range(1, 10000):
            OTP = libmpin.generateOTP(RNG)
            if OTP in s:
                # print i
                match = 1
            s.add(OTP)
        self.assertEqual(match, 1)

    def test_7(self):
        """test_7 Make sure all random values are random i.e. they should collide"""
        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Generate 100 byte random number
        RANDOMlen = 3
        RANDOM = ffi.new("octet*")
        RANDOMval = ffi.new("char []",  RANDOMlen)
        RANDOM[0].val = RANDOMval
        RANDOM[0].max = RANDOMlen
        RANDOM[0].len = RANDOMlen

        s = set()
        match = 0
        for i in range(1, 10000):
            libmpin.generateRandom(RNG, RANDOM)
            random = toHex(RANDOM)
            if random in s:
                # print i
                match = 1
            s.add(random)
        self.assertEqual(match, 1)

    def test_8(self):
        """test_8 Generation of secrets and time permits"""

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.CREATE_CSPRNG(RNG, self.RAW)

        # Generate Client master secret share for MIRACL and Customer
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS1)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.ms1Hex, toHex(MS1))
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS2)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.ms2Hex, toHex(MS2))

        # Generate server secret shares
        rtn = libmpin.MPIN_GET_SERVER_SECRET(MS1, SS1)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.ss1Hex, toHex(SS1))
        rtn = libmpin.MPIN_GET_SERVER_SECRET(MS2, SS2)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.ss2Hex, toHex(SS2))

        # Combine server secret shares
        rtn = libmpin.MPIN_RECOMBINE_G2(SS1, SS2, SERVER_SECRET)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.serverSecretHex, toHex(SERVER_SECRET))

        # Generate client secret shares
        rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS1, self.HASH_MPIN_ID, CS1)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.cs1Hex, toHex(CS1))
        rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS2, self.HASH_MPIN_ID, CS2)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.cs2Hex, toHex(CS2))

        # Combine client secret shares : TOKEN is the full client secret
        rtn = libmpin.MPIN_RECOMBINE_G1(CS1, CS2, TOKEN)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.clientSecretHex, toHex(TOKEN))

        # Generate Time Permit shares
        rtn = libmpin.MPIN_GET_CLIENT_PERMIT(self.date, MS1, self.HASH_MPIN_ID, TP1)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.tp1Hex, toHex(TP1))
        rtn = libmpin.MPIN_GET_CLIENT_PERMIT(self.date, MS2, self.HASH_MPIN_ID, TP2)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.tp2Hex, toHex(TP2))

        # Combine Time Permit shares
        rtn = libmpin.MPIN_RECOMBINE_G1(TP1, TP2, TIME_PERMIT)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.timePermitHex, toHex(TIME_PERMIT))

        # Client extracts PIN from secret to create Token
        PIN = 1234
        rtn = libmpin.MPIN_EXTRACT_PIN(self.MPIN_ID, PIN, TOKEN)
        self.assertEqual(rtn, 0)
        self.assertEqual(self.tokenHex, toHex(TOKEN))

    def test_9(self):
        """test_9 Test successful authentication for different master secrets"""

        for i in range(1, 1000):
            # Assign a seed value
            seed = os.urandom(32)
            RAW = ffi.new("octet*")
            RAWval = ffi.new("char [%s]" % len(seed), seed)
            RAW[0].val = RAWval
            RAW[0].len = len(seed)
            RAW[0].max = len(seed)

            # random number generator
            RNG = ffi.new("csprng*")
            libmpin.CREATE_CSPRNG(RNG, RAW)

            # Generate Client master secret share for MIRACL and Customer
            rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS1)
            self.assertEqual(rtn, 0)
            rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS2)
            self.assertEqual(rtn, 0)

            # Generate server secret shares
            rtn = libmpin.MPIN_GET_SERVER_SECRET(MS1, SS1)
            self.assertEqual(rtn, 0)
            rtn = libmpin.MPIN_GET_SERVER_SECRET(MS2, SS2)
            self.assertEqual(rtn, 0)

            # Combine server secret shares
            rtn = libmpin.MPIN_RECOMBINE_G2(SS1, SS2, SERVER_SECRET)
            self.assertEqual(rtn, 0)

            # Generate client secret shares
            rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS1, self.HASH_MPIN_ID, CS1)
            self.assertEqual(rtn, 0)
            rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS2, self.HASH_MPIN_ID, CS2)
            self.assertEqual(rtn, 0)

            # Combine client secret shares : TOKEN is the full client secret
            rtn = libmpin.MPIN_RECOMBINE_G1(CS1, CS2, TOKEN)
            self.assertEqual(rtn, 0)

            # Generate Time Permit shares
            rtn = libmpin.MPIN_GET_CLIENT_PERMIT(self.date, MS1, self.HASH_MPIN_ID, TP1)
            self.assertEqual(rtn, 0)
            rtn = libmpin.MPIN_GET_CLIENT_PERMIT(self.date, MS2, self.HASH_MPIN_ID, TP2)
            self.assertEqual(rtn, 0)

            # Combine Time Permit shares
            rtn = libmpin.MPIN_RECOMBINE_G1(TP1, TP2, TIME_PERMIT)
            self.assertEqual(rtn, 0)

            # Client extracts PIN from secret to create Token
            PIN = 1234
            rtn = libmpin.MPIN_EXTRACT_PIN(self.MPIN_ID, PIN, TOKEN)
            self.assertEqual(rtn, 0)

            # Client first pass
            rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN, TOKEN, SEC, U, UT, TIME_PERMIT)
            self.assertEqual(rtn, 0)

            # Server calculates H(ID) and H(T|H(ID))
            libmpin.MPIN_SERVER_1(self.date, self.MPIN_ID, HID, HTID)

            # Server generates Random number Y and sends it to Client
            rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
            self.assertEqual(rtn, 0)

            # Client second pass
            rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
            self.assertEqual(rtn, 0)

            # Server second pass
            rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F)
            self.assertEqual(rtn, 0)

    def test_10(self):
        """test_10 Test mss starting with 00 are handled correctly"""

        for i in range(1, 1000):
            # Assign a seed value
            seed = os.urandom(32)
            RAW = ffi.new("octet*")
            RAWval = ffi.new("char [%s]" % len(seed), seed)
            RAW[0].val = RAWval
            RAW[0].len = len(seed)
            RAW[0].max = len(seed)

            # random number generator
            RNG = ffi.new("csprng*")
            libmpin.CREATE_CSPRNG(RNG, RAW)

            # Generate master secret - ms1
            rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, MS1)
            self.assertEqual(rtn, 0)
            ms1_hex = toHex(MS1)
            ms1 = ms1_hex.decode("hex")
            # Assign ms1 to ms2 if it starts with "00"
            if ms1_hex.startswith('00'):
                MS2 = ffi.new("octet*")
                MS2val = ffi.new("char [%s]" % PGS, ms1)
                MS2[0].val = MS2val
                MS2[0].max = PGS
                MS2[0].len = PGS

                # Generate client secret shares
                rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS1, self.HASH_MPIN_ID, CS1)
                self.assertEqual(rtn, 0)
                cs1_hex = toHex(CS1)
                rtn = libmpin.MPIN_GET_CLIENT_SECRET(MS2, self.HASH_MPIN_ID, CS2)
                self.assertEqual(rtn, 0)
                cs2_hex = toHex(CS2)
                self.assertEqual(cs1_hex, cs2_hex)

if __name__ == '__main__':
    # Run tests
    unittest.main()
