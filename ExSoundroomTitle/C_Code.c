#include "C_Code.h"

extern struct Unknown201F148 gUnk_SoundRoom_0201F148;
extern struct SoundRoomEnt NewSoundRoom[];

void DrawSoundRoomSongTitle(int index)
{
    const char* str;

    if (index == -1)
    {
        str = GetStringFromIndex(0x7D0); // TODO: msgid "?????[.]"
    }
    else
    {
        str = GetStringFromIndex(NewSoundRoom[index].nameTextId);
    }

    SetTextFont(&gUnk_SoundRoom_0201F148.font);
    SetTextFontGlyphs(1);

    SpriteText_DrawBackgroundExt(&gUnk_SoundRoom_0201F148.text[1], 0);

    Text_SetCursor(&gUnk_SoundRoom_0201F148.text[1], GetStringTextCenteredPos(224, str));
    Text_SetColor(&gUnk_SoundRoom_0201F148.text[1], 0);
    Text_DrawString(&gUnk_SoundRoom_0201F148.text[1], str);

    SetTextFont(NULL);

    return;
}

void sub_80B0088(int y, u16 unk)
{
    int i;

    if (unk > 32)
    {
        y = OAM0_Y(y);

        // clang-format off
        SetObjAffine(
            0,
            Div(+COS(0) * 16, 256),
            Div(-SIN(0) * 16, unk),
            Div(+SIN(0) * 16, 256),
            Div(+COS(0) * 16, unk)
        );
        // clang-format on

        for (i = 0; i < 7; i++)
        {
            PutSpriteExt(4, 8 + i * 32, y + 264, gObject_32x16, i * 4 + gUnk_SoundRoom_0201F148.unk_50 + 0x1000);      // Loads textSprites
        }

        for (i = 0; i < 3; i++)
        {
            PutSpriteExt(4, i * 32, y + 256, gObject_32x32, 0xcb00 + i * 4);       // Left side of the plate
        }

        PutSpriteExt(4, 96, y + 256, gObject_8x32, 0xcb00 + 12);
        PutSpriteExt(4, 104, y + 256, gObject_32x32, 0xcb00 + 10);      // Midsection of the plate
        PutSpriteExt(4, 136, y + 256, gObject_8x32, 0xcb00 + 11);

       for (i = 0; i < 3; i++)
       {
           PutSpriteExt(4, i * 32 + 144, y + 256, gObject_32x32, 0xcb00 + (i + 3) * 4);       // Right side of the plate
       }

    }

    return;
}