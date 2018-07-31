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

var pf_curves = ['BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'];

for (var i = pf_curves.length - 1; i >= 0; i--) {

    describe('TEST PAIR ' + pf_curves[i], function() {

        var ctx = new CTX(pf_curves[i]);
        var rng = new ctx.RAND();

        var j = i;

        var r = new ctx.BIG(0);
        var x = new ctx.BIG(0);
        var y = new ctx.BIG(0);

        var G = new ctx.ECP(0);
        var G1 = new ctx.ECP(0);
        var G2 = new ctx.ECP(0);
        var G3 = new ctx.ECP(0);

        var Q = new ctx.ECP2(0);
        var Q1 = new ctx.ECP2(0);
        var Q2 = new ctx.ECP2(0);
        var Q3 = new ctx.ECP2(0);

        var g1 = new ctx.FP12(0);
        var g2 = new ctx.FP12(0);
        var g3 = new ctx.FP12(0);

        var qx = new ctx.FP2(0);
        var qy = new ctx.FP2(0);

        // Set curve order
        r.rcopy(ctx.ROM_CURVE.CURVE_Order);

        // Set generator of G1
        x.rcopy(ctx.ROM_CURVE.CURVE_Gx);
        y.rcopy(ctx.ROM_CURVE.CURVE_Gy);
        G.setxy(x,y);

        // Set generator of G2
        x.rcopy(ctx.ROM_CURVE.CURVE_Pxa);
        y.rcopy(ctx.ROM_CURVE.CURVE_Pxb);
        qx.bset(x, y);
        x.rcopy(ctx.ROM_CURVE.CURVE_Pya);
        y.rcopy(ctx.ROM_CURVE.CURVE_Pyb);
        qy.bset(x, y);
        Q.setxy(qx, qy);



        before(function(done) {
            var RAW = [];
            rng.clean();
            for (i = 0; i < 100; i++) RAW[i] = i;
            rng.seed(100, RAW);
            done();
        });

        // Test that e(sQ,G) = e(Q,sG) = e(Q,G)^s, s random
        it('test Bilinearity 1', function(done) {
            this.timeout(0);

            for (var k = 3 ; k > 0; k--) {
                x = ctx.BIG.randomnum(r,rng);
                y = ctx.BIG.randomnum(r,rng);
                s = ctx.BIG.randomnum(r,rng);
                G1 = ctx.PAIR.G1mul(G,x);
                Q1 = ctx.PAIR.G2mul(Q,y);
                G2 = ctx.PAIR.G1mul(G1,s);
                Q2 = ctx.PAIR.G2mul(Q1,s);
                
                g1 = ctx.PAIR.ate(Q1, G2);
                g1 = ctx.PAIR.fexp(g1);
                g2 = ctx.PAIR.ate(Q2, G1);
                g2 = ctx.PAIR.fexp(g2);

                expect(g1.toString()).to.be.equal(g2.toString());
                
                g2 = ctx.PAIR.ate(Q1, G1);
                g2 = ctx.PAIR.fexp(g2);
                g2 = ctx.PAIR.GTpow(g2,s);

                expect(g1.toString()).to.be.equal(g2.toString());
            }
            done();
        });

        // Test that e(Q1+Q2,G1) = e(Q1,G1).e(Q2,G1) and e(Q1,G1+G2) = e(Q1,G1).e(Q1,G2)
        it('test Bilinearity 2', function(done) {
            this.timeout(0);

            for (var k = 3; k > 0; k--) {
                x = ctx.BIG.randomnum(r,rng);
                y = ctx.BIG.randomnum(r,rng);
                G1 = ctx.PAIR.G1mul(G,x);
                Q1 = ctx.PAIR.G2mul(Q,y);
                x = ctx.BIG.randomnum(r,rng);
                y = ctx.BIG.randomnum(r,rng);
                G2 = ctx.PAIR.G1mul(G,x);
                Q2 = ctx.PAIR.G2mul(Q,y);

                g2 = ctx.PAIR.ate(Q1, G1);
                g2 = ctx.PAIR.fexp(g2);
                g3 = ctx.PAIR.ate(Q1, G2);
                g3 = ctx.PAIR.fexp(g3);
                g2.mul(g3);

                G2.add(G1);
                G2.affine();

                g1 = ctx.PAIR.ate(Q1, G2);
                g1 = ctx.PAIR.fexp(g1);

                expect(g1.toString()).to.be.equal(g2.toString());

                g2 = ctx.PAIR.ate(Q1, G1);
                g2 = ctx.PAIR.fexp(g2);
                g3 = ctx.PAIR.ate(Q2, G1);
                g3 = ctx.PAIR.fexp(g3);
                g2.mul(g3);

                Q2.add(Q1);
                Q2.affine();

                g1 = ctx.PAIR.ate(Q2, G1);
                g1 = ctx.PAIR.fexp(g1);

                expect(g1.toString()).to.be.equal(g2.toString());
            }
            done();
        });

        // Test that e(Q1+Q2,G1+G2) = e(Q1,G1).e(Q2,G1).e(Q1,G2).e(Q2,G2)
        it('test Double Pairing', function(done) {
            this.timeout(0);

            for (var k = 3; k > 0; k--) {
                x = ctx.BIG.randomnum(r,rng);
                y = ctx.BIG.randomnum(r,rng);
                G1 = ctx.PAIR.G1mul(G,x);
                Q1 = ctx.PAIR.G2mul(Q,y);
                x = ctx.BIG.randomnum(r,rng);
                y = ctx.BIG.randomnum(r,rng);
                G2 = ctx.PAIR.G1mul(G,x);
                Q2 = ctx.PAIR.G2mul(Q,y);

                g1 = ctx.PAIR.ate(Q1, G1);
                g1 = ctx.PAIR.fexp(g1);
                g2 = ctx.PAIR.ate(Q2, G2);
                g2 = ctx.PAIR.fexp(g2);
                g1.mul(g2);

                g2 = ctx.PAIR.ate2(Q1,G1,Q2,G2);
                g2 = ctx.PAIR.fexp(g2);

                expect(g1.toString()).to.be.equal(g2.toString());
            }
            done();
        });


    });
}