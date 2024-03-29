#ifndef MODULE_DYNAMIC_SOCKETS_MGR_H
#define MODULE_DYNAMIC_SOCKETS_MGR_H

#include "Player.h"
#include "TaskScheduler.h"

#include <vector>
#include <unordered_map>

enum ConditionGemColors
{
    GEM_CONDITION_META = 1,
    GEM_CONDITION_RED = 2,
    GEM_CONDITION_YELLOW = 3,
    GEM_CONDITION_BLUE = 4
};

struct DynamicSocketsMetaState
{
    Item* Item;;
    EnchantmentSlot EnchantSlot;
    bool State;
};

struct DynamicSocketsQueueItem
{
    Item* Gem;
    Item* TargetItem;
    EnchantmentSlot Slot;

    Item* SocketRemoveSender;
    Item* SocketRemoveTarget;
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
            queueItem.TargetItem = nullptr;
            queueItem.Gem = nullptr;
            queueItem.SocketRemoveSender = nullptr;
            queueItem.SocketRemoveTarget = nullptr;
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
    bool IsEnchantRequirementsMet(Player* player, uint32 enchantId);
    void HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition);

    void RefreshMetaGems(Player* player);
    bool IsMetaGemEnchant(uint32 enchantId);
    DynamicSocketsMetaState* GetMetaState(Player* player, Item* item, EnchantmentSlot enchantSlot);
    void UpdateMetaState(Player* player, Item* item, EnchantmentSlot enchantSlot, bool newState);
    void CleanMetaStates(Player* player);

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

    void SendNotification(Player* player, std::string message);

    TaskScheduler* GetScheduler();
private:
    TaskScheduler _scheduler;
    std::unordered_map<Player*, std::vector<DynamicSocketsMetaState>> _metaStates;
public:
    static DynamicSocketsManager* GetInstance();
};  

#define sDynamicSocketsMgr DynamicSocketsManager::GetInstance()

#endif // MODULE_DYNAMIC_SOCKETS_MGR_H
