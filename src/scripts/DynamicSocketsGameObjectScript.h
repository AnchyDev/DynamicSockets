#ifndef MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H
#define MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H

#include "ScriptMgr.h"

enum DynamicSocketsGameObjectConstants {
    MYSTIC_ANVIL_EXIT = 0,

    MYSTIC_ANVIL_TEXT_ID = 441150,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_TEXT_ID = 441151,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_HEAD_TEXT_ID = 441152,

    MYSTIC_ANVIL_SOCKET_ADD = 100,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT = 1000,
    MYSTIC_ANVIL_SOCKET_ADD_SELEPT_SLOT_ACCEPT = 2000,
};

class DynamicSocketsGameObjectScript : public GameObjectScript
{
public:
    DynamicSocketsGameObjectScript() : GameObjectScript("DynamicSocketsGameObjectScript") { }

private:
    bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) override;
    bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H
