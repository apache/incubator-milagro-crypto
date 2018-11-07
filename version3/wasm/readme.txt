
AMCL in Webassembly 

See https://webassembly.org/getting-started/developers-guide/


The AMCL library already has a Javascript version, but can also run up 
to 10 times faster in a browser that supports Webassembly. And thats
most of the popular browsers in use today.*

The C, C++ or Rust version of the AMCL library can be compiled to a 
bitcode that runs directly in the browser, by-passing Javascript 
entirely. Which is good for our type of application, as the way
in which Javascript handles integer arithmetic is very slow.

To install the Emscripten C/C++ compiler follow the instructions
above. Then copy the AMCL C code into a new directory, along with
the config.py file from this directory. In the new directory execute

python3 config.py

Then select options 1, 3, 7, 18, 20, 25, 26 and 27, which are fixed for 
the example programs.

Build the test programs with

emcc -O2 benchtest_all.c amcl.a -s WASM=1 -o benchtest_all.html

and

emcc -O2 testall.c amcl.a -s WASM=1 -o testall.html

Then run a local HTML server (as described in the link above) and load the 
HTML file.

Wait for the benchtest_all program to complete (which will take a while).

*Firefox, Safari, Edge, Chrome


-------------------------------

Webassembly can also be generated from the Rust code. First the Rust compiler 
must be updated to target wasm, by

rustup target add wasm32-unknown-emscripten

The Emscripten toolchain is also required, as above

Edit the Rust file config32.py, where it generates "cargo rustc" instruction 
(around line 372), and include the flag --target wasm32-unknown-emscripten

Build the Rust library as described in the Rust readme.txt file by executing

python3 config32.py

Copy the AMCL library to the current directory

cp amcl/target/wasm32-unknown-emscripten/release/libamcl.rlib .

Finally build one of the test programs by, for example

rustc --target wasm32-unknown-emscripten BenchtestALL.rs --extern amcl=libamcl.rlib -o BenchtestALL.html

Note that this will create a HTML file, which can be loaded into a browser as 
described above.

Right now (6/2018) this doesn't work with the 64-bit build, I suspect because 
Webassembly is not happy with 128-bit integers. But in time it might (and 
would then be even faster).


