# JavaScript tests

## Description 

These tests read test vector files that have been generated from the C code
implementation of MPin. There are two test vector files; BNCX.json for three pass 
and BNCSOnePass.json for one pass. The only curve tested in BNCX.

### Dependencies

Install the following node.js modules to run the tests

npm install assert
npm install fs
npm install crypto

### Configuration

If required set DEBUG = true in config.js to enable more verbose output.

### Run tests

./run_test.sh 

To run individual tests look inside the script for guidance. 
