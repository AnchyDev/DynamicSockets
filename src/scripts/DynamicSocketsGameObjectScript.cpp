#include "DynamicSocketsGameObjectScript.h"
#include "DynamicSocketsMgr.h"

#include "Player.h"
#include <AI/ScriptedAI/ScriptedGossip.h>

bool DynamicSocketsGameObjectScript::OnGossipHello(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Add Socket", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD);
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
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_HEAD))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Head", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_HEAD);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_NECK))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Neck", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_NECK);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_SHOULDERS))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Shoulders", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_SHOULDERS);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_BACK))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_BACK);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_CHEST))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Chest", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_CHEST);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_WRISTS))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Wrist", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_WRISTS);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_HANDS))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Hands", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_HANDS);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_WAIST))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Waist", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_WAIST);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_LEGS))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Legs", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_LEGS);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_FEET))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Feet", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_FEET);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_FINGER1))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Ring 1", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_FINGER1);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_FINGER2))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Ring 2", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_FINGER2);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_TRINKET1))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Trinket 1", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_TRINKET1);
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, SLOT_TRINKET2))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Trinket 2", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT + SLOT_TRINKET2);

        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_TEXT_ID, go->GetGUID());
    }

    if (action >= MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT && action < MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT)
    {
        ClearGossipMenuFor(player);
        auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT;
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Are you sure you want to socket this", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT + slot);
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
        bool result = sDynamicSocketsMgr->TrySocketItem(player, item, gem);
        if (!result)
        {
            CloseGossipMenuFor(player);
            LOG_INFO("module", "Failed socket", slot);
            return true;
        }

        LOG_INFO("module", "Socketing success.", slot);
        player->RemoveItem(gem->GetBagSlot(), gem->GetSlot(), true);
    }

    return true;
}
