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

console.log("Testing addition of shares");
var assert = require('assert');
var fs = require('fs');

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

for(var vector in vectors)
  {
    console.log("Test "+vectors[vector].test_no);
    // Client secrets
    if (DEBUG){console.log("CS1 "+vectors[vector].CS1);}
    if (DEBUG){console.log("CS2 "+vectors[vector].CS2);}
    if (DEBUG){console.log("CLIENT_SECRET "+vectors[vector].CLIENT_SECRET);}
    var client_secret = MPINAuth.addShares(vectors[vector].CS1, vectors[vector].CS2);
    if (DEBUG){console.log("client_secret "+client_secret);}
    try
      {
        assert.equal(client_secret, vectors[vector].CLIENT_SECRET, "Client Secret Addition failed");
      }
    catch(err)
      {
        txt="Error description: " + err.message;
        console.error(txt);
        console.log("TEST FAILED");
        return;
      }
    // Time permits
    if (DEBUG){console.log("TP1 "+vectors[vector].TP1);}
    if (DEBUG){console.log("TP2 "+vectors[vector].TP2);}
    if (DEBUG){console.log("TIME_PERMIT "+vectors[vector].TIME_PERMIT);}
    var time_permit = MPINAuth.addShares(vectors[vector].TP1, vectors[vector].TP2);
    if (DEBUG){console.log("time_permit "+time_permit);}
    try
      {
        assert.equal(time_permit, vectors[vector].TIME_PERMIT, "Time Permit Addition failed");
      }
    catch(err)
      {
        txt="Error description: " + err.message;
        console.error(txt);
        console.log("TEST FAILED");
        return;
      }
  }
console.log("TEST PASSED");
