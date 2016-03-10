<h2>Software dependencies</h2>

CMake is required to build the library and can usually be installed from
the operating system package manager.

<ul type="disc">
  <li>sudo apt-get install cmake</li>
</ul>

If not, then you can download it from www.cmake.org

The C Foreign Function Interface for Python <a href="https://cffi.readthedocs.org/en/latest/">CFFI</a> module
is also required if you wish to use the Python module.

<ul type="disc">
  <li>sudo pip install cffi</li>
</ul>

In order to build the documentation <a href="http://www.stack.nl/~dimitri/doxygen/">doxygen</a> is required.

<h2>Build Instructions</h2>

<p>The default build is for 32 bit machines</p>

<ol type="disc">
  <li>mkdir Release</li>
  <li>cd Release</li>
  <li>cmake ..</li>
  <li>make</li>
  <li>make test</li>
  <li>make doc</li>
  <li>sudo make install</li>
</ol>

<p>The build can be configured using by setting flags on the command line i.e.</p>

<ol type="disc">
  <li>cmake -DWORD_LENGTH=64 ..</li>
</ol>

<h2>Uninstall software</h2>

<ul type="disc">
  <li>sudo make uninstall</li>
</ul>

<h2>Building an installer</h2>

<p>After having built the libraries you can build a binary installer and a source distribution by running this command</p>

<ul type="disc">
  <li>make package</li>
</ul>


