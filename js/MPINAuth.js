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

/*
   MIRACL JavaScript M-Pin Authentication Functions

   Provides these functions:

   calculateMPinToken     Calculates the MPin Token

   getLocalEntropy        Gets an entropy value from the client machine

   initializeRNG          Initialize the Random Number Generator

   addShares              Add two points on the curve that are originally in hex format

   pass1Request           Form the JSON request for pass one of the M-Pin protocol

   pass2Request           Form the JSON request for pass two of the M-Pin protocol

   passRequest      Form the JSON request for one pass M-Pin protocol

*/

/*

Run LINT tool;

jslint MPINAuth.js

expected output;

MPINAuth.js
 #1 Read only.
    MPINAuth = {}; // Line 61, Pos 1
 #2 Unexpected '('.
    if (typeof (window) === 'undefined') { // Line 134, Pos 16
 #3 Unexpected 'typeof'. Use '===' to compare directly with undefined.
    if (typeof (window) === 'undefined') { // Line 134, Pos 9
 #4 Unexpected '('.
    if (typeof (crypto) !== 'undefined') { // Line 139, Pos 16
 #5 Unexpected 'typeof'. Use '===' to compare directly with undefined.
    if (typeof (crypto) !== 'undefined') { // Line 139, Pos 9

*/


/*global MPIN */
/*global MPINAuth */
/*global RAND */
/*global Uint32Array */
/*jslint browser: true*/
/*jslint plusplus: true */

MPINAuth = {};

// Random Number Generator
MPINAuth.rng = new RAND();

// Pass 1 values
MPINAuth.SEC = [];
MPINAuth.X = [];

// Default value for debug output
MPINAuth.DEBUG = false;

// Errors
MPINAuth.BAD_HEX = -20;
MPINAuth.BAD_BYTES = -21;

/* Calculates the MPin Token

   This function convert mpin_id _hex to unicode. It then maps the mpin_id
   to a point on the curve, multiplies this value by PIN and then subtracts
   it from the client_secret curve point to generate the M-Pin token.

   Args:

     PIN: Four digit PIN
     client_secret_hex: Hex encoded client secret
     mpin_id_hex: Hex encoded M-Pin ID

   Returns:

     mpin_token_hex: Hex encoded M-Pin Token

*/
MPINAuth.calculateMPinToken = function (mpin_id_hex, PIN, client_secret_hex) {
    "use strict";
    var client_secret_bytes, mpin_id_bytes, token_hex, error_code;

    client_secret_bytes = [];
    mpin_id_bytes = [];

    if (MPINAuth.DEBUG) {console.log("MPINAuth.calculateMPinToken client_secret_hex: " + client_secret_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.calculateMPinToken mpin_id_hex: " + mpin_id_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.calculateMPinToken PIN: " + PIN); }

    client_secret_bytes = MPINAuth.hextobytes(client_secret_hex);
    mpin_id_bytes = MPINAuth.hextobytes(mpin_id_hex);

    error_code = MPIN.EXTRACT_PIN(mpin_id_bytes, PIN, client_secret_bytes);
    if (error_code !== 0) {
        console.log("MPINAuth.calculateMPinToken error_code: " + error_code);
        return error_code;
    }
    token_hex = MPIN.bytestostring(client_secret_bytes);
    if (MPINAuth.DEBUG) {console.log("MPINAuth.calculateMPinToken token_hex: " + token_hex); }
    return token_hex;
};

/* Get local entropy

   This function makes a call to /dev/urandom for a 256 bit value

   Args:

     NA

   Returns:

     entropy_val: 256 bit random value or null

*/
MPINAuth.getLocalEntropy = function () {
    "use strict";
    var crypto, array, entropy_val, i, hex_val;
    if (typeof (window) === 'undefined') {
        if (MPINAuth.DEBUG) {console.log("MPINAuth.getLocalEntropy Test mode without browser"); }
        return "";
    }
    crypto = (window.crypto || window.msCrypto);
    if (typeof (crypto) !== 'undefined') {
        array = new Uint32Array(8);
        crypto.getRandomValues(array);

        entropy_val = "";
        for (i = 0; i < array.length; i++) {
            hex_val = array[i].toString(16);
            entropy_val = entropy_val + hex_val;
        }
        if (MPINAuth.DEBUG) {console.log("MPINAuth.getLocalEntropy len(entropy_val): " + entropy_val.length + " entropy_val: " + entropy_val); }
        return entropy_val;
    }
    return "";
};

/* Initialize the Random Number Generator (RNG)

   This function uses an external and, where available, a
   local entropy source to initialize a RNG.

   Args:

     seed_value: External seed value for RNGTurn on generation of local entropy

   Returns:

*/
MPINAuth.initializeRNG = function (seed_hex) {
    "use strict";
    var local_entropy_hex, entropy_hex, entropy_bytes;
    local_entropy_hex = MPINAuth.getLocalEntropy();
    entropy_hex = local_entropy_hex + seed_hex;
    if (MPINAuth.DEBUG) {console.log("MPINAuth.initializeRNG seed_val_hex: " + seed_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.initializeRNG local_entropy_hex: " + local_entropy_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.initializeRNG entropy_hex: " + entropy_hex); }

    entropy_bytes = MPINAuth.hextobytes(entropy_hex);

    MPINAuth.rng.clean();
    MPINAuth.rng.seed(entropy_bytes.length, entropy_bytes);
};

/* Add two points on the curve that are originally in hex format

   This function is used to add client secret or time permits shares.

   Args:

     share1_hex: Hex encoded point on the curve which represents
                 a time permit or client secret share
     share2_hex: Hex encoded point on the curve which represents
                 a time permit or client secret share

   Returns:

     sum_hex: Hex encoded sum of the shares

*/
MPINAuth.addShares = function (share1_hex, share2_hex) {
    "use strict";
    var share1_bytes, share2_bytes, sum_bytes, error_code, sum_hex;

    share1_bytes = [];
    share2_bytes = [];
    sum_bytes = [];

    if (MPINAuth.DEBUG) {console.log("MPINAuth.addShares share1_hex: " + share1_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.addShares share2_hex: " + share2_hex); }

    share1_bytes = MPINAuth.hextobytes(share1_hex);
    share2_bytes = MPINAuth.hextobytes(share2_hex);

    error_code = MPIN.RECOMBINE_G1(share1_bytes, share2_bytes, sum_bytes);
    if (error_code !== 0) {
        console.log("MPINAuth.addShares error_code: " + error_code);
        return error_code;
    }
    sum_hex = MPIN.bytestostring(sum_bytes);
    if (MPINAuth.DEBUG) {console.log("MPINAuth.addShares sum_hex: " + sum_hex); }
    return sum_hex;
};


/* Form the JSON request for pass one of the M-Pin protocol

   This function assigns to the property X a random value. It assigns to
   the property SEC the sum of the client secret and time permit. It also
   calculates the values U and UT which are required for M-Pin authentication,
   where U = X.(map_to_curve(MPIN_ID)) and UT = X.(map_to_curve(MPIN_ID) + map_to_curve(DATE|sha256(MPIN_ID))
   UT is called the commitment. U is the required for finding the PIN error.

   Args:

     mpin_id_hex: Hex encoded M-Pin ID
     token_hex: Hex encoded M-Pin Token
     timePermit_hex: Hex encoded Time Permit
     PIN: PIN for authentication
     epoch_days: The number of epoch days.
     X_hex: X value generated externally. This is used for test.

   Returns:

    {
      mpin_id: mpin_id_hex,
      UT: UT_hex,
      U: U_hex,
      pass: 1
    }

    where;

    mpin_id: Hex encoded M-Pin ID
    UT: Hex encoded X.(map_to_curve(MPIN_ID) + map_to_curve(DATE|sha256(MPIN_ID))
    U: Hex encoded X.(map_to_curve(MPIN_ID))
    pass: Protocol first pass

*/
MPINAuth.pass1Request = function (mpin_id_hex, token_hex, timePermit_hex, PIN, epoch_days, X_hex) {
    "use strict";
    var UT_hex, U_hex, date, error_code, mpin_id_bytes, token_bytes, timePermit_bytes, U, UT, request;

    mpin_id_bytes = [];
    token_bytes = [];
    timePermit_bytes = [];
    U = [];
    UT = [];
    request = {};

    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request mpin_id_hex: " + mpin_id_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request token_hex: " + token_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request timePermit_hex: " + timePermit_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request PIN: " + PIN); }
    if (MPINAuth.DEBUG) {console.log("mpinAuth.pass1Request epoch_days: " + epoch_days); }

    // The following is used for test
    if (X_hex !== null) {
        if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request X: " + X_hex); }
        MPINAuth.X = MPINAuth.hextobytes(X_hex);
        MPINAuth.rng = null;
    }

    mpin_id_bytes = MPINAuth.hextobytes(mpin_id_hex);
    token_bytes = MPINAuth.hextobytes(token_hex);
    timePermit_bytes = MPINAuth.hextobytes(timePermit_hex);

    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request date: " + date); }
    error_code = MPIN.CLIENT_1(epoch_days, mpin_id_bytes, MPINAuth.rng, MPINAuth.X, PIN, token_bytes, MPINAuth.SEC, U, UT, timePermit_bytes);
    if (error_code !== 0) {
        console.log("MPINAuth.pass1Request error_code: " + error_code);
        return error_code;
    }
    UT_hex = MPIN.bytestostring(UT);
    U_hex = MPIN.bytestostring(U);

    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request MPINAuth.rng: " + MPINAuth.rng); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request MPINAuth.X: " + MPIN.bytestostring(MPINAuth.X)); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request MPINAuth.SEC: " + MPIN.bytestostring(MPINAuth.SEC)); }

    // Form request
    request = {
        mpin_id: mpin_id_hex,
        UT: UT_hex,
        U: U_hex,
        pass: 1
    };
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass1Request request: "); }
    if (MPINAuth.DEBUG) {console.dir(request); }

    return request;
};


/* Form the JSON request for pass two of the M-Pin protocol

   This function uses the random value y from the server, property X
   and the combined client secret and time permit to calculate
   the value V which is sent to the M-Pin server.

   Args:

     y_hex: Random value supplied by server

   Returns:

    {
      V: V_hex,
      OTP: requestOTP,
      WID: accessNumber,
      pass: 2
    }

    where;

    V: Value required by the server to authenticate user
    OTP: Request OTP: 1 = required
    WID: Number required for mobile authentication
    pass: Protocol second pass

*/
MPINAuth.pass2Request = function (y_hex, requestOTP, accessNumber) {
    "use strict";

    var y_bytes, x_hex, SEC_hex, error_code, V_hex, request;

    request = {};

    y_bytes = MPINAuth.hextobytes(y_hex);
    x_hex = MPIN.bytestostring(MPINAuth.X);
    SEC_hex = MPIN.bytestostring(MPINAuth.SEC);

    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass2Request x_hex: " + x_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass2Request y_hex: " + y_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass2Request SEC_hex: " + SEC_hex); }

    // Compute V
    error_code = MPIN.CLIENT_2(MPINAuth.X, y_bytes, MPINAuth.SEC);
    if (error_code !== 0) {
        console.log("MPINAuth.pass2Request error_code: " + error_code);
        return error_code;
    }
    V_hex = MPIN.bytestostring(MPINAuth.SEC);

    // Form reuest
    request = {
        V: V_hex,
        OTP: requestOTP,
        WID: accessNumber,
        pass: 2
    };
    if (MPINAuth.DEBUG) {console.log("MPINAuth.pass2Request request: "); }
    if (MPINAuth.DEBUG) {console.dir(request); }

    return request;
};


/* Convert a hex representation of a Point to bytes

   This function converts a hex value to a bytes array

   Args:

     hex_value: Hex encoded byte value

   Returns:

     byte_value: Input value in bytes

*/
MPINAuth.hextobytes = function (value_hex) {
    "use strict";
    var len, byte_value, i;

    len = value_hex.length;
    byte_value = [];

    for (i = 0; i < len; i += 2) {
        byte_value[(i / 2)] = parseInt(value_hex.substr(i, 2), 16);
    }
    return byte_value;
};


/* Form the JSON request for single pass M-Pin protocol

   This function performs the client side M-Pin protocol
   It also  calculates the values U and UT which are required for M-Pin authentication,
   where U = X.(map_to_curve(MPIN_ID)) and UT = X.(map_to_curve(MPIN_ID) + map_to_curve(DATE|sha256(MPIN_ID))
   UT is called the commitment. U is the required for finding the PIN error.

   Args:

     mpin_id_hex: Hex encoded M-Pin ID
     token_hex: Hex encoded M-Pin Token
     timePermit_hex: Hex encoded Time Permit
     PIN: PIN for authentication
     requestOTP: Reqeuest a One Time Password
     accessNumber: Access number for desktop authentication
     timeValue: Epoch time

   Returns:

    {
      mpin_id: mpin_id_hex,
      U: U_hex,
      UT: UT_hex,
      V: V_hex,
      T: timeValue,
      OTP: requestOTP,
      WID: accessNumber
    }

    where;

    mpin_id: Hex encoded M-Pin ID
    U: Hex encoded X.(map_to_curve(MPIN_ID))
    UT: Hex encoded X.(map_to_curve(MPIN_ID) + map_to_curve(DATE|sha256(MPIN_ID))
    V: Value required by the server to authenticate user
    T: Epoch time
    OTP: Request OTP: 1 = required
    WID: Number required for mobile authentication

*/
MPINAuth.passRequest = function (mpin_id_hex, token_hex, timePermit_hex, PIN, requestOTP, accessNumber, epoch_days, timeValue, X_hex) {
    "use strict";
    var X, Y, SEC, UT_hex, U_hex, date, error_code, mpin_id_bytes, token_bytes, timePermit_bytes, U, UT, V_hex, request;

    X = [];
    Y = [];
    SEC = [];
    mpin_id_bytes = [];
    token_bytes = [];
    timePermit_bytes = [];
    U = [];
    UT = [];
    request = {};

    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest mpin_id_hex: " + mpin_id_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest token_hex: " + token_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest timePermit_hex: " + timePermit_hex); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest PIN: " + PIN); }
    if (MPINAuth.DEBUG) {console.log("mpinAuth.passRequest timeValue: " + timeValue); }

    mpin_id_bytes = MPINAuth.hextobytes(mpin_id_hex);
    token_bytes = MPINAuth.hextobytes(token_hex);

    if (timePermit_hex === null) {
        date = 0;
    } else {
        timePermit_bytes = MPINAuth.hextobytes(timePermit_hex);
        date = epoch_days;
    }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest date: " + date); }

    // The following is used for test
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest X: " + X_hex); }
    if (X_hex !== null) {
        X = MPINAuth.hextobytes(X_hex);
        MPINAuth.rng = null;
    }

    error_code = MPIN.CLIENT(date, mpin_id_bytes, MPINAuth.rng, X, PIN, token_bytes, SEC, U, UT, timePermit_bytes, timeValue, Y);
    if (error_code !== 0) {
        console.log("MPINAuth.passRequest error_code: " + error_code);
        return error_code;
    }
    UT_hex = MPIN.bytestostring(UT);
    U_hex = MPIN.bytestostring(U);
    V_hex = MPIN.bytestostring(SEC);

    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest MPINAuth.rng: " + MPINAuth.rng); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest X: " + MPIN.bytestostring(X)); }
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest Y: " + MPIN.bytestostring(Y)); }

    // Form request
    request = {
        mpin_id: mpin_id_hex,
        U: U_hex,
        UT: UT_hex,
        V: V_hex,
        T: timeValue,
        OTP: requestOTP,
        WID: accessNumber
    };
    if (MPINAuth.DEBUG) {console.log("MPINAuth.passRequest request: "); }
    if (MPINAuth.DEBUG) {console.dir(request); }

    return request;
};

