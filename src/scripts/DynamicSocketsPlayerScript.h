#ifndef MODULE_DYNAMIC_SOCKETS_PLAYER_SCRIPT_H
#define MODULE_DYNAMIC_SOCKETS_PLAYER_SCRIPT_H

#include "ScriptMgr.h"

class DynamicSocketsPlayerScript : public PlayerScript
{
public:
    DynamicSocketsPlayerScript() : PlayerScript("DynamicSocketsPlayerScript") { }

private:
    bool CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot& slot, bool& apply, bool& applyDuration, bool& ignoreCondition) override;
    void OnLogout(Player* /*player*/) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_PLAYER_SCRIPT_H
