NOTE: This version of the library requires Version 1.17+ of Rust for 64-bit 
support. Unfortunately support for the 128-bit integer type is still
flagged as unstable, and so for now a nightly build of rust must be used.

Namespaces are used to separate different curves.

To build the library and see it in action, copy all of the files in this 
directory to a fresh root directory. Then execute the python3 script 
config32.py or config64.py (depending on whether you want a 32 or 64-bit 
build), and select the curves that you wish to support. Libraries will be 
built automatically including all of the modules that you will need.

As a quick example execute from your root directory

py config64.py

or

python3 config64.py

Then select options 1, 3, 7, 18, 20, 25, 26 and 27 (these are fixed for 
the example program provided). Select 0 to exit.

Then copy the library from amcl/target/release/libamcl.rlib to the
root directory, and execute

rustc TestALL.rs --extern amcl=libamcl.rlib

Run this test program by executing the program TestALL.exe

rustc BenchtestALL.rs --extern amcl=libamcl.rlib

Run this test program by executing the program BenchtestALL.exe




