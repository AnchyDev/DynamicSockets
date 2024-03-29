#include "DynamicSocketsGameObjectScript.h"
#include "DynamicSocketsMgr.h"

#include "Config.h"
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
    if (action == MYSTIC_ANVIL_EXIT)
    {
        GossipHandleExit(player);
    }
    else if (action == MYSTIC_ANVIL_SOCKET_REMOVE_INFORM)
    {
        GossipHandleSocketRemove(player, go);
    }
    else if (action == MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT)
    {
        GossipHandleSocketAddSelectArmor(player, go);
    }
    else if (action >= MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS && action < MYSTIC_ANVIL_SOCKET_ADD_VERIFY)
    {
        GossipHandleSocketAddSelectGem(player, go, action);
    }
    else if (action >= MYSTIC_ANVIL_SOCKET_ADD_VERIFY)
    {
        GossipHandleSocketAddVerify(player, go, action);
    }
    else if (action == MYSTIC_ANVIL_SOCKET_ADD)
    {
        GossipHandleSocketAdd(player, go);
    }

    return true;
}

void DynamicSocketsGameObjectScript::GossipHandleExit(Player* player)
{
    CloseGossipMenuFor(player);
}

void DynamicSocketsGameObjectScript::GossipHandleSocketRemove(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);
    SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_REMOVE_INFORM_TEXT_ID, go->GetGUID());
}

void DynamicSocketsGameObjectScript::GossipHandleSocketAddSelectArmor(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);

    bool hasItems = false;

    for (uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (sDynamicSocketsMgr->IsEquipmentSlotOccupied(player, slot))
        {
            std::string armorIcon = "";
            std::string qualityIcon = "";
            auto item = sDynamicSocketsMgr->GetItemFromSlot(player, slot);
            if (!item)
            {
                continue;
            }

            auto itemProto = item->GetTemplate();

            if (auto displayInfo = sItemDisplayInfoStore.LookupEntry(item->GetTemplate()->DisplayInfoID))
            {
                armorIcon = Acore::StringFormatFmt("|TInterface\\ICONS\\{}:16|t", displayInfo->inventoryIcon);
            }
            qualityIcon = Acore::StringFormatFmt("|TInterface\\MINIMAP\\PartyRaidBlips:16:16:0:0:256:128:128:160:0:32|t");

            std::string availableSocketStr = "";
            auto availableSocket = sDynamicSocketsMgr->GetFreeSocketSlot(player, slot);

            // No more available sockets, skip..
            if (availableSocket == MAX_ENCHANTMENT_SLOT)
            {
                continue;
            }

            switch (availableSocket)
            {
            case SOCK_ENCHANTMENT_SLOT:
                availableSocketStr = "|cff00FFA5First Socket|r";
                break;
            case SOCK_ENCHANTMENT_SLOT_2:
                availableSocketStr = "|cff003BFFSecond Socket|r";
                break;
            case SOCK_ENCHANTMENT_SLOT_3:
                availableSocketStr = "|cffFF002EThird Socket|r";
                break;
            default:
                break;
            }

            std::string option = Acore::StringFormatFmt("{} {}|n{} {}", armorIcon.empty() ? sDynamicSocketsMgr->GetIconForCharacterSlot(slot) : armorIcon, itemProto->Name1, qualityIcon, availableSocketStr);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, option, GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS + slot);

            hasItems = true;
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
}

void DynamicSocketsGameObjectScript::GossipHandleSocketAddSelectGem(Player* player, GameObject* go, uint32 action)
{
    ClearGossipMenuFor(player);

    auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS;

    auto queue = sDynamicSocketsMgr->GetSocketQueue();
    auto queueItem = queue->Get(player);
    queueItem->TargetItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    queueItem->Slot = sDynamicSocketsMgr->GetFreeSocketSlot(player, slot);

    auto gems = sDynamicSocketsMgr->GetGemsFromInventory(player);
    bool hasGems = !gems.empty();

    if (hasGems)
    {
        uint32 gemCount = 0;
        for (auto gem : gems)
        {
            if (!gem)
            {
                continue;
            }

            // Gossip can only have so many menu items before crashing.
            if (gemCount >= GOSSIP_MAX_MENU_ITEMS)
            {
                break;
            }
            else
            {
                gemCount++;
            }

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
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("{}{}|n{}x{}", gemIcon, sDynamicSocketsMgr->GetItemLink(player, gem), costIcon, sDynamicSocketsMgr->GetSocketCost(queueItem->TargetItem, gem, queueItem->Slot)), GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_VERIFY + gem->GetSlot());
        }
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_TEXT_ID, go->GetGUID());
    }
    else
    {
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_NO_GEMS_TEXT_ID, go->GetGUID());
    }
}

void DynamicSocketsGameObjectScript::GossipHandleSocketAddVerify(Player* player, GameObject* go, uint32 action)
{
    ClearGossipMenuFor(player);

    auto slot = action - MYSTIC_ANVIL_SOCKET_ADD_VERIFY;

    auto queue = sDynamicSocketsMgr->GetSocketQueue();
    auto queueItem = queue->Get(player);
    queueItem->Gem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

    auto cost = sDynamicSocketsMgr->GetSocketCost(queueItem->TargetItem, queueItem->Gem, queueItem->Slot);

    uint32 currencyId = sConfigMgr->GetOption<uint32>("DynamicSockets.Cost.CurrencyEntryId", 37711);
    uint32 currencyCount = player->GetItemCount(currencyId);

    auto currencyProto = sObjectMgr->GetItemTemplate(currencyId);

    if (currencyCount < cost)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I understand.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT);
        SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_VERIFY_NO_CURRENCY_TEXT_ID, go->GetGUID());

        return;
    }

    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Yes, I am sure.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD, Acore::StringFormatFmt("|cffFF0000This action costs |cffFFFFFF[{}]x{}|cffFF0000, are you sure you want to do this?", currencyProto->Name1, cost), 0, false);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I'd like to think it over.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_EXIT);
    SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_VERIFY_TEXT_ID, go->GetGUID());
}

void DynamicSocketsGameObjectScript::GossipHandleSocketAdd(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);

    auto queue = sDynamicSocketsMgr->GetSocketQueue();
    auto queueItem = queue->Get(player);

    bool result = sDynamicSocketsMgr->TrySocketItem(player, queueItem->TargetItem, queueItem->Gem, queueItem->Slot);

    if (!result)
    {
        CloseGossipMenuFor(player);
        LOG_INFO("module", "Failed socket, unknown error.", queueItem->Slot);
        return;
    }

    auto cost = sDynamicSocketsMgr->GetSocketCost(queueItem->TargetItem, queueItem->Gem, queueItem->Slot);

    uint32 currencyId = sConfigMgr->GetOption<uint32>("DynamicSockets.Cost.CurrencyEntryId", 37711);
    uint32 currencyCount = player->GetItemCount(currencyId);

    player->DestroyItem(queueItem->Gem->GetBagSlot(), queueItem->Gem->GetSlot(), true);
    player->DestroyItemCount(currencyId, cost, true);

    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I would like to socket another item.", GOSSIP_SENDER_MAIN, MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT);
    SendGossipMenuFor(player, MYSTIC_ANVIL_SOCKET_ADD_TEXT_ID, go->GetGUID());
}
