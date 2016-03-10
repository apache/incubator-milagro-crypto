#!/bin/sh
# Run headless JavaScript client tests 
#
# This script runs a number of successful and then
# unsuccessful authentications for WS and AJAX
#
# usage: ./run_headless_tests.sh [nWS_good] [nWS_bad] [nAJAX_good] [nAJAX_bad]

output_file="test_log_headless.txt"

echo "Run $1 headless JavaScript client tests for WS successful authentication" 
echo "Run $1 headless JavaScript client tests for WS successful authentication" > $output_file
for (( c=1; c<=$1; c++ ))
do
  echo "node test_good_PIN_WS.js iter $c"
  echo "node test_good_PIN_WS.js iter=$c" >> $output_file
  node test_good_PIN_WS.js >> $output_file 2>&1
  if [ -n "$failed" ]; then
     echo "A TEST HAS FAILED. Please review ${output_file}"
     exit 1
  fi
done

echo "Run $2 headless JavaScript client tests for WS failed authentication"
echo "Run $2 headless JavaScript client tests for WS failed authentication" >> $output_file
for (( c=1; c<=$2; c++ ))
do
  echo "node test_bad_PIN_WS.js iter $c"
  echo "node test_bad_PIN_WS.js iter=$c" >> $output_file
  node test_bad_PIN_WS.js >> $output_file 2>&1
  if [ -n "$failed" ]; then
     echo "A TEST HAS FAILED. Please review ${output_file}"
     exit 1
  fi
done

echo "Run $3 headless JavaScript client tests for AJAX successful authentication" 
echo "Run $3 headless JavaScript client tests for AJAX successful authentication" >> $output_file
for (( c=1; c<=$3; c++ ))
do
  echo "node test_good_PIN_AJAX.js iter $c"
  echo "node test_good_PIN_AJAX.js iter=$c" >> $output_file
  node test_good_PIN_AJAX.js >> $output_file 2>&1
  if [ -n "$failed" ]; then
     echo "A TEST HAS FAILED. Please review ${output_file}"
     exit 1
  fi
done

echo "Run $4 headless JavaScript client tests for AJAX failed authentication" 
echo "Run $4 headless JavaScript client tests for AJAX failed authentication" >> $output_file
for (( c=1; c<=$4; c++ ))
do
  echo "node test_bad_PIN_AJAX.js iter $c"
  echo "node test_bad_PIN_AJAX.js iter=$c" >> $output_file
  node test_bad_PIN_AJAX.js >> $output_file 2>&1
  if [ -n "$failed" ]; then
     echo "A TEST HAS FAILED. Please review ${output_file}"
     exit 1
  fi
done

echo "ALL TESTS PASSED"
