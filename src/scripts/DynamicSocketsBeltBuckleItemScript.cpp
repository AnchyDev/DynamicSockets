#include "DynamicSocketsBeltBuckleItemScript.h"
#include "DynamicSocketsMgr.h"

#include "Player.h"
#include "Spell.h"

#include <AI/ScriptedAI/ScriptedGossip.h>

bool DynamicSocketsBeltBucklePlayerScript::CanCastItemUseSpell(Player* player, Item* item, SpellCastTargets const& targets, uint8 /*cast_count*/, uint32 /*glyphIndex*/)
{
    if (!item || item->GetTemplate()->ItemId != 41611)
    {
        return true;
    }

    if (!player)
    {
        return true;
    }

    Item* targetItem = targets.GetItemTarget();
    if (!targetItem)
    {
        return false;
    }

    // Can only use on owned items.
    if (targetItem->GetOwnerGUID() != player->GetGUID())
    {
        return false;
    }

    std::vector<EnchantmentSlot> populatedSlots;

    if (targetItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT))
        populatedSlots.push_back(SOCK_ENCHANTMENT_SLOT);
    if (targetItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2))
        populatedSlots.push_back(SOCK_ENCHANTMENT_SLOT_2);
    if (targetItem->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3))
        populatedSlots.push_back(SOCK_ENCHANTMENT_SLOT_3);

    if (populatedSlots.empty())
    {
        sDynamicSocketsMgr->SendNotification(player, "This item does not have any sockets.");
        return false;
    }

    uint32 randomSlot = urand(0, populatedSlots.size() - 1);
    EnchantmentSlot enchantmentSlot = populatedSlots.at(randomSlot);

    if (targetItem->IsEquipped())
    {
        sDynamicSocketsMgr->HandleApplyEnchantment(player, targetItem, enchantmentSlot, false, false, false);
    }

    targetItem->SetEnchantment(enchantmentSlot, 0, 0, 0);

    return false;
}
