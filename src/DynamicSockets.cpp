#include "DynamicSockets.h"

#include "Chat.h"
#include "Player.h"

void DynamicSocketsPlayerScript::HandleApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool applyDuration, bool ignoreCondition)
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
                        // Random Property Exist - try found basepoints for spell (basepoints depends from item suffix factor)
                        if (item->GetItemRandomPropertyId())
                        {
                            ItemRandomSuffixEntry const* item_rand = sItemRandomSuffixStore.LookupEntry(std::abs(item->GetItemRandomPropertyId()));
                            if (item_rand)
                            {
                                // Search enchant_amount
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
                        // Cast custom spell vs all equal basepoints got from enchant_amount
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

                LOG_DEBUG("entities.player.items", "Adding {} to stat nb {}", enchant_amount, enchant_spell_id);
                switch (enchant_spell_id)
                {
                case ITEM_MOD_MANA:
                    LOG_DEBUG("entities.player.items", "+ {} MANA", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_MANA, BASE_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_HEALTH:
                    LOG_DEBUG("entities.player.items", "+ {} HEALTH", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_MOD_AGILITY:
                    LOG_DEBUG("entities.player.items", "+ {} AGILITY", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_AGILITY, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_STRENGTH:
                    LOG_DEBUG("entities.player.items", "+ {} STRENGTH", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_STRENGTH, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_INTELLECT:
                    LOG_DEBUG("entities.player.items", "+ {} INTELLECT", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_INTELLECT, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_SPIRIT:
                    LOG_DEBUG("entities.player.items", "+ {} SPIRIT", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_SPIRIT, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_STAMINA:
                    LOG_DEBUG("entities.player.items", "+ {} STAMINA", enchant_amount);
                    player->HandleStatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, float(enchant_amount), apply);
                    player->ApplyStatBuffMod(STAT_STAMINA, (float)enchant_amount, apply);
                    break;
                case ITEM_MOD_DEFENSE_SKILL_RATING:
                    player->ApplyRatingMod(CR_DEFENSE_SKILL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} DEFENCE", enchant_amount);
                    break;
                case  ITEM_MOD_DODGE_RATING:
                    player->ApplyRatingMod(CR_DODGE, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} DODGE", enchant_amount);
                    break;
                case ITEM_MOD_PARRY_RATING:
                    player->ApplyRatingMod(CR_PARRY, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} PARRY", enchant_amount);
                    break;
                case ITEM_MOD_BLOCK_RATING:
                    player->ApplyRatingMod(CR_BLOCK, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} SHIELD_BLOCK", enchant_amount);
                    break;
                case ITEM_MOD_HIT_MELEE_RATING:
                    player->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} MELEE_HIT", enchant_amount);
                    break;
                case ITEM_MOD_HIT_RANGED_RATING:
                    player->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} RANGED_HIT", enchant_amount);
                    break;
                case ITEM_MOD_HIT_SPELL_RATING:
                    player->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} SPELL_HIT", enchant_amount);
                    break;
                case ITEM_MOD_CRIT_MELEE_RATING:
                    player->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} MELEE_CRIT", enchant_amount);
                    break;
                case ITEM_MOD_CRIT_RANGED_RATING:
                    player->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} RANGED_CRIT", enchant_amount);
                    break;
                case ITEM_MOD_CRIT_SPELL_RATING:
                    player->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} SPELL_CRIT", enchant_amount);
                    break;
                    //                        Values from ITEM_STAT_MELEE_HA_RATING to ITEM_MOD_HASTE_RANGED_RATING are never used
                    //                        in Enchantments
                    //                        case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
                    //                            ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
                    //                            ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
                    //                            ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_HASTE_MELEE_RATING:
                    //                            ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                    //                            break;
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
                    LOG_DEBUG("entities.player.items", "+ {} HIT", enchant_amount);
                    break;
                case ITEM_MOD_CRIT_RATING:
                    player->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} CRITICAL", enchant_amount);
                    break;
                    //                        Values ITEM_MOD_HIT_TAKEN_RATING and ITEM_MOD_CRIT_TAKEN_RATING are never used in Enchantment
                    //                        case ITEM_MOD_HIT_TAKEN_RATING:
                    //                            ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
                    //                            ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
                    //                            ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
                    //                            break;
                    //                        case ITEM_MOD_CRIT_TAKEN_RATING:
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                    //                            ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                    //                            break;
                case ITEM_MOD_RESILIENCE_RATING:
                    player->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} RESILIENCE", enchant_amount);
                    break;
                case ITEM_MOD_HASTE_RATING:
                    player->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                    player->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} HASTE", enchant_amount);
                    break;
                case ITEM_MOD_EXPERTISE_RATING:
                    player->ApplyRatingMod(CR_EXPERTISE, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} EXPERTISE", enchant_amount);
                    break;
                case ITEM_MOD_ATTACK_POWER:
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, float(enchant_amount), apply);
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                    LOG_DEBUG("entities.player.items", "+ {} ATTACK_POWER", enchant_amount);
                    break;
                case ITEM_MOD_RANGED_ATTACK_POWER:
                    player->HandleStatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                    LOG_DEBUG("entities.player.items", "+ {} RANGED_ATTACK_POWER", enchant_amount);
                    break;
                    //                        case ITEM_MOD_FERAL_ATTACK_POWER:
                    //                            ApplyFeralAPBonus(enchant_amount, apply);
                    //                            LOG_DEBUG("entities.player.items", "+ {} FERAL_ATTACK_POWER", enchant_amount);
                    //                            break;
                case ITEM_MOD_MANA_REGENERATION:
                    player->ApplyManaRegenBonus(enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} MANA_REGENERATION", enchant_amount);
                    break;
                case ITEM_MOD_ARMOR_PENETRATION_RATING:
                    player->ApplyRatingMod(CR_ARMOR_PENETRATION, enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} ARMOR PENETRATION", enchant_amount);
                    break;
                case ITEM_MOD_SPELL_POWER:
                    player->ApplySpellPowerBonus(enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} SPELL_POWER", enchant_amount);
                    break;
                case ITEM_MOD_HEALTH_REGEN:
                    player->ApplyHealthRegenBonus(enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} HEALTH_REGENERATION", enchant_amount);
                    break;
                case ITEM_MOD_SPELL_PENETRATION:
                    player->ApplySpellPenetrationBonus(enchant_amount, apply);
                    LOG_DEBUG("entities.player.items", "+ {} SPELL_PENETRATION", enchant_amount);
                    break;
                case ITEM_MOD_BLOCK_VALUE:
                    player->HandleBaseModValue(SHIELD_BLOCK_VALUE, FLAT_MOD, float(enchant_amount), apply);
                    LOG_DEBUG("entities.player.items", "+ {} BLOCK_VALUE", enchant_amount);
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
            }                                               /*switch (enchant_display_type)*/
        }                                                   /*for*/
    }

    if (applyDuration)
    {
        if (apply)
        {
            // set duration
            uint32 duration = item->GetEnchantmentDuration(slot);
            if (duration > 0)
                player->AddEnchantmentDuration(item, slot, duration);
        }
        else
        {
            // duration == 0 will remove EnchantDuration
            player->AddEnchantmentDuration(item, slot, 0);
        }
    }
}

bool DynamicSocketsPlayerScript::CanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool apply_dur, bool ignore_condition)
{
    if (slot != SOCK_ENCHANTMENT_SLOT && slot != SOCK_ENCHANTMENT_SLOT_2 && slot != SOCK_ENCHANTMENT_SLOT_3 && slot != BONUS_ENCHANTMENT_SLOT)
    {
        return true;
    }

    HandleApplyEnchantment(player, item, slot, apply, apply_dur, ignore_condition);

    return false;
}

void SC_AddDynamicSocketsScripts()
{
    new DynamicSocketsPlayerScript();
}
