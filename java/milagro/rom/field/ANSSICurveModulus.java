package milagro.rom.field;

public class ANSSICurveModulus extends FieldDetails {

	// ANSSI Modulus
	public static final int MODTYPE = 0;
	public static final int[] Modulus = { 0x186E9C03, 0x7E79A9E, 0x12329B7A, 0x35B7957, 0x435B396, 0x16F46721, 0x163C4049, 0x1181675A, 0xF1FD17 };
	public static final int MConst = 0x164E1155;

	@Override
	public int getModType() {
		return MODTYPE;
	}

	@Override
	public int[] getModulus() {
		return Modulus;
	}

	@Override
	public int getMConst() {
		return MConst;
	}

}
