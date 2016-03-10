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

console.log("Testing response for an End-User who has an invalid PIN using webSockets");
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

var fail = 0;
var mpin = {};
var body = "";

// Data for mpin_id
var randomUser = crypto.randomBytes(32);
var userID = randomUser.toString("hex");
var cur_date = new Date();
var issued = cur_date.toISOString();
var salt = crypto.randomBytes(16);
var salt_hex = salt.toString("hex");
// var userID = 'testUser@miracl.com';
// var issued = '2014-01-30T19:17:48Z';

if (DEBUG){console.log(issued);}

// Form MPin ID
var endUserdata = {
  "issued": issued,
  "userID": userID,
  "mobile": 1,
  "salt": salt_hex
};
var mpin_id = JSON.stringify(endUserdata);
var mpin_id_bytes = MPIN.stringtobytes(mpin_id);
hash_mpin_id_bytes = MPIN.HASH_ID(mpin_id_bytes)

mpin.mpin_id_hex = MPIN.bytestostring(mpin_id_bytes);
mpin.hash_mpin_id_hex = MPIN.bytestostring(hash_mpin_id_bytes);
if (DEBUG){console.dir(mpin);}

// Request expiry
cur_date.setSeconds(cur_date.getSeconds() + SIGNATURE_EXPIRES_OFFSET_SECONDS);
var expires = cur_date.toISOString();

// Fixed Seed
// mpin.seedValueHex = seedValueHex;
// Random Seed
var randomSeedValue = crypto.randomBytes(100);

// Turn on debug statements by setting value in config.js
MPINAuth.DEBUG = DEBUG;

// Initiaize RNG
MPINAuth.initializeRNG(randomSeedValue);

////////////////  /clientSecret (GET)  //////////////////////////////

// String to be signed
var path  = "clientSecret"
message = path + app_id + mpin.hash_mpin_id_hex + expires;
if (DEBUG){console.log("message: "+message);}

var hmac = crypto.createHmac('sha256', app_key);
hmac.setEncoding('hex');
// write in the text that you want the hmac digest for
hmac.write(message);
// you can't read from the stream until you call end()
hmac.end();
// read out hmac digest
var signature = hmac.read();
if (DEBUG){console.log("signature " + signature);}


var urlParam = "/v0.3/" + path + "?app_id=" + app_id + "&expires=" + expires + "&hash_mpin_id=" + mpin.hash_mpin_id_hex + "&signature=" + signature + "&mobile=1";
if (DEBUG){console.log("urlParam: "+urlParam);}

// options for MIRACL's Client Secret
var optionsCS1 = {
    host : baseURL,
    port : DTA_proxy,
    path : urlParam,
    method : 'GET'
};
var dataCS1;
var requestCS1 = http.request(optionsCS1, function(res) {
  try
    {
      assert.equal('200', res.statusCode, "Client Secret Request Failed");
    }
  catch(err)
    {
      txt="Error description: " + err.message;
      console.error(txt);
      console.log("TEST FAILED");
      return;
    }
  // uncomment for header details
  // console.log("headers: ", res.headers);
  res.on('data', function(data) {
      if (DEBUG){console.log("client secret data: "+data);}
      dataCS1 = data;
  });

  res.on('end', function () {
    var response = JSON.parse(dataCS1);
    mpin.cs1=response.clientSecret;
    mpin.cs2=response.clientSecret;
    time_permits();
  });
});

requestCS1.end();
requestCS1.on('error', function(e) {
  console.error(e);
});


////////////////  /timePermit (GET)  //////////////////////////////

function time_permits()
{
  if (DEBUG){console.log("Request Time Permit");}

  var path  = "timePermit"
  // String to be signed
  message = mpin.hash_mpin_id_hex;
  if (DEBUG){console.log("message: "+message);}

  var hmac = crypto.createHmac('sha256', app_key);
  hmac.setEncoding('hex');
  // write in the text that you want the hmac digest for
  hmac.write(message);
  // you can't read from the stream until you call end()
  hmac.end();
  // read out hmac digest
  var signature = hmac.read();
  if (DEBUG){console.log("signature " + signature);}

  var urlParam = "/v0.3/" + path + "?app_id=" + app_id + "&hash_mpin_id=" + mpin.hash_mpin_id_hex + "&signature=" + signature + "&mobile=1";
  if (DEBUG){console.log("urlParam: "+urlParam);}

  // options for GET
  var optionsTP1 = {
      host : baseURL,
      port : DTA_proxy,
      path : urlParam,
      method : 'GET'
  };
  var dataTP1;
  var requestTP1 = http.request(optionsTP1, function(res) {
    try
      {
        assert.equal('200', res.statusCode, "Time Permit Request Failed");
      }
    catch(err)
      {
        txt="Error description: " + err.message;
        console.error(txt);
        console.log("TEST FAILED");
        return;
      }
    res.on('data', function(data) {
      dataTP1 = data;
    });

    res.on('end', function () {
      var response = JSON.parse(dataTP1);
      mpin.tp1=response.timePermit;
      mpin.tp2=response.timePermit;
      authenticate();
    });

  });

  requestTP1.end();
  requestTP1.on('error', function(e) {
    console.error(e);
  });
}


function authenticate()
{
  if (DEBUG){console.log("Perform M-Pin authentication");}

  // Add client secret shares
  mpin.client_secret_hex = MPINAuth.addShares(mpin.cs1, mpin.cs2);

  // Add time permit shares
  mpin.time_permit_hex = MPINAuth.addShares(mpin.tp1, mpin.tp2);

  // Create MPin Token
  var PIN1 = 1234;
  mpin.token_hex = MPINAuth.calculateMPinToken(mpin.mpin_id_hex, PIN1, mpin.client_secret_hex);
  if (DEBUG){console.log("mpin.token_hex " + mpin.token_hex);}

  // Open websocket.
  var authServerSocket = new WebSocket(MPinAuthenticationURL);

  authServerSocket.on('open', function() {
      if (DEBUG){console.log("websocket connection open");}
        var date=MPIN.today();
        var PIN2 = 1235
        var request =  MPINAuth.pass1Request(mpin.mpin_id_hex, mpin.token_hex, mpin.time_permit_hex, PIN2, date, null);
        // PASS1 REQUEST
        authServerSocket.send(JSON.stringify(request));
  });

  authServerSocket.on('message', function(message) {
    // PASS1 RESPONSE
    var response = JSON.parse(message);
    if (response.pass == 1)
      {
        if (DEBUG){console.log("PASS: "+response.pass+" message: "+message);}

        // Set OTP switch
        var requestOTP = 1;
        // Set WID
        var accessNumber = 123456;

        // Compute PASS2 request
        var request = MPINAuth.pass2Request(response.y, requestOTP, accessNumber);
        if (DEBUG){console.dir(request);}

        // PASS2 REQUEST
        authServerSocket.send(JSON.stringify(request));
      }
    else if(response.pass == 2)
      {
        // PASS2 RESPONSE
        if (DEBUG){console.log("PASS: "+response.pass+" message: "+message);}
        authServerSocket.close();
        verify(response);
      }
    else
      {
        console.error("Error: Invalid Passcode");
        return;
      }
  });
}

function verify(response)
{
  if (DEBUG){console.log("Request an auth token from RPS");}
  if (DEBUG){console.dir(response);}
  var path  = "/token";
  var urlParam = path + "?mpin_id_hex=" + mpin.mpin_id_hex;
  if (DEBUG){console.log("urlParam: "+urlParam)};

  // options for GET
  var optionsToken = {
      host : baseURL,
      port : MPinRPS,
      path : urlParam,
      method : 'GET'
  };
  var dataToken;
  var requestToken = http.request(optionsToken, function(res) {

    res.on('data', function(data) {
      dataToken = data;
    });

    res.on('end', function () {
      var response = JSON.parse(dataToken);
      if (DEBUG){console.dir(response); };
      try
        {
          assert.equal('-19', response.token.successCode, "Authentication Failed");
        }
      catch(err)
        {
          txt="Error description: " + err.message;
          console.error(txt);
          console.log("TEST FAILED");
          return;
        }
      console.log("TEST PASSED");
    });

  });

  requestToken.end();
  requestToken.on('error', function(e) {
    console.error(e);
  });
}
