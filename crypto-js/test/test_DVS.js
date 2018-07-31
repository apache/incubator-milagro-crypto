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

/* Test DVS - test driver and function exerciser for Designated Veifier Signature API Functions */

var chai = require('chai');

var CTX = require("../index");

pf_curves = ['BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'];

var expect = chai.expect;

for (var i = pf_curves.length - 1; i >= 0; i--) {

    var ctx = new CTX(pf_curves[i]);

    describe('TEST DVS ' + pf_curves[i], function() {

        var rng = new ctx.RAND();
        var sha = ctx.ECP.HASH_TYPE;

        before(function(done) {
            var RAW = [];
            rng.clean();
            for (i = 0; i < 100; i++) RAW[i] = i;
            rng.seed(100, RAW);
            done();
        });

        it('test Good Signature', function(done) {
            this.timeout(0);

            var res;

            var S = [];
            var SST = [];
            var TOKEN = [];
            var SEC = [];
            var xID = [];
            var X = [];
            var Y1 = [];
            var Y2 = [];
            var Z = [];
            var Pa = [];
            var U = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testuser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);

            /* Generate ctx.RANDom public key and z */
            res = ctx.MPIN.GET_DVS_KEYPAIR(rng, Z, Pa);
            expect(res).to.be.equal(0);

            /* Append Pa to ID */
            for (var i = 0; i < Pa.length; i++)
                CLIENT_ID.push(Pa[i]);

            /* Hash Client ID */
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);
            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Compute client secret for key escrow less scheme z.CS */
            res = ctx.MPIN.GET_G1_MULTIPLE(null, 0, Z, TOKEN, TOKEN);
            expect(res).to.be.equal(0);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            res = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(res).to.be.equal(0);

            var date = 0;
            var timeValue = ctx.MPIN.GET_TIME();

            var message = "Message to sign";

            res = ctx.MPIN.CLIENT(sha, 0, CLIENT_ID, rng, X, pin, TOKEN, SEC, U, null, null, timeValue, Y1, message);
            expect(res).to.be.equal(0);

            /* Server  */
            res = ctx.MPIN.SERVER(sha, 0, xID, null, Y2, SST, U, null, SEC, null, null, CLIENT_ID, timeValue, message, Pa);
            expect(res).to.be.equal(0);
            done();
        });

        it('test Bad Signature', function(done) {
            this.timeout(0);

            var res;

            var S = [];
            var SST = [];
            var TOKEN = [];
            var SEC = [];
            var xID = [];
            var X = [];
            var Y1 = [];
            var Y2 = [];
            var Z1 = [];
            var Z2 = [];
            var Pa1 = [];
            var Pa2 = [];
            var U = [];

            /* Trusted Authority set-up */
            ctx.MPIN.RANDOM_GENERATE(rng, S);

            /* Create Client Identity */
            var IDstr = "testuser@miracl.com";
            var CLIENT_ID = ctx.MPIN.stringtobytes(IDstr);

            /* Generate ctx.RANDom public key and z */
            res = ctx.MPIN.GET_DVS_KEYPAIR(rng, Z1, Pa1);
            expect(res).to.be.equal(0);

            /* Generate ctx.RANDom public key and z */
            res = ctx.MPIN.GET_DVS_KEYPAIR(rng, Z2, Pa2);
            expect(res).to.be.equal(0);

            /* Append Pa1 to ID */
            for (var i = 0; i < Pa1.length; i++)
                CLIENT_ID.push(Pa1[i]);

            /* Hash Client ID */
            HCID = ctx.MPIN.HASH_ID(sha, CLIENT_ID);

            /* Client and Server are issued secrets by DTA */
            ctx.MPIN.GET_SERVER_SECRET(S, SST);
            ctx.MPIN.GET_CLIENT_SECRET(S, HCID, TOKEN);

            /* Compute client secret for key escrow less scheme z.CS */
            res = ctx.MPIN.GET_G1_MULTIPLE(null, 0, Z1, TOKEN, TOKEN);
            expect(res).to.be.equal(0);

            /* Client extracts PIN from secret to create Token */
            var pin = 1234;
            res = ctx.MPIN.EXTRACT_PIN(sha, CLIENT_ID, pin, TOKEN);
            expect(res).to.be.equal(0);

            var date = 0;
            var timeValue = ctx.MPIN.GET_TIME();

            var message = "Message to sign";

            res = ctx.MPIN.CLIENT(sha, 0, CLIENT_ID, rng, X, pin, TOKEN, SEC, U, null, null, timeValue, Y1, message);
            expect(res).to.be.equal(0);

            /* Server  */
            res = ctx.MPIN.SERVER(sha, 0, xID, null, Y2, SST, U, null, SEC, null, null, CLIENT_ID, timeValue, message, Pa2);
            expect(res).to.be.equal(ctx.MPIN.BAD_PIN);
            done();
        });

    });

}