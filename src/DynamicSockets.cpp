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

    auto gems = sDynamicSocketsMgr->GetGemsFromInventory(player);

    if (gems.empty())
    {
        LOG_INFO("module", "No gem to socket");
        return;
    }

    LOG_INFO("module", "Found gem to socket");

    // Just get the first gem for testing.
    auto gem = gems.at(0);
    bool result = sDynamicSocketsMgr->TrySocketItem(player, item, gem);
    if (result)
    {
        player->RemoveItem(gem->GetBagSlot(), gem->GetSlot(), true);
        ChatHandler(player->GetSession()).SendSysMessage(Acore::StringFormatFmt("Socketed item {}.", item->GetTemplate()->Name1));

        uint32 classMask = sDynamicSocketsMgr->GetMaskFromValues(std::vector<uint32> {});
        uint32 subClassMask = sDynamicSocketsMgr->GetMaskFromValues(std::vector<uint32> {});

        auto items = sDynamicSocketsMgr->GetItemsFromInventory(player, classMask, subClassMask, EQUIPMENT_SLOT_START, EQUIPMENT_SLOT_END);
        if (!items.empty())
        {
            for (auto it : items)
            {
                ChatHandler(player->GetSession()).SendSysMessage(Acore::StringFormatFmt("Found item {}.", it->GetTemplate()->Name1));
            }
        }
    }
}

void SC_AddDynamicSocketsScripts()
{
    new DynamicSocketsPlayerScript();
}
