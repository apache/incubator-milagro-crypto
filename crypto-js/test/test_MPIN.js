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

/* Test MPIN - test driver and function exerciser for MPIN API Functions */

var CTX = require("../index");

var chai = require('chai');

var expect = chai.expect;

// Curves for consistency test
var pf_curves = ['BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'];

// Curves for test with test vectors
var tv_curves = ['BN254CX'];

hextobytes = function(value_hex) {
    // "use strict";
    var len, byte_value, i;

    len = value_hex.length;
    byte_value = [];

    for (i = 0; i < len; i += 2) {
        byte_value[(i / 2)] = parseInt(value_hex.substr(i, 2), 16);
    }
    return byte_value;
};

for (var i = pf_curves.length - 1; i >= 0; i--) {

    describe('TEST MPIN ' + pf_curves[i], function() {

        var ctx = new CTX(pf_curves[i]);

        var rng = new ctx.RAND();

        var j = i;

        before(function(done) {
            var RAW = [];
            rng.clean();
            for (i = 0; i < 100; i++) RAW[i] = i;
            rng.seed(100, RAW);
            done();
        });

        it('test MPin', function(done) {
            this.timeout(0);

            var i, res;
            var result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var X = [];
            var Y = [];
            var HCID = [];
            var HID = [];

            var G1 = [];
            var G2 = [];
            var R = [];
            var Z = [];
            var W = [];
            var T = [];
            var CK = [];
            var SK = [];

            var HSID = [];

            /* Set configuration */
            var PINERROR = true;

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);

            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            rtn=rtn+3
            expect(rtn).to.be.equal(3);

            var date = 0;
            pin = 1234;

            var pxID = xID;
            var pHID = HID;

            var prHID;
            prHID = pHID;

            rtn = ctx.MPIN.CLIENT_1(sha, date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, null, null);
            rtn = rtn+1
            expect(rtn).to.be.equal(1);

            /* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
            ctx.MPIN.SERVER_1(sha, date, CLIENT_ID, pHID, null);

            /* Server generates Random number Y and sends it to Client */
            ctx.MPIN.RANDOM_GENERATE(rng, Y);

            /* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
            rtn = ctx.MPIN.CLIENT_2(X, Y, SEC);
            rtn = rtn+2
            expect(rtn).to.be.equal(2);

            /* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
            /* If PIN error not required, set E and F = NULL */
            rtn = ctx.MPIN.SERVER_2(date, pHID, null, Y, SST, pxID, null, SEC, null, null);

            rtn = rtn+4
            expect(rtn).to.be.equal(4);
            done();
        });

        it('test MPin Time Permits', function(done) {
            this.timeout(0);
            var i, res;
            var result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var xCID = [];
            var X = [];
            var Y = [];
            var HCID = [];
            var HID = [];
            var HTID = [];

            var HSID = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);
            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(rtn).to.be.equal(0);

            var date = ctx.MPIN.today();
            /* Client gets "Time Token" permit from DTA */
            ctx.MPIN.GET_CLIENT_PERMIT(sha, date, S, HCID, PERMIT);

            /* This encoding makes Time permit look random - Elligator squared */
            ctx.MPIN.ENCODING(rng, PERMIT);
            ctx.MPIN.DECODING(PERMIT);

            pin = 1234;

            var pxCID = xCID;
            var pHID = HID;
            var pHTID = HTID;
            var pPERMIT = PERMIT;
            var prHID;

            prHID = pHTID;

            pxID = null;

            rtn = ctx.MPIN.CLIENT_1(sha, date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT);
            expect(rtn).to.be.equal(0);

            /* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
            ctx.MPIN.SERVER_1(sha, date, CLIENT_ID, pHID, pHTID);

            /* Server generates Random number Y and sends it to Client */
            ctx.MPIN.RANDOM_GENERATE(rng, Y);

            /* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
            rtn = ctx.MPIN.CLIENT_2(X, Y, SEC);
            expect(rtn).to.be.equal(0);

            /* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
            /* If PIN error not required, set E and F = NULL */
            rtn = ctx.MPIN.SERVER_2(date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, null, null);
            expect(rtn).to.be.equal(0);

            done();

        });

        it('test MPin Full One Pass', function(done) {
            this.timeout(0);
            var i, res, result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var xCID = [];
            var X = [];
            var Y = [];
            var E = [];
            var F = [];
            var HCID = [];
            var HID = [];
            var HTID = [];

            var G1 = [];
            var G2 = [];
            var R = [];
            var Z = [];
            var W = [];
            var T = [];
            var CK = [];
            var SK = [];

            var HSID = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);

            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(rtn).to.be.equal(0);

            ctx.MPIN.PRECOMPUTE(TOKEN, HCID, G1, G2);

            var date = 0;

            pin = 1234;

            var pxID = xID;
            var pxCID = xCID;
            var pHID = HID;
            var pHTID = HTID;
            var pE = E;
            var pF = F;
            var pPERMIT = PERMIT;
            var prHID;

            prHID = pHID;
            pPERMIT = null;
            pxCID = null;
            pHTID = null;

            pE = null;
            pF = null;

            timeValue = ctx.MPIN.GET_TIME();

            rtn = ctx.MPIN.CLIENT(sha, date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT, timeValue, Y);
            expect(rtn).to.be.equal(0);

            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */

            rtn = ctx.MPIN.SERVER(sha, date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, pE, pF, CLIENT_ID, timeValue);
            expect(rtn).to.be.equal(0);

            HSID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 0, W, prHID, T); /* Also send T=w.ID to client, remember random w  */

            H = ctx.MPIN.HASH_ALL(sha, HCID, pxID, pxCID, SEC, Y, Z, T);
            ctx.MPIN.CLIENT_KEY(sha, G1, G2, pin, R, X, H, T, CK);

            H = ctx.MPIN.HASH_ALL(sha, HSID, pxID, pxCID, SEC, Y, Z, T);
            ctx.MPIN.SERVER_KEY(sha, Z, SST, W, H, pHID, pxID, pxCID, SK);
            expect(ctx.MPIN.bytestostring(CK)).to.be.equal(ctx.MPIN.bytestostring(SK));

            done();
        });

		it('test MPin bad token', function(done) {
            this.timeout(0);
            var i, res, result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var T = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var xCID = [];
            var X = [];
            var Y = [];
            var HCID = [];
            var HID = [];
            var HTID = [];

            var G1 = [];
            var G2 = [];
            var R = [];
            var Z = [];
            var W = [];
            var T = [];
            var CK = [];
            var SK = [];

            var HSID = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);
            ctx.MPIN.RANDOM_GENERATE(rng, T);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);

            ctx.MPIN.GET_CLIENT_SECRET(T, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(rtn).to.be.equal(0);

            ctx.MPIN.PRECOMPUTE(TOKEN, HCID, G1, G2);

            var date = 0;

            pin = 1234;

            var pxID = xID;
            var pxCID = xCID;
            var pHID = HID;
            var pHTID = HTID;
            var pPERMIT = PERMIT;
            var prHID;

            prHID = pHID;
            pPERMIT = null;
            pxCID = null;
            pHTID = null;

            timeValue = ctx.MPIN.GET_TIME();

            rtn = ctx.MPIN.CLIENT(sha, date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT, timeValue, Y);
            expect(rtn).to.be.equal(0);

            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */

            rtn = ctx.MPIN.SERVER(sha, date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, null, null, CLIENT_ID, timeValue);
            expect(rtn).to.be.equal(ctx.MPIN.BAD_PIN);

            done();
        });

      	it('test MPin bad PIN', function(done) {
            this.timeout(0);
            var i, res, result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var xCID = [];
            var X = [];
            var Y = [];
            var E = [];
            var F = [];
            var HCID = [];
            var HID = [];
            var HTID = [];

            var G1 = [];
            var G2 = [];
            var R = [];
            var Z = [];
            var W = [];
            var T = [];
            var CK = [];
            var SK = [];

            var HSID = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);

            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin1 = 5555;
            var pin2 = 4444;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin1, TOKEN);
            expect(rtn).to.be.equal(0);

            ctx.MPIN.PRECOMPUTE(TOKEN, HCID, G1, G2);

            var date = 0;

            var pxID = xID;
            var pxCID = xCID;
            var pHID = HID;
            var pHTID = HTID;
            var pPERMIT = PERMIT;
            var prHID;

            prHID = pHID;
            pPERMIT = null;
            pxCID = null;
            pHTID = null;

            timeValue = ctx.MPIN.GET_TIME();

            rtn = ctx.MPIN.CLIENT(sha, date, CLIENT_ID, rng, X, pin2, TOKEN, SEC, pxID, pxCID, pPERMIT, timeValue, Y);
            expect(rtn).to.be.equal(0);

            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */

            rtn = ctx.MPIN.SERVER(sha, date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, E, F, CLIENT_ID, timeValue);
            expect(rtn).to.be.equal(ctx.MPIN.BAD_PIN);

            // Retrieve PIN error
            rtn = ctx.MPIN.KANGAROO(E,F);
            expect(rtn).to.be.equal(pin2-pin1);

            done();
        });

        it('test MPin FUll Two Pass', function(done) {
            this.timeout(0);
            var i, res;
            var result;

            var EGS = ctx.MPIN.EGS;
            var EFS = ctx.MPIN.EFS;
            var EAS = ctx.ECP.AESKEY;

            var sha = ctx.ECP.HASH_TYPE;

            var G1S = 2 * EFS + 1; /* Group 1 Size */
            var G2S = 4 * EFS; /* Group 2 Size */

            var S = [];
            var SST = [];
            var TOKEN = [];
            var PERMIT = [];
            var SEC = [];
            var xID = [];
            var xCID = [];
            var X = [];
            var Y = [];
            var E = [];
            var F = [];
            var HCID = [];
            var HID = [];
            var HTID = [];

            var G1 = [];
            var G2 = [];
            var R = [];
            var Z = [];
            var W = [];
            var T = [];
            var CK = [];
            var SK = [];

            var HSID = [];

            /* Set configuration */
            var PERMITS = true;

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testUser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID); /* Either Client or TA calculates Hash(ID) - you decide! */

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);
            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            var rtn = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(rtn).to.be.equal(0);

            ctx.MPIN.PRECOMPUTE(TOKEN, HCID, G1, G2);

            var date;
            if (PERMITS) {
                date = ctx.MPIN.today();
                /* Client gets "Time Token" permit from DTA */
                ctx.MPIN.GET_CLIENT_PERMIT(sha, date, S, HCID, PERMIT);

                /* This encoding makes Time permit look random - Elligator squared */
                ctx.MPIN.ENCODING(rng, PERMIT);
                ctx.MPIN.DECODING(PERMIT);
            } else date = 0;

            pin = 1234;

            var pxID = xID;
            var pxCID = xCID;
            var pHID = HID;
            var pHTID = HTID;
            var pE = E;
            var pF = F;
            var pPERMIT = PERMIT;
            var prHID;

            if (date != 0) {
                prHID = pHTID;
                pxID = null;
            } else {
                prHID = pHID;
                pPERMIT = null;
                pxCID = null;
                pHTID = null;
            }
            pE = null;
            pF = null;

            rtn = ctx.MPIN.CLIENT_1(sha, date, CLIENT_ID, rng, X, pin, TOKEN, SEC, pxID, pxCID, pPERMIT);
            expect(rtn).to.be.equal(0);

            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 1, R, HCID, Z); /* Also Send Z=r.ID to Server, remember random r */

            /* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
            ctx.MPIN.SERVER_1(sha, date, CLIENT_ID, pHID, pHTID);

            /* Server generates Random number Y and sends it to Client */
            ctx.MPIN.RANDOM_GENERATE(rng, Y);

            HSID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);
            ctx.MPIN.GET_G1_MULTIPLE(rng, 0, W, prHID, T); /* Also send T=w.ID to client, remember random w  */

            /* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
            rtn = ctx.MPIN.CLIENT_2(X, Y, SEC);
            expect(rtn).to.be.equal(0);

            /* Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error. */
            /* If PIN error not required, set E and F = NULL */
            rtn = ctx.MPIN.SERVER_2(date, pHID, pHTID, Y, SST, pxID, pxCID, SEC, pE, pF);
            expect(rtn).to.be.equal(0);

            H = ctx.MPIN.HASH_ALL(sha, HCID, pxID, pxCID, SEC, Y, Z, T);
            ctx.MPIN.CLIENT_KEY(sha, G1, G2, pin, R, X, H, T, CK);

            H = ctx.MPIN.HASH_ALL(sha, HSID, pxID, pxCID, SEC, Y, Z, T);
            ctx.MPIN.SERVER_KEY(sha, Z, SST, W, H, pHID, pxID, pxCID, SK);
            expect(ctx.MPIN.bytestostring(CK)).to.be.equal(ctx.MPIN.bytestostring(SK));

            done();
        });

      if (tv_curves.indexOf(pf_curves[i]) != -1) {
        it('test Combine Shares in G1 ' + pf_curves[i] + ' with Test Vectors', function(done) {
            this.timeout(0);
            // Load test vectors
            var vectors = require('../testVectors/mpin/MPIN_' + pf_curves[j] + '.json');

            var sha = ctx.ECP.HASH_TYPE;
            var CS = [];
            var TP = [];
            var TP1bytes = [];
            var TP2bytes = [];
            var TPbytes = [];
            var CS1bytes = [];
            var CS2bytes = [];
            var CSbytes = [];

            for (var vector in vectors) {

                  CS1bytes = hextobytes(vectors[vector].CS1);
                  CS2bytes = hextobytes(vectors[vector].CS2);
                  CSbytes = hextobytes(vectors[vector].CLIENT_SECRET);
                  ctx.MPIN.RECOMBINE_G1(CS1bytes, CS2bytes, CS);
                  expect(ctx.MPIN.comparebytes(CS,CSbytes)).to.be.equal(true);

                  TP1bytes = hextobytes(vectors[vector].TP1);
                  TP2bytes = hextobytes(vectors[vector].TP2);
                  TPbytes = hextobytes(vectors[vector].TIME_PERMIT);
                  ctx.MPIN.RECOMBINE_G1(TP1bytes, TP2bytes, TP);
                  expect(ctx.MPIN.comparebytes(TP,TPbytes)).to.be.equal(true);
            }
            done();
        });

        it('test MPin Two Passes ' + pf_curves[i] + ' with Test Vectors', function(done) {
            this.timeout(0);
            // Load test vectors
            var vectors = require('../testVectors/mpin/MPIN_' + pf_curves[j] + '.json');

            var sha = ctx.ECP.HASH_TYPE;
            var xID = [];
            var xCID = [];
            var SEC = [];
            var Y = [];

            for (var vector in vectors) {
                var rtn = ctx.MPIN.CLIENT_1(sha, vectors[vector].DATE, hextobytes(vectors[vector].MPIN_ID_HEX), null, hextobytes(vectors[vector].X), vectors[vector].PIN2, hextobytes(vectors[vector].TOKEN), SEC, xID, xCID, hextobytes(vectors[vector].TIME_PERMIT));
                expect(rtn).to.be.equal(0);
                expect(ctx.MPIN.bytestostring(xID)).to.be.equal(vectors[vector].U);
                expect(ctx.MPIN.bytestostring(xCID)).to.be.equal(vectors[vector].UT);

                var rtn = ctx.MPIN.CLIENT_2(hextobytes(vectors[vector].X), hextobytes(vectors[vector].Y), SEC);
                expect(rtn).to.be.equal(0);
                expect(ctx.MPIN.bytestostring(SEC)).to.be.equal(vectors[vector].V);
            }
            done();
        });

        it('test MPin One Pass ' + pf_curves[i] + ' with Test Vectors', function(done) {
            this.timeout(0);
            // Load test vectors
            var vectors = require('../testVectors/mpin/MPIN_ONE_PASS_' + pf_curves[j] + '.json');

            var sha = ctx.ECP.HASH_TYPE;
            var xID = [];
            var SEC = [];
            var Y = [];

            for (var vector in vectors) {
                var rtn = ctx.MPIN.CLIENT(sha, 0, hextobytes(vectors[vector].MPIN_ID_HEX), null, hextobytes(vectors[vector].X), vectors[vector].PIN2, hextobytes(vectors[vector].TOKEN), SEC, xID, null, null, vectors[vector].TimeValue, Y);
                expect(rtn).to.be.equal(0);
                expect(ctx.MPIN.bytestostring(xID)).to.be.equal(vectors[vector].U);
                expect(ctx.MPIN.bytestostring(SEC)).to.be.equal(vectors[vector].SEC);
            }
            done();
        });

      }

    });
}