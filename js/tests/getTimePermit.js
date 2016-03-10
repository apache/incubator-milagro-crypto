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

console.log("Get time permit");
var assert = require('assert');
var https = require('https');
var http = require('http');
var fs = require('fs');
var crypto = require('crypto');



// Configuration file
eval(fs.readFileSync('./config.js')+'');

if (TLS) {http = https}

var mpin = {};
var body = "";

// Data for mpin_id
var randomUser = crypto.randomBytes(50);
// var userID = randomUser.toString("hex");
var userID = 'testUser@miracl.com';
var issued = '2014-01-30T19:17:48Z';

// Form MPin ID
var endUserdata = {
  "issued": issued,
  "userID": userID,
  "mobile": 1
};
var mpin_id = JSON.stringify(endUserdata);
hash_mpin_id_hex = crypto.createHash('sha256').update(mpin_id).digest('hex');
console.log("mpin_id: "+mpin_id);
console.log("hash_mpin_id_hex: " + hash_mpin_id_hex);


// String to be signed
var path  = "timePermit"
message = hash_mpin_id_hex;
console.log("message: "+message);

var hmac = crypto.createHmac('sha256', app_key);
hmac.setEncoding('hex');
// write in the text that you want the hmac digest for
hmac.write(message);
// you can't read from the stream until you call end()
hmac.end();
// read out hmac digest
var signature = hmac.read();
console.log("signature " + signature);

var urlParam = "/v0.3/" + path + "?app_id=" + app_id + "&hash_mpin_id=" + hash_mpin_id_hex + "&signature=" + signature + "&mobile=1";
console.log("urlParam: "+urlParam);

// options for GET
var options_get = {
    host : baseURL,
    port : DTA_proxy,
    path : urlParam,
    method : 'GET'
};

console.info('Options prepared:');
console.info(options_get);

// do the GET request
var reqGet = http.request(options_get, function(res) {
    console.log("statusCode: ", res.statusCode);
    // uncomment it for header details
    console.log("headers: ", res.headers);

    res.on('data', function(data) {
        console.info('GET result:\n');
        process.stdout.write(data);
        body = data;
        console.info('\n\nCall completed');
    });

    res.on('end', function () {
      console.log('Body : ' + body);
      display(body);
    });

});

reqGet.end();
reqGet.on('error', function(e) {
    console.error(e);
});

function display(data)
{
  console.info('body '+data);
  var response = JSON.parse(data);
  mpin.timePermitShare1=response.timePermit;
  console.info('Time Permit '+mpin.timePermitShare1);
}
