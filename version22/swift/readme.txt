AMCL is very simple to build for Swift.


This version supports both 32-bit and 64-bit builds. 
If your processor and 
operating system are both 64-bit, a 64-bit build 
will probably be best. 
Otherwise use a 32-bit build.


First - decide the modulus and curve type you want to use. Edit rom32.swift 

or rom64.swift where indicated. You will probably want to use one of the 
curves 
whose details are already in there. You might want to "raid" the 
rom
file from the C version of the library for more curves.

Three example API files are provided, mpin.swift which 
supports our M-Pin (tm) protocol, ecdh.swift which supports elliptic 
curve key exchange, digital signature and public key crypto, and rsa.swift
which supports the RSA method. The first  can be tested using the 
TestMPIN.swift driver programs, the second can be tested using TestECDH.swift, 

and the third with TestRSA.swift

In the rom32.swift/rom64.swift file you must provide the curve constants. 

Several examples are provided there, if you are willing to use one of these.

To help generate the ROM constants for your own curve some MIRACL helper 
programs are included. The programs bngen.cpp and blsgen.cpp generate ROM 
data for a BN and BLS pairing friendly curves, and the program ecgen.cpp 
generates ROM data for regular EC curves.

The MIRACL based program check.cpp helps choose the best number base for
big number representation, given the word-length and the size of the modulus.

The program bigtobig.cpp converts a big number to the AMCL 
BIG format.

For a quick jumpstart:-


Copy rom32.swift to rom.swift for a 32-bit build.



If using Xcode, load all of the swift files into a project. In "Build 
Options",
under "Swift Compiler - Custom Flags", set the compilation 
condition D32. Then 
build the project. 



For a 64-bit build copy rom64.swift instead, and set D64 in Xcode. 

Then build 
and run the program main.swift




Alternatively from a terminal window in a /lib directory create a dynamic 

library using the command

swiftc -DD32 big.swift rom.swift dbig.swift rand.swift hash256.swift hash384.swift hash512.swift fp.swift fp2.swift ecp.swift ecp2.swift aes.swift gcm.swift fp4.swift fp12.swift ff.swift pair.swift rsa.swift ecdh.swift mpin.swift -O -Ounchecked -whole-module-optimization -emit-library -emit-module -module-name amcl

This creates the files 

libamcl.dylib
amcl.swiftmodule

Copy these to a project directory, which contains only the files 

TestECDH.swift
TestRSA.swift
TestMPIN.swift


Edit these files to uncomment the line

 

import amcl

 

at the start of the program, and 



TestXXXX()



at the end of the program


Finally create and run the projects by issuing the commands

swift -lamcl -I. TestMPIN.swift 
swift -lamcl -I. TestECDH.swift 
swift -lamcl -I. TestRSA.swift 




Note that classes and methods that need to be exposed to consuming programs, 
should be made "public" when and if needed. Here we have done this as needed 
just for these example programs.

------------------------------------------------

An alternative method to build applications is to use the swiftc compiler 
directly. For example:-

Edit main.swift to just include a call to BenchtestPAIR()

Copy rom32.swift to rom.swift

Compile directly using swiftc

swiftc -DD32 -O -Ounchecked -whole-module-optimization main.swift BenchtestPAIR.swift pair.swift fp12.swift fp4.swift fp2.swift fp.swift big.swift dbig.swift ecp.swift ecp2.swift hash256.swift hash384.swift hash512.swift aes.swift rand.swift rom.swift -o main 

Run the BenchtestPAIR() program by

./main

For the files needed to build other applications, see go/readme.txt

Change "32" to "64" for a 64-bit build

