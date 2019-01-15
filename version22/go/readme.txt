AMCL is very simple to build for Go.

This version supports both 32-bit and 64-bit builds.
If your processor and operating system are both 64-bit, a 64-bit build 
will probably be best. Otherwise use a 32-bit build.

Next - decide the modulus and curve type you want to use. Edit ROM32.go 
or ROM64.go where indicated. You will probably want to use one of the curves whose 
details are already in there.

Three example API files are provided, TestMPIN.go which 
supports our M-Pin (tm) protocol, TestECDH.go which supports elliptic 
curve key exchange, digital signature and public key crypto, and TestRSA.go
which supports the RSA method.

In the ROM32.go/ROM64.go file you must provide the curve constants. 
Several examples are provided there, if you are willing to use one of these.

Use ROM32.go for a 32-bit build
Use ROM64.go for a 64-bit build

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.


For a quick jumpstart:-

go run TestMPIN.go MPIN.go PAIR.go FP12.go FP4.go FP2.go FP.go BIG.go DBIG.go ECP.go ECP2.go HASH256.go HASH384.go HASH512.go AES.go RAND.go ROM64.go

or 

go run TestECDH.go ECDH.go FP.go BIG.go DBIG.go ECP.go HASH256.go HASH384.go HASH512.go AES.go RAND.go ROM64.go

or

go run TestRSA.go RSA.go FF.go BIG.go DBIG.go HASH256.go HASH384.go HASH512.go AES.go RAND.go ROM64.go

also

go run BenchtestEC.go RSA.go FF.go FP.go BIG.go DBIG.go ECP.go HASH256.go HASH384.go HASH512.go AES.go RAND.go ROM64.go

go run BenchtestPAIR.go PAIR.go FP12.go FP4.go FP2.go FP.go BIG.go DBIG.go ECP.go ECP2.go HASH256.go HASH384.go HASH512.go AES.go RAND.go ROM64.go