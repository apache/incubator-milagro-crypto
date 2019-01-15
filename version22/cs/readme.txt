AMCL is very simple to build for C#.

NOTE: The C# code is automatically generated from the Java code using 
the Java to C# Converter from Tangible Software Solutions. We noted a few minor
fix-ups that were required when we tried the current version of the Converter.


** In HASH384.cs and HASH512.cs change long to ulong. Remove castes in S() and R(). Add (ulong) caste 
in process()
** Comment out debug "main" programs in files other than Test***.cs
** In BIG.cs function mod() change "checked" to "unchecked"
** In BIG.cs in modmul() change BIG.mod(m);BIG.mod(m) to a.mod(m);b.mod(m). Same in modsqr() and modneg()
** In BIG.cs in jacobi() change BIG.mod(p) to x.mod(p) and BIG.mod(m) to t.mod(m)
** In TestMPIN.java change line Scanner ...; pin=scan.next(); to pin = int.Parse(Console.ReadLine());


Three example API files will be generated, MPIN.cs which 
supports our M-Pin (tm) protocol, ECDH.cs which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.cs
which supports the RSA method.

In the ROM.cs file you must provide the curve constants for the curve you want to use. 
Several examples are provided in the Java code, if you are willing to convert and use one of these.

When the translation is complete, for a quick jumpstart:-

csc TestMPIN.cs MPIN.cs FP.cs BIG.cs DBIG.cs AES.cs HASH256.cs HASH384.cs HASH512.cs RAND.cs ROM.cs StringHelperClass.cs ECP.cs FP2.cs ECP2.cs FP4.cs FP12.cs PAIR.cs RectangularArrays.cs

or 

csc TestECDH.cs ECDH.cs FP.cs BIG.cs DBIG.cs AES.cs HASH256.cs HASH384.cs HASH512.cs RAND.cs ROM.cs StringHelperClass.cs ECP.cs

or

csc TestRSA.cs RSA.cs FF.cs BIG.cs DBIG.cs HASH256.cs HASH384.cs HASH512.cs RAND.cs ROM.cs StringHelperClass.cs

