#ifndef MODULE_DYNAMIC_SOCKETS_WORLD_SCRIPT_H
#define MODULE_DYNAMIC_SOCKETS_WORLD_SCRIPT_H

#include "ScriptMgr.h"

class DynamicSocketsWorldScript : public WorldScript
{
public:
    DynamicSocketsWorldScript() : WorldScript("DynamicSocketsWorldScript") { }

private:
    void OnUpdate(uint32 /*diff*/) override;
};

#endif // MODULE_DYNAMIC_SOCKETS_WORLD_SCRIPT_H
