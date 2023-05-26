#include "DynamicSocketsGameObjectScript.h"
#include "DynamicSocketsMgr.h"

#include "Player.h"
#include <AI/ScriptedAI/ScriptedGossip.h>

bool DynamicSocketsGameObjectScript::OnGossipHello(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\ITEMSOCKETINGFRAME\\UI-EMPTYSOCKET:16|t Add Socket", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface\\ITEMSOCKETINGFRAME\\UI-EmptySocket-Red:16|t Remove Socket", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_REMOVE_INFORM);
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

    if (action == MYSTIC_ANVIL_SOCKET_REMOVE_INFORM)
    {
        ClearGossipMenuFor(player);
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_REMOVE_INFORM_TEXT_ID, go->GetGUID());
        return true;
    }

    if (action == MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT)
    {
        ClearGossipMenuFor(player);

        bool hasItems = false;

        for (uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, slot))
            {
                hasItems = true;

                std::string option = Acore::StringFormatFmt("{} {}", sDynamicSocketsMgr->GetIconForCharacterSlot(slot), sDynamicSocketsMgr->GetNameFromCharacterSlot(slot));
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, option, GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS + slot);
            }
        }

        if (hasItems)
        {
            SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_TEXT_ID, go->GetGUID());
        }
        else
        {
            SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_NO_ITEMS_TEXT_ID, go->GetGUID());
        }

        return true;
    }

    if (action >= MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS && action < MYSTIC_ANVIL_SOCKET_ADD_VERIFY)
    {
        ClearGossipMenuFor(player);

        auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS;

        auto queue = sDynamicSocketsMgr->GetSocketQueue();
        auto queueItem = queue->Get(player);
        queueItem->Item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        auto gems = sDynamicSocketsMgr->GetGemsFromInventory(player);
        auto slicedGems = std::vector<Item*>(gems.begin(), gems.begin() + GOSSIP_MAX_MENU_ITEMS);
        bool hasGems = !slicedGems.empty();

        if (hasGems)
        {
            for (auto gem : slicedGems)
            {
                std::string gemIcon = "";
                if (auto displayInfo = sItemDisplayInfoStore.LookupEntry(gem->GetTemplate()->DisplayInfoID))
                {
                    gemIcon = Acore::StringFormatFmt("|TInterface\\ICONS\\{}:16|t ", displayInfo->inventoryIcon);
                }
                std::string costIcon = "";
                if (auto displayInfo = sItemDisplayInfoStore.LookupEntry(sObjectMgr->GetItemTemplate(37711)->DisplayInfoID))
                {
                    costIcon = Acore::StringFormatFmt("|TInterface\\ICONS\\{}:16|t ", displayInfo->inventoryIcon);
                }
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("{}{}|n{}x{}", gemIcon, sDynamicSocketsMgr->GetItemLink(player, gem), costIcon, sDynamicSocketsMgr->GetSocketCost(queueItem->Item, gem, SOCK_ENCHANTMENT_SLOT)), GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_VERIFY + gem->GetSlot());
            }
            SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_TEXT_ID, go->GetGUID());
        }
        else
        {
            SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_NO_GEMS_TEXT_ID, go->GetGUID());
        }

        return true;
    }

    if (action >= MYSTIC_ANVIL_SOCKET_ADD_VERIFY)
    {
        ClearGossipMenuFor(player);

        auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_VERIFY;

        auto queue = sDynamicSocketsMgr->GetSocketQueue();
        auto queueItem = queue->Get(player);
        queueItem->Gem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        auto cost = sDynamicSocketsMgr->GetSocketCost(queueItem->Item, queueItem->Gem, SOCK_ENCHANTMENT_SLOT);

        uint32 currencyId = 37711;
        uint32 currencyCount = player->GetItemCount(currencyId);

        if (currencyCount < cost)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I understand.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT);
            SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_VERIFY_NO_CURRENCY_TEXT_ID, go->GetGUID());

            return true;
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Yes, I am sure.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT, Acore::StringFormatFmt("|cffFF0000This action costs |cffFFFFFF[Big Noodles]x{}|cffFF0000, are you sure you want to do this?|n|n|r{} -> {}", cost, sDynamicSocketsMgr->GetItemLink(player, queueItem->Gem), sDynamicSocketsMgr->GetItemLink(player, queueItem->Item)), 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I'd like to think it over.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT);
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_VERIFY_TEXT_ID, go->GetGUID());

        return true;
        /*

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

        return true;*/
    }

    return true;
}
