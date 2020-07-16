#include "Archs/ARM/ThumbOpcodes.h"

#include "Archs/ARM/Arm.h"
#include "Core/Common.h"

/*	Placeholders:
	d	register
	s	register
	n	register
	o	register
	D	high register
	S	high register
	i	x bit immediate
	I	32 bit immediate
	p	load pc relative immediate
	P	literal pool value
	R	register list
	r	specific register
*/

const tThumbOpcode ThumbOpcodes[] = {
//	Name        Mask                Encod   Type          Len   Flags
	{ L"lsl",   "d,s,/#i\x05",      0x0000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE },
	{ L"lsl",   "d,/#i\x05",        0x0000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_DS },
	{ L"asl",   "d,s,/#i\x05",      0x0000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE },
	{ L"asl",   "d,/#i\x05",        0x0000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_DS },
	{ L"lsr",   "d,s,/#i\x05",      0x0800, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_RIGHTSHIFT_IMMEDIATE },
	{ L"lsr",   "d,/#i\x05",        0x0800, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_RIGHTSHIFT_IMMEDIATE|THUMB_DS },
	{ L"asr",   "d,s,/#i\x05",      0x1000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_RIGHTSHIFT_IMMEDIATE },
	{ L"asr",   "d,/#i\x05",        0x1000, THUMB_TYPE1,    2,  THUMB_IMMEDIATE|THUMB_RIGHTSHIFT_IMMEDIATE|THUMB_DS },

	{ L"add",   "d,s,n",            0x1800, THUMB_TYPE2,    2,  THUMB_REGISTER },
	{ L"add",   "d,n",              0x1800, THUMB_TYPE2,    2,  THUMB_REGISTER|THUMB_DS },
	{ L"sub",   "d,s,n",            0x1A00, THUMB_TYPE2,    2,  THUMB_REGISTER },
	{ L"sub",   "d,n",              0x1A00, THUMB_TYPE2,    2,  THUMB_REGISTER|THUMB_DS },
	{ L"add",   "d,s,/#i\x03",      0x1C00, THUMB_TYPE2,    2,  THUMB_IMMEDIATE },
	{ L"sub",   "d,s,/#i\x03",      0x1E00, THUMB_TYPE2,    2,  THUMB_IMMEDIATE },
	{ L"mov",   "d,s",              0x1C00, THUMB_TYPE2,    2,  0 },

	{ L"mov",   "d,/#i\x08",        0x2000, THUMB_TYPE3,    2,  THUMB_IMMEDIATE },
	{ L"cmp",   "d,/#i\x08",        0x2800, THUMB_TYPE3,    2,  THUMB_IMMEDIATE },
	{ L"add",   "d,/#i\x08",        0x3000, THUMB_TYPE3,    2,  THUMB_IMMEDIATE },
	{ L"sub",   "d,/#i\x08",        0x3800, THUMB_TYPE3,    2,  THUMB_IMMEDIATE },

	{ L"and",   "d,s",              0x4000, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"eor",   "d,s",              0x4040, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"xor",   "d,s",              0x4040, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"lsl",   "d,s",              0x4080, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"lsr",   "d,s",              0x40C0, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"asr",   "d,s",              0x4100, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"adc",   "d,s",              0x4140, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"sbc",   "d,s",              0x4180, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"ror",   "d,s",              0x41C0, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"tst",   "d,s",              0x4200, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"neg",   "d,s",              0x4240, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"cmp",   "d,s",              0x4280, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"cmn",   "d,s",              0x42C0, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"orr",   "d,s",              0x4300, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"mul",   "d,s",              0x4340, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"bic",   "d,s",              0x4380, THUMB_TYPE4,    2,  THUMB_REGISTER },
	{ L"mvn",   "d,s",              0x43C0, THUMB_TYPE4,    2,  THUMB_REGISTER },

	{ L"add",   "D,S",              0x4400, THUMB_TYPE5,    2,  THUMB_D|THUMB_S },
	{ L"cmp",   "D,S",              0x4500, THUMB_TYPE5,    2,  THUMB_D|THUMB_S },
	{ L"mov",   "D,S",              0x4600, THUMB_TYPE5,    2,  THUMB_D|THUMB_S },
	{ L"nop",   "",                 0x46C0, THUMB_TYPE5,    2,  0 },
	{ L"bx",    "S",                0x4700, THUMB_TYPE5,    2,  THUMB_S },
	{ L"blx",   "S",                0x4780, THUMB_TYPE5,    2,  THUMB_S|THUMB_ARM9 },

	{ L"ldr",   "d,[r\xF]",         0x4800, THUMB_TYPE6,    2,  0 },
	{ L"ldr",   "d,[r\xF,/#i\x08]", 0x4800, THUMB_TYPE6,    2,  THUMB_IMMEDIATE|THUMB_WORD },
	{ L"ldr",   "d,[/#I\x20]",      0x4800, THUMB_TYPE6,    2,  THUMB_IMMEDIATE|THUMB_PCR },
	{ L"ldr",   "d,=/#I\x20",       0x4800, THUMB_TYPE6,    2,  THUMB_IMMEDIATE|THUMB_POOL },

	{ L"str",   "d,[s,o]",          0x5000, THUMB_TYPE7,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"strb",  "d,[s,o]",          0x5400, THUMB_TYPE7,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldr",   "d,[s,o]",          0x5800, THUMB_TYPE7,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldrb",  "d,[s,o]",          0x5C00, THUMB_TYPE7,    2,  THUMB_D|THUMB_S|THUMB_O },

	{ L"strh",   "d,[s,o]",         0x5200, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldsb",   "d,[s,o]",         0x5600, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldrsb",  "d,[s,o]",         0x5600, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldrh",   "d,[s,o]",         0x5A00, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldsh",   "d,[s,o]",         0x5E00, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },
	{ L"ldrsh",  "d,[s,o]",         0x5E00, THUMB_TYPE8,    2,  THUMB_D|THUMB_S|THUMB_O },

	{ L"str",   "d,[s,/#i\x05]",    0x6000, THUMB_TYPE9,    2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE|THUMB_WORD },
	{ L"str",   "d,[s]",            0x6000, THUMB_TYPE9,    2,  THUMB_D|THUMB_S },
	{ L"ldr",   "d,[s,/#i\x05]",    0x6800, THUMB_TYPE9,    2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE|THUMB_WORD },
	{ L"ldr",   "d,[s]",            0x6800, THUMB_TYPE9,    2,  THUMB_D|THUMB_S },
	{ L"strb",  "d,[s,/#i\x05]",    0x7000, THUMB_TYPE9,    2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE },
	{ L"strb",  "d,[s]",            0x7000, THUMB_TYPE9,    2,  THUMB_D|THUMB_S },
	{ L"ldrb",  "d,[s,/#i\x05]",    0x7800, THUMB_TYPE9,    2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE },
	{ L"ldrb",  "d,[s]",            0x7800, THUMB_TYPE9,    2,  THUMB_D|THUMB_S },

	{ L"strh",  "d,[s,/#i\x05]",    0x8000, THUMB_TYPE10,   2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE|THUMB_HALFWORD },
	{ L"strh",  "d,[s]",            0x8000, THUMB_TYPE10,   2,  THUMB_D|THUMB_S },
	{ L"ldrh",  "d,[s,/#i\x05]",    0x8800, THUMB_TYPE10,   2,  THUMB_D|THUMB_S|THUMB_IMMEDIATE|THUMB_HALFWORD },
	{ L"ldrh",  "d,[s]",            0x8800, THUMB_TYPE10,   2,  THUMB_D|THUMB_S },

	{ L"str",   "d,[r\xD,/#i\x08]", 0x9000, THUMB_TYPE11,   2,  THUMB_D|THUMB_IMMEDIATE|THUMB_WORD },
	{ L"str",   "d,[r\xD]",         0x9000, THUMB_TYPE11,   2,  THUMB_D },
	{ L"ldr",   "d,[r\xD,/#i\x08]", 0x9800, THUMB_TYPE11,   2,  THUMB_D|THUMB_IMMEDIATE|THUMB_WORD },
	{ L"ldr",   "d,[r\xD]",         0x9800, THUMB_TYPE11,   2,  THUMB_D },

	{ L"add",   "d,r\xF,/#i\x08",   0xA000, THUMB_TYPE12,   2,  THUMB_D|THUMB_IMMEDIATE|THUMB_WORD },
	{ L"add",   "d,=/#i\x20",       0xA000, THUMB_TYPE12,   2,  THUMB_D|THUMB_IMMEDIATE|THUMB_PCR },
	{ L"add",   "d,r\xD,/#i\x08",   0xA800, THUMB_TYPE12,   2,  THUMB_D|THUMB_IMMEDIATE|THUMB_WORD },

	{ L"add",   "r\xD,/#i\x08",     0xB000, THUMB_TYPE13,   2,  THUMB_IMMEDIATE|THUMB_WORD },
	{ L"sub",   "r\xD,/#i\x08",     0xB000, THUMB_TYPE13,   2,  THUMB_IMMEDIATE|THUMB_WORD|THUMB_NEGATIVE_IMMEDIATE },

	{ L"add",   "r\xD,r\xD,/#i\x08",0xB000, THUMB_TYPE13,   2,  THUMB_IMMEDIATE|THUMB_WORD },
	{ L"sub",   "r\xD,r\xD,/#i\x08",0xB000, THUMB_TYPE13,   2,  THUMB_IMMEDIATE|THUMB_WORD|THUMB_NEGATIVE_IMMEDIATE },

	{ L"push",  "/{R\xFF\x40/}",    0xB400, THUMB_TYPE14,   2,  THUMB_RLIST },
	{ L"pop",   "/{R\xFF\x80/}",    0xBC00, THUMB_TYPE14,   2,  THUMB_RLIST },

	{ L"stmia", "/[d/]!,/{R\xFF\x00/}", 0xC000, THUMB_TYPE15,   2,  THUMB_D|THUMB_RLIST },
	{ L"ldmia", "/[d/]!,/{R\xFF\x00/}", 0xC800, THUMB_TYPE15,   2,  THUMB_D|THUMB_RLIST },

	{ L"beq",   "/#I\x08",          0xD000, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH },
	{ L"bne",   "/#I\x08",          0xD100, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bcs",   "/#I\x08",          0xD200, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bhs",   "/#I\x08",          0xD200, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bcc",   "/#I\x08",          0xD300, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"blo",   "/#I\x08",          0xD300, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bmi",   "/#I\x08",          0xD400, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bpl",   "/#I\x08",          0xD500, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bvs",   "/#I\x08",          0xD600, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bvc",   "/#I\x08",          0xD700, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bhi",   "/#I\x08",          0xD800, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bls",   "/#I\x08",          0xD900, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bge",   "/#I\x08",          0xDA00, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"blt",   "/#I\x08",          0xDB00, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"bgt",   "/#I\x08",          0xDC00, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },
	{ L"ble",   "/#I\x08",          0xDD00, THUMB_TYPE16,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },

	{ L"swi",   "/#i\x08",          0xDF00, THUMB_TYPE17,   2,  THUMB_IMMEDIATE },
	{ L"bkpt",  "/#i\x08",          0xBE00, THUMB_TYPE17,   2,  THUMB_IMMEDIATE|THUMB_ARM9 },

	{ L"b",     "/#I\x0B",          0xE000, THUMB_TYPE18,   2,  THUMB_IMMEDIATE|THUMB_BRANCH  },

	{ L"bl",    "r\xE",             0xF800, THUMB_TYPE19,   4,  0 },
	{ L"bl",    "r\xE+/#I\x0B",     0xF800, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_HALFWORD },
	{ L"blh",   "/#I\x0B",          0xF800, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_HALFWORD },
	{ L"bl",    "/#I\x16",          0xF000, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_BRANCH|THUMB_LONG  },
	{ L"blx",   "/#I\x16",          0xF000, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_ARM9|THUMB_EXCHANGE|THUMB_BRANCH|THUMB_LONG },
	{ L"bl",    "/#I\x16",          0xF800, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_BRANCH|THUMB_LONG },
	{ L"blx",   "/#I\x16",          0xF800, THUMB_TYPE19,   4,  THUMB_IMMEDIATE|THUMB_ARM9|THUMB_EXCHANGE|THUMB_BRANCH|THUMB_LONG },

	{ nullptr,  nullptr,            0,      0,              0,  0 }
};

