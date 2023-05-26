#include "DynamicSockets.h"
#include "DynamicSocketsMgr.h"

#include "scripts/DynamicSocketsGameObjectScript.h"
#include "scripts/DynamicSocketsPlayerScript.h"

void SC_AddDynamicSocketsScripts()
{
    new DynamicSocketsGameObjectScript();
    new DynamicSocketsPlayerScript();
}
