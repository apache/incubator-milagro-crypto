AMCL is very simple to build for Swift.

First - decide the modulus and curve type you want to use. Edit rom.swift 
where indicated. You will probably want to use one of the curves whose 
details are already in there.

Three example API files are provided, mpin.swift which 
supports our M-Pin (tm) protocol, ecdh.swift which supports elliptic 
curve key exchange, digital signature and public key crypto, and rsa.swift
which supports the RSA method. The first  can be tested using the 
TestMPIN.swift driver programs, the second can be tested using TestECDH.swift 
and TestECM.swift, and the third with TestRSA.swift

In the rom.swift file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

For a quick jumpstart:-

From a terminal window in a /lib directory create a dynamic library using the command

swiftc big.swift rom.swift dbig.swift rand.swift hash.swift fp.swift fp2.swift ecp.swift ecp2.swift aes.swift gcm.swift fp4.swift fp12.swift ff.swift pair.swift rsa.swift ecdh.swift mpin.swift -Ounchecked -whole-module-optimization -emit-library -emit-module -module-name clint

This creates the files 

libclint.dylib
clint.swiftmodule

Copy these to a project directory, which contains only the files 

TestECDH.swift
TestRSA.swift
TestMPIN.swift

And create and run the projects by issuing the commands

swift -lclint -I. TestMPIN.swift 
swift -lclint -I. TestECDH.swift 
swift -lclint -I. TestRSA.swift 

Note that classes and methods that need to be exposed to consuming programs, 
should be made "public" when and if needed. Here we have done this as needed 
just for these example programs



