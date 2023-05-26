#ifndef MODULE_DYNAMIC_SOCKETS_MGR_H
#define MODULE_DYNAMIC_SOCKETS_MGR_H

#include "Player.h"

class DynamicSocketsManager
{
private:
    DynamicSocketsManager() { }
public:
    /**
    * @brief Handles the enchantments application to player stats.
    * @param player The player to apply the mods to.
    * @param item The item with the enchants.
    * @param slot The enchantment slot being applied
    * @param apply Defines whether the mods are being added or removed.
    * @param applyDuration The duration of the application.
    * @param ignoreCondition Ignores conditions for application.
    */
    void HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition);

    bool TrySocketItem(Player* player, Item* item, Item* socketItem);
public:
    static DynamicSocketsManager* GetInstance();
};  

#define sDynamicSocketsMgr DynamicSocketsManager::GetInstance()

#endif // MODULE_DYNAMIC_SOCKETS_MGR_H
