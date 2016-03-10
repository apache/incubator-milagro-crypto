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

console.log("Testing pass one request generation");
var WebSocket = require('ws');
var assert = require('assert');
var http = require('http');
var fs = require('fs');
var crypto = require('crypto');

// Javascript files from the PIN pad  are included here:
eval(fs.readFileSync('../DBIG.js')+'');
eval(fs.readFileSync('../BIG.js')+'');
eval(fs.readFileSync('../FP.js')+'');
eval(fs.readFileSync('../ROM.js')+'');
eval(fs.readFileSync('../HASH.js')+'');
eval(fs.readFileSync('../RAND.js')+'');
eval(fs.readFileSync('../AES.js')+'');
eval(fs.readFileSync('../GCM.js')+'');
eval(fs.readFileSync('../ECP.js')+'');
eval(fs.readFileSync('../FP2.js')+'');
eval(fs.readFileSync('../ECP2.js')+'');
eval(fs.readFileSync('../FP4.js')+'');
eval(fs.readFileSync('../FP12.js')+'');
eval(fs.readFileSync('../PAIR.js')+'');
eval(fs.readFileSync('./MPIN.js')+'');
eval(fs.readFileSync('../MPINAuth.js')+'');

// Configuration file
eval(fs.readFileSync('./config.js')+'');

// Load test vectors
var vectors = require('./testVectors.json');

// Turn on DEBUG mode in MPINAuth
MPINAuth.DEBUG = DEBUG;

var RAW=[];
for (i=0;i<100;i++) RAW[i]=i;
var RAW_hex = MPIN.bytestostring(RAW);

// Initiaize RNG
MPINAuth.initializeRNG(RAW_hex);

for(var vector in vectors)
  {
    console.log("Test "+vectors[vector].test_no);
    if (DEBUG){console.log("MPIN_ID_HEX "+vectors[vector].MPIN_ID_HEX);}
    if (DEBUG){console.log("TIME_PERMIT "+vectors[vector].TIME_PERMIT);}
    if (DEBUG){console.log("TOKEN "+vectors[vector].TOKEN);}
    if (DEBUG){console.log("PIN2 "+vectors[vector].PIN2);}
    if (DEBUG){console.log("X "+vectors[vector].X);}
    if (DEBUG){console.log("U "+vectors[vector].U);}
    if (DEBUG){console.log("UT "+vectors[vector].UT);}
    var pass1 = MPINAuth.pass1Request(vectors[vector].MPIN_ID_HEX, vectors[vector].TOKEN, vectors[vector].TIME_PERMIT, vectors[vector].PIN2, vectors[vector].DATE, vectors[vector].X);
    if (DEBUG){console.dir("pass1 "+pass1);}
    try
      {
        assert.equal(pass1.U, vectors[vector].U, "U generation failed");
        assert.equal(pass1.UT, vectors[vector].UT, "UT generation failed");
      }
    catch(err)
      {
        txt="Error description: " + err.message;
        console.error(txt);
        var cur_date = new Date();
        console.log("TEST FAILED: "+cur_date.toISOString());
        return;
      }
  }
console.log("TEST PASSED");
