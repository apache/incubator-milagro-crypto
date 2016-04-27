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

from mpin import ffi, G1, G2, HASH_BYTES, IVL, libmpin, PAS, PFS, PGS, toHex

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

    def test_1(self):
        """test_1 Good PIN and good token"""
        PIN1 = 1234
        PIN2 = 1234

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        PIN1 = 1234
        rtn = libmpin.MPIN_EXTRACT_PIN(self.MPIN_ID, PIN1, TOKEN)
        self.assertEqual(rtn, 0)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN2, TOKEN, SEC, U, UT, TIME_PERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.HASH_MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F)
        self.assertEqual(rtn, 0)

    def test_2(self):
        """test_2 Bad PIN and good token"""
        PIN1 = 1234
        PIN2 = 2000

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        PIN1 = 1234
        rtn = libmpin.MPIN_EXTRACT_PIN(self.MPIN_ID, PIN1, TOKEN)
        self.assertEqual(rtn, 0)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN2, TOKEN, SEC, U, UT, TIME_PERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.HASH_MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, SERVER_SECRET, U, UT, SEC, E, F)
        self.assertEqual(rtn, -19)

    def test_3(self):
        """test_3 Good PIN and bad token"""
        PIN1 = 1234
        PIN2 = 1234

        # random number generator
        RNG = ffi.new("csprng*")
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        PIN1 = 1234
        rtn = libmpin.MPIN_EXTRACT_PIN(self.MPIN_ID, PIN1, TOKEN)
        self.assertEqual(rtn, 0)

        # Client first pass
        rtn = libmpin.MPIN_CLIENT_1(self.date, self.MPIN_ID, RNG, X, PIN2, TOKEN, SEC, U, UT, TIME_PERMIT)
        self.assertEqual(rtn, 0)

        # Server calculates H(ID) and H(T|H(ID))
        libmpin.MPIN_SERVER_1(self.date, self.HASH_MPIN_ID, HID, HTID)

        # Server generates Random number Y and sends it to Client
        rtn = libmpin.MPIN_RANDOM_GENERATE(RNG, Y)
        self.assertEqual(rtn, 0)

        # Client second pass
        rtn = libmpin.MPIN_CLIENT_2(X, Y, SEC)
        self.assertEqual(rtn, 0)

        # Server second pass
        # clientSecret aka V is equal to UT to model a bad token
        rtn = libmpin.MPIN_SERVER_2(self.date, HID, HTID, Y, SERVER_SECRET, U, UT, UT, E, F)
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
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        libmpin.MPIN_CREATE_CSPRNG(RNG, self.RAW)

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
        """test_8 AES-GCM: Successful encryption and decryption"""

        # Generate 16 byte key
        key_val = os.urandom(PAS)
        AES_KEY = ffi.new("octet*")
        AES_KEYval = ffi.new("char [%s]" % PAS, key_val)
        AES_KEY[0].val = AES_KEYval
        AES_KEY[0].max = PAS
        AES_KEY[0].len = PAS

        # Generate 12 byte IV
        iv_val = os.urandom(IVL)
        IV = ffi.new("octet*")
        IVval = ffi.new("char [%s]" % IVL, iv_val)
        IV[0].val = IVval
        IV[0].max = IVL
        IV[0].len = IVL

        # Generate a 32 byte random header
        header_val = os.urandom(32)
        HEADER = ffi.new("octet*")
        HEADERval = ffi.new("char [%s]" % len(header_val), header_val)
        HEADER[0].val = HEADERval
        HEADER[0].max = len(header_val)
        HEADER[0].len = len(header_val)

        # Plaintext input
        plaintext1 = "A test message"
        PLAINTEXT1 = ffi.new("octet*")
        PLAINTEXT1val = ffi.new("char [%s]" % len(plaintext1), plaintext1)
        PLAINTEXT1[0].val = PLAINTEXT1val
        PLAINTEXT1[0].max = len(plaintext1)
        PLAINTEXT1[0].len = len(plaintext1)
        # print "Input message: %s" % ffi.string(PLAINTEXT1[0].val, PLAINTEXT1[0].len)

        # Ciphertext
        CIPHERTEXT = ffi.new("octet*")
        CIPHERTEXTval = ffi.new("char []", len(plaintext1))
        CIPHERTEXT[0].val = CIPHERTEXTval
        CIPHERTEXT[0].max = len(plaintext1)

        # 16 byte authentication tag
        TAG1 = ffi.new("octet*")
        TAG1val = ffi.new("char []",  PAS)
        TAG1[0].val = TAG1val
        TAG1[0].max = PAS

        libmpin.MPIN_AES_GCM_ENCRYPT(AES_KEY, IV, HEADER, PLAINTEXT1, CIPHERTEXT, TAG1)
        # Plaintext output
        PLAINTEXT2 = ffi.new("octet*")
        PLAINTEXT2val = ffi.new("char []", CIPHERTEXT[0].len)
        PLAINTEXT2[0].val = PLAINTEXT2val
        PLAINTEXT2[0].max = CIPHERTEXT[0].len
        PLAINTEXT2[0].len = CIPHERTEXT[0].len

        # 16 byte authentication tag
        TAG2 = ffi.new("octet*")
        TAG2val = ffi.new("char []", PAS)
        TAG2[0].val = TAG2val
        TAG2[0].max = PAS

        libmpin.MPIN_AES_GCM_DECRYPT(AES_KEY, IV, HEADER, CIPHERTEXT, PLAINTEXT2, TAG2)
        self.assertEqual(toHex(TAG1), toHex(TAG2))
        self.assertEqual(toHex(PLAINTEXT1), toHex(PLAINTEXT2))
        # print "Output message: %s" % ffi.string(PLAINTEXT2[0].val, PLAINTEXT2[0].len)

    def test_9(self):
        """test_9 AES-GCM: Failed encryption and decryption by changing a ciphertext byte"""

        # Generate 16 byte key
        key_val = os.urandom(PAS)
        AES_KEY = ffi.new("octet*")
        AES_KEYval = ffi.new("char [%s]" % PAS, key_val)
        AES_KEY[0].val = AES_KEYval
        AES_KEY[0].max = PAS
        AES_KEY[0].len = PAS

        # Generate 12 byte IV
        iv_val = os.urandom(IVL)
        IV = ffi.new("octet*")
        IVval = ffi.new("char [%s]" % IVL, iv_val)
        IV[0].val = IVval
        IV[0].max = IVL
        IV[0].len = IVL

        # Generate a 32 byte random header
        header_val = os.urandom(32)
        HEADER = ffi.new("octet*")
        HEADERval = ffi.new("char [%s]" % len(header_val), header_val)
        HEADER[0].val = HEADERval
        HEADER[0].max = len(header_val)
        HEADER[0].len = len(header_val)

        # Plaintext input
        plaintext1 = "A test message"
        PLAINTEXT1 = ffi.new("octet*")
        PLAINTEXT1val = ffi.new("char [%s]" % len(plaintext1), plaintext1)
        PLAINTEXT1[0].val = PLAINTEXT1val
        PLAINTEXT1[0].max = len(plaintext1)
        PLAINTEXT1[0].len = len(plaintext1)
        # print "Input message: %s" % ffi.string(PLAINTEXT1[0].val, PLAINTEXT1[0].len)

        # Ciphertext
        CIPHERTEXT = ffi.new("octet*")
        CIPHERTEXTval = ffi.new("char []", len(plaintext1))
        CIPHERTEXT[0].val = CIPHERTEXTval
        CIPHERTEXT[0].max = len(plaintext1)

        # 16 byte authentication tag
        TAG1 = ffi.new("octet*")
        TAG1val = ffi.new("char []",  PAS)
        TAG1[0].val = TAG1val
        TAG1[0].max = PAS

        libmpin.MPIN_AES_GCM_ENCRYPT(AES_KEY, IV, HEADER, PLAINTEXT1, CIPHERTEXT, TAG1)

        # Change one byte of ciphertext
        CIPHERTEXT[0].val[0] = "\xa5"

        # Plaintext output
        PLAINTEXT2 = ffi.new("octet*")
        PLAINTEXT2val = ffi.new("char []", CIPHERTEXT[0].len)
        PLAINTEXT2[0].val = PLAINTEXT2val
        PLAINTEXT2[0].max = CIPHERTEXT[0].len
        PLAINTEXT2[0].len = CIPHERTEXT[0].len

        # 16 byte authentication tag
        TAG2 = ffi.new("octet*")
        TAG2val = ffi.new("char []", PAS)
        TAG2[0].val = TAG2val
        TAG2[0].max = PAS

        libmpin.MPIN_AES_GCM_DECRYPT(AES_KEY, IV, HEADER, CIPHERTEXT, PLAINTEXT2, TAG2)
        self.assertNotEqual(toHex(TAG1), toHex(TAG2))
        self.assertNotEqual(toHex(PLAINTEXT1), toHex(PLAINTEXT2))
        # print "Output message: %s" % ffi.string(PLAINTEXT2[0].val, PLAINTEXT2[0].len)

    def test_10(self):
        """test_10 AES-GCM: Failed encryption and decryption by changing a header byte"""

        # Generate 16 byte key
        key_val = os.urandom(PAS)
        AES_KEY = ffi.new("octet*")
        AES_KEYval = ffi.new("char [%s]" % PAS, key_val)
        AES_KEY[0].val = AES_KEYval
        AES_KEY[0].max = PAS
        AES_KEY[0].len = PAS

        # Generate 12 byte IV
        iv_val = os.urandom(IVL)
        IV = ffi.new("octet*")
        IVval = ffi.new("char [%s]" % IVL, iv_val)
        IV[0].val = IVval
        IV[0].max = IVL
        IV[0].len = IVL

        # Generate a 32 byte random header
        header_val = os.urandom(32)
        HEADER = ffi.new("octet*")
        HEADERval = ffi.new("char [%s]" % len(header_val), header_val)
        HEADER[0].val = HEADERval
        HEADER[0].max = len(header_val)
        HEADER[0].len = len(header_val)

        # Plaintext input
        plaintext1 = "A test message"
        PLAINTEXT1 = ffi.new("octet*")
        PLAINTEXT1val = ffi.new("char [%s]" % len(plaintext1), plaintext1)
        PLAINTEXT1[0].val = PLAINTEXT1val
        PLAINTEXT1[0].max = len(plaintext1)
        PLAINTEXT1[0].len = len(plaintext1)
        # print "Input message: %s" % ffi.string(PLAINTEXT1[0].val, PLAINTEXT1[0].len)

        # Ciphertext
        CIPHERTEXT = ffi.new("octet*")
        CIPHERTEXTval = ffi.new("char []", len(plaintext1))
        CIPHERTEXT[0].val = CIPHERTEXTval
        CIPHERTEXT[0].max = len(plaintext1)

        # 16 byte authentication tag
        TAG1 = ffi.new("octet*")
        TAG1val = ffi.new("char []",  PAS)
        TAG1[0].val = TAG1val
        TAG1[0].max = PAS

        libmpin.MPIN_AES_GCM_ENCRYPT(AES_KEY, IV, HEADER, PLAINTEXT1, CIPHERTEXT, TAG1)
        # Plaintext output
        PLAINTEXT2 = ffi.new("octet*")
        PLAINTEXT2val = ffi.new("char []", CIPHERTEXT[0].len)
        PLAINTEXT2[0].val = PLAINTEXT2val
        PLAINTEXT2[0].max = CIPHERTEXT[0].len
        PLAINTEXT2[0].len = CIPHERTEXT[0].len

        # Change one byte of header
        HEADER[0].val[0] = "\xa5"

        # 16 byte authentication tag
        TAG2 = ffi.new("octet*")
        TAG2val = ffi.new("char []", PAS)
        TAG2[0].val = TAG2val
        TAG2[0].max = PAS

        libmpin.MPIN_AES_GCM_DECRYPT(AES_KEY, IV, HEADER, CIPHERTEXT, PLAINTEXT2, TAG2)
        self.assertNotEqual(toHex(TAG1), toHex(TAG2))
        self.assertEqual(toHex(PLAINTEXT1), toHex(PLAINTEXT2))

if __name__ == '__main__':
    # Run tests
    unittest.main()
