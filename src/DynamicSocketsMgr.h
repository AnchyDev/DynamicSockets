#ifndef MODULE_DYNAMIC_SOCKETS_MGR_H
#define MODULE_DYNAMIC_SOCKETS_MGR_H

#include "Player.h"

#include <vector>

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

    uint32 GetMaskFromValues(std::vector<uint32> values);
    std::vector<Item*> GetItemsFromInventory(Player* player, uint32 itemClassMask, uint32 itemSubclassMask, uint32 slotStart, uint32 slotEnd);
    std::vector<Item*> GetGemsFromInventory(Player* player);
    Item* GetItemFromSlot(Player* player, uint32 slot);
    std::string GetIconForCharacterSlot(uint32 slot);
    std::string GetNameFromCharacterSlot(uint32 slot);
    bool IsEquipmentSlotOccupied(Player* player, uint32 slot);
    bool TrySocketItem(Player* player, Item* item, Item* socketItem, EnchantmentSlot socketSlot);
public:
    static DynamicSocketsManager* GetInstance();
};  

#define sDynamicSocketsMgr DynamicSocketsManager::GetInstance()

#endif // MODULE_DYNAMIC_SOCKETS_MGR_H
