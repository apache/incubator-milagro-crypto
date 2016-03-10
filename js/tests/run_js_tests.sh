#!/bin/sh
# javascript tests 
#
# This script runs tests that compares the js
# with the expected output from the c code which
# is interfaced through the python wrapper.
#
# usage: ./run_js_tests.sh [success authentication] [failed authentication] [epoch days test]

output_file="test_log_js.txt"

# Generate vectors.
# ./genVectors.py $1 $2 $3

file="testVectors.json"
if [ -f "$file" ]
then
  echo "$file found."
else
  echo "$file not found."
  exit 1
fi

file="testVectorsOnePass.json"
if [ -f "$file" ]
then
  echo "$file found."
else
  echo "$file not found."
  exit 1
fi

echo "TEST 1: node test_add_shares.js"
echo "TEST 1: node test_add_shares.js" > $output_file 
node test_add_shares.js >> $output_file 2>&1

echo "TEST 2: node test_token.js"
echo "TEST 2: node test_token.js" >> $output_file 
node test_token.js >> $output_file 2>&1

echo "TEST 3: node test_pass1.js"
echo "TEST 3: node test_pass1.js" >> $output_file 
node test_pass1.js >> $output_file 2>&1

echo "TEST 4: node test_pass2.js"
echo "TEST 4: node test_pass2.js" >> $output_file 
node test_pass2.js >> $output_file 2>&1

echo "TEST 5: node test_randomX.js"
echo "TEST 5: node test_randomX.js" >> $output_file 
node test_randomX.js >> $output_file 2>&1
./find_duplicates.py >> $output_file 2>&1

echo "TEST 6: node test_sha256.js"
echo "TEST 6: node test_sha265.js" >> $output_file 
node test_sha256.js >> $output_file 2>&1

echo "TEST 7: node test_onepass.js"
echo "TEST 7: node test_onepass.js" >> $output_file 
node test_onepass.js >> $output_file 2>&1

failed=$(grep FAILED "${output_file}" )
if [ -n "$failed" ]; then
   echo "A TEST HAS FAILED. Please review ${output_file}"
   echo "A TEST HAS FAILED. Please review ${output_file}" >> $output_file 
else
   echo "ALL TESTS PASSED"
   echo "ALL TESTS PASSED" >> $output_file 
fi
