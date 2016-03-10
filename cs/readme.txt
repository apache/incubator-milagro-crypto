AMCL is very simple to build for C#.

NOTE: The C# code was automatically generated from the Java64 code using 
the Java to C# Converter from Tangible Software Solutions. A few minor
fix-ups were required.

First - decide the modulus and curve type you want to use. Edit ROM.cs 
where indicated. You will probably want to use one of the curves whose 
details are already in there.

Three example API files are provided, MPIN.cs which 
supports our M-Pin (tm) protocol, ECDH.cs which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.cs
which supports the RSA method.

In the ROM.cs file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

For a quick jumpstart:-

csc TestMPIN.cs MPIN.cs FP.cs BIG.cs DBIG.cs AES.cs HASH.cs RAND.cs ROM.cs StringHelperClass.cs ECP.cs FP2.cs ECP2.cs FP4.cs FP12.cs PAIR.cs RectangularArrays.cs

or 

csc TestECDH.cs ECDH.cs FP.cs BIG.cs DBIG.cs AES.cs HASH.cs RAND.cs ROM.cs StringHelperClass.cs ECP.cs

or

csc TestRSA.cs RSA.cs FF.cs BIG.cs DBIG.cs HASH.cs RAND.cs ROM.cs StringHelperClass.cs

