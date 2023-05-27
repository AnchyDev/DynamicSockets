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

    auto queue = sDynamicSocketsMgr->GetSocketQueue();
    auto queueItem = queue->Get(player);

    queueItem->SocketRemoveSender = item;
    queueItem->SocketRemoveTarget = targetItem;

    ClearGossipMenuFor(player);
    // GOSSIP_ICON_DOT is important here, it will progress the gossip; effectively not showing the gossip window.
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "Remove Random Socket", item->GetSlot(), BELT_BUCKLE_OFFSET + targetItem->GetSlot(), "Are you sure you want to remove a random socket?", 0, false);
    SendGossipMenuFor(player, 441150, player->GetGUID());

    return false;
}

void DynamicSocketsBeltBucklePlayerScript::OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 sender, uint32 action)
{
    if (action >= BELT_BUCKLE_OFFSET)
    {
        uint32 slot = action - BELT_BUCKLE_OFFSET;

        auto queue = sDynamicSocketsMgr->GetSocketQueue();
        auto queueItem = queue->Get(player);

        Item* senderItem = queueItem->SocketRemoveSender;
        Item* targetItem = queueItem->SocketRemoveTarget;

        if (!targetItem || !senderItem)
        {
            sDynamicSocketsMgr->SendNotification(player, "An internal error occured.");
            CloseGossipMenuFor(player);
            return;
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
            CloseGossipMenuFor(player);
            return;
        }

        uint32 randomSlot = urand(0, populatedSlots.size() - 1);
        EnchantmentSlot enchantmentSlot = populatedSlots.at(randomSlot);

        if (targetItem->IsEquipped())
        {
            sDynamicSocketsMgr->HandleApplyEnchantment(player, targetItem, enchantmentSlot, false, false, false);
        }

        targetItem->SetEnchantment(enchantmentSlot, 0, 0, 0);
        uint32 count = 1;
        player->DestroyItemCount(senderItem, count, true);

        queueItem->SocketRemoveSender = nullptr;
        queueItem->SocketRemoveTarget = nullptr;

        CloseGossipMenuFor(player);
    }
}
