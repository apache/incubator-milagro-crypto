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


/* Test ECP ARITHMETICS - test driver and function exerciser for ECP API Functions */

var chai = require('chai');

var CTX = require("../index");

var expect = chai.expect;

var ecp_curves = ['ED25519', 'GOLDILOCKS', 'NIST256', 'BRAINPOOL', 'ANSSI', 'HIFIVE', 'C25519', 'NIST384', 'C41417',
     'NIST521', 'NUMS256W', 'NUMS384W', 'NUMS512W', 'BN254', 'BN254CX', 'BLS383', 'BLS461', 'FP256BN', 'FP512BN'
];

var readScalar = function(string, ctx) {

    while (string.length != ctx.BIG.MODBYTES*2) string = "00"+string;

    return ctx.BIG.fromBytes(new Buffer(string, "hex"));

}

var readPoint = function(string, ctx) {
    
    var P = new ctx.ECP(0);
	var cos = string.split(":");

    while (cos[0].length < ctx.BIG.MODBYTES*2) cos[0] = "0"+cos[0];
    while (cos[1].length < ctx.BIG.MODBYTES*2) cos[1] = "0"+cos[1];

    var x = ctx.BIG.fromBytes(new Buffer(cos[0], "hex"));
    var y = ctx.BIG.fromBytes(new Buffer(cos[1], "hex"));
    P.setxy(x,y);

    return P;
}

describe('TEST ECP ARITHMETIC', function() {

	var j = ecp_curves.length - 1;

    for (var i = ecp_curves.length - 1; i >= 0; i--) {


        it('test '+ecp_curves[i], function(done) {
            this.timeout(0);
            var ctx = new CTX(ecp_curves[j]);
            var vectors = require('../testVectors/ecp/'+ecp_curves[j]+'.json');
            j = j-1;

            for (var k = 0; k <= vectors.length - 1; k++) {

                var P1 = readPoint(vectors[k].ECP1,ctx);
                var Paux1 = new ctx.ECP(0);
                Paux1.copy(P1);
                // test copy and equals
                expect(Paux1.equals(P1)).to.equal(true);

                if (ctx.ECP.CURVETYPE != ctx.ECP.MONTGOMERY) {
                    // test that y^2 = RHS
                    var x = Paux1.getx();
                    var y = Paux1.gety();
                    y.sqr();
                    var res = ctx.ECP.RHS(x);

                    expect(res.toString()).to.equal(y.toString());

		            // test commutativity of the sum
		            var P2 = readPoint(vectors[k].ECP2,ctx);
		            var Psum = readPoint(vectors[k].ECPsum,ctx);
		            var Paux2 = new ctx.ECP(0);
		            Paux1.copy(P1);
		            Paux2.copy(P2);
		            Paux1.add(P2);
		            Paux1.affine();
		            Paux2.add(P1);
		            Paux2.affine();
		            expect(Paux1.toString()).to.equal(Psum.toString());
		            expect(Paux2.toString()).to.equal(Psum.toString());

		            // test associativity of the sum
		            Paux2.copy(P2);
		            Paux2.add(Psum);
		            Paux2.add(P1);
		            Paux2.affine();
		            Paux1.add(Psum)
		            Paux1.affine();
		            expect(Paux1.toString()).to.equal(Paux2.toString());

	                // test negative of a point
	                var Pneg = readPoint(vectors[k].ECPneg,ctx);
	                Paux1.copy(P1);
	                Paux1.neg();
	                Paux1.affine();
	                expect(Paux1.toString()).to.equal(Pneg.toString());

	                // test subtraction between points
	                var Psub = readPoint(vectors[k].ECPsub,ctx);
	                Paux1.copy(P1);
	                Paux1.sub(P2);
	                Paux1.affine();
	                expect(Paux1.toString()).to.equal(Psub.toString());
            	}

                // test doubling
                var Pdbl = readPoint(vectors[k].ECPdbl,ctx);
                Paux1.copy(P1);
                Paux1.dbl();
                Paux1.affine();
                expect(Paux1.toString()).to.equal(Pdbl.toString());

                // test scalar multiplication
                var Pmul = readPoint(vectors[k].ECPmul,ctx);
                var Scalar1 = readScalar(vectors[k].BIGscalar1, ctx);
                Paux1.copy(P1);
                Paux1 = Paux1.mul(Scalar1);
                Paux1.affine();
                expect(Paux1.toString()).to.equal(Pmul.toString());

                if (ctx.ECP.CURVETYPE != ctx.ECP.MONTGOMERY) {
	                // test multiplication by small integer
	                var Ppinmul = readPoint(vectors[k].ECPpinmul,ctx);
	                var Scalar1 = readScalar(vectors[k].BIGscalar1, ctx);
	                Paux1.copy(P1);
	                Paux1 = Paux1.pinmul(1234,14);
	                Paux1.affine();
	                expect(Paux1.toString()).to.equal(Ppinmul.toString());

	                // test mul2
	                var Pmul2 = readPoint(vectors[k].ECPmul2,ctx);
	                var Scalar2 = readScalar(vectors[k].BIGscalar2, ctx);
	                Paux1.copy(P1);
	                Paux2.copy(P2);
	                Paux1.affine();
	                Paux1 = Paux1.mul2(Scalar1,Paux2,Scalar2);
	                expect(Paux1.toString()).to.equal(Pmul2.toString());
	            }

                // test wrong coordinates and infinity point
                var Pwrong = readPoint(vectors[k].ECPwrong,ctx);
                var Pinf = readPoint(vectors[k].ECPinf,ctx);
                // test copy and equals
                expect(Pwrong.is_infinity()).to.equal(true);
                expect(Pinf.is_infinity()).to.equal(true);
                expect(Pwrong.equals(Pinf)).to.equal(true);
            }
            done();
        });

    }
});