#ifndef MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H
#define MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H

#include "ScriptMgr.h"

enum DynamicSocketsGameObjectConstants {
    MYSTIC_ANVIL_EXIT = 0,

    MYSTIC_ANVIL_TEXT_ID = 441150,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_TEXT_ID = 441151,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT_NO_ITEMS_TEXT_ID = 441152,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_TEXT_ID = 441153,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEM_NO_GEMS_TEXT_ID = 441154,
    MYSTIC_ANVIL_SOCKET_ADD_VERIFY_TEXT_ID = 441155,
    MYSTIC_ANVIL_SOCKET_ADD_VERIFY_NO_CURRENCY_TEXT_ID = 441156,
    MYSTIC_ANVIL_SOCKET_REMOVE_INFORM_TEXT_ID = 441157,
    MYSTIC_ANVIL_SOCKET_ADD_TEXT_ID = 441158,
    MYSTIC_ANVIL_UNKNOWN_ERROR_TEXT_ID = 441160,

    MYSTIC_ANVIL_SOCKET_ADD = 100,
    MYSTIC_ANVIL_SOCKET_REMOVE_INFORM = 101,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_SLOT = 1000,
    MYSTIC_ANVIL_SOCKET_ADD_SELECT_GEMS = 2000,
    MYSTIC_ANVIL_SOCKET_ADD_VERIFY = 3000
};

class DynamicSocketsGameObjectScript : public GameObjectScript
{
public:
    DynamicSocketsGameObjectScript() : GameObjectScript("DynamicSocketsGameObjectScript") { }

private:
    bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) override;
    bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) override;

    void GossipHandleExit(Player* player);
    void GossipHandleSocketRemove(Player* player, GameObject* go);
    void GossipHandleSocketAddSelectArmor(Player* player, GameObject* go);
    void GossipHandleSocketAddSelectGem(Player* player, GameObject* go, uint32 action);
    void GossipHandleSocketAddVerify(Player* player, GameObject* go, uint32 action);
    void GossipHandleSocketAdd(Player* player, GameObject* go);
};

#endif // MODULE_DYNAMIC_SOCKETS_GAMEOBJECT_SCRIPT_H
