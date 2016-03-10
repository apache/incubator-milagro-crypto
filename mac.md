<h2>Software dependencies</h2>

Install <a href="http://brew.sh/">Homebrew</a>

<ul type="disc">
  <li>ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"</li>
</ul>

Install  <a href="http://www.cmake.org/download/">cmake</a>

<ul type="disc">
  <li>brew install cmake</li>
</ul>

The C Foreign Function Interface for Python <a href="https://cffi.readthedocs.org/en/latest/">CFFI</a> module
is also required if you wish to use the Python module.

<ul type="disc">
  <li>brew install pkg-config libffi</li>
  <li>sudo pip install cffi</li>
</ul>

In order to build the documentation <a href="http://www.stack.nl/~dimitri/doxygen/">doxygen</a> is required.

<ul type="disc">
  <li>brew install doxygen</li>
</ul>

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



