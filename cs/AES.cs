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

/* AES Encryption */


public class AES
{
	internal int mode;
	private int[] fkey = new int[44];
	private int[] rkey = new int[44];
	public sbyte[] f = new sbyte[16];


	public const int ECB = 0;
	public const int CBC = 1;
	public const int CFB1 = 2;
	public const int CFB2 = 3;
	public const int CFB4 = 5;
	public const int OFB1 = 14;
	public const int OFB2 = 15;
	public const int OFB4 = 17;
	public const int OFB8 = 21;
	public const int OFB16 = 29;

	private static readonly sbyte[] InCo = new sbyte[] {(sbyte)0xB,(sbyte)0xD,(sbyte)0x9,(sbyte)0xE}; // Inverse Coefficients

	public const int KS = 16; // Key Size in bytes
	public const int BS = 16; // Block Size

	private static readonly sbyte[] ptab = new sbyte[] {(sbyte)1,(sbyte)3,(sbyte)5,(sbyte)15,(sbyte)17,(sbyte)51,(sbyte)85,unchecked((sbyte)255),(sbyte)26,(sbyte)46,(sbyte)114,unchecked((sbyte)150),unchecked((sbyte)161),unchecked((sbyte)248),(sbyte)19,(sbyte)53, (sbyte)95,unchecked((sbyte)225),(sbyte)56,(sbyte)72,unchecked((sbyte)216),(sbyte)115,unchecked((sbyte)149),unchecked((sbyte)164),unchecked((sbyte)247),(sbyte)2,(sbyte)6,(sbyte)10,(sbyte)30,(sbyte)34,(sbyte)102,unchecked((sbyte)170), unchecked((sbyte)229),(sbyte)52,(sbyte)92,unchecked((sbyte)228),(sbyte)55,(sbyte)89,unchecked((sbyte)235),(sbyte)38,(sbyte)106,unchecked((sbyte)190),unchecked((sbyte)217),(sbyte)112,unchecked((sbyte)144),unchecked((sbyte)171),unchecked((sbyte)230),(sbyte)49, (sbyte)83,unchecked((sbyte)245),(sbyte)4,(sbyte)12,(sbyte)20,(sbyte)60,(sbyte)68,unchecked((sbyte)204),(sbyte)79,unchecked((sbyte)209),(sbyte)104,unchecked((sbyte)184),unchecked((sbyte)211),(sbyte)110,unchecked((sbyte)178),unchecked((sbyte)205), (sbyte)76,unchecked((sbyte)212),(sbyte)103,unchecked((sbyte)169),unchecked((sbyte)224),(sbyte)59,(sbyte)77,unchecked((sbyte)215),(sbyte)98,unchecked((sbyte)166),unchecked((sbyte)241),(sbyte)8,(sbyte)24,(sbyte)40,(sbyte)120,unchecked((sbyte)136), unchecked((sbyte)131),unchecked((sbyte)158),unchecked((sbyte)185),unchecked((sbyte)208),(sbyte)107,unchecked((sbyte)189),unchecked((sbyte)220),(sbyte)127,unchecked((sbyte)129),unchecked((sbyte)152),unchecked((sbyte)179),unchecked((sbyte)206),(sbyte)73,unchecked((sbyte)219),(sbyte)118,unchecked((sbyte)154), unchecked((sbyte)181),unchecked((sbyte)196),(sbyte)87,unchecked((sbyte)249),(sbyte)16,(sbyte)48,(sbyte)80,unchecked((sbyte)240),(sbyte)11,(sbyte)29,(sbyte)39,(sbyte)105,unchecked((sbyte)187),unchecked((sbyte)214),(sbyte)97,unchecked((sbyte)163), unchecked((sbyte)254),(sbyte)25,(sbyte)43,(sbyte)125,unchecked((sbyte)135),unchecked((sbyte)146),unchecked((sbyte)173),unchecked((sbyte)236),(sbyte)47,(sbyte)113,unchecked((sbyte)147),unchecked((sbyte)174),unchecked((sbyte)233),(sbyte)32,(sbyte)96,unchecked((sbyte)160), unchecked((sbyte)251),(sbyte)22,(sbyte)58,(sbyte)78,unchecked((sbyte)210),(sbyte)109,unchecked((sbyte)183),unchecked((sbyte)194),(sbyte)93,unchecked((sbyte)231),(sbyte)50,(sbyte)86,unchecked((sbyte)250),(sbyte)21,(sbyte)63,(sbyte)65, unchecked((sbyte)195),(sbyte)94,unchecked((sbyte)226),(sbyte)61,(sbyte)71,unchecked((sbyte)201),(sbyte)64,unchecked((sbyte)192),(sbyte)91,unchecked((sbyte)237),(sbyte)44,(sbyte)116,unchecked((sbyte)156),unchecked((sbyte)191),unchecked((sbyte)218),(sbyte)117, unchecked((sbyte)159),unchecked((sbyte)186),unchecked((sbyte)213),(sbyte)100,unchecked((sbyte)172),unchecked((sbyte)239),(sbyte)42,(sbyte)126,unchecked((sbyte)130),unchecked((sbyte)157),unchecked((sbyte)188),unchecked((sbyte)223),(sbyte)122,unchecked((sbyte)142),unchecked((sbyte)137),unchecked((sbyte)128), unchecked((sbyte)155),unchecked((sbyte)182),unchecked((sbyte)193),(sbyte)88,unchecked((sbyte)232),(sbyte)35,(sbyte)101,unchecked((sbyte)175),unchecked((sbyte)234),(sbyte)37,(sbyte)111,unchecked((sbyte)177),unchecked((sbyte)200),(sbyte)67,unchecked((sbyte)197),(sbyte)84, unchecked((sbyte)252),(sbyte)31,(sbyte)33,(sbyte)99,unchecked((sbyte)165),unchecked((sbyte)244),(sbyte)7,(sbyte)9,(sbyte)27,(sbyte)45,(sbyte)119,unchecked((sbyte)153),unchecked((sbyte)176),unchecked((sbyte)203),(sbyte)70,unchecked((sbyte)202), (sbyte)69,unchecked((sbyte)207),(sbyte)74,unchecked((sbyte)222),(sbyte)121,unchecked((sbyte)139),unchecked((sbyte)134),unchecked((sbyte)145),unchecked((sbyte)168),unchecked((sbyte)227),(sbyte)62,(sbyte)66,unchecked((sbyte)198),(sbyte)81,unchecked((sbyte)243),(sbyte)14, (sbyte)18,(sbyte)54,(sbyte)90,unchecked((sbyte)238),(sbyte)41,(sbyte)123,unchecked((sbyte)141),unchecked((sbyte)140),unchecked((sbyte)143),unchecked((sbyte)138),unchecked((sbyte)133),unchecked((sbyte)148),unchecked((sbyte)167),unchecked((sbyte)242),(sbyte)13,(sbyte)23, (sbyte)57,(sbyte)75,unchecked((sbyte)221),(sbyte)124,unchecked((sbyte)132),unchecked((sbyte)151),unchecked((sbyte)162),unchecked((sbyte)253),(sbyte)28,(sbyte)36,(sbyte)108,unchecked((sbyte)180),unchecked((sbyte)199),(sbyte)82,unchecked((sbyte)246),(sbyte)1};

	private static readonly sbyte[] ltab = new sbyte[] {(sbyte)0,unchecked((sbyte)255),(sbyte)25,(sbyte)1,(sbyte)50,(sbyte)2,(sbyte)26,unchecked((sbyte)198),(sbyte)75,unchecked((sbyte)199),(sbyte)27,(sbyte)104,(sbyte)51,unchecked((sbyte)238),unchecked((sbyte)223),(sbyte)3, (sbyte)100,(sbyte)4,unchecked((sbyte)224),(sbyte)14,(sbyte)52,unchecked((sbyte)141),unchecked((sbyte)129),unchecked((sbyte)239),(sbyte)76,(sbyte)113,(sbyte)8,unchecked((sbyte)200),unchecked((sbyte)248),(sbyte)105,(sbyte)28,unchecked((sbyte)193), (sbyte)125,unchecked((sbyte)194),(sbyte)29,unchecked((sbyte)181),unchecked((sbyte)249),unchecked((sbyte)185),(sbyte)39,(sbyte)106,(sbyte)77,unchecked((sbyte)228),unchecked((sbyte)166),(sbyte)114,unchecked((sbyte)154),unchecked((sbyte)201),(sbyte)9,(sbyte)120, (sbyte)101,(sbyte)47,unchecked((sbyte)138),(sbyte)5,(sbyte)33,(sbyte)15,unchecked((sbyte)225),(sbyte)36,(sbyte)18,unchecked((sbyte)240),unchecked((sbyte)130),(sbyte)69,(sbyte)53,unchecked((sbyte)147),unchecked((sbyte)218),unchecked((sbyte)142), unchecked((sbyte)150),unchecked((sbyte)143),unchecked((sbyte)219),unchecked((sbyte)189),(sbyte)54,unchecked((sbyte)208),unchecked((sbyte)206),unchecked((sbyte)148),(sbyte)19,(sbyte)92,unchecked((sbyte)210),unchecked((sbyte)241),(sbyte)64,(sbyte)70,unchecked((sbyte)131),(sbyte)56, (sbyte)102,unchecked((sbyte)221),unchecked((sbyte)253),(sbyte)48,unchecked((sbyte)191),(sbyte)6,unchecked((sbyte)139),(sbyte)98,unchecked((sbyte)179),(sbyte)37,unchecked((sbyte)226),unchecked((sbyte)152),(sbyte)34,unchecked((sbyte)136),unchecked((sbyte)145),(sbyte)16, (sbyte)126,(sbyte)110,(sbyte)72,unchecked((sbyte)195),unchecked((sbyte)163),unchecked((sbyte)182),(sbyte)30,(sbyte)66,(sbyte)58,(sbyte)107,(sbyte)40,(sbyte)84,unchecked((sbyte)250),unchecked((sbyte)133),(sbyte)61,unchecked((sbyte)186), (sbyte)43,(sbyte)121,(sbyte)10,(sbyte)21,unchecked((sbyte)155),unchecked((sbyte)159),(sbyte)94,unchecked((sbyte)202),(sbyte)78,unchecked((sbyte)212),unchecked((sbyte)172),unchecked((sbyte)229),unchecked((sbyte)243),(sbyte)115,unchecked((sbyte)167),(sbyte)87, unchecked((sbyte)175),(sbyte)88,unchecked((sbyte)168),(sbyte)80,unchecked((sbyte)244),unchecked((sbyte)234),unchecked((sbyte)214),(sbyte)116,(sbyte)79,unchecked((sbyte)174),unchecked((sbyte)233),unchecked((sbyte)213),unchecked((sbyte)231),unchecked((sbyte)230),unchecked((sbyte)173),unchecked((sbyte)232), (sbyte)44,unchecked((sbyte)215),(sbyte)117,(sbyte)122,unchecked((sbyte)235),(sbyte)22,(sbyte)11,unchecked((sbyte)245),(sbyte)89,unchecked((sbyte)203),(sbyte)95,unchecked((sbyte)176),unchecked((sbyte)156),unchecked((sbyte)169),(sbyte)81,unchecked((sbyte)160), (sbyte)127,(sbyte)12,unchecked((sbyte)246),(sbyte)111,(sbyte)23,unchecked((sbyte)196),(sbyte)73,unchecked((sbyte)236),unchecked((sbyte)216),(sbyte)67,(sbyte)31,(sbyte)45,unchecked((sbyte)164),(sbyte)118,(sbyte)123,unchecked((sbyte)183), unchecked((sbyte)204),unchecked((sbyte)187),(sbyte)62,(sbyte)90,unchecked((sbyte)251),(sbyte)96,unchecked((sbyte)177),unchecked((sbyte)134),(sbyte)59,(sbyte)82,unchecked((sbyte)161),(sbyte)108,unchecked((sbyte)170),(sbyte)85,(sbyte)41,unchecked((sbyte)157), unchecked((sbyte)151),unchecked((sbyte)178),unchecked((sbyte)135),unchecked((sbyte)144),(sbyte)97,unchecked((sbyte)190),unchecked((sbyte)220),unchecked((sbyte)252),unchecked((sbyte)188),unchecked((sbyte)149),unchecked((sbyte)207),unchecked((sbyte)205),(sbyte)55,(sbyte)63,(sbyte)91,unchecked((sbyte)209), (sbyte)83,(sbyte)57,unchecked((sbyte)132),(sbyte)60,(sbyte)65,unchecked((sbyte)162),(sbyte)109,(sbyte)71,(sbyte)20,(sbyte)42,unchecked((sbyte)158),(sbyte)93,(sbyte)86,unchecked((sbyte)242),unchecked((sbyte)211),unchecked((sbyte)171), (sbyte)68,(sbyte)17,unchecked((sbyte)146),unchecked((sbyte)217),(sbyte)35,(sbyte)32,(sbyte)46,unchecked((sbyte)137),unchecked((sbyte)180),(sbyte)124,unchecked((sbyte)184),(sbyte)38,(sbyte)119,unchecked((sbyte)153),unchecked((sbyte)227),unchecked((sbyte)165), (sbyte)103,(sbyte)74,unchecked((sbyte)237),unchecked((sbyte)222),unchecked((sbyte)197),(sbyte)49,unchecked((sbyte)254),(sbyte)24,(sbyte)13,(sbyte)99,unchecked((sbyte)140),unchecked((sbyte)128),unchecked((sbyte)192),unchecked((sbyte)247),(sbyte)112,(sbyte)7};

	private static readonly sbyte[] fbsub = new sbyte[] {(sbyte)99,(sbyte)124,(sbyte)119,(sbyte)123,unchecked((sbyte)242),(sbyte)107,(sbyte)111,unchecked((sbyte)197),(sbyte)48,(sbyte)1,(sbyte)103,(sbyte)43,unchecked((sbyte)254),unchecked((sbyte)215),unchecked((sbyte)171),(sbyte)118, unchecked((sbyte)202),unchecked((sbyte)130),unchecked((sbyte)201),(sbyte)125,unchecked((sbyte)250),(sbyte)89,(sbyte)71,unchecked((sbyte)240),unchecked((sbyte)173),unchecked((sbyte)212),unchecked((sbyte)162),unchecked((sbyte)175),unchecked((sbyte)156),unchecked((sbyte)164),(sbyte)114,unchecked((sbyte)192), unchecked((sbyte)183),unchecked((sbyte)253),unchecked((sbyte)147),(sbyte)38,(sbyte)54,(sbyte)63,unchecked((sbyte)247),unchecked((sbyte)204),(sbyte)52,unchecked((sbyte)165),unchecked((sbyte)229),unchecked((sbyte)241),(sbyte)113,unchecked((sbyte)216),(sbyte)49,(sbyte)21, (sbyte)4,unchecked((sbyte)199),(sbyte)35,unchecked((sbyte)195),(sbyte)24,unchecked((sbyte)150),(sbyte)5,unchecked((sbyte)154),(sbyte)7,(sbyte)18,unchecked((sbyte)128),unchecked((sbyte)226),unchecked((sbyte)235),(sbyte)39,unchecked((sbyte)178),(sbyte)117, (sbyte)9,unchecked((sbyte)131),(sbyte)44,(sbyte)26,(sbyte)27,(sbyte)110,(sbyte)90,unchecked((sbyte)160),(sbyte)82,(sbyte)59,unchecked((sbyte)214),unchecked((sbyte)179),(sbyte)41,unchecked((sbyte)227),(sbyte)47,unchecked((sbyte)132), (sbyte)83,unchecked((sbyte)209),(sbyte)0,unchecked((sbyte)237),(sbyte)32,unchecked((sbyte)252),unchecked((sbyte)177),(sbyte)91,(sbyte)106,unchecked((sbyte)203),unchecked((sbyte)190),(sbyte)57,(sbyte)74,(sbyte)76,(sbyte)88,unchecked((sbyte)207), unchecked((sbyte)208),unchecked((sbyte)239),unchecked((sbyte)170),unchecked((sbyte)251),(sbyte)67,(sbyte)77,(sbyte)51,unchecked((sbyte)133),(sbyte)69,unchecked((sbyte)249),(sbyte)2,(sbyte)127,(sbyte)80,(sbyte)60,unchecked((sbyte)159),unchecked((sbyte)168), (sbyte)81,unchecked((sbyte)163),(sbyte)64,unchecked((sbyte)143),unchecked((sbyte)146),unchecked((sbyte)157),(sbyte)56,unchecked((sbyte)245),unchecked((sbyte)188),unchecked((sbyte)182),unchecked((sbyte)218),(sbyte)33,(sbyte)16,unchecked((sbyte)255),unchecked((sbyte)243),unchecked((sbyte)210), unchecked((sbyte)205),(sbyte)12,(sbyte)19,unchecked((sbyte)236),(sbyte)95,unchecked((sbyte)151),(sbyte)68,(sbyte)23,unchecked((sbyte)196),unchecked((sbyte)167),(sbyte)126,(sbyte)61,(sbyte)100,(sbyte)93,(sbyte)25,(sbyte)115, (sbyte)96,unchecked((sbyte)129),(sbyte)79,unchecked((sbyte)220),(sbyte)34,(sbyte)42,unchecked((sbyte)144),unchecked((sbyte)136),(sbyte)70,unchecked((sbyte)238),unchecked((sbyte)184),(sbyte)20,unchecked((sbyte)222),(sbyte)94,(sbyte)11,unchecked((sbyte)219), unchecked((sbyte)224),(sbyte)50,(sbyte)58,(sbyte)10,(sbyte)73,(sbyte)6,(sbyte)36,(sbyte)92,unchecked((sbyte)194),unchecked((sbyte)211),unchecked((sbyte)172),(sbyte)98,unchecked((sbyte)145),unchecked((sbyte)149),unchecked((sbyte)228),(sbyte)121, unchecked((sbyte)231),unchecked((sbyte)200),(sbyte)55,(sbyte)109,unchecked((sbyte)141),unchecked((sbyte)213),(sbyte)78,unchecked((sbyte)169),(sbyte)108,(sbyte)86,unchecked((sbyte)244),unchecked((sbyte)234),(sbyte)101,(sbyte)122,unchecked((sbyte)174),(sbyte)8, unchecked((sbyte)186),(sbyte)120,(sbyte)37,(sbyte)46,(sbyte)28,unchecked((sbyte)166),unchecked((sbyte)180),unchecked((sbyte)198),unchecked((sbyte)232),unchecked((sbyte)221),(sbyte)116,(sbyte)31,(sbyte)75,unchecked((sbyte)189),unchecked((sbyte)139),unchecked((sbyte)138), (sbyte)112,(sbyte)62,unchecked((sbyte)181),(sbyte)102,(sbyte)72,(sbyte)3,unchecked((sbyte)246),(sbyte)14,(sbyte)97,(sbyte)53,(sbyte)87,unchecked((sbyte)185),unchecked((sbyte)134),unchecked((sbyte)193),(sbyte)29,unchecked((sbyte)158), unchecked((sbyte)225),unchecked((sbyte)248),unchecked((sbyte)152),(sbyte)17,(sbyte)105,unchecked((sbyte)217),unchecked((sbyte)142),unchecked((sbyte)148),unchecked((sbyte)155),(sbyte)30,unchecked((sbyte)135),unchecked((sbyte)233),unchecked((sbyte)206),(sbyte)85,(sbyte)40,unchecked((sbyte)223), unchecked((sbyte)140),unchecked((sbyte)161),unchecked((sbyte)137),(sbyte)13,unchecked((sbyte)191),unchecked((sbyte)230),(sbyte)66,(sbyte)104,(sbyte)65,unchecked((sbyte)153),(sbyte)45,(sbyte)15,unchecked((sbyte)176),(sbyte)84,unchecked((sbyte)187),(sbyte)22};

	private static readonly sbyte[] rbsub = new sbyte[] {(sbyte)82,(sbyte)9,(sbyte)106,unchecked((sbyte)213),(sbyte)48,(sbyte)54,unchecked((sbyte)165),(sbyte)56,unchecked((sbyte)191),(sbyte)64,unchecked((sbyte)163),unchecked((sbyte)158),unchecked((sbyte)129),unchecked((sbyte)243),unchecked((sbyte)215),unchecked((sbyte)251), (sbyte)124,unchecked((sbyte)227),(sbyte)57,unchecked((sbyte)130),unchecked((sbyte)155),(sbyte)47,unchecked((sbyte)255),unchecked((sbyte)135),(sbyte)52,unchecked((sbyte)142),(sbyte)67,(sbyte)68,unchecked((sbyte)196),unchecked((sbyte)222),unchecked((sbyte)233),unchecked((sbyte)203), (sbyte)84,(sbyte)123,unchecked((sbyte)148),(sbyte)50,unchecked((sbyte)166),unchecked((sbyte)194),(sbyte)35,(sbyte)61,unchecked((sbyte)238),(sbyte)76,unchecked((sbyte)149),(sbyte)11,(sbyte)66,unchecked((sbyte)250),unchecked((sbyte)195),(sbyte)78, (sbyte)8,(sbyte)46,unchecked((sbyte)161),(sbyte)102,(sbyte)40,unchecked((sbyte)217),(sbyte)36,unchecked((sbyte)178),(sbyte)118,(sbyte)91,unchecked((sbyte)162),(sbyte)73,(sbyte)109,unchecked((sbyte)139),unchecked((sbyte)209),(sbyte)37, (sbyte)114,unchecked((sbyte)248),unchecked((sbyte)246),(sbyte)100,unchecked((sbyte)134),(sbyte)104,unchecked((sbyte)152),(sbyte)22,unchecked((sbyte)212),unchecked((sbyte)164),(sbyte)92,unchecked((sbyte)204),(sbyte)93,(sbyte)101,unchecked((sbyte)182),unchecked((sbyte)146), (sbyte)108,(sbyte)112,(sbyte)72,(sbyte)80,unchecked((sbyte)253),unchecked((sbyte)237),unchecked((sbyte)185),unchecked((sbyte)218),(sbyte)94,(sbyte)21,(sbyte)70,(sbyte)87,unchecked((sbyte)167),unchecked((sbyte)141),unchecked((sbyte)157),unchecked((sbyte)132), unchecked((sbyte)144),unchecked((sbyte)216),unchecked((sbyte)171),(sbyte)0,unchecked((sbyte)140),unchecked((sbyte)188),unchecked((sbyte)211),(sbyte)10,unchecked((sbyte)247),unchecked((sbyte)228),(sbyte)88,(sbyte)5,unchecked((sbyte)184),unchecked((sbyte)179),(sbyte)69,(sbyte)6, unchecked((sbyte)208),(sbyte)44,(sbyte)30,unchecked((sbyte)143),unchecked((sbyte)202),(sbyte)63,(sbyte)15,(sbyte)2,unchecked((sbyte)193),unchecked((sbyte)175),unchecked((sbyte)189),(sbyte)3,(sbyte)1,(sbyte)19,unchecked((sbyte)138),(sbyte)107, (sbyte)58,unchecked((sbyte)145),(sbyte)17,(sbyte)65,(sbyte)79,(sbyte)103,unchecked((sbyte)220),unchecked((sbyte)234),unchecked((sbyte)151),unchecked((sbyte)242),unchecked((sbyte)207),unchecked((sbyte)206),unchecked((sbyte)240),unchecked((sbyte)180),unchecked((sbyte)230),(sbyte)115, unchecked((sbyte)150),unchecked((sbyte)172),(sbyte)116,(sbyte)34,unchecked((sbyte)231),unchecked((sbyte)173),(sbyte)53,unchecked((sbyte)133),unchecked((sbyte)226),unchecked((sbyte)249),(sbyte)55,unchecked((sbyte)232),(sbyte)28,(sbyte)117,unchecked((sbyte)223),(sbyte)110, (sbyte)71,unchecked((sbyte)241),(sbyte)26,(sbyte)113,(sbyte)29,(sbyte)41,unchecked((sbyte)197),unchecked((sbyte)137),(sbyte)111,unchecked((sbyte)183),(sbyte)98,(sbyte)14,unchecked((sbyte)170),(sbyte)24,unchecked((sbyte)190),(sbyte)27, unchecked((sbyte)252),(sbyte)86,(sbyte)62,(sbyte)75,unchecked((sbyte)198),unchecked((sbyte)210),(sbyte)121,(sbyte)32,unchecked((sbyte)154),unchecked((sbyte)219),unchecked((sbyte)192),unchecked((sbyte)254),(sbyte)120,unchecked((sbyte)205),(sbyte)90,unchecked((sbyte)244), (sbyte)31,unchecked((sbyte)221),unchecked((sbyte)168),(sbyte)51,unchecked((sbyte)136),(sbyte)7,unchecked((sbyte)199),(sbyte)49,unchecked((sbyte)177),(sbyte)18,(sbyte)16,(sbyte)89,(sbyte)39,unchecked((sbyte)128),unchecked((sbyte)236),(sbyte)95, (sbyte)96,(sbyte)81,(sbyte)127,unchecked((sbyte)169),(sbyte)25,unchecked((sbyte)181),(sbyte)74,(sbyte)13,(sbyte)45,unchecked((sbyte)229),(sbyte)122,unchecked((sbyte)159),unchecked((sbyte)147),unchecked((sbyte)201),unchecked((sbyte)156),unchecked((sbyte)239), unchecked((sbyte)160),unchecked((sbyte)224),(sbyte)59,(sbyte)77,unchecked((sbyte)174),(sbyte)42,unchecked((sbyte)245),unchecked((sbyte)176),unchecked((sbyte)200),unchecked((sbyte)235),unchecked((sbyte)187),(sbyte)60,unchecked((sbyte)131),(sbyte)83,unchecked((sbyte)153),(sbyte)97, (sbyte)23,(sbyte)43,(sbyte)4,(sbyte)126,unchecked((sbyte)186),(sbyte)119,unchecked((sbyte)214),(sbyte)38,unchecked((sbyte)225),(sbyte)105,(sbyte)20,(sbyte)99,(sbyte)85,(sbyte)33,(sbyte)12,(sbyte)125};

	private static readonly sbyte[] rco = new sbyte[] {(sbyte)1,(sbyte)2,(sbyte)4,(sbyte)8,(sbyte)16,(sbyte)32,(sbyte)64,unchecked((sbyte)128),(sbyte)27,(sbyte)54,(sbyte)108,unchecked((sbyte)216),unchecked((sbyte)171),(sbyte)77,unchecked((sbyte)154),(sbyte)47};

	private static readonly int[] ftable = new int[] {unchecked((int)0xa56363c6), unchecked((int)0x847c7cf8), unchecked((int)0x997777ee), unchecked((int)0x8d7b7bf6), 0xdf2f2ff, unchecked((int)0xbd6b6bd6), unchecked((int)0xb16f6fde), 0x54c5c591, 0x50303060, 0x3010102, unchecked((int)0xa96767ce), 0x7d2b2b56, 0x19fefee7, 0x62d7d7b5, unchecked((int)0xe6abab4d), unchecked((int)0x9a7676ec), 0x45caca8f, unchecked((int)0x9d82821f), 0x40c9c989, unchecked((int)0x877d7dfa), 0x15fafaef, unchecked((int)0xeb5959b2), unchecked((int)0xc947478e), 0xbf0f0fb, unchecked((int)0xecadad41), 0x67d4d4b3, unchecked((int)0xfda2a25f), unchecked((int)0xeaafaf45), unchecked((int)0xbf9c9c23), unchecked((int)0xf7a4a453), unchecked((int)0x967272e4), 0x5bc0c09b, unchecked((int)0xc2b7b775), 0x1cfdfde1, unchecked((int)0xae93933d), 0x6a26264c, 0x5a36366c, 0x413f3f7e, 0x2f7f7f5, 0x4fcccc83, 0x5c343468, unchecked((int)0xf4a5a551), 0x34e5e5d1, 0x8f1f1f9, unchecked((int)0x937171e2), 0x73d8d8ab, 0x53313162, 0x3f15152a, 0xc040408, 0x52c7c795, 0x65232346, 0x5ec3c39d, 0x28181830, unchecked((int)0xa1969637), 0xf05050a, unchecked((int)0xb59a9a2f), 0x907070e, 0x36121224, unchecked((int)0x9b80801b), 0x3de2e2df, 0x26ebebcd, 0x6927274e, unchecked((int)0xcdb2b27f), unchecked((int)0x9f7575ea), 0x1b090912, unchecked((int)0x9e83831d), 0x742c2c58, 0x2e1a1a34, 0x2d1b1b36, unchecked((int)0xb26e6edc), unchecked((int)0xee5a5ab4), unchecked((int)0xfba0a05b), unchecked((int)0xf65252a4), 0x4d3b3b76, 0x61d6d6b7, unchecked((int)0xceb3b37d), 0x7b292952, 0x3ee3e3dd, 0x712f2f5e, unchecked((int)0x97848413), unchecked((int)0xf55353a6), 0x68d1d1b9, 0x0, 0x2cededc1, 0x60202040, 0x1ffcfce3, unchecked((int)0xc8b1b179), unchecked((int)0xed5b5bb6), unchecked((int)0xbe6a6ad4), 0x46cbcb8d, unchecked((int)0xd9bebe67), 0x4b393972, unchecked((int)0xde4a4a94), unchecked((int)0xd44c4c98), unchecked((int)0xe85858b0), 0x4acfcf85, 0x6bd0d0bb, 0x2aefefc5, unchecked((int)0xe5aaaa4f), 0x16fbfbed, unchecked((int)0xc5434386), unchecked((int)0xd74d4d9a), 0x55333366, unchecked((int)0x94858511), unchecked((int)0xcf45458a), 0x10f9f9e9, 0x6020204, unchecked((int)0x817f7ffe), unchecked((int)0xf05050a0), 0x443c3c78, unchecked((int)0xba9f9f25), unchecked((int)0xe3a8a84b), unchecked((int)0xf35151a2), unchecked((int)0xfea3a35d), unchecked((int)0xc0404080), unchecked((int)0x8a8f8f05), unchecked((int)0xad92923f), unchecked((int)0xbc9d9d21), 0x48383870, 0x4f5f5f1, unchecked((int)0xdfbcbc63), unchecked((int)0xc1b6b677), 0x75dadaaf, 0x63212142, 0x30101020, 0x1affffe5, 0xef3f3fd, 0x6dd2d2bf, 0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3, unchecked((int)0xe15f5fbe), unchecked((int)0xa2979735), unchecked((int)0xcc444488), 0x3917172e, 0x57c4c493, unchecked((int)0xf2a7a755), unchecked((int)0x827e7efc), 0x473d3d7a, unchecked((int)0xac6464c8), unchecked((int)0xe75d5dba), 0x2b191932, unchecked((int)0x957373e6), unchecked((int)0xa06060c0), unchecked((int)0x98818119), unchecked((int)0xd14f4f9e), 0x7fdcdca3, 0x66222244, 0x7e2a2a54, unchecked((int)0xab90903b), unchecked((int)0x8388880b), unchecked((int)0xca46468c), 0x29eeeec7, unchecked((int)0xd3b8b86b), 0x3c141428, 0x79dedea7, unchecked((int)0xe25e5ebc), 0x1d0b0b16, 0x76dbdbad, 0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14, unchecked((int)0xdb494992), 0xa06060c, 0x6c242448, unchecked((int)0xe45c5cb8), 0x5dc2c29f, 0x6ed3d3bd, unchecked((int)0xefacac43), unchecked((int)0xa66262c4), unchecked((int)0xa8919139), unchecked((int)0xa4959531), 0x37e4e4d3, unchecked((int)0x8b7979f2), 0x32e7e7d5, 0x43c8c88b, 0x5937376e, unchecked((int)0xb76d6dda), unchecked((int)0x8c8d8d01), 0x64d5d5b1, unchecked((int)0xd24e4e9c), unchecked((int)0xe0a9a949), unchecked((int)0xb46c6cd8), unchecked((int)0xfa5656ac), 0x7f4f4f3, 0x25eaeacf, unchecked((int)0xaf6565ca), unchecked((int)0x8e7a7af4), unchecked((int)0xe9aeae47), 0x18080810, unchecked((int)0xd5baba6f), unchecked((int)0x887878f0), 0x6f25254a, 0x722e2e5c, 0x241c1c38, unchecked((int)0xf1a6a657), unchecked((int)0xc7b4b473), 0x51c6c697, 0x23e8e8cb, 0x7cdddda1, unchecked((int)0x9c7474e8), 0x211f1f3e, unchecked((int)0xdd4b4b96), unchecked((int)0xdcbdbd61), unchecked((int)0x868b8b0d), unchecked((int)0x858a8a0f), unchecked((int)0x907070e0), 0x423e3e7c, unchecked((int)0xc4b5b571), unchecked((int)0xaa6666cc), unchecked((int)0xd8484890), 0x5030306, 0x1f6f6f7, 0x120e0e1c, unchecked((int)0xa36161c2), 0x5f35356a, unchecked((int)0xf95757ae), unchecked((int)0xd0b9b969), unchecked((int)0x91868617), 0x58c1c199, 0x271d1d3a, unchecked((int)0xb99e9e27), 0x38e1e1d9, 0x13f8f8eb, unchecked((int)0xb398982b), 0x33111122, unchecked((int)0xbb6969d2), 0x70d9d9a9, unchecked((int)0x898e8e07), unchecked((int)0xa7949433), unchecked((int)0xb69b9b2d), 0x221e1e3c, unchecked((int)0x92878715), 0x20e9e9c9, 0x49cece87, unchecked((int)0xff5555aa), 0x78282850, 0x7adfdfa5, unchecked((int)0x8f8c8c03), unchecked((int)0xf8a1a159), unchecked((int)0x80898909), 0x170d0d1a, unchecked((int)0xdabfbf65), 0x31e6e6d7, unchecked((int)0xc6424284), unchecked((int)0xb86868d0), unchecked((int)0xc3414182), unchecked((int)0xb0999929), 0x772d2d5a, 0x110f0f1e, unchecked((int)0xcbb0b07b), unchecked((int)0xfc5454a8), unchecked((int)0xd6bbbb6d), 0x3a16162c};

	private static readonly int[] rtable = new int[] {0x50a7f451, 0x5365417e, unchecked((int)0xc3a4171a), unchecked((int)0x965e273a), unchecked((int)0xcb6bab3b), unchecked((int)0xf1459d1f), unchecked((int)0xab58faac), unchecked((int)0x9303e34b), 0x55fa3020, unchecked((int)0xf66d76ad), unchecked((int)0x9176cc88), 0x254c02f5, unchecked((int)0xfcd7e54f), unchecked((int)0xd7cb2ac5), unchecked((int)0x80443526), unchecked((int)0x8fa362b5), 0x495ab1de, 0x671bba25, unchecked((int)0x980eea45), unchecked((int)0xe1c0fe5d), 0x2752fc3, 0x12f04c81, unchecked((int)0xa397468d), unchecked((int)0xc6f9d36b), unchecked((int)0xe75f8f03), unchecked((int)0x959c9215), unchecked((int)0xeb7a6dbf), unchecked((int)0xda595295), 0x2d83bed4, unchecked((int)0xd3217458), 0x2969e049, 0x44c8c98e, 0x6a89c275, 0x78798ef4, 0x6b3e5899, unchecked((int)0xdd71b927), unchecked((int)0xb64fe1be), 0x17ad88f0, 0x66ac20c9, unchecked((int)0xb43ace7d), 0x184adf63, unchecked((int)0x82311ae5), 0x60335197, 0x457f5362, unchecked((int)0xe07764b1), unchecked((int)0x84ae6bbb), 0x1ca081fe, unchecked((int)0x942b08f9), 0x58684870, 0x19fd458f, unchecked((int)0x876cde94), unchecked((int)0xb7f87b52), 0x23d373ab, unchecked((int)0xe2024b72), 0x578f1fe3, 0x2aab5566, 0x728ebb2, 0x3c2b52f, unchecked((int)0x9a7bc586), unchecked((int)0xa50837d3), unchecked((int)0xf2872830), unchecked((int)0xb2a5bf23), unchecked((int)0xba6a0302), 0x5c8216ed, 0x2b1ccf8a, unchecked((int)0x92b479a7), unchecked((int)0xf0f207f3), unchecked((int)0xa1e2694e), unchecked((int)0xcdf4da65), unchecked((int)0xd5be0506), 0x1f6234d1, unchecked((int)0x8afea6c4), unchecked((int)0x9d532e34), unchecked((int)0xa055f3a2), 0x32e18a05, 0x75ebf6a4, 0x39ec830b, unchecked((int)0xaaef6040), 0x69f715e, 0x51106ebd, unchecked((int)0xf98a213e), 0x3d06dd96, unchecked((int)0xae053edd), 0x46bde64d, unchecked((int)0xb58d5491), 0x55dc471, 0x6fd40604, unchecked((int)0xff155060), 0x24fb9819, unchecked((int)0x97e9bdd6), unchecked((int)0xcc434089), 0x779ed967, unchecked((int)0xbd42e8b0), unchecked((int)0x888b8907), 0x385b19e7, unchecked((int)0xdbeec879), 0x470a7ca1, unchecked((int)0xe90f427c), unchecked((int)0xc91e84f8), 0x0, unchecked((int)0x83868009), 0x48ed2b32, unchecked((int)0xac70111e), 0x4e725a6c, unchecked((int)0xfbff0efd), 0x5638850f, 0x1ed5ae3d, 0x27392d36, 0x64d90f0a, 0x21a65c68, unchecked((int)0xd1545b9b), 0x3a2e3624, unchecked((int)0xb1670a0c), 0xfe75793, unchecked((int)0xd296eeb4), unchecked((int)0x9e919b1b), 0x4fc5c080, unchecked((int)0xa220dc61), 0x694b775a, 0x161a121c, 0xaba93e2, unchecked((int)0xe52aa0c0), 0x43e0223c, 0x1d171b12, 0xb0d090e, unchecked((int)0xadc78bf2), unchecked((int)0xb9a8b62d), unchecked((int)0xc8a91e14), unchecked((int)0x8519f157), 0x4c0775af, unchecked((int)0xbbdd99ee), unchecked((int)0xfd607fa3), unchecked((int)0x9f2601f7), unchecked((int)0xbcf5725c), unchecked((int)0xc53b6644), 0x347efb5b, 0x7629438b, unchecked((int)0xdcc623cb), 0x68fcedb6, 0x63f1e4b8, unchecked((int)0xcadc31d7), 0x10856342, 0x40229713, 0x2011c684, 0x7d244a85, unchecked((int)0xf83dbbd2), 0x1132f9ae, 0x6da129c7, 0x4b2f9e1d, unchecked((int)0xf330b2dc), unchecked((int)0xec52860d), unchecked((int)0xd0e3c177), 0x6c16b32b, unchecked((int)0x99b970a9), unchecked((int)0xfa489411), 0x2264e947, unchecked((int)0xc48cfca8), 0x1a3ff0a0, unchecked((int)0xd82c7d56), unchecked((int)0xef903322), unchecked((int)0xc74e4987), unchecked((int)0xc1d138d9), unchecked((int)0xfea2ca8c), 0x360bd498, unchecked((int)0xcf81f5a6), 0x28de7aa5, 0x268eb7da, unchecked((int)0xa4bfad3f), unchecked((int)0xe49d3a2c), 0xd927850, unchecked((int)0x9bcc5f6a), 0x62467e54, unchecked((int)0xc2138df6), unchecked((int)0xe8b8d890), 0x5ef7392e, unchecked((int)0xf5afc382), unchecked((int)0xbe805d9f), 0x7c93d069, unchecked((int)0xa92dd56f), unchecked((int)0xb31225cf), 0x3b99acc8, unchecked((int)0xa77d1810), 0x6e639ce8, 0x7bbb3bdb, 0x97826cd, unchecked((int)0xf418596e), 0x1b79aec, unchecked((int)0xa89a4f83), 0x656e95e6, 0x7ee6ffaa, 0x8cfbc21, unchecked((int)0xe6e815ef), unchecked((int)0xd99be7ba), unchecked((int)0xce366f4a), unchecked((int)0xd4099fea), unchecked((int)0xd67cb029), unchecked((int)0xafb2a431), 0x31233f2a, 0x3094a5c6, unchecked((int)0xc066a235), 0x37bc4e74, unchecked((int)0xa6ca82fc), unchecked((int)0xb0d090e0), 0x15d8a733, 0x4a9804f1, unchecked((int)0xf7daec41), 0xe50cd7f, 0x2ff69117, unchecked((int)0x8dd64d76), 0x4db0ef43, 0x544daacc, unchecked((int)0xdf0496e4), unchecked((int)0xe3b5d19e), 0x1b886a4c, unchecked((int)0xb81f2cc1), 0x7f516546, 0x4ea5e9d, 0x5d358c01, 0x737487fa, 0x2e410bfb, 0x5a1d67b3, 0x52d2db92, 0x335610e9, 0x1347d66d, unchecked((int)0x8c61d79a), 0x7a0ca137, unchecked((int)0x8e14f859), unchecked((int)0x893c13eb), unchecked((int)0xee27a9ce), 0x35c961b7, unchecked((int)0xede51ce1), 0x3cb1477a, 0x59dfd29c, 0x3f73f255, 0x79ce1418, unchecked((int)0xbf37c773), unchecked((int)0xeacdf753), 0x5baafd5f, 0x146f3ddf, unchecked((int)0x86db4478), unchecked((int)0x81f3afca), 0x3ec468b9, 0x2c342438, 0x5f40a3c2, 0x72c31d16, 0xc25e2bc, unchecked((int)0x8b493c28), 0x41950dff, 0x7101a839, unchecked((int)0xdeb30c08), unchecked((int)0x9ce4b4d8), unchecked((int)0x90c15664), 0x6184cb7b, 0x70b632d5, 0x745c6c48, 0x4257b8d0};


/* Rotates 32-bit word left by 1, 2 or 3 byte  */

	private static int ROTL8(int x)
	{
		return (((x) << 8) | ((int)((uint)(x)>>24)));
	}

	private static int ROTL16(int x)
	{
		return (((x) << 16) | ((int)((uint)(x)>>16)));
	}

	private static int ROTL24(int x)
	{
		return (((x) << 24) | ((int)((uint)(x)>>8)));
	}

	private static int pack(sbyte[] b)
	{ // pack bytes into a 32-bit Word
		return ((((int)b[3]) & 0xff) << 24) | (((int)b[2] & 0xff) << 16) | (((int)b[1] & 0xff) << 8) | ((int)b[0] & 0xff);
	}

	private static sbyte[] unpack(int a)
	{ // unpack bytes from a word
		sbyte[] b = new sbyte[4];
		b[0] = (sbyte)(a);
		b[1] = (sbyte)((int)((uint)a >> 8));
		b[2] = (sbyte)((int)((uint)a >> 16));
		b[3] = (sbyte)((int)((uint)a >> 24));
		return b;
	}

	private static sbyte bmul(sbyte x, sbyte y)
	{ // x.y= AntiLog(Log(x) + Log(y))

		int ix = ((int)x) & 0xff;
		int iy = ((int)y) & 0xff;
		int lx = ((int)ltab[ix]) & 0xff;
		int ly = ((int)ltab[iy]) & 0xff;
		if (x != 0 && y != 0)
		{
			return ptab[(lx + ly) % 255];
		}
		else
		{
			return (sbyte)0;
		}
	}

  //  if (x && y)

	private static int SubByte(int a)
	{
		sbyte[] b = unpack(a);
		b[0] = fbsub[(int)b[0] & 0xff];
		b[1] = fbsub[(int)b[1] & 0xff];
		b[2] = fbsub[(int)b[2] & 0xff];
		b[3] = fbsub[(int)b[3] & 0xff];
		return pack(b);
	}

	private static sbyte product(int x, int y)
	{ // dot product of two 4-byte arrays
		sbyte[] xb; //=new byte[4];
		sbyte[] yb; //=new byte[4];
		xb = unpack(x);
		yb = unpack(y);

		return (sbyte)(bmul(xb[0],yb[0]) ^ bmul(xb[1],yb[1]) ^ bmul(xb[2],yb[2]) ^ bmul(xb[3],yb[3]));
	}

	private static int InvMixCol(int x)
	{ // matrix Multiplication
		int y, m;
		sbyte[] b = new sbyte[4];

		m = pack(InCo);
		b[3] = product(m,x);
		m = ROTL24(m);
		b[2] = product(m,x);
		m = ROTL24(m);
		b[1] = product(m,x);
		m = ROTL24(m);
		b[0] = product(m,x);
		y = pack(b);
		return y;
	}

/* reset cipher */
	public virtual void reset(int m, sbyte[] iv)
	{ // reset mode, or reset iv
		mode = m;
		for (int i = 0;i < 16;i++)
		{
			f[i] = 0;
		}
		if (mode != ECB && iv != null)
		{
			for (int i = 0;i < 16;i++)
			{
				f[i] = iv[i];
			}
		}
	}

	public virtual sbyte[] getreg()
	{
		sbyte[] ir = new sbyte[16];
		for (int i = 0;i < 16;i++)
		{
			ir[i] = f[i];
		}
		return ir;
	}

/* Initialise cipher */
	public virtual void init(int m, sbyte[] key, sbyte[] iv)
	{ // Key=16 bytes
		/* Key Scheduler. Create expanded encryption key */
		int i, j, k, N, nk;
		int[] CipherKey = new int[4];
		sbyte[] b = new sbyte[4];
		nk = 4;
		reset(m,iv);
		N = 44;

		for (i = j = 0;i < nk;i++,j += 4)
		{
			for (k = 0;k < 4;k++)
			{
				b[k] = key[j + k];
			}
			CipherKey[i] = pack(b);
		}
		for (i = 0;i < nk;i++)
		{
			fkey[i] = CipherKey[i];
		}
		for (j = nk,k = 0;j < N;j += nk,k++)
		{
			fkey[j] = fkey[j - nk] ^ SubByte(ROTL24(fkey[j - 1])) ^ ((int)rco[k]) & 0xff;
			for (i = 1;i < nk && (i + j) < N;i++)
			{
				fkey[i + j] = fkey[i + j - nk] ^ fkey[i + j - 1];
			}
		}

 /* now for the expanded decrypt key in reverse order */

		for (j = 0;j < 4;j++)
		{
			rkey[j + N - 4] = fkey[j];
		}
		for (i = 4;i < N - 4;i += 4)
		{
			k = N - 4 - i;
			for (j = 0;j < 4;j++)
			{
				rkey[k + j] = InvMixCol(fkey[i + j]);
			}
		}
		for (j = N - 4;j < N;j++)
		{
			rkey[j - N + 4] = fkey[j];
		}
	}

/* Encrypt a single block */
	public virtual void ecb_encrypt(sbyte[] buff)
	{
		int i, j, k;
		int t;
		sbyte[] b = new sbyte[4];
		int[] p = new int[4];
		int[] q = new int[4];

		for (i = j = 0;i < 4;i++,j += 4)
		{
			for (k = 0;k < 4;k++)
			{
				b[k] = buff[j + k];
			}
			p[i] = pack(b);
			p[i] ^= fkey[i];
		}

		k = 4;

/* State alternates between p and q */
		for (i = 1;i < 10;i++)
		{
			q[0] = fkey[k] ^ ftable[p[0] & 0xff] ^ ROTL8(ftable[((int)((uint)p[1] >> 8)) & 0xff]) ^ ROTL16(ftable[((int)((uint)p[2] >> 16)) & 0xff]) ^ ROTL24(ftable[((int)((uint)p[3] >> 24)) & 0xff]);
			q[1] = fkey[k + 1] ^ ftable[p[1] & 0xff] ^ ROTL8(ftable[((int)((uint)p[2] >> 8)) & 0xff]) ^ ROTL16(ftable[((int)((uint)p[3] >> 16)) & 0xff]) ^ ROTL24(ftable[((int)((uint)p[0] >> 24)) & 0xff]);
			q[2] = fkey[k + 2] ^ ftable[p[2] & 0xff] ^ ROTL8(ftable[((int)((uint)p[3] >> 8)) & 0xff]) ^ ROTL16(ftable[((int)((uint)p[0] >> 16)) & 0xff]) ^ ROTL24(ftable[((int)((uint)p[1] >> 24)) & 0xff]);
			q[3] = fkey[k + 3] ^ ftable[p[3] & 0xff] ^ ROTL8(ftable[((int)((uint)p[0] >> 8)) & 0xff]) ^ ROTL16(ftable[((int)((uint)p[1] >> 16)) & 0xff]) ^ ROTL24(ftable[((int)((uint)p[2] >> 24)) & 0xff]);

			k += 4;
			for (j = 0;j < 4;j++)
			{
				t = p[j];
				p[j] = q[j];
				q[j] = t;
			}
		}

/* Last Round */

		q[0] = fkey[k] ^ ((int)fbsub[p[0] & 0xff] & 0xff) ^ ROTL8((int)fbsub[((int)((uint)p[1] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)fbsub[((int)((uint)p[2] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)fbsub[((int)((uint)p[3] >> 24)) & 0xff] & 0xff);

		q[1] = fkey[k + 1] ^ ((int)fbsub[p[1] & 0xff] & 0xff) ^ ROTL8((int)fbsub[((int)((uint)p[2] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)fbsub[((int)((uint)p[3] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)fbsub[((int)((uint)p[0] >> 24)) & 0xff] & 0xff);

		q[2] = fkey[k + 2] ^ ((int)fbsub[p[2] & 0xff] & 0xff) ^ ROTL8((int)fbsub[((int)((uint)p[3] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)fbsub[((int)((uint)p[0] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)fbsub[((int)((uint)p[1] >> 24)) & 0xff] & 0xff);

		q[3] = fkey[k + 3] ^ ((int)fbsub[(p[3]) & 0xff] & 0xff) ^ ROTL8((int)fbsub[((int)((uint)p[0] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)fbsub[((int)((uint)p[1] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)fbsub[((int)((uint)p[2] >> 24)) & 0xff] & 0xff);

		for (i = j = 0;i < 4;i++,j += 4)
		{
			b = unpack(q[i]);
			for (k = 0;k < 4;k++)
			{
				buff[j + k] = b[k];
			}
		}
	}

/* Decrypt a single block */
	public virtual void ecb_decrypt(sbyte[] buff)
	{
		int i, j, k;
		int t;
		sbyte[] b = new sbyte[4];
		int[] p = new int[4];
		int[] q = new int[4];

		for (i = j = 0;i < 4;i++,j += 4)
		{
			for (k = 0;k < 4;k++)
			{
				b[k] = buff[j + k];
			}
			p[i] = pack(b);
			p[i] ^= rkey[i];
		}

		k = 4;

/* State alternates between p and q */
		for (i = 1;i < 10;i++)
		{
			q[0] = rkey[k] ^ rtable[p[0] & 0xff] ^ ROTL8(rtable[((int)((uint)p[3] >> 8)) & 0xff]) ^ ROTL16(rtable[((int)((uint)p[2] >> 16)) & 0xff]) ^ ROTL24(rtable[((int)((uint)p[1] >> 24)) & 0xff]);
			q[1] = rkey[k + 1] ^ rtable[p[1] & 0xff] ^ ROTL8(rtable[((int)((uint)p[0] >> 8)) & 0xff]) ^ ROTL16(rtable[((int)((uint)p[3] >> 16)) & 0xff]) ^ ROTL24(rtable[((int)((uint)p[2] >> 24)) & 0xff]);
			q[2] = rkey[k + 2] ^ rtable[p[2] & 0xff] ^ ROTL8(rtable[((int)((uint)p[1] >> 8)) & 0xff]) ^ ROTL16(rtable[((int)((uint)p[0] >> 16)) & 0xff]) ^ ROTL24(rtable[((int)((uint)p[3] >> 24)) & 0xff]);
			q[3] = rkey[k + 3] ^ rtable[p[3] & 0xff] ^ ROTL8(rtable[((int)((uint)p[2] >> 8)) & 0xff]) ^ ROTL16(rtable[((int)((uint)p[1] >> 16)) & 0xff]) ^ ROTL24(rtable[((int)((uint)p[0] >> 24)) & 0xff]);

			k += 4;
			for (j = 0;j < 4;j++)
			{
				t = p[j];
				p[j] = q[j];
				q[j] = t;
			}
		}

/* Last Round */

		q[0] = rkey[k] ^ ((int)rbsub[p[0] & 0xff] & 0xff) ^ ROTL8((int)rbsub[((int)((uint)p[3] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)rbsub[((int)((uint)p[2] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)rbsub[((int)((uint)p[1] >> 24)) & 0xff] & 0xff);
		q[1] = rkey[k + 1] ^ ((int)rbsub[p[1] & 0xff] & 0xff) ^ ROTL8((int)rbsub[((int)((uint)p[0] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)rbsub[((int)((uint)p[3] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)rbsub[((int)((uint)p[2] >> 24)) & 0xff] & 0xff);
		q[2] = rkey[k + 2] ^ ((int)rbsub[p[2] & 0xff] & 0xff) ^ ROTL8((int)rbsub[((int)((uint)p[1] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)rbsub[((int)((uint)p[0] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)rbsub[((int)((uint)p[3] >> 24)) & 0xff] & 0xff);
		q[3] = rkey[k + 3] ^ ((int)rbsub[p[3] & 0xff] & 0xff) ^ ROTL8((int)rbsub[((int)((uint)p[2] >> 8)) & 0xff] & 0xff) ^ ROTL16((int)rbsub[((int)((uint)p[1] >> 16)) & 0xff] & 0xff) ^ ROTL24((int)rbsub[((int)((uint)p[0] >> 24)) & 0xff] & 0xff);

		for (i = j = 0;i < 4;i++,j += 4)
		{
			b = unpack(q[i]);
			for (k = 0;k < 4;k++)
			{
				buff[j + k] = b[k];
			}
		}

	}

/* Encrypt using selected mode of operation */
	public virtual int encrypt(sbyte[] buff)
	{
		int j, bytes;
		sbyte[] st = new sbyte[16];
		int fell_off;

// Supported Modes of Operation

		fell_off = 0;
		switch (mode)
		{
		case ECB:
			ecb_encrypt(buff);
			return 0;
		case CBC:
			for (j = 0;j < 16;j++)
			{
				buff[j] ^= f[j];
			}
			ecb_encrypt(buff);
			for (j = 0;j < 16;j++)
			{
				f[j] = buff[j];
			}
			return 0;

		case CFB1:
		case CFB2:
		case CFB4:
			bytes = mode - CFB1 + 1;
			for (j = 0;j < bytes;j++)
			{
				fell_off = (fell_off << 8) | f[j];
			}
			for (j = 0;j < 16;j++)
			{
				st[j] = f[j];
			}
			for (j = bytes;j < 16;j++)
			{
				f[j - bytes] = f[j];
			}
			ecb_encrypt(st);
			for (j = 0;j < bytes;j++)
			{
				buff[j] ^= st[j];
				f[16 - bytes + j] = buff[j];
			}
			return fell_off;

		case OFB1:
		case OFB2:
		case OFB4:
		case OFB8:
		case OFB16:

			bytes = mode - OFB1 + 1;
			ecb_encrypt(f);
			for (j = 0;j < bytes;j++)
			{
				buff[j] ^= f[j];
			}
			return 0;

	default:
			return 0;
		}
	}

/* Decrypt using selected mode of operation */
	public virtual int decrypt(sbyte[] buff)
	{
		int j, bytes;
		sbyte[] st = new sbyte[16];
		int fell_off;

   // Supported modes of operation
		fell_off = 0;
		switch (mode)
		{
		case ECB:
			ecb_decrypt(buff);
			return 0;
		case CBC:
			for (j = 0;j < 16;j++)
			{
				st[j] = f[j];
				f[j] = buff[j];
			}
			ecb_decrypt(buff);
			for (j = 0;j < 16;j++)
			{
				buff[j] ^= st[j];
				st[j] = 0;
			}
			return 0;
		case CFB1:
		case CFB2:
		case CFB4:
			bytes = mode - CFB1 + 1;
			for (j = 0;j < bytes;j++)
			{
				fell_off = (fell_off << 8) | f[j];
			}
			for (j = 0;j < 16;j++)
			{
				st[j] = f[j];
			}
			for (j = bytes;j < 16;j++)
			{
				f[j - bytes] = f[j];
			}
			ecb_encrypt(st);
			for (j = 0;j < bytes;j++)
			{
				f[16 - bytes + j] = buff[j];
				buff[j] ^= st[j];
			}
			return fell_off;
		case OFB1:
		case OFB2:
		case OFB4:
		case OFB8:
		case OFB16:
			bytes = mode - OFB1 + 1;
			ecb_encrypt(f);
			for (j = 0;j < bytes;j++)
			{
				buff[j] ^= f[j];
			}
			return 0;


		default:
			return 0;
		}
	}

/* Clean up and delete left-overs */
	public virtual void end()
	{ // clean up
		int i;
		for (i = 0;i < 44;i++)
		{
			fkey[i] = rkey[i] = 0;
		}
		for (i = 0;i < 16;i++)
		{
			f[i] = 0;
		}
	}
/*
	public static void main(String[] args) {
		int i;

		byte[] key=new byte[16];
		byte[] block=new byte[16];
		byte[] iv=new byte[16];

		for (i=0;i<16;i++) key[i]=0;
		key[0]=1;
		for (i=0;i<16;i++) iv[i]=(byte)i;
		for (i=0;i<16;i++) block[i]=(byte)i;

		AES a=new AES();

		a.init(CBC,key,iv);
		System.out.println("Plain= ");
		for (i=0;i<16;i++)  System.out.format("%02X ", block[i]&0xff);
		System.out.println("");

		a.encrypt(block);

		System.out.println("Encrypt= ");
		for (i=0;i<16;i++)  System.out.format("%02X ", block[i]&0xff);
		System.out.println("");

		a.reset(CBC,iv);
		a.decrypt(block);

		System.out.println("Decrypt= ");
		for (i=0;i<16;i++)  System.out.format("%02X ", block[i]&0xff);
		System.out.println("");

		a.end();

	} */
}
