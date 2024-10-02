
#include "include/gbafe.h"

typedef struct Unit Unit;
typedef struct walletTable walletTable;

extern u32 walletTableRam;

struct walletTable {
	u16 gold;
};

enum { LINES_MAX = 5 };

struct UnitInfoWindowProc {
	/* 00 */ PROC_HEADER;

	/* 2C */ struct Unit* unit;

	/* 30 */ struct Text name;
	/* 38 */ struct Text lines[LINES_MAX];

	/* 60 */ u8 x;
	/* 61 */ u8 y;
	/* 62 */ u8 xUnitSprite;
	/* 63 */ u8 xNameText;
};

