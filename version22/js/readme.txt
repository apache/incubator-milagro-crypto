AMCL is very simple to build for JavaScript.

First - decide the modulus type and curve type you want to use. Edit ROM.js 
where indicated. You might want to use one of the curves whose details are
already in there.

Three example API files are provided, MPIN.js which 
supports our M-Pin (tm) protocol, ECDH.js which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.js
which supports RSA encryption. The first  can be tested using the 
TestMPIN.html driver programs, the second can be tested using TestECDH.html, 
and the third using TestRSA.html

In the ROM.js file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.


For quick jumpstart:-

Run Chrome browser and navigate to TestECDH.html

or TestMPIN.html

or BenchtestEC.html

or BenchtestPAIR.html

You might need to wait a couple of minutes for the output to appear.


