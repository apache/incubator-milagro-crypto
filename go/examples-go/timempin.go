/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

package main

import (
	"encoding/hex"
	"flag"
	"log"
	"os"
	"runtime/pprof"
	"time"

	amcl "git.apache.org/incubator-milagro-crypto.git/go/amcl-go"
)

// Number of iterations to time functions
const nIter int = 10

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")

func main() {
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}

	// Assign the End-User an ID
	IDstr := "testUser@miracl.com"
	ID := []byte(IDstr)

	// Epoch time in days
	date := 16673

	// Epoch time in seconds
	timeValue := 1440594584

	SSHex := "07f8181687f42ce22ea0dee4ba9df3f2cea67ad2d79e59adc953142556d510831bbd59e9477ac479019887020579aed16af43dc7089ae8c14262e64b5d09740109917efd0618c557fbf7efaa68fb64e8d46b3766bb184dea9bef9638f23bbbeb03aedbc6e4eb9fbd658719aab26b849638690521723c0efb9c8622df2a8efa3c"
	SS, _ := hex.DecodeString(SSHex)
	UHex := "0403e76a28df08ea591912e0ff84ebf419e21aadf8ec5aed4b0f3cd0fc1cdea14a06f05a3be4f9f2d16530c6b4934da2e3439ea287796faac079d396f8cdb9f565"
	U, _ := hex.DecodeString(UHex)
	UTHex := "041012e53c991edc9514889de50fb7d893c406dc9bf4c89d46fec9ba408cc5f596226402e7c468c823a28b9003a3944c4600a1b797f10cf01060d3729729212932"
	UT, _ := hex.DecodeString(UTHex)
	SECHex := "04051b0d3e9dfdb2a378f0ac7056fb264a900d0867e39c334950527d8c460d76132346bf8ed8a419e2eab4ad52a8b7a51d8c09cbcfa4e80bc0487965ece72ab0ce"
	SEC, _ := hex.DecodeString(SECHex)
	var MESSAGE []byte
	// MESSAGE := []byte("test sign message")

	const EGS = amcl.MPIN_EGS
	const EFS = amcl.MPIN_EFS
	const G1S = 2*EFS + 1 /* Group 1 Size */
	const G2S = 4 * EFS   /* Group 2 Size */

	var Y [EGS]byte
	var E [12 * EFS]byte
	var F [12 * EFS]byte
	var HID [G1S]byte
	var HTID [G1S]byte

	//////   Server   //////
	t0 := time.Now()
	rtn := 0
	for i := 0; i < nIter; i++ {
		rtn = amcl.MPIN_SERVER(date, HID[:], HTID[:], Y[:], SS[:], U[:], UT[:], SEC[:], E[:], F[:], ID, MESSAGE, timeValue)
	}
	t1 := time.Now()
	log.Printf("Number Iterations: %d Time: %v\n", nIter, t1.Sub(t0))

	if rtn == amcl.MPIN_BAD_PIN {
		log.Printf("Authentication failed Error Code %d\n", rtn)
		return
	} else {
		log.Printf("Authenticated ID: %s \n", IDstr)
	}
}
