import os
import sys

deltext=""
slashtext=""
copytext=""
if sys.platform.startswith("darwin")  :
	copytext="cp "
	deltext="rm "
	slashtext="/"
if sys.platform.startswith("linux")  :
	copytext="cp "
	deltext="rm "
	slashtext="/"
if sys.platform.startswith("win") :
	copytext="copy "
	deltext="del "
	slashtext="\\"

chosen=[]
cptr=0

def replace(namefile,oldtext,newtext):
	f = open(namefile,'r')
	filedata = f.read()
	f.close()

	newdata = filedata.replace(oldtext,newtext)

	f = open(namefile,'w')
	f.write(newdata)
	f.close()


def rsaset(tb,nb,base,ml) :
	global deltext,slashtext,copytext
	global cptr,chosen

	chosen.append(tb)
	cptr=cptr+1

	fpath="amcl"+slashtext+tb+slashtext
	os.system("mkdir amcl"+slashtext+tb)

	os.system(copytext+"ARCH32.go "+fpath+"ARCH.go")
	os.system(copytext+"BIG32.go "+fpath+"BIG.go")
	os.system(copytext+"DBIG.go "+fpath+"DBIG.go")
	os.system(copytext+"FF32.go "+fpath+"FF.go")
	os.system(copytext+"RSA.go "+fpath+"RSA.go")

	replace(fpath+"ARCH.go","XXX",tb)
	replace(fpath+"BIG.go","XXX",tb)
	replace(fpath+"DBIG.go","XXX",tb)
	replace(fpath+"FF.go","XXX",tb)
	replace(fpath+"RSA.go","XXX",tb)

	replace(fpath+"BIG.go","@NB@",nb)
	replace(fpath+"BIG.go","@BASE@",base)

	replace(fpath+"FF.go","@ML@",ml);

def curveset(tc,nb,base,nbt,m8,mt,ct,pf,stw,sx,cs) :
	global deltext,slashtext,copytext
	global cptr,chosen

	chosen.append(tc)
	cptr=cptr+1

	fpath="amcl"+slashtext+tc+slashtext
	os.system("mkdir amcl"+slashtext+tc)

	os.system(copytext+"ARCH32.go "+fpath+"ARCH.go")
	os.system(copytext+"BIG32.go "+fpath+"BIG.go")
	os.system(copytext+"DBIG.go "+fpath+"DBIG.go")
	os.system(copytext+"FP.go "+fpath+"FP.go")
	os.system(copytext+"ECP.go "+fpath+"ECP.go")
	os.system(copytext+"ECDH.go "+fpath+"ECDH.go")
	os.system(copytext+"ROM_"+tc+"_32.go "+fpath+"ROM.go")

	replace(fpath+"ARCH.go","XXX",tc)
	replace(fpath+"BIG.go","XXX",tc)
	replace(fpath+"DBIG.go","XXX",tc)
	replace(fpath+"FP.go","XXX",tc)
	replace(fpath+"ECP.go","XXX",tc)
	replace(fpath+"ECDH.go","XXX",tc)

	replace(fpath+"BIG.go","@NB@",nb)
	replace(fpath+"BIG.go","@BASE@",base)

	replace(fpath+"FP.go","@NBT@",nbt)
	replace(fpath+"FP.go","@M8@",m8)
	replace(fpath+"FP.go","@MT@",mt)

	ib=int(base)
	inb=int(nb)
	inbt=int(nbt)
	sh=ib*(1+((8*inb-1)//ib))-inbt
	if sh > 30 :
		sh=30
	replace(fpath+"FP.go","@SH@",str(sh))


	replace(fpath+"ECP.go","@CT@",ct)
	replace(fpath+"ECP.go","@PF@",pf)

	replace(fpath+"ECP.go","@ST@",stw)
	replace(fpath+"ECP.go","@SX@",sx)

	if cs == "128" :
		replace(fpath+"ECP.go","@HT@","32")
		replace(fpath+"ECP.go","@AK@","16")
	if cs == "192" :
		replace(fpath+"ECP.go","@HT@","48")
		replace(fpath+"ECP.go","@AK@","24")
	if cs == "256" :
		replace(fpath+"ECP.go","@HT@","64")
		replace(fpath+"ECP.go","@AK@","32")

	if pf != "NOT" :

		os.system(copytext+"FP2.go "+fpath+"FP2.go")
		os.system(copytext+"FP4.go "+fpath+"FP4.go")

		replace(fpath+"FP2.go","XXX",tc)
		replace(fpath+"FP4.go","XXX",tc)

		if cs == "128" :
			os.system(copytext+"ECP2.go "+fpath+"ECP2.go")
			os.system(copytext+"FP12.go "+fpath+"FP12.go")
			os.system(copytext+"PAIR.go "+fpath+"PAIR.go")
			os.system(copytext+"MPIN.go "+fpath+"MPIN.go")

			replace(fpath+"FP12.go","XXX",tc)
			replace(fpath+"ECP2.go","XXX",tc)
			replace(fpath+"PAIR.go","XXX",tc)
			replace(fpath+"MPIN.go","XXX",tc)

		if cs == "192" :
			os.system(copytext+"FP24.go "+fpath+"FP24.go")
			os.system(copytext+"FP8.go "+fpath+"FP8.go")
			os.system(copytext+"ECP4.go "+fpath+"ECP4.go")
			os.system(copytext+"PAIR192.go "+fpath+"PAIR192.go")
			os.system(copytext+"MPIN192.go "+fpath+"MPIN192.go")

			replace(fpath+"FP24.go","XXX",tc)
			replace(fpath+"FP8.go","XXX",tc)
			replace(fpath+"ECP4.go","XXX",tc)
			replace(fpath+"PAIR192.go","XXX",tc)
			replace(fpath+"MPIN192.go","XXX",tc)

		if cs == "256" :
			os.system(copytext+"FP48.go "+fpath+"FP48.go")
			os.system(copytext+"FP16.go "+fpath+"FP16.go")
			os.system(copytext+"FP8.go "+fpath+"FP8.go")
			os.system(copytext+"ECP8.go "+fpath+"ECP8.go")
			os.system(copytext+"PAIR256.go "+fpath+"PAIR256.go")
			os.system(copytext+"MPIN256.go "+fpath+"MPIN256.go")

			replace(fpath+"FP48.go","XXX",tc)
			replace(fpath+"FP16.go","XXX",tc)
			replace(fpath+"FP8.go","XXX",tc)
			replace(fpath+"ECP8.go","XXX",tc)
			replace(fpath+"PAIR256.go","XXX",tc)
			replace(fpath+"MPIN256.go","XXX",tc)



os.system("mkdir amcl")
os.system(copytext+ "HASH*.go amcl"+slashtext+".")
os.system(copytext+ "SHA3.go amcl"+slashtext+".")
os.system(copytext+ "RAND.go amcl"+slashtext+".")
os.system(copytext+ "AES.go amcl"+slashtext+".")
os.system(copytext+ "GCM.go amcl"+slashtext+".")
os.system(copytext+ "NHS.go amcl"+slashtext+".")

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

# curveset(curve,big_length_bytes,bits_in_base,modulus_bits,modulus_mod_8,modulus_type,curve_type,pairing_friendly,curve security)
# where "curve" is the common name for the elliptic curve
# big_length_bytes is the modulus size rounded up to a number of bytes
# bits_in_base gives the number base used for 32 bit architectures, as n where the base is 2^n
# modulus_bits is the actual bit length of the modulus.
# modulus_mod_8 is the remainder when the modulus is divided by 8
# modulus_type is NOT_SPECIAL, or PSEUDO_MERSENNE, or MONTGOMERY_Friendly, or GENERALISED_MERSENNE (supported for GOLDILOCKS only)
# curve_type is WEIERSTRASS, EDWARDS or MONTGOMERY
# pairing_friendly is BN, BLS or NOT (if not pairing friendly
# curve security is AES equiavlent, rounded up.


	if x==1:
		curveset("ED25519","32","29","255","5","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","128")
		curve_selected=True
	if x==2:
		curveset("C25519","32","29","255","5","PSEUDO_MERSENNE","MONTGOMERY","NOT","NOT","NOT","128")
		curve_selected=True
	if x==3:
		curveset("NIST256","32","28","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","NOT","NOT","128")
		curve_selected=True
	if x==4:
		curveset("BRAINPOOL","32","28","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","NOT","NOT","128")
		curve_selected=True
	if x==5:
		curveset("ANSSI","32","28","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","NOT","NOT","128")
		curve_selected=True

	if x==6:
		curveset("HIFIVE","42","29","336","5","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","192")
		curve_selected=True
	if x==7:
		curveset("GOLDILOCKS","56","29","448","7","GENERALISED_MERSENNE","EDWARDS","NOT","NOT","NOT","256")
		curve_selected=True
	if x==8:
		curveset("NIST384","48","29","384","7","NOT_SPECIAL","WEIERSTRASS","NOT","NOT","NOT","192")
		curve_selected=True
	if x==9:
		curveset("C41417","52","29","414","7","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","256")
		curve_selected=True
	if x==10:
		curveset("NIST521","66","28","521","7","PSEUDO_MERSENNE","WEIERSTRASS","NOT","NOT","NOT","256")
		curve_selected=True

	if x==11:
		curveset("NUMS256W","32","28","256","3","PSEUDO_MERSENNE","WEIERSTRASS","NOT","NOT","NOT","128")
		curve_selected=True
	if x==12:
		curveset("NUMS256E","32","29","256","3","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","128")
		curve_selected=True
	if x==13:
		curveset("NUMS384W","48","29","384","3","PSEUDO_MERSENNE","WEIERSTRASS","NOT","NOT","NOT","192")
		curve_selected=True
	if x==14:
		curveset("NUMS384E","48","29","384","3","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","192")
		curve_selected=True
	if x==15:
		curveset("NUMS512W","64","29","512","7","PSEUDO_MERSENNE","WEIERSTRASS","NOT","NOT","NOT","256")
		curve_selected=True
	if x==16:
		curveset("NUMS512E","64","29","512","7","PSEUDO_MERSENNE","EDWARDS","NOT","NOT","NOT","256")
		curve_selected=True

	if x==17:
		curveset("SECP256K1","32","28","256","7","NOT_SPECIAL","WEIERSTRASS","NOT","NOT","NOT","128")
		curve_selected=True

	if x==18:
		curveset("BN254","32","28","254","3","NOT_SPECIAL","WEIERSTRASS","BN","D_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==19:
		curveset("BN254CX","32","28","254","3","NOT_SPECIAL","WEIERSTRASS","BN","D_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==20:
		curveset("BLS383","48","29","383","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","128")
		pfcurve_selected=True

	if x==21:
		curveset("BLS381","48","29","381","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True

	if x==22:
		curveset("FP256BN","32","28","256","3","NOT_SPECIAL","WEIERSTRASS","BN","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True
	if x==23:
		curveset("FP512BN","64","29","512","3","NOT_SPECIAL","WEIERSTRASS","BN","M_TYPE","POSITIVEX","128")
		pfcurve_selected=True
# https://eprint.iacr.org/2017/334.pdf
	if x==24:
		curveset("BLS461","58","28","461","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","NEGATIVEX","128")
		pfcurve_selected=True

	if x==25:
		curveset("BLS24","60","29","479","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","192")
		pfcurve_selected=True

	if x==26:
		curveset("BLS48","70","29","556","3","NOT_SPECIAL","WEIERSTRASS","BLS","M_TYPE","POSITIVEX","256")
		pfcurve_selected=True


# rsaset(rsaname,big_length_bytes,bits_in_base,multiplier)
# The RSA name reflects the modulus size, which is a 2^m multiplier
# of the underlying big length

# There are choices here, different ways of getting the same result, but some faster than others
	if x==27:
		#256 is slower but may allow reuse of 256-bit BIGs used for elliptic curve
		#512 is faster.. but best is 1024
		rsaset("RSA2048","128","28","2")
		#rsaset("RSA2048","64","29","4")
		#rsaset("RSA2048","32","29","8")
		rsa_selected=True
	if x==28:
		rsaset("RSA3072","48","28","8")
		rsa_selected=True
	if x==29:
		#rsaset("RSA4096","32","29","16")
		rsaset("RSA4096","64","29","8")
		rsa_selected=True

