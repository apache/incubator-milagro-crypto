copy amcl_.h amcl.h

cl /c /O2 big.c
cl /c /O2 fp.c
cl /c /O2 ecp.c
cl /c /O2 hash.c
cl /c /O2 rand.c
cl /c /O2 aes.c
cl /c /O2 gcm.c
cl /c /O2 oct.c
cl /c /O2 rom.c
cl /c /O2 fp.c
cl /c /O2 fp2.c
cl /c /O2 ecp2.c
cl /c /O2 fp4.c
cl /c /O2 fp12.c
cl /c /O2 pair.c

del amcl.lib
lib /OUT:amcl.lib big.obj fp.obj ecp.obj hash.obj
lib /OUT:amcl.lib amcl.lib rand.obj aes.obj gcm.obj oct.obj rom.obj

lib /OUT:amcl.lib amcl.lib pair.obj fp2.obj ecp2.obj fp4.obj fp12.obj

cl /O2 testmpin.c mpin.c amcl.lib

del amcl.h
del *.obj
