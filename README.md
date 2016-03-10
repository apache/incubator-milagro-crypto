<h2>AMCL</h2>

<p>This directory contains the source code for the AMCL Library.</p>

<p>The directory structure is as follows</p>

<dl>
  <dt>./c</dt>
  <dd>- C Source code</dd>
  <dt>./js</dt>
  <dd>- JavaScript code</dd>
  <dt>./java</dt>
  <dd>- JAVA code</dd>
  <dt>./java64</dt>
  <dd>- JAVA code optimal for a 64-bit Virtual Machine</dd>
  <dt>./go</dt>
  <dd>- GO code</dd>
  <dt>./swift</dt>
  <dd>- swift code</dd>
  <dt>./cs</dt>
  <dd>- C# code</dd>
  <dt>./pythonCFFI</dt>
  <dd>- Python code that accesses the C library via the CFFI module</dd>
  <dt>./testVectors</dt>
  <dd>- Test Vectors</dd>
  <dt>./docs</dt>
  <dd>- Documentation</dd>
</dl>

<h2>Build Instructions</h2>

<p>AMCL is a standards compliant C library with no external dependencies. It
can be built using the <a href="http://www.cmake.org">CMake</a> build system.
In order to use the  Python wrappers <a href="https://cffi.readthedocs.org/en/release-0.8/">CFFI</a> is also
required. There are instructions provided for both Linux and Windows based systems.</p>

<p>Instructions for the Linux build are in ./linux.md</p>

<p>Instructions for the Mac OS build are in ./mac.md</p>

<p>Instructions for the Windows build are in ./windows.md</p>

