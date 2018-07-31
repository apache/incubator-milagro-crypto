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


/* Test FP2 ARITHMETICS - test driver and function exerciser for FP2 API Functions */

var chai = require('chai');

var CTX = require("../index");

var expect = chai.expect;

var pf_curves = ['BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'];

var readScalar = function(string, ctx) {

    while (string.length != ctx.BIG.MODBYTES*2) string = "00"+string;

    return ctx.BIG.fromBytes(new Buffer(string, "hex"));

}

var readFP2 = function(string, ctx) {

    string = string.split(",");
    var cox = string[0].slice(1);
    var coy = string[1].slice(0,-1);
    var fp2 = new ctx.FP2(0);

    while (cox.length != ctx.BIG.MODBYTES*2) cox = "00"+cox;
    while (coy.length != ctx.BIG.MODBYTES*2) coy = "00"+coy;

    var bigx = ctx.BIG.fromBytes(new Buffer(cox, "hex"));
    var bigy = ctx.BIG.fromBytes(new Buffer(coy, "hex"));
    fp2.bset(bigx,bigy);

    return fp2;
}

describe('TEST FP2 ARITHMETIC', function() {

    var j =0;

    for (var i = 0; i < pf_curves.length; i++) {


        it('test '+pf_curves[i], function(done) {
            this.timeout(0);
            var curve = pf_curves[j];
            j++;
            var ctx = new CTX(curve);

            var vectors = require('../testVectors/fp2/'+curve+'.json');

            for (var k = 0; k < vectors.length; k++) {

                // test commutativity of addition
                var fp21 = readFP2(vectors[k].FP21,ctx);
                var fp22 = readFP2(vectors[k].FP22,ctx);
                var fp2add = readFP2(vectors[k].FP2add,ctx);
                var a1 = new ctx.FP2(0);
                var a2 = new ctx.FP2(0);
                a1.copy(fp21);
                a2.copy(fp22);
                a1.add(a2);
                expect(a1.toString()).to.equal(fp2add.toString());
                a1.copy(fp21);
                a2.add(a1);
                expect(a2.toString()).to.equal(fp2add.toString());

                // test associativity of addition
                a2.add(fp2add);
                a1.copy(fp21);
                a1.add(fp2add);
                a1.add(fp22);
                expect(a1.toString()).to.equal(a2.toString());

                // test subtraction
                var fp2sub = readFP2(vectors[k].FP2sub, ctx);
                a1.copy(fp21);
                a2.copy(fp22);
                a1.sub(a2);
                a1.reduce();
                expect(a1.toString()).to.equal(fp2sub.toString());

                // test negative of a FP2
                var fp2neg = readFP2(vectors[k].FP2neg, ctx);
                a1.copy(fp21);
                a1.neg();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2neg.toString());

                // test conjugate of a FP2
                var fp2conj = readFP2(vectors[k].FP2conj, ctx);
                a1.copy(fp21);
                a1.conj();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2conj.toString());

                // test scalar multiplication
                var fp2pmul = readFP2(vectors[k].FP2pmul, ctx);
                var scalar = readScalar(vectors[k].BIGsc, ctx);
                var fpsc = new ctx.FP(0);
                fpsc.bcopy(scalar);
                a1.copy(fp21);
                a1.pmul(fpsc);
                a1.reduce();
                expect(a1.toString()).to.equal(fp2pmul.toString());

                // test small scalar multiplication
                var fp2imul = readFP2(vectors[k].FP2imul, ctx);
                a1.copy(fp21);
                a1.imul(k);
                a1.reduce();
                expect(a1.toString()).to.equal(fp2imul.toString());

                // test square and square root
                var fp2sqr = readFP2(vectors[k].FP2sqr, ctx);
                a1.copy(fp21);
                a1.sqr();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2sqr.toString());
                a1.sqrt();
                a1.sqr();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2sqr.toString());

                // test multiplication
                var fp2mul = readFP2(vectors[k].FP2mul, ctx);
                a1.copy(fp21);
                a2.copy(fp22);
                a1.mul(a2);
                a1.reduce();
                expect(a1.toString()).to.equal(fp2mul.toString());

                // test power
                var fp2pow = readFP2(vectors[k].FP2pow, ctx);
                a1.copy(fp21);
                scalar.norm();
                a1 = a1.pow(scalar);
                a1.reduce();
                expect(a1.toString()).to.equal(fp2pow.toString());

                // test inverse
                var fp2inv = readFP2(vectors[k].FP2inv, ctx);
                a1.copy(fp21);
                a1.inverse();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2inv.toString());

                // test division by 2
                var fp2div2 = readFP2(vectors[k].FP2div2, ctx);
                a1.copy(fp21);
                a1.div2();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2div2.toString());

                // test multiplication by (1+sqrt(-1))
                var fp2mulip = readFP2(vectors[k].FP2mulip, ctx);
                a1.copy(fp21);
                a1.mul_ip();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2mulip.toString());

                // test division by (1+sqrt(-1))
                var fp2divip = readFP2(vectors[k].FP2divip, ctx);
                a1.copy(fp21);
                a1.div_ip();
                a1.reduce();
                expect(a1.toString()).to.equal(fp2divip.toString());

            }
            done();
        });

    }
});