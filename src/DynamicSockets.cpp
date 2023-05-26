#include "DynamicSockets.h"
#include "DynamicSocketsMgr.h"

#include "Chat.h"
#include "Player.h"

bool DynamicSocketsPlayerScript::CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition)
{
    if (!player || !item)
    {
        return true;
    }

    if (slot != SOCK_ENCHANTMENT_SLOT && slot != SOCK_ENCHANTMENT_SLOT_2 && slot != SOCK_ENCHANTMENT_SLOT_3 && slot != BONUS_ENCHANTMENT_SLOT)
    {
        return true;
    }

    sDynamicSocketsMgr->HandleApplyEnchantment(player, item, slot, apply, applyDuration, ignoreCondition);

    return false;
}

void DynamicSocketsPlayerScript::OnStoreNewItem(Player* player, Item* item, uint32 /*count*/)
{
    if (!player || !item)
    {
        return;
    }

    auto itemProto = item->GetTemplate();
    if (itemProto->Class != ITEM_CLASS_ARMOR && itemProto->Class != ITEM_CLASS_WEAPON)
    {
        return;
    }

    Item* socketItem = nullptr;
    bool foundGem = false;

    for (uint16 i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
    {
        Item* slotItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!slotItem)
        {
            continue;
        }

        auto slotItemProto = slotItem->GetTemplate();

        // Simple gems include, tigerseyes, malachite, etc..
        // So we cannot applies those to gear.
        if (slotItemProto->Class == ITEM_CLASS_GEM &&
            slotItemProto->SubClass != ITEM_SUBCLASS_GEM_SIMPLE)
        {
            socketItem = slotItem;
            foundGem = true;
            break;
        }
    }

    if (!foundGem)
    {
        LOG_INFO("module", "No gem to socket");
        return;
    }

    LOG_INFO("module", "Found gem to socket");

    bool result = sDynamicSocketsMgr->TrySocketItem(player, item, socketItem);
    if (result)
    {
        player->RemoveItem(socketItem->GetBagSlot(), socketItem->GetSlot(), true);
        ChatHandler(player->GetSession()).SendSysMessage(Acore::StringFormatFmt("Socketed item {}.", item->GetTemplate()->Name1));
    }
}

void SC_AddDynamicSocketsScripts()
{
    new DynamicSocketsPlayerScript();
}
