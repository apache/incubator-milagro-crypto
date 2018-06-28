The Apache Milagro Cryptographic Library

AMCL v3.1 uses a standard Python 3 script to build libraries in all
supported languages. New users should use this version.

The main improvement is that AMCL v3 can optionally simultaneously support 
multiple elliptic curves and RSA key sizes within a single appliction.

Note that AMCL is largely configure at compile time. In version 3 this
configuration is handled by the Python script.

AMCL is available in 32-bit and 64-bit versions in most languages. Limited 
support for 16-bit processors is provided by the C version.

Now languages like to remain "standard" irrespective of the underlying 
hardware. However when it comes to optimal performance, it is impossible 
to remain architecture-agnostic. If a processor supports 64-bit 
instructions that operate on 64-bit registers, it will be a waste not to
use them. Therefore the 64-bit language versions should always be used
on 64-bit processors.

Version 3.1 is a major "under the hood" upgrade. Field arithmetic is 
performed using ideas from http://eprint.iacr.org/2017/437 to ensure 
that critical calculations are performed in constant time. This strongly 
mitigates against side-channel attacks. Exception-free formulae are 
now used for Weierstrass elliptic curves. A new standardised script 
builds for the same set of curves across all languages.


Several helper programs are provided to assist with the addition of
new elliptic curves. Note that these programs will not be needed if using
one of the supported curves. These programs must be build using the MIRACL
library. See source code for compilation instructions

bigtobig.cpp - converts to BIG number format

check.cpp - checks for optimal choice of number base

bestpair.cpp - finds best BN, BLS12 and BLS24 pairing-friendly curves
(Note - the library does not currently support BLS24 curves)

romgen.cpp - rough-and-ready program used to help generate ROM files for
all of the different languages.
