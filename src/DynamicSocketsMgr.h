#ifndef MODULE_DYNAMIC_SOCKETS_MGR_H
#define MODULE_DYNAMIC_SOCKETS_MGR_H

#include "Player.h"

#include <vector>
#include <unordered_map>

struct DynamicSocketsQueueItem
{
    Item* Gem;
    Item* Item;
    EnchantmentSlot Slot;
};

class DynamicSocketsQueue
{
private:
    DynamicSocketsQueue() { }

    std::unordered_map<Player*, DynamicSocketsQueueItem> _queue;
public:
    DynamicSocketsQueueItem* Get(Player* player)
    {
        auto it = _queue.find(player);
        if (it == _queue.end())
        {
            DynamicSocketsQueueItem queueItem;
            queueItem.Item = nullptr;
            queueItem.Gem = nullptr;
            it = _queue.emplace(player, queueItem).first;
        }

        return &it->second;
    }

    static DynamicSocketsQueue* GetInstance()
    {
        static DynamicSocketsQueue instance;
        return &instance;
    }
};

class DynamicSocketsManager
{
private:
    DynamicSocketsManager() { }
public:
    void HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition);
    uint32 GetMaskFromValues(std::vector<uint32> values);
    std::vector<Item*> GetItemsFromInventory(Player* player, uint32 itemClassMask, uint32 itemSubclassMask, uint32 slotStart, uint32 slotEnd);
    std::vector<Item*> GetGemsFromInventory(Player* player);
    Item* GetItemFromSlot(Player* player, uint32 slot);

    std::string GetItemLink(Player* player, Item* item);

    std::string GetIconForCharacterSlot(uint32 slot);
    std::string GetNameFromCharacterSlot(uint32 slot);
    bool IsEquipmentSlotOccupied(Player* player, uint32 itemSlot);
    bool IsSocketSlotOccupied(Player* player, uint32 itemSlot, EnchantmentSlot socketSlot);
    EnchantmentSlot GetFreeSocketSlot(Player* player, uint32 itemSlot);
    bool TrySocketItem(Player* player, Item* item, Item* socketItem, EnchantmentSlot socketSlot);
    DynamicSocketsQueue* GetSocketQueue();

    uint32 GetSocketCost(Item* item, Item* gem, EnchantmentSlot socket);
public:
    static DynamicSocketsManager* GetInstance();
};  

#define sDynamicSocketsMgr DynamicSocketsManager::GetInstance()

#endif // MODULE_DYNAMIC_SOCKETS_MGR_H
