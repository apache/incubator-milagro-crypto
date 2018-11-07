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

//
//  TestMPIN.swift
//
//  Created by Michael Scott on 08/07/2015.
//  Copyright (c) 2015 Michael Scott. All rights reserved.
//

import Foundation
//import amcl  // comment out for Xcode

public func TestMPIN()
{
    let PERMITS=true
    let PINERROR=true
    let FULL=true
    let SINGLE_PASS=false
   
    let rng=RAND()
    
    var RAW=[UInt8](repeating: 0,count: 100)
    
    for i in 0 ..< 100 {RAW[i]=UInt8((i+1)&0xff)}
    rng.seed(100,RAW)
    
    let EGS=MPIN.EFS
    let EFS=MPIN.EGS
    let G1S=2*EFS+1    // Group 1 Size
    let G2S=4*EFS;     // Group 2 Size
    let EAS=MPIN.PAS
    
    let sha=MPIN.HASH_TYPE
    
    var S=[UInt8](repeating: 0,count: EGS)
    var SST=[UInt8](repeating: 0,count: G2S)
    var TOKEN=[UInt8](repeating: 0,count: G1S)
    var PERMIT=[UInt8](repeating: 0,count: G1S)
    var SEC=[UInt8](repeating: 0,count: G1S)
    var xID=[UInt8](repeating: 0,count: G1S)
    var xCID=[UInt8](repeating: 0,count: G1S)
    var X=[UInt8](repeating: 0,count: EGS)
    var Y=[UInt8](repeating: 0,count: EGS)
    var E=[UInt8](repeating: 0,count: 12*EFS)
    var F=[UInt8](repeating: 0,count: 12*EFS)
    var HID=[UInt8](repeating: 0,count: G1S)
    var HTID=[UInt8](repeating: 0,count: G1S)

    var G1=[UInt8](repeating: 0,count: 12*EFS)
    var G2=[UInt8](repeating: 0,count: 12*EFS)
    var R=[UInt8](repeating: 0,count: EGS)
    var Z=[UInt8](repeating: 0,count: G1S)
    var W=[UInt8](repeating: 0,count: EGS)
    var T=[UInt8](repeating: 0,count: G1S)
    var CK=[UInt8](repeating: 0,count: EAS)
    var SK=[UInt8](repeating: 0,count: EAS)

    var HSID=[UInt8]()

    // Trusted Authority set-up
    
    MPIN.RANDOM_GENERATE(rng,&S)
    print("Master Secret s: 0x",terminator: "");  MPIN.printBinary(S)
    
    // Create Client Identity
    let IDstr = "testUser@miracl.com"
    let CLIENT_ID=[UInt8](IDstr.utf8)
    
    var HCID=MPIN.HASH_ID(sha,CLIENT_ID)  // Either Client or TA calculates Hash(ID) - you decide!
    
    print("Client ID= "); MPIN.printBinary(CLIENT_ID)
    
    // Client and Server are issued secrets by DTA
    MPIN.GET_SERVER_SECRET(S,&SST);
    print("Server Secret SS: 0x",terminator: "");  MPIN.printBinary(SST);
    
    MPIN.GET_CLIENT_SECRET(&S,HCID,&TOKEN);
    print("Client Secret CS: 0x",terminator: ""); MPIN.printBinary(TOKEN);
    
    // Client extracts PIN from secret to create Token
    var pin:Int32=1234
    print("Client extracts PIN= \(pin)")
    var rtn=MPIN.EXTRACT_PIN(sha,CLIENT_ID,pin,&TOKEN)
    if rtn != 0 {print("FAILURE: EXTRACT_PIN rtn: \(rtn)")}
    
    print("Client Token TK: 0x",terminator: ""); MPIN.printBinary(TOKEN);

    if FULL
    {
        MPIN.PRECOMPUTE(TOKEN,HCID,&G1,&G2);
    }
    
    var date:Int32=0
    if (PERMITS)
    {
        date=MPIN.today()
        // Client gets "Time Token" permit from DTA
        MPIN.GET_CLIENT_PERMIT(sha,date,S,HCID,&PERMIT)
        print("Time Permit TP: 0x",terminator: "");  MPIN.printBinary(PERMIT)
        
        // This encoding makes Time permit look random - Elligator squared
        MPIN.ENCODING(rng,&PERMIT);
        print("Encoded Time Permit TP: 0x",terminator: "");  MPIN.printBinary(PERMIT)
        MPIN.DECODING(&PERMIT)
        print("Decoded Time Permit TP: 0x",terminator: "");  MPIN.printBinary(PERMIT)
    }

    // ***** NOW ENTER PIN *******
    
        pin=1234
    
    // **************************
    
    // Set date=0 and PERMIT=null if time permits not in use
    
    //Client First pass: Inputs CLIENT_ID, optional RNG, pin, TOKEN and PERMIT. Output xID =x .H(CLIENT_ID) and re-combined secret SEC
    //If PERMITS are is use, then date!=0 and PERMIT is added to secret and xCID = x.(H(CLIENT_ID)+H(date|H(CLIENT_ID)))
    //Random value x is supplied externally if RNG=null, otherwise generated and passed out by RNG
    
    //IMPORTANT: To save space and time..
    //If Time Permits OFF set xCID = null, HTID=null and use xID and HID only
    //If Time permits are ON, AND pin error detection is required then all of xID, xCID, HID and HTID are required
    //If Time permits are ON, AND pin error detection is NOT required, set xID=null, HID=null and use xCID and HTID only.
    
    
    var pxID:[UInt8]?=xID
    var pxCID:[UInt8]?=xCID
    var pHID:[UInt8]=HID
    var pHTID:[UInt8]?=HTID
    var pE:[UInt8]?=E
    var pF:[UInt8]?=F
    var pPERMIT:[UInt8]?=PERMIT
    
    if date != 0
    {
        if (!PINERROR)
        {
            pxID=nil;
   //         pHID=nil;
        }
    }
    else
    {
        pPERMIT=nil;
        pxCID=nil;
        pHTID=nil;
    }
    if (!PINERROR)
    {
        pE=nil;
        pF=nil;
    }
    
    if (SINGLE_PASS)
    {
        print("MPIN Single Pass")
        let timeValue = MPIN.GET_TIME()

        rtn=MPIN.CLIENT(sha,date,CLIENT_ID,rng,&X,pin,TOKEN,&SEC,&pxID,&pxCID,pPERMIT!,timeValue,&Y)
        
        if rtn != 0 {print("FAILURE: CLIENT rtn: \(rtn)")}
        
        if (FULL)
        {
            HCID=MPIN.HASH_ID(sha,CLIENT_ID);
            MPIN.GET_G1_MULTIPLE(rng,1,&R,HCID,&Z); // Also Send Z=r.ID to Server, remember random r
        }
        rtn=MPIN.SERVER(sha,date,&pHID,&pHTID,&Y,SST,pxID,pxCID!,SEC,&pE,&pF,CLIENT_ID,timeValue)
        if rtn != 0 {print("FAILURE: SERVER rtn: \(rtn)")}
        
        if (FULL)
        { // Also send T=w.ID to client, remember random w
            HSID=MPIN.HASH_ID(sha,CLIENT_ID);	
            if date != 0 {MPIN.GET_G1_MULTIPLE(rng,0,&W,pHTID!,&T)}
            else {MPIN.GET_G1_MULTIPLE(rng,0,&W,pHID,&T)}
            
        }
    }
    else
    {
        print("MPIN Multi Pass");
        // Send U=x.ID to server, and recreate secret from token and pin
        rtn=MPIN.CLIENT_1(sha,date,CLIENT_ID,rng,&X,pin,TOKEN,&SEC,&pxID,&pxCID,pPERMIT!)
        if rtn != 0 {print("FAILURE: CLIENT_1 rtn: \(rtn)")}
            
        if (FULL)
        {
            HCID=MPIN.HASH_ID(sha,CLIENT_ID);
            MPIN.GET_G1_MULTIPLE(rng,1,&R,HCID,&Z);  // Also Send Z=r.ID to Server, remember random r
        }
            
        // Server calculates H(ID) and H(T|H(ID)) (if time permits enabled), and maps them to points on the curve HID and HTID resp.
        MPIN.SERVER_1(sha,date,CLIENT_ID,&pHID,&pHTID!);
            
            // Server generates Random number Y and sends it to Client
        MPIN.RANDOM_GENERATE(rng,&Y);
            
        if (FULL)
        { // Also send T=w.ID to client, remember random w
            HSID=MPIN.HASH_ID(sha,CLIENT_ID);
            if date != 0 {MPIN.GET_G1_MULTIPLE(rng,0,&W,pHTID!,&T)}
            else {MPIN.GET_G1_MULTIPLE(rng,0,&W,pHID,&T)}
        }
            
        // Client Second Pass: Inputs Client secret SEC, x and y. Outputs -(x+y)*SEC
        rtn=MPIN.CLIENT_2(X,Y,&SEC);
        if rtn != 0 {print("FAILURE: CLIENT_2 rtn: \(rtn)")}
            
        // Server Second pass. Inputs hashed client id, random Y, -(x+y)*SEC, xID and xCID and Server secret SST. E and F help kangaroos to find error.
        // If PIN error not required, set E and F = null
            
        rtn=MPIN.SERVER_2(date,pHID,pHTID!,Y,SST,pxID!,pxCID!,SEC,&pE,&pF);
            
        if rtn != 0 {print("FAILURE: SERVER_1 rtn: \(rtn)")}
    }
    if (rtn == MPIN.BAD_PIN)
    {
        print("Server says - Bad Pin. I don't know you. Feck off.\n");
        if (PINERROR)
        {
            let err=MPIN.KANGAROO(pE!,pF!);
            if err != 0 {print("(Client PIN is out by \(err))\n")}
        }
        return;
    }
    else {print("Server says - PIN is good! You really are "+IDstr)}

    if (FULL)
    {
        var H=MPIN.HASH_ALL(sha,HCID,pxID!,pxCID!,SEC,Y,Z,T);
        MPIN.CLIENT_KEY(sha,G1,G2,pin,R,X,H,T,&CK);
        print("Client Key =  0x",terminator: "");  MPIN.printBinary(CK)
        
        H=MPIN.HASH_ALL(sha,HSID,pxID!,pxCID!,SEC,Y,Z,T);
        MPIN.SERVER_KEY(sha,Z,SST,W,H,pHID,pxID!,pxCID!,&SK);
        print("Server Key =  0x",terminator: "");  MPIN.printBinary(SK)
    }
    
}

//TestMPIN() // comment out for Xcode

