
Each supported primitive is implemented inside of its own swift namespace. 

So for example to support both ed25519 and the NIST P256 curves, one
could import into a particular module both "ed25519" and "nist256"

Separate ROM files provide the constants required for each curve. Some
files (big.swift, fp.swift, ecp.swift) also specify certain constants 
that must be set for the particular curve.

--------------------------------------

To build the library and see it in action, copy all of the files in this 
directory to a fresh root directory. Then execute the python3 script 
config32.py or config64.py (depending om whether you want a 32 or 
64-bit build), and select the curves that you wish to support. Libraries 
will be built automatically including all of the modules that you will need.

As a quick example execute from your root directory

py config64.py

or

python3 config64.py

Then select options 1, 3, 7, 18, 20, 25, 26 and 27 (these are fixed for the 
example program provided). Select 0 to exit.

Then execute

swift -I. -L. -led25519 -lnist256 -lgoldilocks -lbn254 -lbls383 -lbls24 -lbls48 -lrsa2048 TestALL.swift 

and

swift -I. -L. -led25519 -lnist256 -lgoldilocks -lbn254 -lbls383 -lbls24 -lbls48 -lrsa2048 BenchtestALL.swift 

