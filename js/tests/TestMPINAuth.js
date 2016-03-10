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

console.log("JavaScript Test MPIN Example using MPINAuth");
var fs = require('fs');

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

var i,res;
var result;

var EGS=MPIN.EGS;
var EFS=MPIN.EFS;
var EAS=16;

var RAW=[];
for (i=0;i<100;i++) RAW[i]=i;
var RAW_hex = MPIN.bytestostring(RAW);


var G1S=2*EFS+1; /* Group 1 Size */
var G2S=4*EFS; /* Group 2 Size */

var S=[];
var server_secret_bytes=[];
var client_secret_bytes = [];
var token_bytes = [];
var time_permit_bytes = [];
var SEC = [];
var V = [];
var U = [];
var UT = [];
var X= [];
var Y= [];
var E=[];
var F=[];
var HID= [];
var HTID = [];

var PIN_setup = 1234;
var PIN_authenticate = 1234;

// Set OTP switch
var requestOTP = 1;
// Set WID
var accessNumber = 123456;

// Turn on debug statements by setting value in config.js
MPINAuth.DEBUG = DEBUG;

// Initiaize RNG
MPINAuth.initializeRNG(RAW_hex);

/* Trusted Authority set-up */
MPIN.RANDOM_GENERATE(MPINAuth.rng,S);
console.log("Master Secret s: 0x"+MPIN.bytestostring(S));

var IDstr = "testUser@miracl.com";
var mpin_id_bytes =MPIN.stringtobytes(IDstr);

var hash_mpin_id_bytes=[];
hash_mpin_id_bytes = MPIN.HASH_ID(mpin_id_bytes)

/* Client and Server are issued secrets by DTA */
MPIN.GET_SERVER_SECRET(S,server_secret_bytes);
console.log("Server Secret SS: 0x"+MPIN.bytestostring(server_secret_bytes));

MPIN.GET_CLIENT_SECRET(S,hash_mpin_id_bytes, client_secret_bytes);
console.log("Client Secret CS: 0x"+MPIN.bytestostring(client_secret_bytes));

// Client extracts PIN from secret to create Token
var mpin_id_hex = MPIN.bytestostring(mpin_id_bytes);
var client_secret_hex = MPIN.bytestostring(client_secret_bytes);
var token_hex = MPINAuth.calculateMPinToken(mpin_id_hex, PIN_setup, client_secret_hex);
token_bytes = MPINAuth.hextobytes(token_hex);
if (token_hex < 0)
	console.log("Failed to extract PIN ");

console.log("Client Token TK: 0x"+token_hex);

var date=MPIN.today();

/* Get "Time Token" permit from DTA */
MPIN.GET_CLIENT_PERMIT(date,S,hash_mpin_id_bytes, time_permit_bytes);
timePermit_hex = MPIN.bytestostring(time_permit_bytes);
console.log("Time Permit TP: 0x"+timePermit_hex);

// Client First pass
request = MPINAuth.pass1Request(mpin_id_hex, token_hex, timePermit_hex, PIN_authenticate, date, null);
if (request < 0)
	console.log("ERROR MPINAuth.pass1Request error_code: " + request);
UT_hex = request.UT;
U_hex = request.U;
UT_bytes = MPINAuth.hextobytes(UT_hex);
U_bytes = MPINAuth.hextobytes(U_hex);

/* Server generates Random number Y and sends it to Client */
MPIN.RANDOM_GENERATE(MPINAuth.rng,Y);
y_hex = MPIN.bytestostring(Y);

/* Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC */
request = MPINAuth.pass2Request(y_hex, requestOTP, accessNumber);
if (request < 0)
	console.log("ERROR MPINAuth.pass2Request error_code: " + request);
console.log("PASS 2 Request: ");
console.dir(request)

V_hex = request.V;
V_bytes = MPINAuth.hextobytes(V_hex);
console.log("V_hex: "+V_hex);

/* Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp. */
MPIN.SERVER_1(date,mpin_id_bytes,HID,HTID);

// Server Second pass
rtn=MPIN.SERVER_2(date,HID,HTID,Y,server_secret_bytes, U_bytes, UT_bytes, V_bytes,E,F);
if (rtn != 0)
  console.log("FAILURE: SERVER_1 rtn: " + rtn);

if (rtn != 0){
   console.log("Server Error:");
   var err=MPIN.KANGAROO(E,F);
   if (err==0) console.log("Client probably does not have a valid Token!");
   else console.log("(Client PIN is out by "+err);
 } else {
  console.log("Server says - PIN is good! You really are "+IDstr);
 }
