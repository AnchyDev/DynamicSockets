#include "DynamicSocketsPlayerScript.h"
#include "DynamicSocketsMgr.h"

bool DynamicSocketsPlayerScript::CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot& slot, bool& apply, bool& applyDuration, bool& ignoreCondition)
{
    if (!player || !item)
    {
        return true;
    }

    if (slot != SOCK_ENCHANTMENT_SLOT && slot != SOCK_ENCHANTMENT_SLOT_2 && slot != SOCK_ENCHANTMENT_SLOT_3 && slot != BONUS_ENCHANTMENT_SLOT)
    {
        return true;
    }

    auto enchantId = item->GetEnchantmentId(slot);
    if (!enchantId)
    {
        return true;
    }

    auto isMeta = sDynamicSocketsMgr->IsMetaGemEnchant(enchantId);

    if (!isMeta)
    {
        sDynamicSocketsMgr->HandleApplyEnchantment(player, item, slot, apply, applyDuration, ignoreCondition);
    }

    if (isMeta && !apply)
    {
        auto state = sDynamicSocketsMgr->GetMetaState(player, item, slot);
        if (state && state->State)
        {
            sDynamicSocketsMgr->HandleApplyEnchantment(player, item, slot, false, applyDuration, ignoreCondition);
            sDynamicSocketsMgr->UpdateMetaState(player, item, slot, false);
        }
    }

    // Run on the next update.
    sDynamicSocketsMgr->GetScheduler()->Schedule(0s, [player](TaskContext context)
    {
        if (player && player->IsInWorld())
        {
            sDynamicSocketsMgr->RefreshMetaGems(player);
        }
    });

    return false;
}

void DynamicSocketsPlayerScript::OnLogout(Player* player)
{
    if (!player)
    {
        return;
    }

    sDynamicSocketsMgr->CleanMetaStates(player);
}
