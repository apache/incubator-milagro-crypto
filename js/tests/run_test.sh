#!/bin/sh
# javascript tests 
#
# This script runs tests that compares the js
# with the expected output from the c code
#
# usage: ./run_test.sh

output_file="log.txt"
if [[ -f "$output_file" ]]
then
  echo "rm $output_file"
  rm $output_file
fi

ln -s BNCX.json testVectors.json
ln -s BNCXOnePass.json testVectorsOnePass.json

echo "cp ../MPIN.js ."
cp ../MPIN.js .
sed -i 's/var MPIN/MPIN/' MPIN.js

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

error=$(grep -i error "${output_file}" )
if [[ -n "$error" ]]; then
   echo "ERROR. Please review ${output_file}"
   exit 1
fi

failed=$(grep FAILED "${output_file}" )
if [[ -n "$failed" ]]; then
   echo "A TEST HAS FAILED. Please review ${output_file}"
   echo "A TEST HAS FAILED. Please review ${output_file}" >> $output_file 
else
   echo "ALL TESTS PASSED"
   echo "ALL TESTS PASSED" >> $output_file 
fi

rm testVectors.json
rm testVectorsOnePass.json
