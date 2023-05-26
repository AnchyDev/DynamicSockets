#include "DynamicSocketsPlayerScript.h"
#include "DynamicSocketsMgr.h"

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
