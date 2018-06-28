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

var CTXLIST = {
    "ED25519": {
        "BITS": "256",
        "FIELD": "25519",
        "CURVE": "ED25519",
        "@NB": 32,		/* Number of bytes in Modulus */
        "@BASE": 24,	/* Number base as power of 2 */
        "@NBT": 255,	/* Number of bits in modulus */
        "@M8": 5,		/* Modulus mod 8 */
        "@MT": 1,		/* Modulus Type (pseudo-mersenne,...) */
        "@CT": 1,		/* Curve Type (Weierstrass,...) */
        "@PF": 0,		/* Pairing Friendly */
        "@ST": 0,		/* Sextic Twist Type */
        "@SX": 0,		/* Sign of x parameter */
		"@HT": 32,		/* Hash output size */
		"@AK": 16		/* AES key size */
    },

    "C25519": {
        "BITS": "256",
        "FIELD": "25519",
        "CURVE": "C25519",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 255,
        "@M8": 5,
        "@MT": 1,
        "@CT": 2,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },


    "SECP256K1": {
        "BITS": "256",
        "FIELD": "SECP256K1",
        "CURVE": "SECP256K1",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 7,
        "@MT": 0,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "NIST256": {
        "BITS": "256",
        "FIELD": "NIST256",
        "CURVE": "NIST256",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 7,
        "@MT": 0,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "NIST384": {
        "BITS": "384",
        "FIELD": "NIST384",
        "CURVE": "NIST384",
        "@NB": 48,
        "@BASE": 23,
        "@NBT": 384,
        "@M8": 7,
        "@MT": 0,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 48,
		"@AK": 24
    },

    "BRAINPOOL": {
        "BITS": "256",
        "FIELD": "BRAINPOOL",
        "CURVE": "BRAINPOOL",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 7,
        "@MT": 0,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "ANSSI": {
        "BITS": "256",
        "FIELD": "ANSSI",
        "CURVE": "ANSSI",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 7,
        "@MT": 0,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "HIFIVE": {
        "BITS": "336",
        "FIELD": "HIFIVE",
        "CURVE": "HIFIVE",
        "@NB": 42,
        "@BASE": 23,
        "@NBT": 336,
        "@M8": 5,
        "@MT": 1,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 48,
		"@AK": 24
    },

    "GOLDILOCKS": {
        "BITS": "448",
        "FIELD": "GOLDILOCKS",
        "CURVE": "GOLDILOCKS",
        "@NB": 56,
        "@BASE": 23,
        "@NBT": 448,
        "@M8": 7,
        "@MT": 2,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },

    "C41417": {
        "BITS": "416",
        "FIELD": "C41417",
        "CURVE": "C41417",
        "@NB": 52,
        "@BASE": 22,
        "@NBT": 414,
        "@M8": 7,
        "@MT": 1,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },

    "NIST521": {
        "BITS": "528",
        "FIELD": "NIST521",
        "CURVE": "NIST521",
        "@NB": 66,
        "@BASE": 23,
        "@NBT": 521,
        "@M8": 7,
        "@MT": 1,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },

    "NUMS256W": {
        "BITS": "256",
        "FIELD": "256PM",
        "CURVE": "NUMS256W",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 3,
        "@MT": 1,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "NUMS256E": {
        "BITS": "256",
        "FIELD": "256PM",
        "CURVE": "NUMS256E",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 3,
        "@MT": 1,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "NUMS384W": {
        "BITS": "384",
        "FIELD": "384PM",
        "CURVE": "NUMS384W",
        "@NB": 48,
        "@BASE": 23,
        "@NBT": 384,
        "@M8": 3,
        "@MT": 1,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 48,
		"@AK": 24
    },

    "NUMS384E": {
        "BITS": "384",
        "FIELD": "384PM",
        "CURVE": "NUMS384E",
        "@NB": 48,
        "@BASE": 23,
        "@NBT": 384,
        "@M8": 3,
        "@MT": 1,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 48,
		"@AK": 24
    },

    "NUMS512W": {
        "BITS": "512",
        "FIELD": "512PM",
        "CURVE": "NUMS512W",
        "@NB": 64,
        "@BASE": 23,
        "@NBT": 512,
        "@M8": 7,
        "@MT": 1,
        "@CT": 0,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },

    "NUMS512E": {
        "BITS": "512",
        "FIELD": "512PM",
        "CURVE": "NUMS512E",
        "@NB": 64,
        "@BASE": 23,
        "@NBT": 512,
        "@M8": 7,
        "@MT": 1,
        "@CT": 1,
        "@PF": 0,
        "@ST": 0,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },

    "FP256BN": {
        "BITS": "256",
        "FIELD": "FP256BN",
        "CURVE": "FP256BN",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 256,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 1,
        "@ST": 1,
        "@SX": 1,
		"@HT": 32,
		"@AK": 16
    },

    "FP512BN": {
        "BITS": "512",
        "FIELD": "FP512BN",
        "CURVE": "FP512BN",
        "@NB": 64,
        "@BASE": 23,
        "@NBT": 512,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 1,
        "@ST": 1,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },

    "BN254": {
        "BITS": "256",
        "FIELD": "BN254",
        "CURVE": "BN254",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 254,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 1,
        "@ST": 0,
        "@SX": 1,
		"@HT": 32,
		"@AK": 16
    },

    "BN254CX": {
        "BITS": "256",
        "FIELD": "BN254CX",
        "CURVE": "BN254CX",
        "@NB": 32,
        "@BASE": 24,
        "@NBT": 254,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 1,
        "@ST": 0,
        "@SX": 1,
		"@HT": 32,
		"@AK": 16
    },

    "BLS383": {
        "BITS": "384",
        "FIELD": "BLS383",
        "CURVE": "BLS383",
        "@NB": 48,
        "@BASE": 23,
        "@NBT": 383,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 2,
        "@ST": 1,
        "@SX": 0,
		"@HT": 32,
		"@AK": 16
    },



    "BLS24": {
        "BITS": "480",
        "FIELD": "BLS24",
        "CURVE": "BLS24",
        "@NB": 60,
        "@BASE": 23,
        "@NBT": 479,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 3,
        "@ST": 1,
        "@SX": 0,
		"@HT": 48,
		"@AK": 24
    },


    "BLS48": {
        "BITS": "560",
        "FIELD": "BLS48",
        "CURVE": "BLS48",
        "@NB": 70,
        "@BASE": 23,
        "@NBT": 556,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 4,
        "@ST": 1,
        "@SX": 0,
		"@HT": 64,
		"@AK": 32
    },


    "BLS381": {
        "BITS": "381",
        "FIELD": "BLS381",
        "CURVE": "BLS381",
        "@NB": 48,
        "@BASE": 23,
        "@NBT": 381,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 2,
        "@ST": 1,
        "@SX": 1,
		"@HT": 32,
		"@AK": 16
    },

    "BLS461": {
        "BITS": "464",
        "FIELD": "BLS461",
        "CURVE": "BLS461",
        "@NB": 58,
        "@BASE": 23,
        "@NBT": 461,
        "@M8": 3,
        "@MT": 0,
        "@CT": 0,
        "@PF": 2,
        "@ST": 1,
        "@SX": 1,
		"@HT": 32,
		"@AK": 16
    },

    "RSA2048": {
        "BITS": "1024",
        "TFF": "2048",
        "@NB": 128,
        "@BASE": 22,
        "@ML": 2,
    },

    "RSA3072": {
        "BITS": "384",
        "TFF": "3072",
        "@NB": 48,
        "@BASE": 23,
        "@ML": 8,
    },

    "RSA4096": {
        "BITS": "512",
        "TFF": "4096",
        "@NB": 64,
        "@BASE": 23,
        "@ML": 8,
    },
};

var CTX = function(input_parameter) {

    this.AES = AES();
    this.GCM = GCM(this);
    this.UInt64 = UInt64();
    this.HASH256 = HASH256();
    this.HASH384 = HASH384(this);
    this.HASH512 = HASH512(this);
    this.SHA3 = SHA3(this);
    this.RAND = RAND(this);
    //this.NewHope = NewHope();
    this.NHS = NHS(this);

    if (input_parameter === undefined) {
        return;
    }

    this.config = CTXLIST[input_parameter];

    // Set BIG parameters
    this.BIG = BIG(this);
    this.DBIG = DBIG(this);

    // Set RSA parameters
    if (this.config['TFF'] !== undefined) {
        this.FF = FF(this);
        this.RSA = RSA(this);
        this.rsa_public_key = rsa_public_key(this);
        this.rsa_private_key = rsa_private_key(this);
        return;
    }

    // Set Elliptic Curve parameters
    if (this.config['CURVE'] !== undefined) {

        switch (this.config['CURVE']) {
            case "ED25519":
                this.ROM_CURVE = ROM_CURVE_ED25519();
                break;
            case "C25519":
                this.ROM_CURVE = ROM_CURVE_C25519();
                break;
            case "NIST256":
                this.ROM_CURVE = ROM_CURVE_NIST256();
                break;
            case "SECP256K1":
                this.ROM_CURVE = ROM_CURVE_SECP256K1();
                break;
            case "NIST384":
                this.ROM_CURVE = ROM_CURVE_NIST384();
                break;
            case "BRAINPOOL":
                this.ROM_CURVE = ROM_CURVE_BRAINPOOL();
                break;
            case "ANSSI":
                this.ROM_CURVE = ROM_CURVE_ANSSI();
                break;
            case "HIFIVE":
                this.ROM_CURVE = ROM_CURVE_HIFIVE();
                break;
            case "GOLDILOCKS":
                this.ROM_CURVE = ROM_CURVE_GOLDILOCKS();
                break;
            case "C41417":
                this.ROM_CURVE = ROM_CURVE_C41417();
                break;
            case "NIST521":
                this.ROM_CURVE = ROM_CURVE_NIST521();
                break;
            case "NUMS256W":
                this.ROM_CURVE = ROM_CURVE_NUMS256W();
                break;
            case "NUMS256E":
                this.ROM_CURVE = ROM_CURVE_NUMS256E();
                break;
            case "NUMS384W":
                this.ROM_CURVE = ROM_CURVE_NUMS384W();
                break;
            case "NUMS384E":
                this.ROM_CURVE = ROM_CURVE_NUMS384E();
                break;
            case "NUMS512W":
                this.ROM_CURVE = ROM_CURVE_NUMS512W();
                break;
            case "NUMS512E":
                this.ROM_CURVE = ROM_CURVE_NUMS512E();
                break;
            case "FP256BN":
                this.ROM_CURVE = ROM_CURVE_FP256BN();
                break;
            case "FP512BN":
                this.ROM_CURVE = ROM_CURVE_FP512BN();
                break;
            case "BN254":
                this.ROM_CURVE = ROM_CURVE_BN254();
                break;
            case "BN254CX":
                this.ROM_CURVE = ROM_CURVE_BN254CX();
                break;
            case "BLS383":
                this.ROM_CURVE = ROM_CURVE_BLS383();
                break;
            case "BLS381":
                this.ROM_CURVE = ROM_CURVE_BLS381();
                break;

            case "BLS24":
                this.ROM_CURVE = ROM_CURVE_BLS24();
                break;

            case "BLS48":
                this.ROM_CURVE = ROM_CURVE_BLS48();
                break;

            case "BLS461":
                this.ROM_CURVE = ROM_CURVE_BLS461();
                break;
            default:
                this.ROM_CURVE = undefined;
        }


        switch (this.config['FIELD']) {
            case "25519":
                this.ROM_FIELD = ROM_FIELD_25519();
                break;
            case "NIST256":
                this.ROM_FIELD = ROM_FIELD_NIST256();
                break;
            case "SECP256K1":
                this.ROM_FIELD = ROM_FIELD_SECP256K1();
                break;
            case "NIST384":
                this.ROM_FIELD = ROM_FIELD_NIST384();
                break;
            case "BRAINPOOL":
                this.ROM_FIELD = ROM_FIELD_BRAINPOOL();
                break;
            case "ANSSI":
                this.ROM_FIELD = ROM_FIELD_ANSSI();
                break;
            case "HIFIVE":
                this.ROM_FIELD = ROM_FIELD_HIFIVE();
                break;
            case "GOLDILOCKS":
                this.ROM_FIELD = ROM_FIELD_GOLDILOCKS();
                break;
            case "C41417":
                this.ROM_FIELD = ROM_FIELD_C41417();
                break;
            case "NIST521":
                this.ROM_FIELD = ROM_FIELD_NIST521();
                break;
            case "256PM":
                this.ROM_FIELD = ROM_FIELD_256PM();
                break;
            case "384PM":
                this.ROM_FIELD = ROM_FIELD_384PM();
                break;
            case "512PM":
                this.ROM_FIELD = ROM_FIELD_512PM();
                break;
            case "FP256BN":
                this.ROM_FIELD = ROM_FIELD_FP256BN();
                break;
            case "FP512BN":
                this.ROM_FIELD = ROM_FIELD_FP512BN();
                break;
            case "BN254":
                this.ROM_FIELD = ROM_FIELD_BN254();
                break;
            case "BN254CX":
                this.ROM_FIELD = ROM_FIELD_BN254CX();
                break;
            case "BLS383":
                this.ROM_FIELD = ROM_FIELD_BLS383();
                break;

            case "BLS24":
                this.ROM_FIELD = ROM_FIELD_BLS24();
                break;

            case "BLS48":
                this.ROM_FIELD = ROM_FIELD_BLS48();
                break;


            case "BLS381":
                this.ROM_FIELD = ROM_FIELD_BLS381();
                break;
            case "BLS461":
                this.ROM_FIELD = ROM_FIELD_BLS461();
                break;
            default:
                this.ROM_FIELD = undefined;
        }

        this.FP = FP(this);
        this.ECP = ECP(this);
        this.ECDH = ECDH(this);

        if (this.config['@PF'] == 1   || this.config['@PF'] == 2) {
            this.FP2 = FP2(this);
            this.FP4 = FP4(this);
            this.FP12 = FP12(this);
            this.ECP2 = ECP2(this);
            this.PAIR = PAIR(this);
            this.MPIN = MPIN(this);
        }

        if (this.config['@PF'] == 3) {
            this.FP2 = FP2(this);
            this.FP4 = FP4(this);
			this.FP8 = FP8(this);
            this.FP24 = FP24(this);
            this.ECP4 = ECP4(this);
            this.PAIR192 = PAIR192(this);
            this.MPIN192 = MPIN192(this);
        }

        if (this.config['@PF'] == 4) {
            this.FP2 = FP2(this);
            this.FP4 = FP4(this);
			this.FP8 = FP8(this);
			this.FP16 = FP16(this);
            this.FP48 = FP48(this);
            this.ECP8 = ECP8(this);
            this.PAIR256 = PAIR256(this);
            this.MPIN256 = MPIN256(this);
        }

        return;
    }

};
