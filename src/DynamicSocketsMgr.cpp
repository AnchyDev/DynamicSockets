#include "DynamicSocketsMgr.h"

#include "Config.h"

bool DynamicSocketsManager::IsEnchantRequirementsMet(Player* player, uint32 enchantCondition)
{
    if (!enchantCondition)
    {
        return true;
    }

    auto condition = sSpellItemEnchantmentConditionStore.LookupEntry(enchantCondition);
    if (!condition)
    {
        return true;
    }

    uint32 needRed = 0;
    uint32 needYellow = 0;
    uint32 needBlue = 0;

    // There can only be 5 colors/values.
    for (uint8 i = 0; i < 5; ++i)
    {
        auto color = condition->Color[i];
        auto value = condition->Value[i];

        if (color && value)
        {
            switch (color)
            {
            case GEM_CONDITION_RED:
                needRed += value;
                break;

            case GEM_CONDITION_YELLOW:
                needYellow += value;
                break;

            case GEM_CONDITION_BLUE:
                needBlue += value;
                break;
            }
        }
    }

    uint32 gemRed = 0;
    uint32 gemYellow = 0;
    uint32 gemBlue = 0;
    for (uint32 equipSlot = EQUIPMENT_SLOT_START; equipSlot < EQUIPMENT_SLOT_END; ++equipSlot)
    {
        auto item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipSlot);
        if (!item)
        {
            continue;
        }

        for (uint32 socketSlot = 0; socketSlot < 3; ++socketSlot)
        {
            auto enchantSlot = socketSlot == 0 ? SOCK_ENCHANTMENT_SLOT : socketSlot == 1 ? SOCK_ENCHANTMENT_SLOT_2 : SOCK_ENCHANTMENT_SLOT_3;
            auto enchantId = item->GetEnchantmentId(enchantSlot);

            if (!enchantId)
            {
                continue;
            }

            auto enchantStore = sSpellItemEnchantmentStore.LookupEntry(enchantId);
            if (!enchantStore)
            {
                continue;
            }

            auto gemId = enchantStore->GemID;
            if (!gemId)
            {
                continue;
            }

            ItemTemplate const* gemProto = sObjectMgr->GetItemTemplate(gemId);
            if (!gemProto)
            {
                continue;
            }

            GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
            if (!gemProperty)
            {
                continue;
            }

            // Iterate over all colors, skipping meta.
            for (uint32 i = 1; i < 4; ++i)
            {
                uint32 colorBitmask = 1 << i;
                if (gemProperty->color & colorBitmask)
                {
                    switch (i)
                    {
                    case GEM_CONDITION_RED - 1:
                        gemRed++;
                        break;
                    case GEM_CONDITION_YELLOW - 1:
                        gemYellow++;
                        break;
                    case GEM_CONDITION_BLUE - 1:
                        gemBlue++;
                        break;
                    }
                }
            }
        }
    }

    LOG_INFO("module", "Need {} red, {} yellow, {} blue gems.", needRed, needYellow, needBlue);
    LOG_INFO("module", "Found {} red, {} yellow, {} blue gems.", gemRed, gemYellow, gemBlue);

    return (gemRed >= needRed) && (gemYellow >= needYellow) && (gemBlue >= needBlue);
}

void DynamicSocketsManager::HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition)
{
    if (!item || !item->IsEquipped())
        return;

    if (slot >= MAX_ENCHANTMENT_SLOT)
        return;

    uint32 enchant_id = item->GetEnchantmentId(slot);
    if (!enchant_id)
        return;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
        return;

    if (!ignoreCondition && pEnchant->EnchantmentCondition && !IsEnchantRequirementsMet(player, pEnchant->EnchantmentCondition))
        return;

    if (pEnchant->requiredLevel > player->GetLevel())
        return;

    if (pEnchant->requiredSkill > 0 && pEnchant->requiredSkillValue > player->GetSkillValue(pEnchant->requiredSkill))
        return;

    if (!item->IsBroken())
    {
        for (int s = 0; s < MAX_SPELL_ITEM_ENCHANTMENT_EFFECTS; ++s)
        {
            uint32 enchant_display_type = pEnchant->type[s];
            uint32 enchant_amount = pEnchant->amount[s];
            uint32 enchant_spell_id = pEnchant->spellid[s];

            switch (enchant_display_type)
            {
            case ITEM_ENCHANTMENT_TYPE_NONE:
                break;
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
                // processed in Player::CastItemCombatSpell
                break;
            case ITEM_ENCHANTMENT_TYPE_DAMAGE:
                if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                    player->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(enchant_amount), apply);
                else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    player->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(enchant_amount), apply);
                else if (item->GetSlot() == EQUIPMENT_SLOT_RANGED)
                    player->HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                break;
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
                if (enchant_spell_id)
                {
                    if (apply)
                    {
                        int32 basepoints = 0;
                        if (item->GetItemRandomPropertyId())
                        {
                            ItemRandomSuffixEntry const* item_rand = sItemRandomSuffixStore.LookupEntry(std::abs(item->GetItemRandomPropertyId()));
                            if (item_rand)
                            {
                                for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
                                {
                                    if (item_rand->Enchantment[k] == enchant_id)
                                    {
                                        basepoints = int32((item_rand->AllocationPct[k] * item->GetItemSuffixFactor()) / 10000);
                                        break;
                                    }
                                }
                            }
                        }

                        if (basepoints)
                            player->CastCustomSpell(player, enchant_spell_id, &basepoints, &basepoints, &basepoints, true, item);
                        else
                            player->CastSpell(player, enchant_spell_id, true, item);
                    }
                    else
                        player->RemoveAurasDueToItemSpell(enchant_spell_id, item->GetGUID());
                }
                break;
            case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
                if (!enchant_amount)
                {
                    ItemRandomSuffixEntry const* item_rand = sItemRandomSuffixStore.LookupEntry(std::abs(item->GetItemRandomPropertyId()));
                    if (item_rand)
                    {
                        for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
                        {
                            if (item_rand->Enchantment[k] == enchant_id)
                            {
                                enchant_amount = uint32((item_rand->AllocationPct[k] * item->GetItemSuffixFactor()) / 10000);
                                break;
                            }
                        }
                    }
                }

                player->HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + enchant_spell_id), TOTAL_VALUE, float(enchant_amount), apply);
                break;
            case ITEM_ENCHANTMENT_TYPE_STAT:
            {
                if (!enchant_amount)
                {
                    ItemRandomSuffixEntry const* item_rand_suffix = sItemRandomSuffixStore.LookupEntry(std::abs(item->GetItemRandomPropertyId()));
                    if (item_rand_suffix)
                    {
                        for (int k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
                        {
                            if (item_rand_suffix->Enchantment[k] == enchant_id)
                            {
                                enchant_amount = uint32((item_rand_suffix->AllocationPct[k] * item->GetItemSuffixFactor()) / 10000);
                                break;
                            }
                        }
                    }
                }

                switch (enchant_spell_id)
                {
                case ITEM_MOD_MANA:
                    player->HandleStatModifier(UNIT_MOD_MANA, BASE_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_HEALTH:
                    player->HandleStatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_AGILITY:
                    player->HandleStatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_AGILITY, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_STRENGTH:
                    player->HandleStatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_STRENGTH, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_INTELLECT:
                    player->HandleStatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_INTELLECT, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_SPIRIT:
                    player->HandleStatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_SPIRIT, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_STAMINA:
                    player->HandleStatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_STAMINA, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_DEFENSE_SKILL_RATING:
                    player->ApplyRatingMod(CR_DEFENSE_SKILL, enchant_amount, apply);
                    break;
                case  ITEM_MOD_DODGE_RATING:
                    player->ApplyRatingMod(CR_DODGE, enchant_amount, apply);
                    break;
                case ITEM_MOD_PARRY_RATING:
                    player->ApplyRatingMod(CR_PARRY, enchant_amount, apply);
                    break;
                case ITEM_MOD_BLOCK_RATING:
                    player->ApplyRatingMod(CR_BLOCK, enchant_amount, apply);
                    break;
                case ITEM_MOD_HIT_MELEE_RATING:
                    player->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                    break;
                case ITEM_MOD_HIT_RANGED_RATING:
                    player->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                    break;
                case ITEM_MOD_HIT_SPELL_RATING:
                    player->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_CRIT_MELEE_RATING:
                    player->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                    break;
                case ITEM_MOD_CRIT_RANGED_RATING:
                    player->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                    break;
                case ITEM_MOD_CRIT_SPELL_RATING:
                    player->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_HASTE_RANGED_RATING:
                    player->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                    break;
                case ITEM_MOD_HASTE_SPELL_RATING:
                    player->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_HIT_RATING:
                    player->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_CRIT_RATING:
                    player->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_RESILIENCE_RATING:
                    player->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_HASTE_RATING:
                    player->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                    break;
                case ITEM_MOD_EXPERTISE_RATING:
                    player->ApplyRatingMod(CR_EXPERTISE, enchant_amount, apply);
                    break;
                case ITEM_MOD_ATTACK_POWER:
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(enchant_amount), apply);
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_RANGED_ATTACK_POWER:
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_MANA_REGENERATION:
                    player->ApplyManaRegenBonus(enchant_amount, apply);
                    break;
                case ITEM_MOD_ARMOR_PENETRATION_RATING:
                    player->ApplyRatingMod(CR_ARMOR_PENETRATION, enchant_amount, apply);
                    break;
                case ITEM_MOD_SPELL_POWER:
                    player->ApplySpellPowerBonus(enchant_amount, apply);
                    break;
                case ITEM_MOD_HEALTH_REGEN:
                    player->ApplyHealthRegenBonus(enchant_amount, apply);
                    break;
                case ITEM_MOD_SPELL_PENETRATION:
                    player->ApplySpellPenetrationBonus(enchant_amount, apply);
                    break;
                case ITEM_MOD_BLOCK_VALUE:
                    player->HandleBaseModValue(SHIELD_BLOCK_VALUE, FLAT_MOD, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_SPELL_HEALING_DONE:   // deprecated
                case ITEM_MOD_SPELL_DAMAGE_DONE:    // deprecated
                default:
                    break;
                }
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_TOTEM:           // Shaman Rockbiter Weapon
            {
                if (player->getClass() == CLASS_SHAMAN)
                {
                    float addValue = 0.0f;
                    if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                    {
                        addValue = float(enchant_amount * item->GetTemplate()->Delay / 1000.0f);
                        player->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, addValue, apply);
                    }
                    else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    {
                        addValue = float(enchant_amount * item->GetTemplate()->Delay / 1000.0f);
                        player->HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, addValue, apply);
                    }
                }
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_USE_SPELL:
                // processed in Player::CastItemUseSpell
                break;
            case ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET:
                // nothing do..
                break;
            default:
                LOG_ERROR("entities.player", "Unknown item enchantment (id = {}) display type: {}", enchant_id, enchant_display_type);
                break;
            }
        }
    }

    if (applyDuration)
    {
        if (apply)
        {
            uint32 duration = item->GetEnchantmentDuration(slot);
            if (duration > 0)
                player->AddEnchantmentDuration(item, slot, duration);
        }
        else
        {
            player->AddEnchantmentDuration(item, slot, 0);
        }
    }
}

uint32 DynamicSocketsManager::GetMaskFromValues(std::vector<uint32> values)
{
    uint32 mask = 0;

    for (auto value : values)
    {
        mask += (1 << value);
    }

    return mask;
}

std::vector<Item*> DynamicSocketsManager::GetItemsFromInventory(Player* player, uint32 itemClassMask, uint32 itemSubclassMask, uint32 slotStart, uint32 slotEnd)
{
    std::vector<Item*> items;

    for (uint16 slotBag = INVENTORY_SLOT_BAG_START; slotBag < INVENTORY_SLOT_BAG_END + 1; ++slotBag)
    {
        for (uint16 i = slotStart; i < slotEnd; ++i)
        {
            Item* slotItem = player->GetItemByPos(slotBag == INVENTORY_SLOT_BAG_END ? INVENTORY_SLOT_BAG_0 : slotBag, i);
            if (!slotItem)
            {
                continue;
            }

            auto slotItemProto = slotItem->GetTemplate();

            uint32 classMask = 1 << slotItemProto->Class;
            uint32 subclassMask = 1 << slotItemProto->SubClass;

            if ((itemClassMask == 0 || itemClassMask & classMask) &&
                (itemSubclassMask == 0 || itemSubclassMask & subclassMask))
            {
                items.push_back(slotItem);
            }
        }
    }

    return items;
}

std::vector<Item*> DynamicSocketsManager::GetGemsFromInventory(Player* player)
{
    uint32 classMask = GetMaskFromValues(std::vector<uint32> { ITEM_CLASS_GEM });
    uint32 subClassMask = GetMaskFromValues(std::vector<uint32>
    {
        ITEM_SUBCLASS_GEM_RED, ITEM_SUBCLASS_GEM_BLUE,
        ITEM_SUBCLASS_GEM_YELLOW, ITEM_SUBCLASS_GEM_PURPLE,
        ITEM_SUBCLASS_GEM_GREEN, ITEM_SUBCLASS_GEM_ORANGE,
        ITEM_SUBCLASS_GEM_META, ITEM_SUBCLASS_GEM_PRISMATIC
    });

    return GetItemsFromInventory(player, classMask, subClassMask, PLAYER_SLOT_START, PLAYER_SLOT_END);
}

Item* DynamicSocketsManager::GetItemFromSlot(Player* player, uint32 slot)
{
    return player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
}

std::string DynamicSocketsManager::GetItemLink(Player* player, Item* item)
{
    auto itemProto = item->GetTemplate();

    // Retrieve the locale name.
    int localeIndex = player->GetSession()->GetSessionDbLocaleIndex();
    if (localeIndex >= 0)
    {
        uint8 ulocaleIndex = uint8(localeIndex);
        if (ItemLocale const* il = sObjectMgr->GetItemLocale(itemProto->ItemId))
        {
            // Set locale name, or default name if locale = 0.
            std::string name = (il->Name.size() > ulocaleIndex && !il->Name[ulocaleIndex].empty()) ? il->Name[ulocaleIndex] : itemProto->Name1;

            if (!name.empty())
            {
                std::ostringstream oss;

                // Build item link.
                oss << "|c" << std::hex << ItemQualityColors[itemProto->Quality] << std::dec << "|Hitem:" << itemProto->ItemId;

                oss << ":";
                auto enchantId = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT);
                if (enchantId)
                {
                    oss << enchantId;
                }

                oss << ":::::";
                auto randProp = item->GetItemRandomPropertyId();
                if (randProp != 0)
                {
                    oss << randProp;
                }
                oss << "::";
                oss << player->GetLevel();
                oss << "|h[";
                oss << name;
                oss << "]|h|r";

                return oss.str();
            }
        }
    }

    return "";
}

std::string DynamicSocketsManager::GetIconForCharacterSlot(uint32 slot)
{
    switch (slot)
    {
    case CharacterSlot::SLOT_HEAD:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Head:32|t";
    case CharacterSlot::SLOT_NECK:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Neck:32|t";;
    case CharacterSlot::SLOT_SHOULDERS:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Shoulder:32|t";
    case CharacterSlot::SLOT_BACK:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Chest:32|t";
    case CharacterSlot::SLOT_CHEST:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Chest:32|t";
    case CharacterSlot::SLOT_SHIRT:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Shirt:32|t";
    case CharacterSlot::SLOT_TABARD:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Tabard:32|t";
    case CharacterSlot::SLOT_WRISTS:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Wrists:32|t";
    case CharacterSlot::SLOT_HANDS:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Hands:32|t";
    case CharacterSlot::SLOT_WAIST:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Waist:32|t";
    case CharacterSlot::SLOT_LEGS:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Legs:32|t";
    case CharacterSlot::SLOT_FEET:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Feet:32|t";
    case CharacterSlot::SLOT_FINGER1:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Finger:32|t";
    case CharacterSlot::SLOT_FINGER2:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Finger:32|t";
    case CharacterSlot::SLOT_TRINKET1:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Trinket:32|t";
    case CharacterSlot::SLOT_TRINKET2:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Trinket:32|t";
    case CharacterSlot::SLOT_MAIN_HAND:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-MainHand:32|t";
    case CharacterSlot::SLOT_OFF_HAND:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-SecondaryHand:32|t";
    case CharacterSlot::SLOT_RANGED:
        return "|TInterface\\PaperDoll\\UI-PaperDoll-Slot-Ranged:32|t";
    }

    return "";
}

std::string DynamicSocketsManager::GetNameFromCharacterSlot(uint32 slot)
{
    switch (slot)
    {
    case CharacterSlot::SLOT_HEAD:
        return "Head";
    case CharacterSlot::SLOT_NECK:
        return "Neck";
    case CharacterSlot::SLOT_SHOULDERS:
        return "Shoulders";
    case CharacterSlot::SLOT_BACK:
        return "Back";
    case CharacterSlot::SLOT_CHEST:
        return "Chest";
    case CharacterSlot::SLOT_SHIRT:
        return "Shirt";
    case CharacterSlot::SLOT_TABARD:
        return "Tabard";
    case CharacterSlot::SLOT_WRISTS:
        return "Wrists";
    case CharacterSlot::SLOT_HANDS:
        return "Hands";
    case CharacterSlot::SLOT_WAIST:
        return "Waist";
    case CharacterSlot::SLOT_LEGS:
        return "Legs";
    case CharacterSlot::SLOT_FEET:
        return "Feet";
    case CharacterSlot::SLOT_FINGER1:
        return "Ring 1";
    case CharacterSlot::SLOT_FINGER2:
        return "Ring 2";
    case CharacterSlot::SLOT_TRINKET1:
        return "Trinket 1";
    case CharacterSlot::SLOT_TRINKET2:
        return "Trinket 2";
    case CharacterSlot::SLOT_MAIN_HAND:
        return "Main-Hand";
    case CharacterSlot::SLOT_OFF_HAND:
        return "Off-Hand";
    case CharacterSlot::SLOT_RANGED:
        return "Ranged/Relic/Totem";
    }

    return Acore::StringFormatFmt("Error: Unnamed slot {}", slot);
}

bool DynamicSocketsManager::IsEquipmentSlotOccupied(Player* player, uint32 itemSlot)
{
    return GetItemFromSlot(player, itemSlot);
}

bool DynamicSocketsManager::IsSocketSlotOccupied(Player* player, uint32 itemSlot, EnchantmentSlot socketSlot)
{
    auto item = GetItemFromSlot(player, itemSlot);
    if (!item)
    {
        return false;
    }

    return item->GetEnchantmentId(socketSlot);
}

EnchantmentSlot DynamicSocketsManager::GetFreeSocketSlot(Player* player, uint32 itemSlot)
{
    auto item = GetItemFromSlot(player, itemSlot);
    if (!item)
    {
        return MAX_ENCHANTMENT_SLOT;
    }

    if (!IsSocketSlotOccupied(player, itemSlot, SOCK_ENCHANTMENT_SLOT))
    {
        return SOCK_ENCHANTMENT_SLOT;
    }

    if (!IsSocketSlotOccupied(player, itemSlot, SOCK_ENCHANTMENT_SLOT_2))
    {
        return SOCK_ENCHANTMENT_SLOT_2;
    }

    if (!IsSocketSlotOccupied(player, itemSlot, SOCK_ENCHANTMENT_SLOT_3))
    {
        return SOCK_ENCHANTMENT_SLOT_3;
    }

    return MAX_ENCHANTMENT_SLOT;
}

bool DynamicSocketsManager::TrySocketItem(Player* player, Item* item, Item* socketItem, EnchantmentSlot socketSlot)
{
    LOG_INFO("module", "Trying to socket item.");

    auto socketItemProto = socketItem->GetTemplate();
    if (!socketItemProto->GemProperties)
    {
        LOG_WARN("module", "Tried to socket gem {} which is missing GemProperties from ItemTemplate.", socketItemProto->ItemId);
        return false;
    }

    auto gemProperties = sGemPropertiesStore.LookupEntry(socketItemProto->GemProperties);
    if (!gemProperties)
    {
        LOG_WARN("module", "Could not find gem properties for gem {}, id {}.", socketItemProto->ItemId, socketItemProto->GemProperties);
        return false;
    }

    auto enchantId = gemProperties->spellitemenchantement;
    if (!enchantId)
    {
        LOG_WARN("module", "GemProperties contained an invalid enchant id for item {}.", socketItemProto->ItemId);
        return false;
    }

    if (!item->GetEnchantmentId(socketSlot))
    {
        item->SetEnchantment(socketSlot, enchantId, 0, 0);

        if (item->IsEquipped())
        {
            sDynamicSocketsMgr->HandleApplyEnchantment(player, item, socketSlot, true, 0, 0);
        }

        return true;
    }

    return false;
}

DynamicSocketsQueue* DynamicSocketsManager::GetSocketQueue()
{
    return DynamicSocketsQueue::GetInstance();
}

uint32 DynamicSocketsManager::GetSocketCost(Item* item, Item* gem, EnchantmentSlot socket)
{
    if (!item || !gem)
    {
        return 0;
    }

    float slotMultiplier = 0;
    switch (socket)
    {
        case EnchantmentSlot::SOCK_ENCHANTMENT_SLOT:
            slotMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Slot.1", 1);
            break;
        case EnchantmentSlot::SOCK_ENCHANTMENT_SLOT_2:
            slotMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Slot.2", 2);
            break;
        case EnchantmentSlot::SOCK_ENCHANTMENT_SLOT_3:
            slotMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Slot.3", 3);
            break;
        default:
            return 0;
    }

    float qualityMultiplier = 0;
    switch (gem->GetTemplate()->Quality)
    {
    case ITEM_QUALITY_NORMAL:
        qualityMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Gem.Quality.Common", 1);
        break;
    case ITEM_QUALITY_UNCOMMON:
        qualityMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Gem.Quality.Uncommon", 1.5);
        break;
    case ITEM_QUALITY_RARE:
        qualityMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Gem.Quality.Rare", 2);
        break;
    case ITEM_QUALITY_EPIC:
        qualityMultiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier.Gem.Quality.Epic", 2.5);
        break;
    default:
        return 0;
    }

    float itemLevel = item->GetTemplate()->ItemLevel;
    float baseCost = sConfigMgr->GetOption<float>("DynamicSockets.Cost.BaseCost", 5);
    float multiplier = sConfigMgr->GetOption<float>("DynamicSockets.Cost.Multiplier", 1);

    return ((itemLevel * slotMultiplier * qualityMultiplier) * baseCost) * multiplier;
}

void DynamicSocketsManager::SendNotification(Player* player, std::string message)
{
    WorldPacket data(SMSG_NOTIFICATION, (message.size() + 1));
    data << message;

    player->SendDirectMessage(&data);
}

DynamicSocketsManager* DynamicSocketsManager::GetInstance()
{
    static DynamicSocketsManager instance;

    return &instance;
}
