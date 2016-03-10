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

console.log("Testing randomX");
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

// Turn on DEBUG mode in MPINAuth
MPINAuth.DEBUG = DEBUG;

var x_vals=[];

// Initiaize RNG
var seed = crypto.randomBytes(32);
var seed_hex = seed.toString("hex");
MPINAuth.initializeRNG(seed_hex);

mpin_id_hex = "7b226d6f62696c65223a20312c2022697373756564223a2022323031342d31322d31385431303a32303a32395a222c2022757365724944223a20223531306263313033353530616465636332316438393730303835323763323666406365727469766f782e636f6d222c202273616c74223a202234656233336433356366323963653161227d";
token_hex = "040128e30db2a7e5a26770498f558eab68920f58b4f707e738390160b2b4883bfb0521fe217597f279286818496a303e8d4b1a7e97b9c30d6c9fae99362c043e26";
timePermit_hex = "041019f24b3dbae8727fef08323e38fd36dcd1193f6de3286e7c4b224c539850a8200ee94fa5c45fa3350b14d015ebc4834ac57c4705712206655252a1a57939be";
PIN = 777
date = 16639

// Assign values of x to array
for(var i = 0; i < 10;i++)
  {
    var pass1 = MPINAuth.pass1Request(mpin_id_hex, token_hex, timePermit_hex, PIN, date,  null);
    x_hex = MPIN.bytestostring(MPINAuth.X);
    if(DEBUG){console.log("iter: "+i+" X: "+x_hex);}
    x_vals.push(x_hex);
  }

// Model re-rendering of page
var seed = crypto.randomBytes(32);
var seed_hex = seed.toString("hex");
MPINAuth.initializeRNG(seed_hex);

// Assign values of x to array
for(var i = 0; i < 10;i++)
  {
    var pass1 = MPINAuth.pass1Request(mpin_id_hex, token_hex, timePermit_hex, PIN, date, null);
    x_hex = MPIN.bytestostring(MPINAuth.X);
    if(DEBUG){console.log("iter: "+i+" X: "+x_hex);}
    x_vals.push(x_hex);
  }

if(DEBUG){console.dir(x_vals);}

// Write values to file for further processing
x_json = JSON.stringify(x_vals);
if(DEBUG){console.log(x_json);}
var output_file = "./randomX.json";
fs.writeFile(output_file, x_json, function(err) {
    if(err) {
        console.log(err);
        console.log("Error writing to "+output_file);
    }
});
console.log(output_file+" generated");

