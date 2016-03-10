AMCL is very simple to build for JavaScript.

First - decide the modulus type and curve type you want to use. Edit ROM.js 
where indicated. You might want to use one of the curves whose details are
already in there.

Three example API files are provided, MPIN.js which 
supports our M-Pin (tm) protocol, ECDH.js which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.js
which supports RSA encryption. The first  can be tested using the 
TestMPIN.html driver programs, the second can be tested using TestECDH.html 
and TestECM.html, and the third using TestRSA.html

In the ROM.js file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The program bngen.cpp generates the ROM details for a 
BN curve, and the program ecgen.cpp generates the ROM for EC curves.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.


For quick jumpstart:-

Run Chrome browser and navigate to TestMPIN.html

