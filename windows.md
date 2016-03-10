<h2>Software dependencies</h2>

<p>Minimalist GNU for Windows <a href="http://www.mingw.org/">MinGW</a> provides the 
tool set used to build the library and should be installed. When the MinGW installer
starts select the mingw32-base and mingw32-gcc-g++ components. From the menu select
"Installation" -> "Apply Changes", then click "Apply". Finally add C:\MinGW\bin
to the PATH variable.</p>

<p>CMake is required to build the library and can be downloaded from www.cmake.org</p>

<p>The C Foreign Function Interface for Python <a href="https://cffi.readthedocs.org/en/latest/">CFFI</a> module
is also required, if you wish to use the Python module.</p>

<ul type="disc">
  <li>pip install cffi</li>
</ul>

In order to build the documentation <a href="http://www.stack.nl/~dimitri/doxygen/">doxygen</a> is required.

<h2>Build Instructions</h2>

<p>Start a command prompt as an administrator</p>

<p>The default build is for 32 bit machines</p>

<ol type="disc">
  <li>mkdir Release</li>
  <li>cd Release</li>
  <li>cmake -G "MinGW Makefiles" ..</li>
  <li>mingw32-make</li>
  <li>mingw32-make test</li>
  <li>mingw32-make doc</li>
  <li>mingw32-make install</li>
</ol>

<p>

Post install append the PATH system variable to point to the install ./lib.

My Computer -> Properties -> Advanced > Environment Variables

</p>


<p>The build can be configured using by setting flags on the command line i.e.</p>

<ol type="disc">
  <li>cmake -G "MinGW Makefiles" -DWORD_LENGTH=64 ..</li>
</ol>

<h2>Uninstall software</h2>

<ul type="disc">
  <li>mingw32-make uninstall</li>
</ul>

<h2>Building an installer</h2>

<p>After having built the libraries you can build a Windows installer using this command</p>

<ul type="disc">
  <li>sudo mingw32-make package</li>
</ul>

<p>In order for this to work <a href="http://nsis.sourceforge.net/Download">NSSI</a> has
to have been installed</p>




