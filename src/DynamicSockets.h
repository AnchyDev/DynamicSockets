#ifndef MODULE_DYNAMIC_SOCKETS_H
#define MODULE_DYNAMIC_SOCKETS_H

#include "ScriptMgr.h"

class DynamicSocketsPlayerScript : public PlayerScript
{
public:
    DynamicSocketsPlayerScript() : PlayerScript("DynamicSocketsPlayerScript") { }

private:
    bool CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool apply_dur, bool ignore_condition) override;
    void OnStoreNewItem(Player* /*player*/, Item* /*item*/, uint32 /*count*/) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_H
