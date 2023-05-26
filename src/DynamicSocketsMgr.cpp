#include "DynamicSocketsMgr.h"


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

    if (!ignoreCondition && pEnchant->EnchantmentCondition && !player->EnchantmentFitsRequirements(pEnchant->EnchantmentCondition, -1))
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

    for (uint16 i = slotStart; i < slotEnd; ++i)
    {
        Item* slotItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
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

bool DynamicSocketsManager::TrySocketItem(Player* player, Item* item, Item* socketItem)
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

    if (!item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT))
    {
        item->SetEnchantment(SOCK_ENCHANTMENT_SLOT, enchantId, 0, 0);
        return true;
    }

    if (!item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2))
    {
        item->SetEnchantment(SOCK_ENCHANTMENT_SLOT_2, enchantId, 0, 0);
        return true;
    }

    if (!item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3))
    {
        item->SetEnchantment(SOCK_ENCHANTMENT_SLOT_3, enchantId, 0, 0);
        return true;
    }

    return false;
}

DynamicSocketsManager* DynamicSocketsManager::GetInstance()
{
    static DynamicSocketsManager instance;

    return &instance;
}