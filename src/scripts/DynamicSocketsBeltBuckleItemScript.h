#ifndef MODULE_DYNAMIC_SOCKETS_BELTBUCKLE_PLAYER_SCRIPT_H
#define MODULE_DYNAMIC_SOCKETS_BELTBUCKLE_PLAYER_SCRIPT_H

#include "ScriptMgr.h"

class DynamicSocketsBeltBucklePlayerScript : public PlayerScript
{
public:
    DynamicSocketsBeltBucklePlayerScript() : PlayerScript("DynamicSocketsBeltBucklePlayerScript") { }

private:
    bool CanCastItemUseSpell(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/, uint8 /*cast_count*/, uint32 /*glyphIndex*/) override;
    void OnGossipSelect(Player* /*player*/, uint32 /*menu_id*/, uint32 /*sender*/, uint32 /*action*/) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_BELTBUCKLE_PLAYER_SCRIPT_H
