#include "C_Code.h"
#include "stdlib.h"
#include "include/bmshop.h"
#include "include/unitinfowindow.h"

extern int maxWalletValue;
extern struct PlaySt gPlaySt;
extern struct KeyStatusBuffer* CONST_DATA gKeyStatusPtr;
extern struct walletTable* gWalletTableRam;
extern int GetUnitInfoWindowX(struct Unit* unit, int width);
extern void ForEachAdjacentUnit(int x, int y, void(*func)(struct Unit* unit));
extern const struct ProcCmd* gProcScr_UnitInfoWindow;
extern void UnitInfoWindow_PositionUnitName(struct UnitInfoWindowProc* proc);
extern struct UnitInfoWindowProc* UnitInfoWindow_DrawBase(struct UnitInfoWindowProc* proc, struct Unit* unit, int x, int y, int width, int lines);
extern struct UnitInfoWindowProc* NewUnitInfoWindow(ProcPtr parent);
extern int UNIT_ACTION_STEAL_GOLD;
extern int UNIT_ACTION_GIVE_GOLD;
extern int minSupportRank;
extern int GiveGoldTextId;


//Get/Set Gold Functions
struct walletTable* GetWalletEntry(int index) {
    
    if (index > 0x54) {
        index = (index - 0x26); //0x1-0x40 allies, 0x41-0x54 npc, 0x55-0x87 enemy
    }
    return &gWalletTableRam[index];
}

u32 GetPartyGoldAmount() {

    return GetWalletEntry(gActiveUnit->index)->gold;

};

u32 GetGoldAmountByUnitIndex(u8 unitIdx) {
    return GetWalletEntry(unitIdx)->gold;
};

void SetPartyGoldAmount(s32 amt) {
    GetWalletEntry(gActiveUnit->index)->gold = amt;
    if (amt > maxWalletValue) {
        GetWalletEntry(gActiveUnit->index)->gold = maxWalletValue;
    }
}

void SetGoldAmountByUnitIndex(u8 unitIdx, s16 amt) {
    GetWalletEntry(unitIdx)->gold = amt;
    if (amt > maxWalletValue) {
        GetWalletEntry(unitIdx)->gold = maxWalletValue;
    }
}

// addToPartyGold
void sub_8024E20(u32 amt) {
    s32 new_amt = GetWalletEntry(gActiveUnit->index)->gold + amt;
    GetWalletEntry(gActiveUnit->index)->gold = new_amt;
    if (new_amt > maxWalletValue) {
        GetWalletEntry(gActiveUnit->index)->gold = maxWalletValue;
    }
}

void SetGoldEvent() {
    u8 idx = gEventSlots[EVT_SLOT_1];
    s16 amt = gEventSlots[EVT_SLOT_2];
    SetGoldAmountByUnitIndex(idx, amt);
}

void GetGoldEvent() {
    gEventSlots[EVT_SLOT_2] = GetGoldAmountByUnitIndex(gEventSlots[EVT_SLOT_1]);
}

// Prep screen stuff
//TODO: fix gold display when returning from unit selection
//! FE8U = 0x080985B8
void PrepItemScreen_DrawFunds(struct PrepItemScreenProc* proc) {
    PutText(&gPrepItemTexts[30], TILEMAP_LOCATED(gBG0TilemapBuffer, 19, 17));
    if (proc->pUnits[0] != NULL) {
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 17), 2, GetGoldAmountByUnitIndex(proc->pUnits[0]->index));
    }
    else {
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 17), 2, GetGoldAmountByUnitIndex(GetUnitFromPrepList(proc->hoverUnitIdx)->index));
    }
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, 28, 17), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_G);
    ShowPrepFundsSpriteAt(168, 133);
    BG_EnableSyncByMask(1);
    return;
}

void DisplayPrepsGoldBoxText(u16 * tm, u8 unitIdx)
{
    SetTextFont(0);
    InitSystemTextFont();
    ClearGoldBoxTextTm2Line(tm, SHOP_TEXT_LINES + 1);
    PutNumber(tm, 2, GetGoldAmountByUnitIndex(unitIdx));
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

//! FE8U = 0x08099120
void sub_8099120(struct PrepItemScreenProc* proc) {
    int tmp = proc->unk_34;

    if (!(tmp & 0xf)) {

        if (gKeyStatusPtr->newKeys & R_BUTTON) {
            Proc_Break(proc);
            return;
        }

        if (gKeyStatusPtr->newKeys & A_BUTTON) {
            if (gGMData.state.bits.state_0) {
                switch (GetGMapBaseMenuKind()) {
                case 3:
                    proc->selectedUnitIdx = proc->hoverUnitIdx;

                    if (((proc->hoverUnitIdx % 3) <= 1) && (proc->hoverUnitIdx < PrepGetUnitAmount() - 1)) {
                        proc->hoverUnitIdx++;
                    }
                    else {
                        proc->hoverUnitIdx--;
                    }

                    break;

                case 2:
                    if (UnitHasItem(GetUnitFromPrepList(proc->hoverUnitIdx), ITEM_MEMBERCARD)) {
                        PlaySoundEffect(0x6a);
                        Proc_Goto(proc, 13);
                        return;
                    }

                    PlaySoundEffect(0x6c);

                    return;

                default:
                    Proc_Goto(proc, 0xd);
                    PlaySoundEffect(0x6a);
                    return;
                }
            }
            else {
                proc->selectedUnitIdx = proc->hoverUnitIdx;

                if (((proc->hoverUnitIdx % 3) < 2) && (proc->hoverUnitIdx < PrepGetUnitAmount() - 1)) {
                    proc->hoverUnitIdx++;
                }
                else {
                    proc->hoverUnitIdx--;
                }
            }

            SetUiCursorHandConfig(
                0,
                ((proc->selectedUnitIdx % 3) * 64) + 24,
                (proc->selectedUnitIdx / 3) * 16 + 4 - proc->unk_34,
                2
            );

            Proc_Goto(proc, 2);
            PlaySoundEffect(0x6a);
            return;
        }

        if (gKeyStatusPtr->newKeys & B_BUTTON) {
            if (gGMData.state.bits.state_0) {
                proc->unk_29 = 1;
            }

            Proc_Goto(proc, 13);
            PlaySoundEffect(0x6b);
            return;
        }

        if (PrepItemScreen_DpadKeyHandler(proc) != 0) {
            UpdatePrepItemScreenFace(0, GetUnitFromPrepList(proc->hoverUnitIdx), 60, 76, 0x0503);
            PrepItemScreen_DrawFunds(proc);
            sub_8099F7C(&gPrepItemTexts[15], TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9), GetUnitFromPrepList(proc->hoverUnitIdx), 2);
            BG_EnableSyncByMask(1);
        }
    }

    sub_8098FAC(proc);

    return;
}

//Shop Stuff
void Shop_Loop_BuyKeyHandler(struct ProcShop* proc)
{
    u8 head_loc;
    u32 cursor_at_head;
    int price;
    int a;
    int b;

    Shop_TryMoveHandPage();

    BG_SetPosition(2, 0, ShopSt_GetBg2Offset());

    head_loc = proc->head_loc;
    cursor_at_head = ShopSt_GetHeadLoc() != head_loc;

    proc->head_loc = ShopSt_GetHeadLoc();
    proc->hand_loc = ShopSt_GetHandLoc();

    proc->head_idx = proc->head_loc;
    proc->hand_idx = proc->hand_loc;

    a = proc->head_loc;
    a *= 16;

    b = ((proc->hand_loc * 16)) - 72;

    DisplayUiHand(56, a - b);

    if ((proc->helpTextActive != 0) && (cursor_at_head != 0))
    {
        a = (proc->head_loc * 16);
        b = ((proc->hand_loc * 16) - 72);
        StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);
    }

    DisplayShopUiArrows();

    if (IsShopPageScrolling() != 0)
        return;

    if (proc->helpTextActive != 0)
    {
        if (gKeyStatusPtr->newKeys & (B_BUTTON | R_BUTTON))
        {
            proc->helpTextActive = 0;
            CloseHelpBox();
        }
        return;
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON)
    {
        proc->helpTextActive = 1;
        a = (proc->head_loc * 16);
        b = ((proc->hand_loc * 16) - 72);
        StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);
        return;
    }

    price = GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]);
 
    if (gKeyStatusPtr->newKeys & A_BUTTON)
    {   
        int unitGold;
        if (gPlaySt.chapterStateBits & PLAY_FLAG_PREPSCREEN) {      //Check if in preps
            unitGold = (int)GetGoldAmountByUnitIndex(proc->unit->index);
        }
        else {
            unitGold = (int)GetPartyGoldAmount();
        }

        if (price > unitGold)
        {
            StartShopDialogue(0x8B2, proc);
            // SHOP_TYPE_ARMORY: "You don't have the money![.][A]"
            // SHOP_TYPE_VENDOR: "You're short of funds.[A]"
            // SHOP_TYPE_SECRET_SHOP: "Heh! Not enough money![A]"

            Proc_Goto(proc, 1);
        }
        else
        {
            SetTalkNumber(price);
            StartShopDialogue(0x8B5, proc);
            // SHOP_TYPE_ARMORY: "How does [.][G] gold[.][NL]sound to you?[.][Yes]"
            // SHOP_TYPE_VENDOR: "That's worth [.][G] gold.[NL]Is that all right?[Yes]"
            // SHOP_TYPE_SECRET_SHOP: "That is worth [G] gold.[NL]Is that acceptable?[.][Yes]"

            Proc_Break(proc);
        }
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON)
    {
        PlaySoundEffect(0x6B);
        Proc_Goto(proc, PL_SHOP_SELL_NOITEM);
        return;
    }
}

void Shop_HandleSellConfirmPrompt(struct ProcShop* proc)
{
    if (GetTalkChoiceResult() == TALK_CHOICE_YES)
    {
        PlaySeDelayed(0xB9, 8);

        gActionData.unitActionType = UNIT_ACTION_SHOPPED;

        if (gPlaySt.chapterStateBits & PLAY_FLAG_PREPSCREEN) {      //Check if in preps
            SetGoldAmountByUnitIndex(
                proc->unit->index, GetGoldAmountByUnitIndex(proc->unit->index) + GetItemSellPrice(proc->unit->items[proc->head_loc]));
            
            UnitRemoveItem(proc->unit, proc->head_loc);

            UpdateShopItemCounts(proc);
            ShopInitTexts_OnSell(proc);
            DisplayPrepsGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6), proc->unit->index);
        }
        else {
            SetPartyGoldAmount(GetPartyGoldAmount() + GetItemSellPrice(proc->unit->items[proc->head_loc]));
        
            UnitRemoveItem(proc->unit, proc->head_loc);

            UpdateShopItemCounts(proc);
            ShopInitTexts_OnSell(proc);
            DisplayGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6));
        
        }

        if (proc->unitItemCount == 0)
            Proc_Goto(proc, PL_SHOP_SELL_NOITEM);
    }
    else
        Proc_Goto(proc, PL_SHOP_SELL);
}

void DrawShopItemPriceLine(struct Text* th, int item, struct Unit* unit, u16* dst)
{
    bool unuseable;

    int price = GetItemPurchasePrice(unit, item);

    if (unit == 0)
        unuseable = true;
    else
        unuseable = IsItemDisplayUsable(unit, item);

    DrawItemMenuLine(th, item, unuseable, dst);

    if (gPlaySt.chapterStateBits & PLAY_FLAG_PREPSCREEN) {
        PutNumber(
            dst + 0x11,
            (int)GetGoldAmountByUnitIndex(unit->pCharacterData->number) >= price
            ? TEXT_COLOR_SYSTEM_BLUE
            : TEXT_COLOR_SYSTEM_GRAY,
            price
        );
    }
    else {
        PutNumber(
            dst + 0x11,
            (int)GetPartyGoldAmount() >= price
            ? TEXT_COLOR_SYSTEM_BLUE
            : TEXT_COLOR_SYSTEM_GRAY,
            price
        );
    }
}

void HandleShopBuyAction(struct ProcShop* proc)
{
    PlaySeDelayed(0xB9, 8);

    gActionData.unitActionType = UNIT_ACTION_SHOPPED;

    if (gPlaySt.chapterStateBits & PLAY_FLAG_PREPSCREEN) {      //Check if in preps
        SetGoldAmountByUnitIndex(
            proc->unit->index, GetGoldAmountByUnitIndex(proc->unit->index) - GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]));
    
        UpdateShopItemCounts(proc);
        DrawShopSoldItems(proc);

        DisplayPrepsGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6), proc->unit->index);
    }
    else {
        SetPartyGoldAmount(
            GetPartyGoldAmount() - GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]));

        UpdateShopItemCounts(proc);
        DrawShopSoldItems(proc);

        DisplayGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6));
    }
}

void StartUiGoldBox(ProcPtr parent)
{

    struct ProcShop* proc2 = parent;

    struct ProcShop* proc;

    Decompress(Img_ShopGoldBox, OBJ_CHR_ADDR(OBJCHR_SHOP_GOLDBOX));

    proc = Proc_Start(gProcScr_GoldBox, parent);
    proc->goldbox_x = 0xAC;
    proc->goldbox_y = 0x2D;
    proc->goldbox_oam2 = OBJ_PALETTE(OBJPAL_SHOP_GOLDBOX) + OBJ_CHAR(OBJCHR_SHOP_GOLDBOX);
    ApplyPalette(gUiFramePaletteA, 0x10 + OBJPAL_SHOP_GOLDBOX);
    InitGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 28, 6));

    if (gPlaySt.chapterStateBits & PLAY_FLAG_PREPSCREEN) {
        DisplayPrepsGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6), proc2->unit->index);
    }

    else {
        DisplayGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6));
    }

}

//EMS Modules
void saveGold(void* target, unsigned size) {
    WriteAndVerifySramFast((void*)gWalletTableRam, target, size);
}

void loadGold(void* source, unsigned size) {
    ReadSramFast(source, (void*)gWalletTableRam, size);
}


//Menu Command Stuff
void AddAsTarget_IfCanStealGoldFrom(struct Unit* unit) {

    if (UNIT_FACTION(unit) != FACTION_RED) {
        return;
    }

    if (gActiveUnit->spd < unit->spd) {
        return;
    }

    if (GetGoldAmountByUnitIndex(unit->index) == 0) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
    }


void MakeTargetListForGoldSteal(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, AddAsTarget_IfCanStealGoldFrom);

    return;
}

u8 GoldStealCommandUsability(const struct MenuItemDef* def, int number) {
    if (!(UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL)) {
        return MENU_NOTSHOWN;
    }

    if (gActiveUnit->state & US_HAS_MOVED) {
        return MENU_NOTSHOWN;
    }

    MakeTargetListForGoldSteal(gActiveUnit);
    if (GetSelectTargetCount() == 0) {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

void DrawUnitGoldText(struct Text* text, struct Unit* unit) {
    ClearText(text);

    Text_InsertDrawString(text, 0, 3, GetStringFromIndex(0x03F2)); // TODO: msgid "Gold[.]"
    Text_InsertDrawNumberOrBlank(text, 56, 2, GetGoldAmountByUnitIndex(unit->index));

    return;
}

void StartUnitGoldInfoWindow(ProcPtr parent) {
    struct UnitInfoWindowProc* proc = NewUnitInfoWindow(parent);
    InitTextDb(proc->lines + 0, 8);

    return;
}

void RefreshUnitGoldInfoWindow(struct Unit* unit) {

    int y = 0;
    int x = GetUnitInfoWindowX(unit, 10);

    struct UnitInfoWindowProc* proc = UnitInfoWindow_DrawBase(0, unit, x, 0, 10, 1);

    DrawUnitGoldText(proc->lines + 0, unit);
    PutText(proc->lines + 0, gBG0TilemapBuffer + TILEMAP_INDEX(x + 1, y + 3));

    return;
}

u8 GoldStealMapSelect_SwitchIn(ProcPtr proc, struct SelectTarget* target) {
    ChangeActiveUnitFacing(target->x, target->y);

    RefreshUnitGoldInfoWindow(GetUnit(target->uid));

    return 0; // BUG?
};

void GoldStealMapSelect_Init(ProcPtr menu) {
    StartUnitGoldInfoWindow(menu);
    StartSubtitleHelp(menu, GetStringFromIndex(0x86D)); // TODO: msgid "Select which unit to steal from."

    return;
}

u8 GoldStealMapSelect_Select(ProcPtr proc, struct SelectTarget* target) {
    gActionData.targetIndex = target->uid;
    gActionData.unitActionType = UNIT_ACTION_STEAL_GOLD;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

struct SelectInfo const gSelectInfo_GoldSteal =
{
    .onInit = GoldStealMapSelect_Init,
    .onEnd = MISMATCHED_SIGNATURE(ClearBg0Bg1),
    .onSwitchIn = GoldStealMapSelect_SwitchIn,
    .onSelect = GoldStealMapSelect_Select,
    .onCancel = GenericSelection_BackToUM,
};

u8 GoldStealCommandEffect(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    ClearBg0Bg1();

    MakeTargetListForGoldSteal(gActiveUnit);

    NewTargetSelection(&gSelectInfo_GoldSteal);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

void BeginMapAnimForGoldSteal(void)
{
    gBattleActor.weaponBefore = ITEM_SWORD_IRON;

    gManimSt.hp_changing = 0;
    gManimSt.u62 = 0;       //This turned stole item popup off
    gManimSt.actorCount_maybe = 2;

    gManimSt.subjectActorId = 0;
    gManimSt.targetActorId = 1;

    SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);
    Proc_Start(ProcScr_MapAnimSteal, PROC_TREE_3);
}

s8 ActionStealGold(ProcPtr proc) {

    int item;

    struct Unit* target = GetUnit(gActionData.targetIndex);

    item = GetUnit(gActionData.targetIndex)->items[gActionData.itemSlotIndex];

    sub_8024E20(GetGoldAmountByUnitIndex(target->index));
    SetGoldAmountByUnitIndex(target->index, 0);

    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), -1);
    gBattleTarget.terrainId = TERRAIN_PLAINS;
    InitBattleUnit(&gBattleTarget, GetUnit(gActionData.targetIndex));
    gBattleTarget.weapon = item;
    BattleApplyMiscAction(proc);

    EndAllMus();
    BeginMapAnimForGoldSteal();

    return 0;

}

void AddAsTarget_IfCanGiveGoldTo(struct Unit* unit) {

    if (UNIT_FACTION(unit) != FACTION_BLUE) {
        return;
    }

    if (!(UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL)) {

        int i;
        int count = GetUnitSupporterCount(unit);
        for (i = 0; i < count; i++) {

            struct Unit* other = GetUnitSupporterUnit(unit, i);

            if (other == gActiveUnit && GetSupportLevelBySupportIndex(unit, i) >= minSupportRank) {
                AddTarget(unit->xPos, unit->yPos, unit->index, 0);
            }
        }
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForGoldGive(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, AddAsTarget_IfCanGiveGoldTo);

    return;
}

u8 GoldGiveCommandUsability(const struct MenuItemDef* def, int number) {

    if (GetPartyGoldAmount() == 0) {
        return MENU_NOTSHOWN;
    }

    if (gActiveUnit->state & US_HAS_MOVED) {
        return MENU_NOTSHOWN;
    }

    MakeTargetListForGoldGive(gActiveUnit);
    if (GetSelectTargetCount() == 0) {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

void GoldGiveMapSelect_Init(ProcPtr menu) {
    StartUnitGoldInfoWindow(menu);
    StartSubtitleHelp(menu, GetStringFromIndex(GiveGoldTextId));

    return;
}

u8 GoldGiveMapSelect_Select(ProcPtr proc, struct SelectTarget* target) {
    gActionData.targetIndex = target->uid;
    gActionData.unitActionType = UNIT_ACTION_GIVE_GOLD;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

struct SelectInfo const gSelectInfo_GoldGive =
{
    .onInit = GoldGiveMapSelect_Init,
    .onEnd = MISMATCHED_SIGNATURE(ClearBg0Bg1),
    .onSwitchIn = GoldStealMapSelect_SwitchIn,
    .onSelect = GoldGiveMapSelect_Select,
    .onCancel = GenericSelection_BackToUM,
};

s8 ActionGiveGold(ProcPtr proc) {

    struct Unit* target = GetUnit(gActionData.targetIndex);

    u16 receiverGold = GetGoldAmountByUnitIndex(target->index);
    
    u16 amt = GetPartyGoldAmount();

    if (amt + receiverGold > maxWalletValue) {
        SetGoldAmountByUnitIndex(target->index, maxWalletValue);
        SetPartyGoldAmount(GetPartyGoldAmount - (maxWalletValue - receiverGold));
    }
    else {
        SetGoldAmountByUnitIndex(target->index, (amt + receiverGold));
        SetPartyGoldAmount(GetPartyGoldAmount() - amt);
    }

    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), -1);
    gBattleTarget.terrainId = TERRAIN_PLAINS;
    InitBattleUnit(&gBattleTarget, GetUnit(gActionData.targetIndex));

    EndAllMus();
    BeginMapAnimForGoldSteal();

    return 0;

}

u8 GoldGiveCommandEffect(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    ClearBg0Bg1();

    MakeTargetListForGoldGive(gActiveUnit);

    NewTargetSelection(&gSelectInfo_GoldGive);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}
