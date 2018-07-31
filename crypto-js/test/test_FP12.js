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


/* Test FP12 ARITHMETICS - test driver and function exerciser for FP4 API Functions */

var chai = require('chai');

var CTX = require("../index");

var expect = chai.expect;

var pf_curves = ['BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'];

var readBIG = function(string, ctx) {
    while (string.length != ctx.BIG.MODBYTES*2){string = "00"+string;}
    return ctx.BIG.fromBytes(new Buffer(string, "hex"));
}

var readFP2 = function(string, ctx) {
    string = string.split(",");
    var cox = string[0].slice(1);
    var coy = string[1].slice(0,-1);

    var bigx = readBIG(cox,ctx);
    var bigy = readBIG(coy,ctx);
    var fp2 = new ctx.FP2(0);
    fp2.bset(bigx,bigy);

    return fp2;
}

var readFP4 = function(string, ctx) {
    var X, Y;

    string = string.split("],[");
    var cox = string[0].slice(1) + "]";
    var coy = "[" + string[1].slice(0,-1);

    X = readFP2(cox,ctx);
    Y = readFP2(coy,ctx);
    var fp4 = new ctx.FP4(0);
    fp4.set(X,Y);

    return fp4;
}

var readFP12= function(string, ctx) {
    var X,Y,Z;

    string = string.split("]],[[");
    var cox = string[0].slice(1) + "]]";
    var coy = "[[" + string[1] + "]]";
    var coz = "[[" + string[2].slice(0,-1);

    X = readFP4(cox,ctx);
    Y = readFP4(coy,ctx);
    Z = readFP4(coz,ctx);
    var fp12 = new ctx.FP12(0);
    fp12.set(X,Y,Z);

    return fp12;
}

describe('TEST FP12 ARITHMETIC', function() {

    var j =0;

    for (var i = 0; i < pf_curves.length; i++) {

        it('test '+pf_curves[i], function(done) {
            this.timeout(0);
            curve = pf_curves[j];
            j++;
            var ctx = new CTX(curve);
            var vectors = require('../testVectors/fp12/'+curve+'.json');

            for (var k = 0; k < vectors.length; k++) {

                var fp121,fp122,fp123,fp124,fp12c;
                fp121 = readFP12(vectors[k].FP121, ctx);
                fp122 = readFP12(vectors[k].FP122, ctx);
                fp123 = readFP12(vectors[k].FP123, ctx);
                fp124 = readFP12(vectors[k].FP124, ctx);
                fp12c = readFP12(vectors[k].FP12c, ctx);

                var BIGsc1,BIGsc2,BIGsc3,BIGsc4,BIGscs,BIGsco;
                BIGsc1 = readBIG(vectors[k].BIGsc1, ctx);
                BIGsc2 = readBIG(vectors[k].BIGsc2, ctx);
                BIGsc3 = readBIG(vectors[k].BIGsc3, ctx);
                BIGsc4 = readBIG(vectors[k].BIGsc4, ctx);
                BIGscs = readBIG(vectors[k].BIGscs, ctx);
                BIGsco = readBIG(vectors[k].BIGsco, ctx);

                var a1 = new ctx.FP12(0);
                var a2 = new ctx.FP12(0);

                // test conjugate of a FP4
                var fp12conj = readFP12(vectors[k].FP12conj, ctx);
                a1.copy(fp121);
                a1.conj();
                a1.reduce();
                expect(a1.toString()).to.equal(fp12conj.toString());

                // test multiplication and commutativity
                var fp12mul = readFP12(vectors[k].FP12mul, ctx);
                a1.copy(fp121);
                a2.copy(fp122);
                a1.mul(fp122);
                a1.reduce();
                a2.mul(fp121);
                a2.reduce();
                expect(a1.toString()).to.equal(fp12mul.toString());
                expect(a2.toString()).to.equal(fp12mul.toString());

                // test square
                var fp12sqr = readFP12(vectors[k].FP12square, ctx);
                a1.copy(fp121);
                a1.sqr();
                a1.reduce();
                expect(a1.toString()).to.equal(fp12sqr.toString());

                // test unitary square
                var fp12usqr = readFP12(vectors[k].FP12usquare, ctx);
                a1.copy(fp121);
                a1.usqr();
                a1.reduce();
                expect(a1.toString()).to.equal(fp12usqr.toString());

                // test inverse
                var fp12inv = readFP12(vectors[k].FP12inv, ctx);
                a1.copy(fp121);
                a1.inverse();
                a1.reduce();
                expect(a1.toString()).to.equal(fp12inv.toString());

                // test smultiplication for D-TYPE
                var fp12smulydtype = readFP12(vectors[k].FP12smulydtype,ctx);
                var fp12smuldtype = readFP12(vectors[k].FP12smuldtype,ctx);
                a1.copy(fp121);
                a1.smul(fp12smulydtype, ctx.ECP.D_TYPE);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12smuldtype.toString());

                // test smultiplication for M-TYPE
                var fp12smulymtype = readFP12(vectors[k].FP12smulymtype,ctx);
                var fp12smulmtype = readFP12(vectors[k].FP12smulmtype,ctx);
                a1.copy(fp121);
                a1.smul(fp12smulymtype, ctx.ECP.M_TYPE);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12smulmtype.toString());

                // test power
                var fp12pow = readFP12(vectors[k].FP12pow, ctx);
                a1 = fp121.pow(BIGsc1);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12pow.toString());

                // test power by small integer
                var fp12pinpow = readFP12(vectors[k].FP12pinpow, ctx);
                a1.copy(fp121);
                a1.pinpow(k+1,10);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12pinpow.toString());

                // test compressed power with big integer
                var fp12compow = readFP4(vectors[k].FP12compow, ctx);
                a1.norm();
                a1 = fp12c.compow(BIGsc1,BIGsco);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12compow.toString());

                // test compressed power with small integer
                var fp12compows = readFP4(vectors[k].FP12compows, ctx);
                a1.norm();
                a1 = fp12c.compow(BIGscs,BIGsco);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12compows.toString());

                // test pow4
                var fp12pow4 = readFP12(vectors[k].FP12pow4, ctx);
                a1 = ctx.FP12.pow4([fp121,fp122,fp123,fp124],[BIGsc1,BIGsc2,BIGsc3,BIGsc4]);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12pow4.toString());

                // test frobenius
                var fp12frob = readFP12(vectors[k].FP12frob, ctx);
                var Fra = new ctx.FP(0),
                    Frb = new ctx.FP(0),
                    Fr;
                Fra.rcopy(ctx.ROM_FIELD.Fra);
                Frb.rcopy(ctx.ROM_FIELD.Frb);
                Fr = new ctx.FP2(Fra,Frb);
                a1.copy(fp121);
                a1.frob(Fr);
                a1.reduce();
                expect(a1.toString()).to.equal(fp12frob.toString());

                //test trace
                var fp4trace = readFP4(vectors[k].FP4trace, ctx);
                a1 = fp121.trace();
                a1.reduce();
                expect(a1.toString()).to.equal(fp4trace.toString());
            }
            done();
        });
    }
});
