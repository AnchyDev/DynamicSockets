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
        LOG_WARN("module", "No target item found when trying to whack a socket with Eternal Belt Buckle.");
        return false;
    }

    // Can only use on owned items.
    if (targetItem->GetOwnerGUID() != player->GetGUID())
    {
        LOG_WARN("module", "Tried to whack unowned item with Eternal Belt Buckle.");
        return false;
    }

    targetItem->SetEnchantment(SOCK_ENCHANTMENT_SLOT, 0, 0, 0);
    if (targetItem->IsEquipped())
    {
        sDynamicSocketsMgr->HandleApplyEnchantment(player, targetItem, SOCK_ENCHANTMENT_SLOT, false, 0, 0);
    }

    LOG_INFO("module", "Used belt buckle item.");

    player->DestroyItemCount(item->GetTemplate()->ItemId, 1, true);

    ClearGossipMenuFor(player);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Test", 0, 1);
    SendGossipMenuFor(player, 441150, player->GetGUID());

    return false;
}

void DynamicSocketsBeltBucklePlayerScript::OnGossipSelect(Player* player, uint32 /*menu_id*/, uint32 /*sender*/, uint32 action)
{
    if (action == 1)
    {
        player->HandleEmoteCommand(EMOTE_ONESHOT_WORK_CHOPWOOD);
        CloseGossipMenuFor(player);
    }
}
