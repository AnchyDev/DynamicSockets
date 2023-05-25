#ifndef MODULE_DYNAMIC_SOCKETS_H
#define MODULE_DYNAMIC_SOCKETS_H

#include "ScriptMgr.h"

class DynamicSocketsPlayerScript : public PlayerScript
{
public:
    DynamicSocketsPlayerScript() : PlayerScript("DynamicSocketsPlayerScript") { }

private:
    void HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition);
    bool CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool apply_dur, bool ignore_condition) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_H
