AMCL is very simple to build for Java. 

The first decision is whether to do a 32-bit or 64-bit build. In general a 
64-bit build will probably be faster if both your processor and operating 
system are 64-bit. Otherwise a 32-bit build is probably best.

For a 32-bit build, copy BIG32.java, DBIG32.java and ROM32.java to BIG.java,
DBIG.java and ROM.java respectively.

For a 64-bit build, copy BIG64.java, DBIG64.java and ROM64.java to BIG.java,
DBIG.java and ROM.java respectively.

Next - decide the modulus type and curve type you want to use. Edit ROM.java 
where indicated. You might want to use one of the curves whose details are
already in there.

Three example API files are provided, MPIN.java which 
supports our M-Pin (tm) protocol, ECDH.java which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.java
which supports the RSA method. The first  can be tested using the 
TestMPIN.java driver programs, the second can be tested using TestECDH.java, 
and the third with TestRSA.java

In the ROM.java file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.

Don't forget to delete all .class files before rebuilding projects.

For a quick jumpstart:-

del *.class
javac TestECDH.java
java TestECDH

del *.class
javac TestRSA.java
java TestRSA

del *.class
javac TestMPIN.java
java TestMPIN

del *.class
javac BenchtestEC.java
java BenchtestEC

del *.class
javac BenchtestPAIR.java
java BenchtestPAIR

