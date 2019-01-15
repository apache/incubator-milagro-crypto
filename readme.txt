The Apache Milagro Cryptographic Library

Note that the AMCL currently comes in two versions, version 2.2 
and version 3.2

---------------------------------------

AMCL v2.2 is presented in what might be called a pre-library state.

In the various supported languages the source code is made available,
but it is not organised into rigid packages/crates/jars/whatever
It is expected that the consumer will themselves take this final step,
depending on the exact requirements of their project.

Note that version 2.2 is no longer supported.

-----------------------------------

AMCL v3.2 incorporates many minor improvements

Python version
Web Assembly support
Improved side channel resistance
Faster Swift code
Better Rust build system
Improved modular inversion algorithm
General speed optimizations
Improved Javascript testbed
More curves supported
New BLS signature API
Post quantum New Hope Implementation

-----------------------------------

AMCL v3.1 uses a standard Python 3 script to build libraries in all
supported languages. New users should use this version.

The main improvement is that AMCL v3 can optionally simultaneously support 
multiple elliptic curves and RSA key sizes within a single appliction.

Note that AMCL is largely configured at compile time. In version 3 this
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

---------------------------------------------
