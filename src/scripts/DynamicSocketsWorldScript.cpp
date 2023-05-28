#include "DynamicSocketsWorldScript.h"
#include "DynamicSocketsMgr.h"

void DynamicSocketsWorldScript::OnUpdate(uint32 diff)
{
    sDynamicSocketsMgr->GetScheduler()->Update(diff);
}
