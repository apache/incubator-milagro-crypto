package rom.field;

public class BrainpoolModulus extends FieldDetails {

	// Brainpool Modulus
	public static final int MODTYPE = 0;
	public static final int[] Modulus = { 0x1F6E5377, 0x9A40E8, 0x9880A08, 0x17EC47AA, 0x18D726E3, 0x5484EC1, 0x6F0F998, 0x1B743DD5, 0xA9FB57 };
	public static final int MConst = 0xEFD89B9;
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
