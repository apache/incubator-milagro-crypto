AMCL is very simple to build.

The examples here are for GCC under Linux and Windows (using MINGW).

First indicate your computer/compiler architecture by setting the wordlength 
in arch.h

Next - decide what you want to do. Edit amcl.h - note there is only
one area where USER CONFIGURABLE input is requested.

Here choose your curve.

Once this is done, build the library, and compile and link your program 
with an API file and the ROM file rom.c that contains curve constants.

Three example API files are provided, mpin.c which supports our M-Pin 
(tm) protocol, ecdh.c which supports standard elliptic 
curve key exchange, digital signature and public key crypto, and rsa.c 
which supports the RSA method. The first 
can be tested using the testmpin.c driver programs, the second can 
be tested using testecdh.c, and the third can be tested using
testrsa.c

In the ROM file you must provide the curve constants. Several examples
are provided there, and if you are willing to use one of these, simply
select your curve of CHOICE in amcl.h

Example (1), in amcl.h choose

#define CHOICE BN254

and

#define CURVETYPE WEIERSTRASS

Under windows run the batch file build_pair.bat to build the amcl.a library
and the testmpin.exe applications.

For linux execute "bash build_pair"

Example (2), in amcl.h choose

#define CHOICE C25519

and

#define CURVETYPE EDWARDS

to select the Edwards curve ed25519.

Under Windows run the batch file build_ec.bat to build the amcl.a library and
the testecdh.exe application.

For Linux execute "bash build_ec"


To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.


For quick jumpstart:-

(Linux)
bash build_pair
./testmpin

(Windows + MingW)
build_pair
testmpin
