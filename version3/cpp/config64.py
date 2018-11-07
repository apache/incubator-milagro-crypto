import os
import sys

deltext=""
if sys.platform.startswith("linux")  :
	deltext="rm"
	copytext="cp"
if sys.platform.startswith("darwin")  :
	deltext="rm"
	copytext="cp"
if sys.platform.startswith("win") :
	deltext="del"
	copytext="copy"

def replace(namefile,oldtext,newtext):
	f = open(namefile,'r')
	filedata = f.read()
	f.close()

	newdata = filedata.replace(oldtext,newtext)

	f = open(namefile,'w')
	f.write(newdata)
	f.close()


def rsaset(tb,tff,nb,base,ml) :
	bd="B"+tb+"_"+base
	fnameh="config_big_"+bd+".h"
	os.system(copytext+" config_big.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"@NB@",nb)
	replace(fnameh,"@BASE@",base)

	fnameh="config_ff_"+tff+".h"
	os.system(copytext+" config_ff.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"WWW",tff)
	replace(fnameh,"@ML@",ml)

	fnamec="big_"+bd+".cpp"
	fnameh="big_"+bd+".h"

	os.system(copytext+" big.cpp "+fnamec)
	os.system(copytext+" big.h "+fnameh)

	replace(fnamec,"XXX",bd)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	fnamec="ff_"+tff+".cpp"
	fnameh="ff_"+tff+".h"

	os.system(copytext+" ff.cpp "+fnamec)
	os.system(copytext+" ff.h "+fnameh)

	replace(fnamec,"WWW",tff)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"WWW",tff)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	fnamec="rsa_"+tff+".cpp"
	fnameh="rsa_"+tff+".h"

	os.system(copytext+" rsa.cpp "+fnamec)
	os.system(copytext+" rsa.h "+fnameh)

	replace(fnamec,"WWW",tff)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"WWW",tff)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

def curveset(tb,tf,tc,nb,base,nbt,m8,mt,ct,pf,stw,sx,cs) :
	bd="B"+tb+"_"+base
	fnameh="config_big_"+bd+".h"
	os.system(copytext+" config_big.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"@NB@",nb)
	replace(fnameh,"@BASE@",base)

	fnameh="config_field_"+tf+".h"
	os.system(copytext+" config_field.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"@NBT@",nbt)
	replace(fnameh,"@M8@",m8)
	replace(fnameh,"@MT@",mt)

	ib=int(base)
	inb=int(nb)
	inbt=int(nbt)
	sh=ib*(1+((8*inb-1)//ib))-inbt

	if sh > 30 :
		sh=30
	replace(fnameh,"@SH@",str(sh))

	fnameh="config_curve_"+tc+".h"	
	os.system(copytext+" config_curve.h "+fnameh)
	replace(fnameh,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"@CT@",ct)
	replace(fnameh,"@PF@",pf)

	replace(fnameh,"@ST@",stw)
	replace(fnameh,"@SX@",sx)
	replace(fnameh,"@CS@",cs)


	fnamec="big_"+bd+".cpp"
	fnameh="big_"+bd+".h"

	os.system(copytext+" big.cpp "+fnamec)
	os.system(copytext+" big.h "+fnameh)

	replace(fnamec,"XXX",bd)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	fnamec="fp_"+tf+".cpp"
	fnameh="fp_"+tf+".h"

	os.system(copytext+" fp.cpp "+fnamec)
	os.system(copytext+" fp.h "+fnameh)

	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	os.system("g++ -O3 -c rom_field_"+tf+".cpp")

	fnamec="ecp_"+tc+".cpp"
	fnameh="ecp_"+tc+".h"

	os.system(copytext+" ecp.cpp "+fnamec)
	os.system(copytext+" ecp.h "+fnameh)

	replace(fnamec,"ZZZ",tc)
	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	fnamec="ecdh_"+tc+".cpp"
	fnameh="ecdh_"+tc+".h"

	os.system(copytext+" ecdh.cpp "+fnamec)
	os.system(copytext+" ecdh.h "+fnameh)

	replace(fnamec,"ZZZ",tc)
	replace(fnamec,"YYY",tf)
	replace(fnamec,"XXX",bd)
	replace(fnameh,"ZZZ",tc)
	replace(fnameh,"YYY",tf)
	replace(fnameh,"XXX",bd)
	os.system("g++ -O3 -c "+fnamec)

	os.system("g++ -O3 -c rom_curve_"+tc+".cpp")

	if pf != "NOT" :
		fnamec="fp2_"+tf+".cpp"
		fnameh="fp2_"+tf+".h"

		os.system(copytext+" fp2.cpp "+fnamec)
		os.system(copytext+" fp2.h "+fnameh)
		replace(fnamec,"YYY",tf)
		replace(fnamec,"XXX",bd)
		replace(fnameh,"YYY",tf)
		replace(fnameh,"XXX",bd)
		os.system("g++ -O3 -c "+fnamec)

		fnamec="fp4_"+tf+".cpp"
		fnameh="fp4_"+tf+".h"

		os.system(copytext+" fp4.cpp "+fnamec)
		os.system(copytext+" fp4.h "+fnameh)
		replace(fnamec,"YYY",tf)
		replace(fnamec,"XXX",bd)
		replace(fnamec,"ZZZ",tc)
		replace(fnameh,"YYY",tf)
		replace(fnameh,"XXX",bd)
		replace(fnameh,"ZZZ",tc)
		os.system("g++ -O3 -c "+fnamec)

		if cs == "128" :
			fnamec="fp12_"+tf+".cpp"
			fnameh="fp12_"+tf+".h"

			os.system(copytext+" fp12.cpp "+fnamec)
			os.system(copytext+" fp12.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="ecp2_"+tc+".cpp"
			fnameh="ecp2_"+tc+".h"

			os.system(copytext+" ecp2.cpp "+fnamec)
			os.system(copytext+" ecp2.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="pair_"+tc+".cpp"
			fnameh="pair_"+tc+".h"

			os.system(copytext+" pair.cpp "+fnamec)
			os.system(copytext+" pair.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="mpin_"+tc+".cpp"
			fnameh="mpin_"+tc+".h"

			os.system(copytext+" mpin.cpp "+fnamec)
			os.system(copytext+" mpin.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

		if cs == "192" :
			fnamec="fp8_"+tf+".cpp"
			fnameh="fp8_"+tf+".h"

			os.system(copytext+" fp8.cpp "+fnamec)
			os.system(copytext+" fp8.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnamec,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			os.system("g++ -O3 -c "+fnamec)


			fnamec="fp24_"+tf+".cpp"
			fnameh="fp24_"+tf+".h"

			os.system(copytext+" fp24.cpp "+fnamec)
			os.system(copytext+" fp24.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="ecp4_"+tc+".cpp"
			fnameh="ecp4_"+tc+".h"

			os.system(copytext+" ecp4.cpp "+fnamec)
			os.system(copytext+" ecp4.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="pair192_"+tc+".cpp"
			fnameh="pair192_"+tc+".h"

			os.system(copytext+" pair192.cpp "+fnamec)
			os.system(copytext+" pair192.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="mpin192_"+tc+".cpp"
			fnameh="mpin192_"+tc+".h"

			os.system(copytext+" mpin192.cpp "+fnamec)
			os.system(copytext+" mpin192.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

		if cs == "256" :

			fnamec="fp8_"+tf+".cpp"
			fnameh="fp8_"+tf+".h"

			os.system(copytext+" fp8.cpp "+fnamec)
			os.system(copytext+" fp8.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnamec,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			os.system("g++ -O3 -c "+fnamec)


			fnamec="ecp8_"+tc+".cpp"
			fnameh="ecp8_"+tc+".h"

			os.system(copytext+" ecp8.cpp "+fnamec)
			os.system(copytext+" ecp8.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)


			fnamec="fp16_"+tf+".cpp"
			fnameh="fp16_"+tf+".h"

			os.system(copytext+" fp16.cpp "+fnamec)
			os.system(copytext+" fp16.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnamec,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			os.system("g++ -O3 -c "+fnamec)


			fnamec="fp48_"+tf+".cpp"
			fnameh="fp48_"+tf+".h"

			os.system(copytext+" fp48.cpp "+fnamec)
			os.system(copytext+" fp48.h "+fnameh)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)


			fnamec="pair256_"+tc+".cpp"
			fnameh="pair256_"+tc+".h"

			os.system(copytext+" pair256.cpp "+fnamec)
			os.system(copytext+" pair256.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

			fnamec="mpin256_"+tc+".cpp"
			fnameh="mpin256_"+tc+".h"

			os.system(copytext+" mpin256.cpp "+fnamec)
			os.system(copytext+" mpin256.h "+fnameh)
			replace(fnamec,"ZZZ",tc)
			replace(fnamec,"YYY",tf)
			replace(fnamec,"XXX",bd)
			replace(fnameh,"ZZZ",tc)
			replace(fnameh,"YYY",tf)
			replace(fnameh,"XXX",bd)
			os.system("g++ -O3 -c "+fnamec)

replace("arch.h","@WL@","64")
print("Elliptic Curves")
print("1. ED25519")
print("2. C25519")
print("3. NIST256")
print("4. BRAINPOOL")
print("5. ANSSI")
print("6. HIFIVE")
print("7. GOLDILOCKS")
print("8. NIST384")
print("9. C41417")
print("10. NIST521\n")
print("11. NUMS256W")
print("12. NUMS256E")
print("13. NUMS384W")
print("14. NUMS384E")
print("15. NUMS512W")
print("16. NUMS512E")
print("17. SECP256K1\n")

print("Pairing-Friendly Elliptic Curves")
print("18. BN254")
print("19. BN254CX")
print("20. BLS383")
print("21. BLS381")
print("22. FP256BN")
print("23. FP512BN")
print("24. BLS461\n")
print("25. BLS24")
print("26. BLS48\n")

print("RSA")
print("27. RSA2048")
print("28. RSA3072")
print("29. RSA4096")

selection=[]
ptr=0
max=30

curve_selected=False
pfcurve_selected=False
rsa_selected=False

while ptr<max:
	x=int(input("Choose a Scheme to support - 0 to finish: "))
	if x == 0:
		break
#	print("Choice= ",x)
	already=False
	for i in range(0,ptr):
		if x==selection[i]:
			already=True
			break
	if already:
		continue
	
	selection.append(x)
	ptr=ptr+1

# curveset(big,field,curve,big_length_bytes,bits_in_base,modulus_bits,modulus_mod_8,modulus_type,curve_type,pairing_friendly,sextic twist,sign of x,curve security)
# for each curve give names for big, field and curve. In many cases the latter two will be the same. 
# Typically "big" is the size in bits, always a multiple of 8, "field" describes the modulus, and "curve" is the common name for the elliptic curve   
# big_length_bytes is "big" divided by 8
# Next give the number base used for 64 bit architectures, as n where the base is 2^n (note that these must be fixed for the same "big" name, if is ever re-used for another curve)
# modulus_bits is the bit length of the modulus, typically the same or slightly smaller than "big"
# modulus_mod_8 is the remainder when the modulus is divided by 8
# modulus_type is NOT_SPECIAL, or PSEUDO_MERSENNE, or MONTGOMERY_Friendly, or GENERALISED_MERSENNE (supported for GOLDILOCKS only)
# curve_type is WEIERSTRASS, EDWARDS or MONTGOMERY
# pairing_friendly is BN, BLS or NOT (if not pairing friendly)
# if pairing friendly. M or D type twist, and sign of the family parameter x
# curve security is AES equiavlent, rounded up.


	if x==1:
		curveset("256","F25519","ED25519","32","56","255","5","PSEUDO_MERSENNE","EDWARDS","NOT","","","128")
		curve_selected=True
	if x==2:
		curveset("256","F25519","C25519","32","56","255","5","PSEUDO_MERSENNE","MONTGOMERY","NOT","","","128")
		curve_selected=True
	if x==3:
		curveset("256","NIST256","NIST256","32","56","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","","","128")
		curve_selected=True
	if x==4:
		curveset("256","BRAINPOOL","BRAINPOOL","32","56","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","","","128")
		curve_selected=True
	if x==5:
		curveset("256","ANSSI","ANSSI","32","56","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","","","128")
		curve_selected=True

	if x==6:
		curveset("336","HIFIVE","HIFIVE","42","60","336","5","PSEUDO_MERSENNE","EDWARDS","NOT","","","192")
		curve_selected=True
	if x==7:
		curveset("448","GOLDILOCKS","GOLDILOCKS","56","58","448","7","GENERALISED_MERSENNE","EDWARDS","NOT","","","256")
		curve_selected=True
	if x==8:
		curveset("384","NIST384","NIST384","48","56","384","7","NOT_SPECIAL","WEIERSTRASS","NOT","","","192")
		curve_selected=True
	if x==9:
		curveset("416","C41417","C41417","52","60","414","7","PSEUDO_MERSENNE","EDWARDS","NOT","","","256")
		curve_selected=True
	if x==10:
		curveset("528","NIST521","NIST521","66","60","521","7","PSEUDO_MERSENNE","WEIERSTRASS","NOT","","","256")
		curve_selected=True

	if x==11:
		curveset("256","F256PMW","NUMS256W","32","56","256","3","PSEUDO_MERSENNE","WEIERSTRASS","NOT","","","128")
		curve_selected=True
	if x==12:
		curveset("256","F256PME","NUMS256E","32","56","256","3","PSEUDO_MERSENNE","EDWARDS","NOT","","","128")
		curve_selected=True
	if x==13:
		curveset("384","F384PM","NUMS384W","48","56","384","3","PSEUDO_MERSENNE","WEIERSTRASS","NOT","","","192")
		curve_selected=True
	if x==14:
		curveset("384","F384PM","NUMS384E","48","56","384","3","PSEUDO_MERSENNE","EDWARDS","NOT","","","192")
		curve_selected=True
	if x==15:
		curveset("512","F512PM","NUMS512W","64","56","512","7","PSEUDO_MERSENNE","WEIERSTRASS","NOT","","","256")
		curve_selected=True
	if x==16:
		curveset("512","F512PM","NUMS512E","64","56","512","7","PSEUDO_MERSENNE","EDWARDS","NOT","","","256")
		curve_selected=True

	if x==17:
		curveset("256","SECP256K1","SECP256K1","32","56","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","","","128")
		curve_selected=True

	if x==18:
		curveset("256","BN254","BN254","32","56","254","3","NOT_SPECIAL","WEIERSTRASS","BN","D_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==19:
		curveset("256","BN254CX","BN254CX","32","56","254","3","NOT_SPECIAL","WEIERSTRASS","BN","D_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==20:
		curveset("384","BLS383","BLS383","48","58","383","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","128")
		pfcurve_selected=True

	if x==21:
		curveset("384","BLS381","BLS381","48","58","381","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True

	if x==22:
		curveset("256","FP256BN","FP256BN","32","56","256","3","NOT_SPECIAL","WEIERSTRASS","BN","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==23:
		curveset("512","FP512BN","FP512BN","64","60","512","3","NOT_SPECIAL","WEIERSTRASS","BN","M_TYPE","POSITIVEX","128")
		pfcurve_selected=True
# https://eprint.iacr.org/2017/334.pdf
	if x==24:
		curveset("464","BLS461","BLS461","58","60","461","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True

	if x==25:
		curveset("480","BLS24","BLS24","60","56","479","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","192")
		pfcurve_selected=True

	if x==26:
		curveset("560","BLS48","BLS48","70","58","556","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","256")
		pfcurve_selected=True


# rsaset(big,ring,big_length_bytes,bits_in_base,multiplier)
# for each choice give distinct names for "big" and "ring".
# Typically "big" is the length in bits of the underlying big number type
# "ring" is the RSA modulus size = "big" times 2^m
# big_length_bytes is "big" divided by 8
# Next give the number base used for 64 bit architectures, as n where the base is 2^n
# multiplier is 2^m (see above)

# There are choices here, different ways of getting the same result, but some faster than others
	if x==27:
		#256 is slower but may allow reuse of 256-bit BIGs used for elliptic curve
		#512 is faster.. but best is 1024
		rsaset("1024","RSA2048","128","58","2")
		#rsaset("512","RSA2048","64","60","4")
		#rsaset("256","RSA2048","32","56","8")
		rsa_selected=True
	if x==28:
		rsaset("384","RSA3072","48","56","8")
		rsa_selected=True
	if x==29:
		#rsaset("256","RSA4096","32","56","16")
		rsaset("512","RSA4096","64","60","8")
		rsa_selected=True


os.system(deltext+" big.*")
os.system(deltext+" fp.*")
os.system(deltext+" ecp.*")
os.system(deltext+" ecdh.*")
os.system(deltext+" ff.*")
os.system(deltext+" rsa.*")
os.system(deltext+" config_big.h")
os.system(deltext+" config_field.h")
os.system(deltext+" config_curve.h")
os.system(deltext+" config_ff.h")
os.system(deltext+" fp2.*")
os.system(deltext+" fp4.*")
os.system(deltext+" fp8.*")
os.system(deltext+" fp16.*")

os.system(deltext+" fp12.*")
os.system(deltext+" fp24.*")
os.system(deltext+" fp48.*")

os.system(deltext+" ecp2.*")
os.system(deltext+" ecp4.*")
os.system(deltext+" ecp8.*")

os.system(deltext+" pair.*")
os.system(deltext+" mpin.*")

os.system(deltext+" pair192.*")
os.system(deltext+" mpin192.*")

os.system(deltext+" pair256.*")
os.system(deltext+" mpin256.*")


# create library
os.system("g++ -O3 -c randapi.cpp")
if curve_selected :
	os.system("g++ -O3 -c ecdh_support.cpp")
if rsa_selected :
	os.system("g++ -O3 -c rsa_support.cpp")
if pfcurve_selected :
	os.system("g++ -O3 -c pbc_support.cpp")

os.system("g++ -O3 -c hash.cpp")
os.system("g++ -O3 -c rand.cpp")
os.system("g++ -O3 -c oct.cpp")
os.system("g++ -O3 -c aes.cpp")
os.system("g++ -O3 -c gcm.cpp")
os.system("g++ -O3 -c newhope.cpp")

if sys.platform.startswith("win") :
	os.system("for %i in (*.o) do @echo %~nxi >> f.list")
	os.system("ar rc amcl.a @f.list")
	os.system(deltext+" f.list")

else :
	os.system("ar rc amcl.a *.o")

os.system(deltext+" *.o")


#print("Your section was ")
#for i in range(0,ptr):
#	print (selection[i])

