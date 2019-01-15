AMCL is very simple to build for Rust.

This version supports both 32-bit and 64-bit builds.
If your processor and operating system are both 64-bit, a 64-bit build 
will probably be best. Otherwise use a 32-bit build.

First - decide the modulus and curve type you want to use. Edit rom32.rs 
or rom64.rs where indicated. You will probably want to use one of the curves 
whose details are already in there. You might want to "raid" the rom 
file from the C version of the library for more curves.

Three example API files are provided, mpin.rs which 
supports our M-Pin (tm) protocol, ecdh.rs which supports elliptic 
curve key exchange, digital signature and public key crypto, and rsa.rs
which supports the RSA method. The first can be tested using the 
TestMPIN.rs driver program, the second can be tested using TestECDH.rs,
and the third with TestRSA.rs


In the rom32.rs/rom64.rs file you must provide the curve constants. Several 
examples are provided there, if you are willing to use one of these.

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.

For a quick jumpstart:-

Copy rom32.rs to rom.rs for a 32-bit build

rustc --cfg D32 -O -A dead_code TestMPIN.rs

or 

rustc --cfg D32 -O -A dead_code TestECDH.rs

or

rustc --cfg D32 -O -A dead_code TestRSA.rs

also

rustc --cfg D32 -O -A dead_code BenchtestEC.rs

rustc --cfg D32 -O -A dead_code BenchtestPAIR.rs


For a 64-bit build copy rom64.rs to rom.rs, and use instead the 
flag --cfg D64
