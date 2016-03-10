The directory above contains the file MPINAuth.js 
which is example of how to use the AMCL 
JavaScript in order to authenticate with an 
M-Pin server. An example of how to use these
functions in given in TestMPINAuth.js and can
be run like so;

ln -s config.js_local config.js
node TestMPINAuth.js

or 

node TestMPINAuthOnePass.js

nb Insert your app_id and app_key into config.js

for one pass M-Pin

In this directory there are also two sets of 
tests. One will test the interaction between the 
JavaScript and C code using test vectors; the 
other tests this interaction using the web 
services.

################################################

Test Vectors:

1. Install these node.js modules;

   npm install ws
   npm install assert
   npm install http
   npm install fs
   npm install crypto

2. Configuration file 

   Set DEBUG = true in config.js to enable
   more verbose output, if required

3. Run a number of test vectors.

   Copy test vector file to this directory;
 
   cp ../../testVectors/mpin/BNCX.json testVectors.json
   cp ../../testVectors/mpin/BNCXOnePass.json testVectorsOnePass.json

   These files can be created using the generator
   scripts as long as the libraries are installed.

   ./genVectors.py [successful authentication] [failed authentication] [epoch days in future]
   ./genVectorsOnePass.py [successful authentication] [failed authentication] [epoch days in future]

   The JavaScript tests are then run using this script;

   ./run_js_tests.sh 

   To run individual tests look inside the script for guidance. 

################################################

Headless:

In order to run these tests the MIRACL D-TA, 
Customer D-TA, D-TA Proxy, M-Pin Auth and 
RPS Model servers are required.

1. Start MIRACL D-TA  
 
   cd mpin/webService/dtaCert
   ln -s config/config.py_encrypted config.py
   ln -s mss_backup/backup.json_encrypted backup.json
   ./dta.py 

2. Start D-TA Proxy
 
   n.b. Make sure MySQL is running and 8c63aa9f7639f15bf46f142a84fedc82 has been added
   to the Applications table

   cd mpin/webService/dtaProxy
   ln -s config.py_paid_tier_no_sqs config.py
   ln -s keys.json_test keys.json
   ./dtaProxy.py 

3. Start Customer D-TA  

   cd mpin/webService/dtaCust
   ln -s mpin-backend/servers/dta/dta.py .
   ln -s ./mss_backup/backup.json_encrypted backup.json
   ln -s ./config/config.py_encrypted config.py 
   ln -s ./credentials.json_test credentials.json
   ./dta.py

4. Start the M-Pin server 

   cd mpin/webService/mpinAuth
   ln mpin-backend/servers/mpin/mpinAuth.py .
   ln -s credentials.json_test credentials.json
   ln -s config.py_test config.py
   ./mpinAuth.py 
 
5. Start the RPS model server
 
   cd mpin/webService/mpinAuth/rpsModel
   ./rps.py

6. Run tests.

   ./run_headless_tests.sh [nWS_good] [nWS_bad] [nAJAX_good] [nAJAX_bad]

