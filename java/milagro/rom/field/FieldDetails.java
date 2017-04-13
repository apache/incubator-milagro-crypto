package milagro.rom.field;

public abstract class FieldDetails {
	public static final int NOT_SPECIAL=0;
	public static final int PSEUDO_MERSENNE=1;
	public static final int MONTGOMERY_FRIENDLY=2;
	public static final int WEIERSTRASS=0;
	public static final int EDWARDS=1;
	public static final int MONTGOMERY=2;
	
	public abstract int getModType();
	public abstract int[] getModulus();
	public abstract int getMConst();
	
	public boolean isNotSpecial() {
		return getModType() == NOT_SPECIAL;
	}
	
	public boolean isPseudoMersenne() {
		return getModType() == PSEUDO_MERSENNE;
	}

	public boolean isMontgomeryFriendly() {
		return getModType() == MONTGOMERY_FRIENDLY;
	}

	public boolean isWeierstrass() {
		return getModType() == WEIERSTRASS;
	}

	public boolean isEdwards() {
		return getModType() == EDWARDS;
	}

	public boolean isMontgomery() {
		return getModType() == MONTGOMERY;
	}

}
