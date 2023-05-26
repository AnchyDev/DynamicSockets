#include "DynamicSocketsGameObjectScript.h"
#include "DynamicSocketsMgr.h"

#include "Player.h"
#include <AI/ScriptedAI/ScriptedGossip.h>

bool DynamicSocketsGameObjectScript::OnGossipHello(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\ITEMSOCKETINGFRAME\\UI-EMPTYSOCKET:16|t Add Socket", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD);
    SendGossipMenuFor(player, MYSTIC_ANVIL_TEXT_ID, go->GetGUID());

    return true;
}

bool DynamicSocketsGameObjectScript::OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
{
    if (action == 0)
    {
        CloseGossipMenuFor(player);
        return true;
    }

    if (action == MYSTIC_ANVIL_SOCKET_ADD)
    {
        ClearGossipMenuFor(player);

        // TODO: Clean this up with a loop.

        for (uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            // Items I don't want socketable.
            if (slot == SLOT_SHIRT || slot == SLOT_TABARD)
            {
                continue;
            }

            if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, slot))
            {
                std::string option = Acore::StringFormatFmt("{} {}", sDynamicSocketsMgr->GetIconForCharacterSlot(slot), sDynamicSocketsMgr->GetNameFromCharacterSlot(slot));
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, option, GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + slot);
            }
        }

        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_TEXT_ID, go->GetGUID());
    }

    if (action >= MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT && action < MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT)
    {
        ClearGossipMenuFor(player);
        auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT;
        auto freeSocketSlot = sDynamicSocketsMgr->GetFreeSocketSlot(player, slot);
        if (freeSocketSlot != MAX_ENCHANTMENT_SLOT)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\ICONS\\INV_Misc_Gem_Variety_02:16|t Use random gem from inventory.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT + slot, "|cffFF0000Are you sure you want to gem this item?|r", 0, false);
        }
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_HEAD_TEXT_ID, go->GetGUID());
    }

    if (action >= MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT)
    {
        ClearGossipMenuFor(player);
        auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT;
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Thank you.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT);
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_HEAD_TEXT_ID, go->GetGUID());

        auto gems = sDynamicSocketsMgr->GetGemsFromInventory(player);
        if (gems.empty())
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Found no gems.");
            return true;
        }
        auto gem = gems.at(0);
        auto item = sDynamicSocketsMgr->GetItemFromSlot(player, slot);
        if (!item)
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Found no item at slot {}.", slot);
            return true;
        }

        if (!sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, slot))
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Failed socket, item not equipped in slot {}.", slot);
            return true;
        }

        auto freeSocketSlot = sDynamicSocketsMgr->GetFreeSocketSlot(player, slot);
        if (freeSocketSlot == MAX_ENCHANTMENT_SLOT)
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Failed socket, no free sockets for slot {}.", slot);
            return true;
        }

        bool result = sDynamicSocketsMgr->TrySocketItem(player, item, gem, freeSocketSlot);

        if (!result)
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Failed socket, unknown error.", slot);
            return true;
        }

        LOG_INFO("module", "Socketing success.", slot);
        player->DestroyItem(gem->GetBagSlot(), gem->GetSlot(), true);

        return true;
    }

    return true;
}
