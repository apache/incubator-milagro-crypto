#!/bin/bash
#
# build-browser-examples.sh
#
# Convert Node.js example code to browser compatible code
#
# @author Kealan McCusker <kealan.mccusker@miracl.com>
# ------------------------------------------------------------------------------

# NOTES:

# EXAMPLE USAGE:
# ./build-browser-examples.sh

function build_examples {
    rm -f browser/*.html
    cp node/*.js browser/

    sed -i -e "/require(.*)/d" ./browser/*.js
    sed -i -e "/eval(.*)/d" ./browser/*.js
    sed -i -e "/return (-1)/d" ./browser/*.js

    for f in browser/*.js
    do
        sed -e "35r$f" template.html > "${f%.js}.html"
    done

    rm -f browser/*.js
}

build_examples
