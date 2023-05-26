#include "DynamicSockets.h"
#include "DynamicSocketsMgr.h"

#include "scripts/DynamicSocketsBeltBuckleItemScript.h"
#include "scripts/DynamicSocketsGameObjectScript.h"
#include "scripts/DynamicSocketsPlayerScript.h"

void SC_AddDynamicSocketsScripts()
{
    new DynamicSocketsBeltBucklePlayerScript();
    new DynamicSocketsGameObjectScript();
    new DynamicSocketsPlayerScript();
}
