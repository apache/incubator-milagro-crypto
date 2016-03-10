AMCL is very simple to build for Go.

First - decide the modulus and curve type you want to use. Edit ROM.go 
where indicated. You will probably want to use one of the curves whose 
details are already in there.

Three example API files are provided, MPIN.go which 
supports our M-Pin (tm) protocol, ECDH.go which supports elliptic 
curve key exchange, digital signature and public key crypto, and RSA.go
which supports the RSA method.

In the ROM.go file you must provide the curve constants. Several examples
are provided there, if you are willing to use one of these.

For a quick jumpstart:-

export GOPATH=$PWD

go run ./src/github.com/miracl/examples-go/mpin.go

or 

go run ./src/github.com/miracl/examples-go/ecdh.go

or

go run ./src/github.com/miracl/examples-go/rsa.go

