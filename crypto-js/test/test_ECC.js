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


/* Test ECC - test driver and function exerciser for ECDH/ECIES/ECDSA API Functions */

var chai = require('chai');

var CTX = require("../index");

var expect = chai.expect;

var all_curves = ['ED25519', 'GOLDILOCKS', 'NIST256', 'BRAINPOOL', 'ANSSI', 'HIFIVE', 'C25519', 'NIST384', 'C41417', 'NIST521', 'NUMS256W',
    'NUMS256E', 'NUMS384W', 'NUMS384E', 'NUMS512W', 'NUMS512E', 'BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'
];

for (var i = all_curves.length - 1; i >= 0; i--) {

    var ctx = new CTX(all_curves[i]);

    describe('TEST ECC ' + all_curves[i], function() {

        var i = 0,
            j = 0,
            res;
        var result;
        var pp = "M0ng00se";
        var EGS = ctx.ECDH.EGS;
        var EFS = ctx.ECDH.EFS;
        var EAS = ctx.ECP.AESKEY;
        var sha = ctx.ECP.HASH_TYPE;
        var S1 = [];
        var W0 = [];
        var W1 = [];
        var Z0 = [];
        var Z1 = [];
        var RAW = [];
        var SALT = [];
        var P1 = [];
        var P2 = [];
        var V = [];
        var M = [];
        var CS = [];
        var DS = [];
        var S0;
        var rng = new ctx.RAND();
        var T = new Array(12); // must specify required length
        var PW;
        var KEY;
        var C;

        before(function(done) {
            this.timeout(0);
            rng.clean();
            for (i = 0; i < 100; i++) RAW[i] = i;
            rng.seed(100, RAW);
            for (i = 0; i < 8; i++) SALT[i] = (i + 1); // set Salt
            PW = ctx.ECDH.stringtobytes(pp);
            // private key S0 of size EGS bytes derived from Password and Salt 
            S0 = ctx.ECDH.PBKDF2(sha, PW, SALT, 1000, EGS);
            done();
        });


        it('test ECDH', function(done) {
            this.timeout(0);

            // Generate Key pair S/W 
            ctx.ECDH.KEY_PAIR_GENERATE(null, S0, W0);


            res = ctx.ECDH.PUBLIC_KEY_VALIDATE(W0);
            expect(res).to.be.equal(0);
            // Random private key for other party 
            ctx.ECDH.KEY_PAIR_GENERATE(rng, S1, W1);

            res = ctx.ECDH.PUBLIC_KEY_VALIDATE(W1);
            expect(res).to.be.equal(0);

            // Calculate common key using DH - IEEE 1363 method 

            ctx.ECDH.ECPSVDP_DH(S0, W1, Z0);
            ctx.ECDH.ECPSVDP_DH(S1, W0, Z1);

            var same = true;
            for (i = 0; i < ctx.ECDH.EFS; i++)
                if (Z0[i] != Z1[i]) same = false;


            KEY = ctx.ECDH.KDF2(sha, Z0, null, ctx.ECDH.EAS);

            expect(same).to.be.equal(true);
            done();
        });

        if (ctx.ECP.CURVETYPE != ctx.ECP.MONTGOMERY) {
            it('test ECIES', function(done) {
                this.timeout(0);
                P1[0] = 0x0;
                P1[1] = 0x1;
                P1[2] = 0x2;
                P2[0] = 0x0;
                P2[1] = 0x1;
                P2[2] = 0x2;
                P2[3] = 0x3;

                for (i = 0; i <= 16; i++) M[i] = i;

                C = ctx.ECDH.ECIES_ENCRYPT(sha, P1, P2, rng, W1, M, V, T);

                M = ctx.ECDH.ECIES_DECRYPT(sha, P1, P2, V, C, T, S1);

                expect(M.length).to.not.equal(0);

                done();
            });

            it('test ECDSA', function(done) {
                this.timeout(0);
                expect(ctx.ECDH.ECPSP_DSA(sha, rng, S0, M, CS, DS)).to.be.equal(0);
                expect(ctx.ECDH.ECPVP_DSA(sha, W0, M, CS, DS)).to.be.equal(0);
                done();
            });
        }
    });
}