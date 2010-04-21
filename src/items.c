/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/**
 * This file contains all the functions managing the items in the game.
 */

#define _items_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

enum {
	WEAPON_SLOT = 0,
	SHIELD_SLOT,
	SPECIAL_SLOT,
	ARMOUR_SLOT,
	DRIVE_SLOT,
	FIRST_INV_SLOT
};
item create_item_with_name(const char *item_name, int full_duration, int multiplicity)
{
	item new_item;
	
	new_item.type = GetItemIndexByName(item_name);
	new_item.prefix_code = -1;
	new_item.suffix_code = -1;
	FillInItemProperties(&new_item, full_duration, multiplicity);
	
	return new_item;
}

void equip_item(item *new_item)
{
	item *old_item;
	itemspec *new_itemspec;
	
	new_itemspec = &ItemMap[new_item->type];
	
	/*
	 * now that we know the item type the case
	 * of a 2 handed weapon needs to be handled
	 */
	if (new_itemspec->item_gun_requires_both_hands)
	{
		/*
		 * We're equipping a 2 handed weapon so both the
		 * shield and weapon need to be unequipped.  Here
		 * we handle moving the shield to inventory/floor.
		 * the normal case below will handle the weapon
		 * since it looks for the equipped item of the same
		 * type.  
		 */
		item *shield_item = &Me.shield_item;
		
		// if the shield item exists and we fail to put it in the inventory
		if (shield_item && copy_item_into_inventory(shield_item, shield_item->multiplicity)) {
			DropItemToTheFloor(shield_item, Me.pos.x, Me.pos.y, Me.pos.z);
		} else {
			/* otherwise it was successfully copied into the inventory.
			 * copy_item_into_inventory doesn't delete the source item
			 * though so we have to do that here. 
			 */
			DeleteItem(shield_item);
		}
	}
	
	// get the currently equipped item of the same type, if any
	old_item = get_equipped_item_in_slot_for(new_item->type);

	
	if  (old_item && copy_item_into_inventory(old_item, old_item->multiplicity)) {
		// the item didn't fit in the inventory
		DropItemToTheFloor(old_item, Me.pos.x, Me.pos.y, Me.pos.z);
	}
	CopyItem(new_item, old_item, FALSE);
}


/**
 * Gets a pointer to the currently equipped item of the
 * specified type
 */
item *get_equipped_item_in_slot_for(int item_type)
{
	item *equipped_item;
	itemspec *spec;

	spec = &ItemMap[item_type];
	
	if (spec->item_can_be_installed_in_weapon_slot) {
		equipped_item = &Me.weapon_item;
	}
	else if (spec->item_can_be_installed_in_drive_slot) {
		equipped_item = &Me.drive_item;
	}
	else if (spec->item_can_be_installed_in_armour_slot) {
		equipped_item = &Me.armour_item;
	}
	else if (spec->item_can_be_installed_in_shield_slot) {
		equipped_item = &Me.shield_item;
	}
	else if (spec->item_can_be_installed_in_special_slot) {
		equipped_item = &Me.special_item;
	}
	else
	{
		equipped_item = NULL;
	}
	
	return equipped_item;
}

/**
 * When the player closes the inventory screen, items currently held in 
 * hand should not be held in hand any more.  This is what this function
 * should do:  It should make all items unheld by the player.
 */
void silently_unhold_all_items(void)
{
	int i;
	Level ItemLevel = curShip.AllLevels[Me.pos.z];

	Item_Held_In_Hand = NULL;

	//--------------------
	// Now we remove all 'currently held' markers from all
	// items in inventory and in slots and also from all items on the floor.
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		ItemLevel->ItemList[i].currently_held_in_hand = FALSE;
	}

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		Me.Inventory[i].currently_held_in_hand = FALSE;
	}

	Me.weapon_item.currently_held_in_hand = FALSE;
	Me.drive_item.currently_held_in_hand = FALSE;
	Me.armour_item.currently_held_in_hand = FALSE;
	Me.shield_item.currently_held_in_hand = FALSE;
	Me.special_item.currently_held_in_hand = FALSE;

};				// void silently_unhold_all_items ( void )

/**
 * This function does the home made item repair, i.e. the item repair via
 * the repair skill in contrast to the item repair via the shop, which of
 * course works much better.
 */
void HomeMadeItemRepair(Item RepairItem)
{
	//--------------------
	// At this point we know, that we have just selected an item
	// for home-made repair.
	//
	if (RepairItem->max_duration == (-1)) {
		PlayOnceNeededSoundSample("effects/tux_ingame_comments/Tux_Item_Cant_Be_0.ogg", FALSE, FALSE);
	} else {
		RepairItem->current_duration =
		    RepairItem->current_duration +
		    ceil((RepairItem->max_duration - RepairItem->current_duration) * 0.08 * (1.0 + MyRandom(9)));
		RepairItem->max_duration = RepairItem->current_duration;
		// Play_Shop_ItemRepairedSound( );
		PlayOnceNeededSoundSample("effects/tux_ingame_comments/Tux_This_Quick_Fix_0.ogg", FALSE, FALSE);
	}
};				// void HomeMadeItemRepair ( Item RepairItem ) 

/**
 * This function calculates the price of a given item, taking into account
 * (*) the items base list price 
 * (*) the items given prefix modifier
 * (*) the items given suffix modifier
 * (*) AND THE CURRENT DURATION of the item in relation to its max duration.
 */
unsigned long calculate_item_buy_price(item * BuyItem)
{
	float PrefixMultiplier = 1;
	float SuffixMultiplier = 1;
	float Multiplicity = BuyItem->multiplicity;

	//--------------------
	// Maybe the item is magical in one way or the other.  Then we have to
	// multiply a factor to the price, no matter whether repairing or buying
	// or selling the item.
	//
	if (BuyItem->prefix_code != (-1)) {
		PrefixMultiplier = PrefixList[BuyItem->prefix_code].price_factor;
	}
	if (BuyItem->suffix_code != (-1))
		SuffixMultiplier = SuffixList[BuyItem->suffix_code].price_factor;

	return (Multiplicity * ItemMap[BuyItem->type].base_list_price * SuffixMultiplier * PrefixMultiplier);

};				// long calculate_item_buy_price ( item* BuyItem )

/**
 * This function calculates the price of a given item, taking into account
 * (*) the items base list price 
 * (*) the items given prefix modifier
 * (*) the items given suffix modifier
 * (*) AND THE CURRENT DURATION of the item in relation to its max duration.
 */
unsigned long calculate_item_sell_price(item * BuyItem)
{
	float PrefixMultiplier = 1;
	float SuffixMultiplier = 1;
	float Multiplicity = BuyItem->multiplicity;

	//--------------------
	// Maybe the item is magical in one way or the other.  Then we have to
	// multiply a factor to the price, no matter whether repairing or buying
	// or selling the item.
	//
	if (BuyItem->prefix_code != (-1)) {
		PrefixMultiplier = PrefixList[BuyItem->prefix_code].price_factor;
	}
	if (BuyItem->suffix_code != (-1))
		SuffixMultiplier = SuffixList[BuyItem->suffix_code].price_factor;

	//--------------------
	// When selling an item, you don't get the full value of the item, but
	// instead, only half of the original list price, cause it's a used good.
	//

	// Temporary disabling lowering value for damaged item to test if its a needed/wanted feature.
	// Also to evaluate if the repair skill is ever used for items you use, or just for items you sell.
	// Compensation to sell price
#define SELL_PRICE_FACTOR (0.3)
//#define SELL_PRICE_FACTOR (0.5)
	PrefixMultiplier *= SELL_PRICE_FACTOR;

	return (Multiplicity * ItemMap[BuyItem->type].base_list_price * SuffixMultiplier * PrefixMultiplier);

};				// long calculate_item_sell_price ( item* BuyItem )

/**
 * This function calculates the price of a given item, taking into account
 * (*) the items base list price 
 * (*) the items given prefix modifier
 * (*) the items given suffix modifier
 * (*) AND THE CURRENT DURATION of the item in relation to its max duration.
 */
unsigned long calculate_item_repair_price(item * repair_item)
{
	float PrefixMultiplier = 1;
	float SuffixMultiplier = 1;
	float Multiplicity = repair_item->multiplicity;

	//--------------------
	// Maybe the item is magical in one way or the other.  Then we have to
	// multiply a factor to the price, no matter whether repairing or buying
	// or selling the item.
	//
	if (repair_item->prefix_code != (-1)) {
		PrefixMultiplier = PrefixList[repair_item->prefix_code].price_factor;
	}
	if (repair_item->suffix_code != (-1))
		SuffixMultiplier = SuffixList[repair_item->suffix_code].price_factor;

	//--------------------
	// For repair, it's not the full 'buy' cost...
	//
#define REPAIR_PRICE_FACTOR (0.5)
	PrefixMultiplier *= REPAIR_PRICE_FACTOR;

	//--------------------
	// This is the price of the DAMAGE in the item, haha
	// This can only be requested for repair items
	//
	if (repair_item->max_duration != (-1)) {
		unsigned long price = (Multiplicity * ItemMap[repair_item->type].base_list_price * SuffixMultiplier *
		    PrefixMultiplier * (repair_item->max_duration - repair_item->current_duration) / repair_item->max_duration);

		// Never repair for free, minimum price is 1
		return price ? price : 1;
	} else {
		DebugPrintf(0, "\n\nERROR!! CALCULATING REPAIR PRICE FOR INDESTRUCTIBLE ITEM!! \n\n Terminating...");
		Terminate(ERR);
	}
	return 0;
};				// long calculate_item_repair_price ( item* repair_item )

/**
 *
 *
 */
void FillInItemProperties(item * ThisItem, int FullDuration, int multiplicity)
{
	if (ThisItem->type < 0)
		return;

	ThisItem->damage = ItemMap[ThisItem->type].base_item_gun_damage;
	ThisItem->damage_modifier = ItemMap[ThisItem->type].item_gun_damage_modifier;
	ThisItem->damred_bonus = ItemMap[ThisItem->type].base_damred_bonus + MyRandom(ItemMap[ThisItem->type].damred_bonus_modifier);
	ThisItem->multiplicity = multiplicity;
	ThisItem->ammo_clip = 0;
	if (ItemMap[ThisItem->type].item_gun_ammo_clip_size)
		ThisItem->ammo_clip = MyRandom(ItemMap[ThisItem->type].item_gun_ammo_clip_size);
	//--------------------
	// We now have to set a duration : a maximum duration
	// and a current duration. The latter is
	// a fraction of the maximum duration.
	//
	if (ItemMap[ThisItem->type].base_item_duration != (-1)) {
		ThisItem->max_duration = ItemMap[ThisItem->type].base_item_duration +
		    MyRandom(ItemMap[ThisItem->type].item_duration_modifier);
		if (FullDuration)
			ThisItem->current_duration = ThisItem->max_duration;
		else
			ThisItem->current_duration = ThisItem->max_duration / 4 + MyRandom(ThisItem->max_duration / 2);
	} else {
		ThisItem->max_duration = (-1);
		ThisItem->current_duration = 1;
	}

	//--------------------
	// Now it's time to fill in the magic properties of the item.  We have
	// the basic ranges for the modifiers given in the prefix and suffix lists
	// and just need to create random values in the given ranges for the item.
	//
	ThisItem->bonus_to_str = 0;
	ThisItem->bonus_to_dex = 0;
	ThisItem->bonus_to_mag = 0;
	ThisItem->bonus_to_vit = 0;
	ThisItem->bonus_to_all_attributes = 0;

	ThisItem->bonus_to_life = 0;
	ThisItem->bonus_to_health_recovery = 0;
	ThisItem->bonus_to_cooling_rate = 0;
	ThisItem->bonus_to_force = 0;
	ThisItem->bonus_to_tohit = 0;
	ThisItem->bonus_to_damred_or_damage = 0;

	ThisItem->bonus_to_resist_fire = 0;
	ThisItem->bonus_to_resist_electricity = 0;

	ThisItem->throw_time = 0;

	ThisItem->is_identified = TRUE;

	if (ThisItem->suffix_code != (-1)) {
		ThisItem->bonus_to_str += SuffixList[ThisItem->suffix_code].base_bonus_to_str +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_str);
		ThisItem->bonus_to_dex += SuffixList[ThisItem->suffix_code].base_bonus_to_dex +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_dex);
		ThisItem->bonus_to_mag += SuffixList[ThisItem->suffix_code].base_bonus_to_mag +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_mag);
		ThisItem->bonus_to_vit += SuffixList[ThisItem->suffix_code].base_bonus_to_vit +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_vit);
		ThisItem->bonus_to_all_attributes += SuffixList[ThisItem->suffix_code].base_bonus_to_all_attributes +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_all_attributes);

		ThisItem->bonus_to_life += SuffixList[ThisItem->suffix_code].base_bonus_to_life +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_life);
		ThisItem->bonus_to_health_recovery += SuffixList[ThisItem->suffix_code].base_bonus_to_health_recovery;
		ThisItem->bonus_to_force += SuffixList[ThisItem->suffix_code].base_bonus_to_force +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_force);
		ThisItem->bonus_to_cooling_rate += SuffixList[ThisItem->suffix_code].base_bonus_to_cooling_rate;

		ThisItem->bonus_to_tohit += SuffixList[ThisItem->suffix_code].base_bonus_to_tohit +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_tohit);
		ThisItem->bonus_to_damred_or_damage += SuffixList[ThisItem->suffix_code].base_bonus_to_damred_or_damage +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_damred_or_damage);

		ThisItem->bonus_to_resist_fire += SuffixList[ThisItem->suffix_code].base_bonus_to_resist_fire +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_resist_fire);
		ThisItem->bonus_to_resist_electricity += SuffixList[ThisItem->suffix_code].base_bonus_to_resist_electricity +
		    MyRandom(SuffixList[ThisItem->suffix_code].modifier_to_bonus_to_resist_electricity);

		ThisItem->is_identified = FALSE;
	}
	if (ThisItem->prefix_code != (-1)) {
		ThisItem->bonus_to_str += PrefixList[ThisItem->prefix_code].base_bonus_to_str +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_str);
		ThisItem->bonus_to_dex += PrefixList[ThisItem->prefix_code].base_bonus_to_dex +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_dex);
		ThisItem->bonus_to_mag += PrefixList[ThisItem->prefix_code].base_bonus_to_mag +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_mag);
		ThisItem->bonus_to_vit += PrefixList[ThisItem->prefix_code].base_bonus_to_vit +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_vit);
		ThisItem->bonus_to_all_attributes += PrefixList[ThisItem->prefix_code].base_bonus_to_all_attributes +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_all_attributes);

		ThisItem->bonus_to_life += PrefixList[ThisItem->prefix_code].base_bonus_to_life +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_life);
		ThisItem->bonus_to_health_recovery += PrefixList[ThisItem->prefix_code].base_bonus_to_health_recovery;
		ThisItem->bonus_to_force += PrefixList[ThisItem->prefix_code].base_bonus_to_force +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_force);
		ThisItem->bonus_to_cooling_rate += PrefixList[ThisItem->prefix_code].base_bonus_to_cooling_rate;

		ThisItem->bonus_to_tohit += PrefixList[ThisItem->prefix_code].base_bonus_to_tohit +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_tohit);
		ThisItem->bonus_to_damred_or_damage += PrefixList[ThisItem->prefix_code].base_bonus_to_damred_or_damage +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_damred_or_damage);

		ThisItem->bonus_to_resist_fire += PrefixList[ThisItem->prefix_code].base_bonus_to_resist_fire +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_resist_fire);
		ThisItem->bonus_to_resist_electricity += PrefixList[ThisItem->prefix_code].base_bonus_to_resist_electricity +
		    MyRandom(PrefixList[ThisItem->prefix_code].modifier_to_bonus_to_resist_electricity);

		ThisItem->is_identified = FALSE;
	}

};				// void FillInItemProperties( item* ThisItem , int FullDuration )

void write_full_item_name_into_string(item * ShowItem, char *full_item_name)
{
	strcpy(full_item_name, "");

	// --------------------
	// First clear the string and the we print out the item name.  That's simple.
	// we also add the extension of the name, the 'suffix' to it.
	//
	if ((ShowItem->suffix_code != (-1)) || (ShowItem->prefix_code != (-1))) {
		strcat(full_item_name, font_switchto_blue);
	} else {
		strcat(full_item_name, font_switchto_neon);
	}

	if (MatchItemWithName(ShowItem->type, "Valuable Circuits"))
		sprintf(full_item_name, "%d ", ShowItem->multiplicity);

	strcat(full_item_name, D_(ItemMap[ShowItem->type].item_name));

	//--------------------
	// If the item is magical but not identified, we might add the word
	// in parentheses and red font afterwards...
	//
	if (((ShowItem->suffix_code != (-1)) || (ShowItem->prefix_code != (-1))) && (!ShowItem->is_identified)) {
		strcat(full_item_name, font_switchto_red);
		strcat(full_item_name, _(" (Unidentified)"));
	}
	//--------------------
	// Now that the item name is out, we can switch back to the standard font color...
	//
	strcat(full_item_name, font_switchto_neon);

};				// void write_full_item_name_into_string ( item* ShowItem , char* full_item_name ) 

/**
 * This function drops an item at a given place, assigning it the given
 * suffix and prefix code.
 */
item *DropItemAt(int ItemType, int level_num, float x, float y, int prefix, int suffix, int multiplicity)
{
	int i;
	gps item_pos;
	level *item_drop_map_level = NULL;

	//--------------------
	// Some check against illegal item types
	//
	if ((ItemType < 0) || (ItemType >= Number_Of_Item_Types)) {
		DebugPrintf(-1000, "\n\nItemType received: %d.", ItemType);
		ErrorMessage(__FUNCTION__, "\
				Received an item type that was outside of range of the allowed item types.", PLEASE_INFORM, IS_FATAL);
	}
	//--------------------
	// Maybe the given position is from a virtual position of a dying robot.
	// Then of course we must fix it first.  But fortunately we have suitable
	// methods already...
	//
	item_pos.x = x;
	item_pos.y = y;
	item_pos.z = level_num;
	if (!resolve_virtual_position(&item_pos, &item_pos))
		return NULL;

	x = item_pos.x;
	y = item_pos.y;
	level_num = item_pos.z;

	//--------------------
	// Now we can properly set the level...
	//
	item_drop_map_level = curShip.AllLevels[level_num];

	//--------------------
	// If given a non-existent item type, we don't do anything
	// of course (and also don't produce a SEGFAULT or something...)
	//
	if (ItemType == (-1))
		return NULL;
	if (ItemType >= Number_Of_Item_Types) {
		fprintf(stderr, "\n\nItemType: '%d'.\n", ItemType);
		ErrorMessage(__FUNCTION__, "\
				There was an item code for an item to drop given to the function \n\
				DropItemAt( ... ), which is pointing beyond the scope of the known\n\
				item types.  This indicates a severe bug in Freedroid.", PLEASE_INFORM, IS_FATAL);
	}
	//--------------------
	// At first we must find a free item index on this level,
	// so that we can enter the new item there.
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (item_drop_map_level->ItemList[i].type == (-1)) {
			break;
		}
	}
	if (i >= MAX_ITEMS_PER_LEVEL) {
		ErrorMessage(__FUNCTION__, "\
				Couldn't find another array entry to drop another item.", PLEASE_INFORM, IS_FATAL);
	}
	//--------------------
	// Now we can construct the new item
	//
	item_drop_map_level->ItemList[i].type = ItemType;
	item_drop_map_level->ItemList[i].pos.x = x;
	item_drop_map_level->ItemList[i].pos.y = y;
	item_drop_map_level->ItemList[i].pos.z = level_num;
	item_drop_map_level->ItemList[i].prefix_code = prefix;
	item_drop_map_level->ItemList[i].suffix_code = suffix;

	FillInItemProperties(&(item_drop_map_level->ItemList[i]), FALSE, multiplicity);

	item_drop_map_level->ItemList[i].throw_time = 0.01;	// something > 0 
	if ((prefix == (-1)) && (suffix == (-1)))
		item_drop_map_level->ItemList[i].is_identified = TRUE;
	else
		item_drop_map_level->ItemList[i].is_identified = FALSE;

	play_item_sound(ItemType);

	return &item_drop_map_level->ItemList[i];
};

/**
 * This function checks whether a given item can be equipped.
 *
 */
int item_type_cannot_be_equipped(int drop_item_type)
{

	if (ItemMap[drop_item_type].item_can_be_installed_in_weapon_slot)
		return (FALSE);
	if (ItemMap[drop_item_type].item_can_be_installed_in_drive_slot)
		return (FALSE);
	if (ItemMap[drop_item_type].item_can_be_installed_in_armour_slot)
		return (FALSE);
	if (ItemMap[drop_item_type].item_can_be_installed_in_shield_slot)
		return (FALSE);
	if (ItemMap[drop_item_type].item_can_be_installed_in_special_slot)
		return (FALSE);

	return (TRUE);
};				// int item_type_cannot_be_equipped ( int drop_item_type )

/**
 * When a random item is being dropped, there is a chance of it being
 * magical.  In that case, there might be a modifying suffix attached to
 * the item.  This function is supposed to find a suitable suffix for
 * the item in question.
 **- */
int find_suitable_bonus_for_item(int drop_item_type, int TreasureChestRange, item_bonus * btype)
{
	//--------------------
	// First we catch the case of non-equipable items being magical.
	// This does not make sense and therefore is caught.
	//
	if (item_type_cannot_be_equipped(drop_item_type))
		return (-1);

	if (btype != SuffixList && btype != PrefixList)
		ErrorMessage(__FUNCTION__, "The bonus list to choose a prefix from is incorrect.\n", PLEASE_INFORM, IS_FATAL);

	if (TreasureChestRange > 4)	/* Temporary workaround for our lack of prefixes: there are only 4 classes for now. */
		TreasureChestRange = 4;

	int bcount = 0;
	int i = 0;

	while (btype[i].bonus_name != NULL) {
		if (btype[i].level == TreasureChestRange)
			bcount++;
		i++;
	}

	int pick = MyRandom(bcount);

	i = 0;
	while (btype[i].bonus_name != NULL) {
		if (btype[i].level == TreasureChestRange) {
			pick--;
			if (pick <= 0)
				return i;
		}
		i++;

	}

	ErrorMessage(__FUNCTION__,
		     "Could not find a bonus for item type %d, treasure chest %d, in bonus list %p starting with bonus name %s\n",
		     PLEASE_INFORM, IS_WARNING_ONLY, drop_item_type, TreasureChestRange, btype, btype[0].bonus_name);

	return 0;		//by default, return bonus number 0
};				// int find_suitable_bonus_for_item ( int drop_item_type , int TreasureChestRange, item_bonus * btype )

int get_random_item_type(int class)
{

	if (class > 9) {
		ErrorMessage(__FUNCTION__, "Random item class %d exceeds 9.\n", class, PLEASE_INFORM, IS_FATAL);
	}

	int a = MyRandom(item_count_per_class[class] - 1) + 1;

	//printf("Choosing in class %d among %d items, taking item %d\n", class, item_count_per_class[class], a);

	int i;
	for (i = 0; i < Number_Of_Item_Types; i++) {
		if (ItemMap[i].min_drop_class != -1) {
			if (ItemMap[i].min_drop_class <= class && ItemMap[i].max_drop_class >= class)
				a--;
			if (!a)
				break;
		}
	}

	if (a) {
		ErrorMessage(__FUNCTION__, "Looking for random item with class %d, a = %d after the loop.", PLEASE_INFORM, IS_FATAL, class,
			     a);
	}
	//printf("Dropping item %s (%d <= class <= %d), class %d\n", ItemMap[i].item_name, ItemMap[i].min_drop_class, ItemMap[i].max_drop_class, class);
	return i;
}

/**
 * This function drops a random item to the floor of the current level
 * at position ( x , y ).
 *
 * The strategy in droping the item is that one can easily set up and
 * modify the tabe of items to be dropped.
 *
 */
void DropRandomItem(int level_num, float x, float y, int class, int ForceMagical)
{
	int Suf;
	int Pre;
	int DropDecision;
	int drop_item_type = 1;
	int drop_item_multiplicity = 1;

	//--------------------
	// First we determine if there is something dropped at all or not,
	// cause in the later case, we can return immediately.  If a drop is
	// forced, we need not check for not do drop.
	//
	DropDecision = MyRandom(100);

	//--------------------
	// We decide whether we drop something at all or not
	//
	if ((DropDecision < 100 - GOLD_DROP_PERCENTAGE) && (DropDecision > ITEM_DROP_PERCENTAGE))
		return;

	//--------------------
	// Perhaps it's some gold that will be dropped.  That's rather
	// simple, so we do this first.
	//
	if ((DropDecision > 100 - GOLD_DROP_PERCENTAGE)) {
		// If class == 0, we want to avoid to drop 0-1 valuable circuits
		int how_many = 0;
		do {
			how_many = 10 * class + MyRandom(9);
		} while (how_many < 2);
		
		DropItemAt(GetItemIndexByName("Valuable Circuits"), level_num, x, y, -1, -1, how_many);
	}

	if ((DropDecision < ITEM_DROP_PERCENTAGE)) {
		drop_item_type = get_random_item_type(class);
		drop_item_multiplicity = 1;	//for now...  

		Suf = (-1);
		Pre = (-1);
		if (ForceMagical || (MyRandom(100) <= 2)) {
			Suf = find_suitable_bonus_for_item(drop_item_type, class, SuffixList);
		}
		if (ForceMagical || (MyRandom(100) <= 2)) {
			Pre = find_suitable_bonus_for_item(drop_item_type, class, PrefixList);
		}

		DropItemAt(drop_item_type, level_num, x, y, Pre, Suf, drop_item_multiplicity);
	}

};

/**
 * When the influencer gets hit, all of his equipment suffers some damage.
 * This is exactly what this function does:  apply the damage.
 */
void DamageItem(item * CurItem)
{

	//--------------------
	// If the item mentioned as parameter exists and if it is of 
	// a destructable sort, then we apply the usual damage to it
	if ((CurItem->type != (-1)) && (CurItem->max_duration != (-1))) {
		CurItem->current_duration -= (MyRandom(100) < ARMOUR_DURABILITYLOSS_PERCENTAGE_WHEN_HIT) ? 1 : 0;
		
		//--------------------
		// Make sound denoting some protective item was damaged
		BulletReflectedSound();

		//--------------------
		// If the item has gone over it's threshhold of duration, it finally
		// breaks and vaporizes
		//
		if (rintf(CurItem->current_duration) <= 0) {
			DeleteItem(CurItem);
		}
	}

};				// void DamageItem( item* CurItem )

/* Now we do the same for a weapon that has been fired */
void DamageWeapon(item * CurItem)
{

	if ((CurItem->type != (-1)) && (CurItem->max_duration != (-1))) {
		CurItem->current_duration -= (MyRandom(100) < WEAPON_DURABILITYLOSS_PERCENTAGE_WHEN_USED) ? 1 : 0;

		if (rintf(CurItem->current_duration) <= 0) {
			DeleteItem(CurItem);
		}
	}

};				// void DamageWeapon( item* CurItem )

/**
 * When the influencer gets hit, some of his equipment might suffer some damage.
 * This is exactly what this function does:  apply the damage.
 */
void DamageProtectiveEquipment()
{
	int ItemHit = MyRandom(6);

	if (ItemHit < 2)
		DamageItem(&(Me.armour_item));
	else if (ItemHit < 4)
		DamageItem(&(Me.shield_item));
	else if (ItemHit < 5)
		DamageItem(&(Me.drive_item));
	else		
                DamageItem(&(Me.special_item));

};				// void DamageProtectiveEquipment( void )

/**
 * This function is used when an equipment EXCHANGE is performed, i.e.
 * one weapon equiped is replaced by a new item using the mouse.  This 
 * leads to an exchange in the items.  Yet, if the new item came from 
 * inventory, the old item can't be just put in the same place where the
 * new item was, cause it might be bigger.  So, attemting to solve the
 * problem, the old item from the slot can just be made into an item on
 * the floor, but not visible yet of course, cause it still gets the 
 * held in hand attribute.
 */
void MakeHeldFloorItemOutOf(item * SourceItem)
{
	int i;

	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (CURLEVEL()->ItemList[i].type == (-1))
			break;
	}
	if (i >= MAX_ITEMS_PER_LEVEL) {
		DebugPrintf(0, "\n No free position to drop item!!! ");
		i = 0;
		Terminate(ERR);
	}
	// --------------------
	// Now we enter the item into the item list of this level
	//
	CopyItem(SourceItem, &(CURLEVEL()->ItemList[i]), FALSE);

	CURLEVEL()->ItemList[i].pos.x = Me.pos.x;
	CURLEVEL()->ItemList[i].pos.y = Me.pos.y;
	CURLEVEL()->ItemList[i].pos.z = Me.pos.z;
	CURLEVEL()->ItemList[i].currently_held_in_hand = TRUE;

	Item_Held_In_Hand = &(CURLEVEL()->ItemList[i]);

	DeleteItem(SourceItem);
};				// void MakeHeldFloorItemOutOf( item* SourceItem )

/**
 * This function looks through the inventory list and returns the index
 * of the first inventory item that is marked as 'held in hand'.
 * If no such item is found, an index of (-1) is returned.
 */
int GetHeldItemInventoryIndex(void)
{
	int InvPos;

	// --------------------
	// First we find out the inventory index of the item we want to
	// drop
	//
	for (InvPos = 0; InvPos < MAX_ITEMS_IN_INVENTORY; InvPos++) {
		if (Me.Inventory[InvPos].currently_held_in_hand && (Me.Inventory[InvPos].type > 0)) {
			return (InvPos);
		}
	}

	return (-1);
};				// int GetHeldItemInventoryIndex( void )

/**
 * This function generates a pointer to the (hopefully one and only) item
 * that is currently marked as held in hand.  If no such item can be 
 * found, the returned pointer will be NULL.
 */
item *GetHeldItemPointer(void)
{
	int InvIndex;
	int i;

	InvIndex = GetHeldItemInventoryIndex();

	if (InvIndex != (-1)) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in inventory was held in hand.  Good.");
		return (&(Me.Inventory[InvIndex]));
	} else if (Me.weapon_item.currently_held_in_hand > 0) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in weapon slot was held in hand.  Good.");
		return (&(Me.weapon_item));
	} else if (Me.drive_item.currently_held_in_hand > 0) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in weapon slot was held in hand.  Good.");
		return (&(Me.drive_item));
	} else if (Me.shield_item.currently_held_in_hand > 0) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in weapon slot was held in hand.  Good.");
		return (&(Me.shield_item));
	} else if (Me.armour_item.currently_held_in_hand > 0) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in weapon slot was held in hand.  Good.");
		return (&(Me.armour_item));
	} else if (Me.special_item.currently_held_in_hand > 0) {
		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : An item in weapon slot was held in hand.  Good.");
		return (&(Me.special_item));
	} else {
		// --------------------
		// Not that we find that no item is held in hand in the entire inventory 
		// and all the slots, we go and look if one of the items on this levels
		// map is perhaps held in hand, but if that also fails, then no item at
		// all was held in hand.
		//
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			if (CURLEVEL()->ItemList[i].type == (-1))
				continue;
			if (!CURLEVEL()->ItemList[i].currently_held_in_hand)
				continue;
			return (&(CURLEVEL()->ItemList[i]));
		}

		// DebugPrintf( 2 , "\nitem* GetHeldItemPointer( void ) : NO ITEM AT ALL SEEMS TO HAVE BEEN HELD IN HAND!!");
		return (NULL);
	}

};				// item* GetHeldItemPointer( void )

/**
 * This function DELETES an item from the source location.
 */
void DeleteItem(item * Item)
{

	Item->type = -1;
	Item->currently_held_in_hand = FALSE;

};				// void DeleteItem( item* Item )

/**
 * This function COPIES an item from the source location to the destination
 * location.  The source location is then marked as unused inventory 
 * entry.
 */
void CopyItem(item * SourceItem, item * DestItem, int MakeSound)
{

	memcpy(DestItem, SourceItem, sizeof(item));

	if (MakeSound) {
		// PlayItemSound( ItemMap[ SourceItem->type ].sound_number );
		play_item_sound(SourceItem->type);
	}

};				// void MoveItem( item* SourceItem , item* DestItem )

/**
 * This function COPIES an item from the source location to the destination
 * location.  The source location is then marked as unused inventory 
 * entry.
 */
void CopyItemWithoutHeldProperty(item * SourceItem, item * DestItem, int MakeSound)
{

	int temp = DestItem->currently_held_in_hand;
	CopyItem(SourceItem, DestItem, MakeSound);
	DestItem->currently_held_in_hand = temp;

};				// void CopyItemWithoutHeldProperty ( item* SourceItem , item* DestItem , int MakeSound ) 

/**
 * This function MOVES an item from the source location to the destination
 * location.  The source location is then marked as unused inventory 
 * entry.
 */
void MoveItem(item * SourceItem, item * DestItem)
{

	memcpy(DestItem, SourceItem, sizeof(item));

	SourceItem->type = (-1);

};				// void MoveItem( item* SourceItem , item* DestItem )

/**
 * This function applies a given item (to the influencer) and maybe 
 * eliminates the item after that, if it's an item that gets used up.
 */
void Quick_ApplyItem(int ItemKey)
{
	int FoundItemNr;

	if (ItemKey == 0) {
		//quick_inventory0 is slot 10
		ItemKey = 10;
	}

	FoundItemNr = GetInventoryItemAt(ItemKey - 1, INVENTORY_GRID_HEIGHT - 1);
	if (FoundItemNr == (-1))
		return;

	ApplyItem(&(Me.Inventory[FoundItemNr]));

};				// void Quick_ApplyItem( item* CurItem )

/**
 * There are many spellbooks.  These are handled inside the code via 
 * their item_type, an integer.  Now these spellbooks affect skills.  The
 * skills are handled internally via their index in the skill list.  So
 * at some points, it's convenient to look up the skill associated with
 * the spellbook.  So we do this once and have it available all the time
 * in a convenient and easy to maintain fashion.
 */
int associate_skill_with_item(int item_type)
{
	int associated_skill = (-1);
	if (!item_type || item_type == -1)
		return 0;	//we have a problem here...

	if (!strstr(ItemMap[item_type].item_name, "Source Book of")) {
		fprintf(stderr, "Game took item %d for a source book - this is a bug\n", item_type);
		return 0;
	}

	char *pos = strstr(ItemMap[item_type].item_name, "Source Book of");
	pos += strlen("Source Book of ");

	associated_skill = get_program_index_with_name(pos);

	return (associated_skill);

};				// int associate_skill_with_item ( int item_type )

/**
 *
 *
 */
int required_magic_stat_for_next_level_and_item(int item_type)
{
	return 0;
/*
    int level_index;
    int associated_skill = (-1) ;
    associated_skill = associate_skill_with_item ( item_type );

    //--------------------
    // In case we're not dealing with a spell book, the question
    // for skill requirement for the next level is senseless.  In
    // that case, we always allow to apply the item in question.
    //
    if ( associated_skill == (-1) ) 
	return ( 0 );

    //--------------------
    // Now we know the associated skill, so all we need to do is
    // return the proper value from the magic requirements entry
    // of the SkillMap.
    //
    level_index = Me . SkillLevel [ associated_skill ] ;
    if ( ( level_index >= 0 ) && ( level_index < NUMBER_OF_SKILL_LEVELS ) )
    {
	return ( SpellSkillMap [ associated_skill ] . magic_requirement_table [ level_index ] );
    }
    else
    {
	fprintf ( stderr , "\nlevel_index=%d." , level_index );
	ErrorMessage ( __FUNCTION__  , "\
There was a skill level mentioned, that exceeds the range of allowed skill levels.",
				   PLEASE_INFORM, IS_FATAL );
    }
TODO I removed this, need to put this back in*/
	//--------------------
	// Just to make the compiler happy.  (This line can't be
	// ever reached from inside the code...)
	//
	return (0);

};				// int required_magic_stat_for_next_level_and_item ( int item_type )

/**
 * Some items, that can be applied directly (like spellbooks) do have a
 * certain stat requirement.  This function checks, if the corresponding
 * requirements are met or not.
 */
int requirements_for_item_application_met(item * CurItem)
{

	if (Me.Magic >= required_magic_stat_for_next_level_and_item(CurItem->type)
	    || !required_magic_stat_for_next_level_and_item(CurItem->type)) {
		return (TRUE);
	} else
		return (FALSE);

};				// int requirements_for_item_application_met ( item* CurItem ) 

/**
 * This function checks whether a given item has the name specified. This is
 * used to match an item which its type in a flexible way (match by name instead
 * of matching by index value)
 */
int GetItemIndexByName(const char *name)
{
	int cidx = 0;

	for (; cidx < Number_Of_Item_Types; cidx++) {
		if (!strcmp(ItemMap[cidx].item_name, name))
			return cidx;
	}

	ErrorMessage(__FUNCTION__, "Unable to find item name %s\n", PLEASE_INFORM, IS_FATAL, name);
	return -1;
}

/**
 * This function checks whether a given item has the name specified. This is
 * used to match an item which its type in a flexible way (match by name instead
 * of matching by index value)
 */
int MatchItemWithName(int type, const char *name)
{
	if (type < 0 || type >= Number_Of_Item_Types)
		return 0;

	if (!strcmp(ItemMap[type].item_name, name))
		return 1;
	else
		return 0;
}

/**
 * This function applies a given item (to the influencer) and maybe 
 * eliminates the item after that, if it's an item that gets used up.
 */
void ApplyItem(item * CurItem)
{
	DebugPrintf(1, "\n%s(): function call confirmed.", __FUNCTION__);
	int failed_usage = 0; // if an item cannot be applied, to not remove it from inventory

	// If the inventory slot is not at all filled, we need not do anything more...
	if (CurItem->type < 0)
		return;

	if (ItemMap[CurItem->type].item_can_be_applied_in_combat == FALSE) {
		Me.TextVisibleTime = 0;
		Me.TextToBeDisplayed = _("I can't use this item here.");
		return;
	}

	if (!requirements_for_item_application_met(CurItem)) {
		application_requirements_not_met_sound();
		return;
	}

	if (Me.busy_time > 0) {
		char *msg;
		switch (Me.busy_type) {
		case DRINKING_POTION:
			msg = _("You are drinking a potion!");
			break;
		case WEAPON_FIREWAIT:
			msg = _("Your are waiting for your weapon to fire again!");
			break;
		case WEAPON_RELOAD:
			msg = _("You are reloading your weapon!");
			break;
		case THROWING_GRENADE:
			msg = _("You are throwing a grenade!");
			break;
		case RUNNING_PROGRAM:
			msg = _("You are running a program!");
			break;
		default:
			msg = _("You are doing something so weird the game does not understand what it is");
		}
		append_new_game_message(_("How do you expect to do two things at a time? %s"), msg);
		return;		//if the player is busy reloading or anything
	}
	//--------------------
	// At this point we know that the item is applicable in combat situation
	// and therefore all we need to do from here on is execute the item effect
	// upon the influencer or his environment.
	//
	if (MatchItemWithName(CurItem->type, "Barf's Energy Drink")) {
		Me.energy += 15;
		Me.temperature -= 15;
		Me.running_power += 15;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Diet supplement")) {
		Me.energy += 25;
		Me.busy_type = DRINKING_POTION;
		Me.busy_time = 1;
	} else if (MatchItemWithName(CurItem->type, "Antibiotic")) {
		Me.energy += 50;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Doc-in-a-can")) {
		Me.energy += Me.maxenergy;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Bottled ice")) {
		Me.temperature -= 50;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Industrial coolant")) {
		Me.temperature -= 100;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Liquid nitrogen")) {
		Me.temperature = 0;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Running Power Capsule")) {
		Me.running_power = Me.max_running_power;
		Me.running_must_rest = FALSE;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Strength Capsule")) {
		Me.current_power_bonus = 30;
		Me.power_bonus_end_date = Me.current_game_date + 2.0 * 60;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Dexterity Capsule")) {
		Me.current_dexterity_bonus = 30;
		Me.dexterity_bonus_end_date = Me.current_game_date + 2.0 * 60;
		Me.busy_time = 1;
		Me.busy_type = DRINKING_POTION;
	} else if (MatchItemWithName(CurItem->type, "Map Maker")) {
		Me.map_maker_is_present = TRUE;
		GameConfig.Automap_Visible = TRUE;
		Play_Spell_ForceToEnergy_Sound();
	} else if (MatchItemWithName(CurItem->type, "VMX Gas Grenade")) {
		DoSkill(get_program_index_with_name("Gas grenade"), 0);
		Me.busy_time = 1;
		Me.busy_type = THROWING_GRENADE;
	} else if (MatchItemWithName(CurItem->type, "EMP Shockwave Generator")) {
		DoSkill(get_program_index_with_name("EMP grenade"), 0);
		Me.busy_time = 1;
		Me.busy_type = THROWING_GRENADE;
	} else if (MatchItemWithName(CurItem->type, "Plasma Shockwave Emitter")) {
		DoSkill(get_program_index_with_name("Plasma grenade"), 0);
		Me.busy_time = 1;
		Me.busy_type = THROWING_GRENADE;
	} else if (MatchItemWithName(CurItem->type, "Strength Pill")) {
		Me.base_strength++;
	} else if (MatchItemWithName(CurItem->type, "Dexterity Pill")) {
		Me.base_dexterity++;
	} else if (MatchItemWithName(CurItem->type, "Code Pill")) {
		Me.base_magic++;
	} else if (MatchItemWithName(CurItem->type, "Identification Script")) {
		DoSkill(get_program_index_with_name("Analyze item"), 0);
	} else if (MatchItemWithName(CurItem->type, "Teleporter homing beacon")) {
		DoSkill(get_program_index_with_name("Sanctuary"), 0);
	} else if (strstr(ItemMap[CurItem->type].item_name, "Source Book of")) {
		int sidx = associate_skill_with_item(CurItem->type);
		failed_usage = improve_program(sidx);

		if(failed_usage == 0) {
			Play_Spell_ForceToEnergy_Sound();
		} else {
			append_new_game_message(_("You have reached the maximum skill level for %s"),ItemMap[CurItem->type].item_name + strlen("Source Book of "));
			Takeover_Game_Deadlock_Sound();
		}
	}

	if (Me.energy > Me.maxenergy)
		Me.energy = Me.maxenergy;
	if (Me.temperature < 0)
		Me.temperature = 0;

	play_item_sound(CurItem->type);

	//--------------------
	// In some cases the item concerned is a one-shot-device like a health potion, which should
	// evaporize after the first application.  Therefore we delete the item from the inventory list.
	//

	if (!failed_usage) {
		if (CurItem->multiplicity > 1)
			CurItem->multiplicity--;
		else
			DeleteItem(CurItem);
	}

	while (MouseRightPressed())
		SDL_Delay(1);
};				// void ApplyItem( int ItemNum )

/**
 * This function checks if a given coordinate within the influencers
 * inventory grid can be considered as free or as occupied by some item.
 */
int Inv_Pos_Is_Free(int x, int y)
{
	int i;
	int item_width;
	int item_height;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
		if (Me.Inventory[i].type == (-1))
			continue;
		if (Me.Inventory[i].currently_held_in_hand)
			continue;

		// for ( item_height = 0 ; item_height < ItemSizeTable[ Me.Inventory[ i ].type ].y ; item_height ++ )
		for (item_height = 0; item_height < ItemMap[Me.Inventory[i].type].inv_image.inv_size.y; item_height++) {
			for (item_width = 0; item_width < ItemMap[Me.Inventory[i].type].inv_image.inv_size.x; item_width++) {
				if (((Me.Inventory[i].inventory_position.x + item_width) == x) &&
				    ((Me.Inventory[i].inventory_position.y + item_height) == y))
					return (FALSE);
			}
		}
	}
	return (TRUE);

};				// int Inv_Pos_Is_Free( Inv_Loc.x , Inv_Loc.y )

/**
 * This function returns the index in the invenotry list of the object
 * at the inventory position x y.  If no object is found to occupy that
 * square, an index of (-1) is returned.
 * 
 * NOTE: The mentioned coordinates refer to the squares of the inventory grid!!
 *
 */
int GetInventoryItemAt(int x, int y)
{
	int i;
	int item_width;
	int item_height;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
		if (Me.Inventory[i].type == (-1))
			continue;

		for (item_height = 0; item_height < ItemMap[Me.Inventory[i].type].inv_image.inv_size.y; item_height++) {
			for (item_width = 0; item_width < ItemMap[Me.Inventory[i].type].inv_image.inv_size.x; item_width++) {
				if (((Me.Inventory[i].inventory_position.x + item_width) == x) &&
				    ((Me.Inventory[i].inventory_position.y + item_height) == y)) {
					return (i);
				}
			}
		}
	}
	return (-1);		// Nothing found at this grabbing location!!

};				// int GetInventoryItemAt ( int x , int y )

/**
 *
 * Often, especially in dialogs and in order to determine if some answer
 * should be allowed for the Tux or not, it is important to know if the
 * Tux has some special item of a given type in inventory or not and also
 * how many of those items the Tux really has.  
 *
 * This function is now intended to count the number of items of a given
 * type in the inventory of the Me.
 *
 */
int CountItemtypeInInventory(int Itemtype)
{
	int i;
	int NumberOfItemsFound = 0;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype)
			NumberOfItemsFound += Me.Inventory[i].multiplicity;
	}
	return NumberOfItemsFound;

};				// int CountItemtypeInInventory( int Itemtype )

/**
 *
 *
 */
int FindFirstInventoryIndexWithItemType(int Itemtype)
{
	int i;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype)
			return (i);
	}

	//--------------------
	// Severe error:  Item type NOT found in inventory!!!
	//
	fprintf(stderr, "\n\nItemType: '%d'.\n", Itemtype);
	ErrorMessage(__FUNCTION__, "\
There was an item code for an item to locate in inventory, but inventory\n\
did not contain this item type at all!  This indicates a severe bug in Freedroid.", PLEASE_INFORM, IS_FATAL);

	return (-1);

};				// int FindFirstInventoryIndexWithItemType ( ItemPointer->type , PLAYER_NR_0 )

/**
 * At some point the Tux will hand over all his items of a given type
 * to a dialog partner.  This function is intended to do exactly this:
 * To remove all items of a given type from the inventory of a given 
 * player.
 */
void DeleteInventoryItemsOfType(int Itemtype, int amount)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype) {
			if (Me.Inventory[i].multiplicity > amount)
				Me.Inventory[i].multiplicity -= amount;
			else
				DeleteItem(&(Me.Inventory[i]));
			return;
		}
	}
};				// void DeleteInventoryItemsOfType( int Itemtype )

/**
 * This deletes ONE item of the given type, like one bullet that has 
 * just been expended.
 */
void DeleteOneInventoryItemsOfType(int Itemtype)
{
	int i;
	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		if (Me.Inventory[i].type == Itemtype) {
			if (Me.Inventory[i].multiplicity > 1)
				Me.Inventory[i].multiplicity--;
			else
				DeleteItem(&(Me.Inventory[i]));
			return;
		}
	}

	//--------------------
	// This point must never be reached or a severe error has occured...
	//
	fprintf(stderr, "\n\nItemType: '%d'.\n", Itemtype);
	ErrorMessage(__FUNCTION__, "\
One single item of all the items of a given type in the Tux inventory\n\
should be removed, but there was not even one such item ever found in\n\
Tux invenrtory.  Something must have gone awry...", PLEASE_INFORM, IS_FATAL);

};				// void DeleteOneInventoryItemsOfType( int Itemtype  )

int MouseCursorIsInSkiERect(int x, int y)
{
	if (x > 320 || x < 0)
		return FALSE;
	if (y > 480 || y < 0)
		return FALSE;

	return TRUE;
}

/**
 * This function checks if a given screen position lies within the inventory
 * rectangle or not.
 */
int MouseCursorIsInInvRect(int x, int y)
{
	if (!GameConfig.Inventory_Visible)
		return FALSE;
	if (x > InventoryRect.x + InventoryRect.w)
		return (FALSE);
	if (x < InventoryRect.x)
		return (FALSE);
	if (y > InventoryRect.y + InventoryRect.h)
		return (FALSE);
	if (y < InventoryRect.y)
		return (FALSE);
	return (TRUE);
};				// int MouseCursorIsInInvRect( int x , int y )

/**
 * This function checks if a given screen position lies within the character
 * rectangle or not.
 */
int MouseCursorIsInChaRect(int x, int y)
{
	if (!GameConfig.CharacterScreen_Visible)
		return FALSE;
	if (x > CharacterRect.x + CharacterRect.w)
		return (FALSE);
	if (x < CharacterRect.x)
		return (FALSE);
	if (y > CharacterRect.y + CharacterRect.h)
		return (FALSE);
	if (y < CharacterRect.y)
		return (FALSE);
	return (TRUE);
};				// int MouseCursorIsInChaRect( int x , int y )

/**
 * This function checks if a given screen position lies within the skill
 * rectangle or not.
 */
int MouseCursorIsInSkiRect(int x, int y)
{
	if (!GameConfig.SkillScreen_Visible)
		return FALSE;
	if (x > SkillScreenRect.x + SkillScreenRect.w)
		return (FALSE);
	if (x < SkillScreenRect.x)
		return (FALSE);
	if (y > SkillScreenRect.y + SkillScreenRect.h)
		return (FALSE);
	if (y < SkillScreenRect.y)
		return (FALSE);
	return (TRUE);
};				// int MouseCursorIsInSkiRect( int x , int y )

/**
 * This function checks if a given screen position lies within the grid
 * where the inventory of the player is usually located or not.
 */
int MouseCursorIsInInventoryGrid(int x, int y)
{
	if (!GameConfig.Inventory_Visible)
		return FALSE;
	if ((x >= INVENTORY_RECT_X) && (x <= INVENTORY_RECT_X + INVENTORY_GRID_WIDTH * INV_SUBSQUARE_WIDTH)) {
		if ((y >= User_Rect.y + INVENTORY_RECT_Y) &&
		    (y <= User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * INVENTORY_GRID_HEIGHT)) {
			return (TRUE);
		}
	}
	return (FALSE);
};				// int MouseCursorIsInInventoryGrid( int x , int y )

/**
 * This function checks if a given screen position lies within the user
 * i.e. combat rectangle or not.
 */
int MouseCursorIsInUserRect(int x, int y)
{
	if (y < User_Rect.y)
		return (FALSE);
	if (y > User_Rect.y + User_Rect.h)
		return (FALSE);

	if ((!GameConfig.Inventory_Visible) && (!GameConfig.CharacterScreen_Visible) && (!GameConfig.SkillScreen_Visible)) {
		if (x > User_Rect.x + User_Rect.w)
			return (FALSE);
		if (x < User_Rect.x)
			return (FALSE);
		return (TRUE);
	}
	if ((GameConfig.Inventory_Visible && MouseCursorIsInInvRect(x, y))
	    || (GameConfig.CharacterScreen_Visible && MouseCursorIsInChaRect(x, y)) || (GameConfig.SkillScreen_Visible
											&& MouseCursorIsInSkiRect(x, y))
	    || (GameConfig.skill_explanation_screen_visible && MouseCursorIsInSkiERect(x, y)))
		return FALSE;
	return TRUE;
};				// int MouseCursorIsInUserRect( int x , int y )

/**
 * This function gives the x coordinate of the inventory square that 
 * corresponds to the mouse cursor location given to the function.
 */
int GetInventorySquare_x(int x)
{
	return ((x - INVENTORY_RECT_X) / INV_SUBSQUARE_WIDTH);
};				// int GetInventorySquare_x( x )

/**
 * This function gives the y coordinate of the inventory square that 
 * corresponds to the mouse cursor location given to the function.
 */
int GetInventorySquare_y(int y)
{
	return ((y - (User_Rect.y + INVENTORY_RECT_Y)) / INV_SUBSQUARE_HEIGHT);
};				// int GetInventorySquare_y( y )

/**
 * This function gives the item type of the currently held item.  
 *
 * THIS IS NOT NOT NOT THE PICTURE NUMBER, BUT THE ITEM TYPE!!!!
 *
 */
int GetHeldItemCode(void)
{
	item *ItemPointer;

	ItemPointer = GetHeldItemPointer();

	if (ItemPointer != NULL) {
		return (ItemPointer->type);
	}
	//--------------------
	// If we ever reach this point, that means that the held items code
	// could not be correctly computed, which should mean a reason to
	// terminate immediately with severe error
	//

	// DebugPrintf( 2 , "\nint GetHeldItemCode ( void ):  COULDN't FIND HELD ITEM!! " );
	// Terminate( ERR );
	return (-1);

};				// int GetHeldItemCode ( void )

/**
 * This function checks if a given item type could be dropped into the 
 * inventory grid at location x y.  Only the space is taken into account
 * and if other items block the way or not.
 */
int ItemCanBeDroppedInInv(int ItemType, int InvPos_x, int InvPos_y)
{
	int item_height;
	int item_width;

	//--------------------
	// Perhaps the item reaches even outside the inventory grid.  Then of course
	// it does not fit and we need/should not even test the details...
	//
	if (InvPos_x < 0 || InvPos_y < 0)
		return FALSE;
	if (ItemMap[ItemType].inv_image.inv_size.x - 1 + InvPos_x >= INVENTORY_GRID_WIDTH)
		return (FALSE);
	if (ItemMap[ItemType].inv_image.inv_size.y - 1 + InvPos_y >= INVENTORY_GRID_HEIGHT)
		return (FALSE);

	// --------------------
	// Now that we know, that the desired position is at least inside the inventory
	// grid, we can start to test for the details of the available inventory space
	//
	for (item_height = 0; item_height < ItemMap[ItemType].inv_image.inv_size.y; item_height++) {
		for (item_width = 0; item_width < ItemMap[ItemType].inv_image.inv_size.x; item_width++) {
			if (!Inv_Pos_Is_Free(InvPos_x + item_width, InvPos_y + item_height))
				return (FALSE);
		}
	}
	return (TRUE);

};				// int ItemCanBeDroppedInInv ( int ItemType , int InvPos_x , int InvPos_y )

/** 
 * This function should drop a given item to the floor.
 */
void DropItemToTheFloor(Item DropItemPointer, float x, float y, int levelnum)
{
	int i;
	Level DropLevel = curShip.AllLevels[levelnum];

	// --------------------
	// Now we want to drop the item to the floor.
	// We therefore find a free position in the item list of this level
	// where we can add the item later.
	//
	i = find_free_floor_items_index(levelnum);

	//--------------------
	// Now we enter the item into the item list of this level
	//
	CopyItem(DropItemPointer, &(DropLevel->ItemList[i]), TRUE);
	DropLevel->ItemList[i].pos.x = x;
	DropLevel->ItemList[i].pos.y = y;
	DropLevel->ItemList[i].pos.z = levelnum;
	DropLevel->ItemList[i].currently_held_in_hand = FALSE;
	DropLevel->ItemList[i].throw_time = 0.01;	// something > 0 

	DeleteItem(DropItemPointer);
};				// void DropItemToTheFloor ( void )

/** 
 * This function should drop a held item to the floor. 
 */
int DropHeldItemToTheFloor(void)
{
	float x, y;

	// Check validity of held item
	
	if (Item_Held_In_Hand == NULL) {
		DebugPrintf(0, "\nDropHeldItemToTheFloor() : No item seems to be currently held in hand...");
		return 1;		
	}
	
	// Get the actual mouse cursor position on the map
	
	x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
	y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);
	gps drop_pos = { x, y, Me.pos.z };
	gps drop_rpos;
	int can_resolve = resolve_virtual_position(&drop_rpos, &drop_pos);

	// If this position is valid, drop the item there
	
	if (can_resolve && DirectLineColldet(Me.pos.x, Me.pos.y, x, y, Me.pos.z, NULL)) {
		DropItemToTheFloor(Item_Held_In_Hand, drop_rpos.x, drop_rpos.y, drop_rpos.z);
		Item_Held_In_Hand = NULL;
	} else {
		fprintf(stderr, "Item drop failed because position is invalid.\n");
		return 1;
	}

	timeout_from_item_drop = 0.4;
	return 0;
};				// void DropHeldItemToTheFloor ( void )

/**
 * This function checks if the usage requirements for a given item are
 * met by the influencer or not.
 */
int ItemUsageRequirementsMet(item * UseItem, int MakeSound)
{
	if (Me.Strength < ItemMap[UseItem->type].item_require_strength && ItemMap[UseItem->type].item_require_strength > 0) {
		if (MakeSound)
			Not_Enough_Power_Sound();
		return (FALSE);
	}
	if (Me.Dexterity < ItemMap[UseItem->type].item_require_dexterity && ItemMap[UseItem->type].item_require_dexterity > 0) {
		if (MakeSound)
			Not_Enough_Dist_Sound();
		return (FALSE);
	}
	if (Me.Magic < ItemMap[UseItem->type].item_require_magic && ItemMap[UseItem->type].item_require_magic > 0) {
		return (FALSE);
	}
	if (!requirements_for_item_application_met(UseItem)) {
		return (FALSE);
	}
	return (TRUE);
};				// int ItemUsageRequirementsMet( item* UseItem )

/**
 * This function checks, if the influencer mets the requirements of the
 * item currently held in hand by the player/influencer.  Which item this
 * is will be found out by the function.
 */
int HeldItemUsageRequirementsMet(void)
{
	// Check validity of HeldItem
	if (Item_Held_In_Hand == NULL) {
		DebugPrintf(0, "\nvoid HeldItemUsageRequirementsMet ( void ) : No item in inventory seems to be currently held in hand...");
		return (FALSE);		
	}
	
	return (ItemUsageRequirementsMet(Item_Held_In_Hand, TRUE));
};				// int HeldItemUsageRequirementsMet( void )

/**
 * This function installs an item into a slot.  The given parameter is 
 * only the slot where this item should be installed.  The source item
 * will be found out from inside this function.  Very convenient.
 */
void DropHeldItemToSlot(item * SlotItem)
{
	item *DropItemPointer; // temporary storage

	// Chech validity of held item
	if (Item_Held_In_Hand == NULL) {
		DebugPrintf(0, "\nvoid DropHeldItemToSlot ( void ) : No item in inventory seems to be currently held in hand...");
		return;		
	}

	//--------------------
	// If there is an old item in the slot, we make a held item on the
	// floor out of it and also set the HeldItemType accordingly, so that
	// after the new item was placed successfully, the old item will
	// be out of all inventory slots, but still in the hand of the 
	// player and ready to be put somewhere else
	//
	// But this may only be done of course, if the 'old item' is not
	// the item we want to put there itself!!!!  HAHAHAHA!!!!
	//
	DropItemPointer = Item_Held_In_Hand;
	if ((SlotItem->type != (-1)) && (SlotItem->currently_held_in_hand == FALSE))
		MakeHeldFloorItemOutOf(SlotItem);
	else
		Item_Held_In_Hand = NULL;
	
	//--------------------
	// Now the item is installed into the weapon slot of the influencer
	//
	CopyItem(DropItemPointer, SlotItem, TRUE);
	SlotItem->currently_held_in_hand = FALSE;

	// Now the item is removed from the source location and no longer held in hand as well, 
	// but of course only if it is not the same as the original item
	if (DropItemPointer != SlotItem)
		DeleteItem(DropItemPointer);
};				// void DropHeldItemToSlot ( item* SlotItem )

/**
 * This function looks for a free inventory index.  Since there are more
 * inventory indices than squares in the inventory grid, the function 
 * should always be able to find a free inventory index.  If not, this is
 * considered a severe program error, which will cause immediate 
 * termination of FreeDroid.
 */
int GetFreeInventoryIndex(void)
{
	int InvPos;

	// --------------------
	// We find out the first free inventory index:
	//
	for (InvPos = 0; InvPos < MAX_ITEMS_IN_INVENTORY - 1; InvPos++) {
		if (Me.Inventory[InvPos].type == (-1)) {
			return (InvPos);
		}
	}

	// --------------------
	// If this point is reached, the severe error mentioned above has
	// occured, an error message must be printed out and the program
	// must be terminated.
	//
	ErrorMessage(__FUNCTION__, "\
A FREE INVENTORY INDEX POSITION COULD NOT BE FOUND.\n\
This is an internal error, that must never happen unless there are\n\
severe bugs in the inventory system.", PLEASE_INFORM, IS_FATAL);
	return (-1);		// just to make compilers happy.
};				// int GetFreeInventoryIndex( void )

/**
 * If an item is held and then clicked again in the inventory field, this
 * item should be dropped into the inventory field, provided there is room
 * enough in it at that location.  If that is the case, then the item is
 * dropped onto this inventory location, else nothing is done.
 */
void DropHeldItemToInventory(void)
{
	point CurPos;
	int FreeInvIndex;
	int i;
	FreeInvIndex = GetFreeInventoryIndex();

	// --------------------
	// First we check validity of held item
	//
	if (Item_Held_In_Hand == NULL) {
		DebugPrintf(0, "\nvoid DropHeldItemToInventory ( void ) : No item in inventory seems to be currently held in hand...");
		return;
	}

	// --------------------
	// Now we want to drop the item to the right location again.
	// Therefore we need to find out the right position, which of course
	// depends as well on current mouse cursor location as well as the
	// size of the dropped item.
	//
	CurPos.x = GetMousePos_x() - (16 * (ItemMap[Item_Held_In_Hand->type].inv_image.inv_size.x - 1));
	CurPos.y = GetMousePos_y() - (16 * (ItemMap[Item_Held_In_Hand->type].inv_image.inv_size.y - 1));

	if (ItemCanBeDroppedInInv(Item_Held_In_Hand->type, GetInventorySquare_x(CurPos.x), GetInventorySquare_y(CurPos.y))) {
		CopyItem(Item_Held_In_Hand, &(Me.Inventory[FreeInvIndex]), TRUE);
		Me.Inventory[FreeInvIndex].inventory_position.x = GetInventorySquare_x(CurPos.x);
		Me.Inventory[FreeInvIndex].inventory_position.y = GetInventorySquare_y(CurPos.y);
		Me.Inventory[FreeInvIndex].currently_held_in_hand = FALSE;

		// --------------------
		// Now that we know that the item could be dropped directly to inventory 
		// without swapping any spaces, we can as well make the item
		// 'not held in hand' immediately and return
		//
		DeleteItem(Item_Held_In_Hand);
		Item_Held_In_Hand = NULL;
		return;
	} else {
		//--------------------
		// So the item could not be placed into inventory directly, but maybe
		// it can be placed there if we swap our dropitem with some other item.
		// Let's test this opportunity here.
		//
		for (i = 0; i < MAX_ITEMS_IN_INVENTORY - 1; i++) {
			//--------------------
			// FIRST: Security check against segfaults:  It might happen that we 
			// delete the Dropitem itself while trying several items as candidates
			// for removal.  This would cause testing dropability with a -1 item
			// type and a SEGFAULT would result...
			//
			if (&(Me.Inventory[i]) == Item_Held_In_Hand)
				continue;

			//--------------------
			// So we make a copy of each of the items we remove in order to 
			// try to create new space for the drop item.  After that, we can
			// remove it.
			//
			CopyItem(&(Me.Inventory[i]), &(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]), FALSE);
			Me.Inventory[i].type = (-1);

			if (ItemCanBeDroppedInInv(Item_Held_In_Hand->type, GetInventorySquare_x(CurPos.x), GetInventorySquare_y(CurPos.y))) {
				
				// Copy the HelItem to the now free position
				CopyItem(Item_Held_In_Hand, &(Me.Inventory[FreeInvIndex]), TRUE);
				Me.Inventory[FreeInvIndex].inventory_position.x = GetInventorySquare_x(CurPos.x);
				Me.Inventory[FreeInvIndex].inventory_position.y = GetInventorySquare_y(CurPos.y);
				Me.Inventory[FreeInvIndex].currently_held_in_hand = FALSE;
				DeleteItem(Item_Held_In_Hand);

				// The removed item Nr. i is put in hand in replacement of the
				// prior HeldItem.
				MakeHeldFloorItemOutOf(&(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]));

				return;
			}

			//--------------------
			// But if even the removal of one item was not enough, so that the new
			// item would fit into the inventory, then of course we should re-add the
			// removed item to the inventory, so that no other items get lost.
			//
			CopyItem(&(Me.Inventory[MAX_ITEMS_IN_INVENTORY - 1]), &(Me.Inventory[i]), FALSE);

		}		// for: try all items if removal is the solution
	}			// if not immediately place findable
};				// void DropHeldItemToInventory( void )

/**
 * This function shows the quick inventory items on the right side of
 * the screen.
 */
void ShowQuickInventory(void)
{
	int i;
	SDL_Rect TargetRect;
	int Index;

	//--------------------
	// Now we can blit all the objects in the quick inventory, but of course only
	// those small objects, that have a 1x1 inventory grid size, so that they really
	// can be drawn from the 'belt' that is actually the quick inventory.
	//
	for (i = 0; i < 10; i++) {
		PutCharFont(Screen, Messagestat_BFont, (130 + i * 40 - 9) * GameConfig.screen_width / 640, GameConfig.screen_height - 16,
			    (i < 9) ? ('1' + i) : '0');
		if (((Index = GetInventoryItemAt(i, INVENTORY_GRID_HEIGHT - 1)) != (-1)) && (Me.Inventory[Index].inventory_position.x == i)
		    && (Me.Inventory[Index].inventory_position.y == INVENTORY_GRID_HEIGHT - 1)) {
			TargetRect.x = (130 + i * 40) * GameConfig.screen_width / 640;
			TargetRect.y = GameConfig.screen_height - 32;

			our_SDL_blit_surface_wrapper(ItemMap[Me.Inventory[Index].type].inv_image.Surface, NULL, Screen, &TargetRect);

		}
	}
};				// void ShowQuickInventory ( void )

/**
 *
 *
 */
int get_floor_item_index_under_mouse_cursor(level **item_lvl)
{
	gps mouse_pos;
	int i;

	//--------------------
	// In the case that X was pressed, we don't use the item positions but rather
	// we use the item slot rectangles from the item texts.
	//
	if (XPressed()) {
		struct visible_level *vis_lvl, *n;
		
		BROWSE_VISIBLE_LEVELS(vis_lvl, n) {	
			level *lvl = vis_lvl->lvl_pointer;

			for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
				if (lvl->ItemList[i].type == (-1))
					continue;
	
				if (MouseCursorIsInRect(&(lvl->ItemList[i].text_slot_rectangle), GetMousePos_x(), GetMousePos_y())) {
					*item_lvl = lvl;
					return (i);
				}
			}
		}
	}
	//--------------------
	// If no X was pressed, we only use the floor position the mouse
	// has pointed to and see if we can find an item that has geographically
	// that very same (or a similar enough) position.
	//
	else {
		mouse_pos.x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
		mouse_pos.y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);
		mouse_pos.z = Me.pos.z;
		
		gps virt_mouse_pos;
		struct visible_level *vis_lvl, *n;
		
		BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
			
			level *lvl = vis_lvl->lvl_pointer;
			update_virtual_position(&virt_mouse_pos, &mouse_pos, lvl->levelnum);
			
			for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
				if (lvl->ItemList[i].type == (-1))
					continue;
	
				if ((fabsf(virt_mouse_pos.x - lvl->ItemList[i].pos.x) < 0.5) &&
					(fabsf(virt_mouse_pos.y - lvl->ItemList[i].pos.y) < 0.5)) {
					*item_lvl = lvl;
					return (i);
				}
			}
		}
	}

	return (-1);

};				// int get_floor_item_index_under_mouse_cursor ( )

/**
 * When the player has given an identification command, i.e. triggered
 * the mouse button when in identification mode, we see if maybe the
 * mouse cursor has pointed to an item, that can be identified.  In that
 * case, we'll try to identify it, otherwise we'll simply cancel the
 * whole identification operation.
 */
void handle_player_identification_command()
{
	point Inv_GrabLoc;
	point CurPos;
	int Grabbed_InvPos = (-1);
	item *GrabbedItem = NULL;

	//--------------------
	// If the inventory sceen isn't open at all, then we can cancel
	// the whole operation right away
	//
	if (!GameConfig.Inventory_Visible) {
		append_new_game_message(_("Identified air."));
		return;
	}
	// --------------------
	// We will need the current mouse position on several spots...
	//
	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
		Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
		Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);
		Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);
		GrabbedItem = &Me.Inventory[Grabbed_InvPos];
	} else {
		if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y))
			GrabbedItem = &Me.weapon_item;
		if (MouseCursorIsOnButton(DRIVE_RECT_BUTTON, CurPos.x, CurPos.y))
			GrabbedItem = &Me.drive_item;
		if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y))
			GrabbedItem = &Me.shield_item;
		if (MouseCursorIsOnButton(ARMOUR_RECT_BUTTON, CurPos.x, CurPos.y))
			GrabbedItem = &Me.armour_item;
		if (MouseCursorIsOnButton(HELMET_RECT_BUTTON, CurPos.x, CurPos.y))
			GrabbedItem = &Me.special_item;
	}

	if (GrabbedItem == NULL)
		append_new_game_message(_("Identified air."));
	else if (!GrabbedItem->type || GrabbedItem->type == -1)
		append_new_game_message(_("Identified air."));
	else {
		char iname[500];
		*iname = '\0';

		if (MatchItemWithName(GrabbedItem->type, "Valuable Circuits"))
			sprintf(iname, "%d ", GrabbedItem->multiplicity);

		if ((GrabbedItem->prefix_code != (-1)))
			strcat(iname, PrefixList[GrabbedItem->prefix_code].bonus_name);
		strcat(iname, ItemMap[GrabbedItem->type].item_name);
		if ((GrabbedItem->suffix_code != (-1)))
			strcat(iname, SuffixList[GrabbedItem->suffix_code].bonus_name);

		if (GrabbedItem->is_identified == TRUE) {
			PlayOnceNeededSoundSample("effects/is_already_indentif.ogg", FALSE, FALSE);
			append_new_game_message(_("You already know all there is to know about the %s."), iname);
		} else {
			GrabbedItem->is_identified = TRUE;
			Play_Spell_ForceToEnergy_Sound();
			append_new_game_message(_("Identified %s"), iname);
		}
	}
};				// void handle_player_identification_command( )

/* ------------------------------------------
 * Handle inventory screen stuff: interact with items on floor, in inventory grid, in 
 * inventory slots, and apply (right click) items.
 * ------------------------------------------*/
void HandleInventoryScreen(void)
{
	point CurPos;
	finepoint MapPositionOfMouse = { 0.0, 0.0 };
	
	struct {
		int buttonidx;
		item *slot;
	} allslots[] =		/*list of all slots and their associated item */
	{ {
	WEAPON_RECT_BUTTON, &(Me.weapon_item)}, {
	DRIVE_RECT_BUTTON, &(Me.drive_item)}, {
	SHIELD_RECT_BUTTON, &(Me.shield_item)}, {
	ARMOUR_RECT_BUTTON, &(Me.armour_item)}, {
	HELMET_RECT_BUTTON, &(Me.special_item)},};

	//--------------------
	// In case the Tux is dead already, we do not need to display any inventory screen
	// or even to pick up any stuff for the Tux...
	//
	if (Me.energy <= 0) {
		return;
	}
	// --------------------
	// We will need the current mouse position on several spots...
	//
	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	// If the inventory is not visible we don't handle the screen itself but we still pick up items on the ground
	if (GameConfig.Inventory_Visible == FALSE) {
		silently_unhold_all_items();
	}

	//--------------------
	// Case 1: The user left-clicks while not holding an item
	//
	if (MouseLeftClicked() && (Item_Held_In_Hand == NULL) && (global_ingame_mode != GLOBAL_INGAME_MODE_IDENTIFY)) {
		
		//--------------------
		// Case 1.1: The user left-clicks in the "UserRect" -> try to pick an
		//           item in the floor
		//
		if (MouseCursorIsInUserRect(CurPos.x, CurPos.y)) {
		
			int item_idx = (-1);
			level *item_lvl = NULL;
			item *picked_item = NULL;
			gps *picked_item_pos = NULL;
			gps picked_item_vpos;
	
			// Pick up something under the mouse cursor
			
			MapPositionOfMouse.x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
			MapPositionOfMouse.y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);
			item_idx = get_floor_item_index_under_mouse_cursor(&item_lvl);
			
			if (item_idx != -1 && item_lvl != NULL) {
				picked_item = &(item_lvl->ItemList[item_idx]);
				picked_item_pos = &(picked_item->pos);
				update_virtual_position(&picked_item_vpos, picked_item_pos, Me.pos.z);
			}
			
			// We only take the item directly into our 'hand' i.e. the mouse cursor,
			// if the item in question can be reached directly and isn't blocked by
			// some walls or something...
			
			if (picked_item) {
				if (GameConfig.Inventory_Visible == FALSE || MatchItemWithName(picked_item->type, "Valuable Circuits")) {
					/* Handled in check_for_items_to_pickup() */
					return;
				}
				if ((fabsf(picked_item_vpos.x - Me.pos.x) < ITEM_TAKE_DIST) &&
					(fabsf(picked_item_vpos.y - Me.pos.y) < ITEM_TAKE_DIST) &&
					DirectLineColldet(Me.pos.x, Me.pos.y, picked_item_vpos.x, picked_item_vpos.y, Me.pos.z, NULL)) {
					Item_Held_In_Hand = picked_item;
					Item_Held_In_Hand->currently_held_in_hand = TRUE;
					return;
				} else {
					/* Handled in check_for_items_to_pickup() */
					return;
				}
			}
			
			// No item was picked
			
			return;
		}

		//--------------------
		// Case 1.2: The user left-clicks on the inventory grid
		//
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			
			point Inv_GrabLoc;
			int Grabbed_InvPos;

			Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
			Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);
			Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);
			
			if (Grabbed_InvPos == (-1)) {
				/* No item under the cursor */
				return;
			}
			
			// At this point we know, that we have just grabbed something from the inventory
			// So we set, that something should be displayed in the 'hand', and it should of
			// course be the image of the item grabbed from inventory.
			if (global_ingame_mode == GLOBAL_INGAME_MODE_NORMAL) {
				Item_Held_In_Hand = &(Me.Inventory[Grabbed_InvPos]);
				Item_Held_In_Hand->currently_held_in_hand = TRUE;
				play_item_sound(Item_Held_In_Hand->type);
			}

			return;
		}
		
		//--------------------
		// Case 1.3: The user left-clicks on one of the equipment slots
		//
		unsigned int i;

		for (i = 0; i < sizeof(allslots) / sizeof(allslots[0]); i++) {
			if (MouseCursorIsOnButton(allslots[i].buttonidx, CurPos.x, CurPos.y)) {
				if (allslots[i].slot->type > 0) {
					Item_Held_In_Hand = allslots[i].slot;
					Item_Held_In_Hand->currently_held_in_hand = TRUE;
					return;
				}
			}
		}
		
		// No item was picked
		return;
	}
	
	//--------------------
	// Case 2: The user left-clicks somewhere to drop a held item
	//
	if (MouseLeftClicked() && (Item_Held_In_Hand != NULL)) {
		
		//--------------------
		// Case 2.1: The left-clicks on the inventory grid -> we must see if 
		//           the item was dropped onto a correct inventory location and 
		//           should from then on not only no longer be in the players
		//           hand but also remain at the newly assigned position.
		//
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			DropHeldItemToInventory();
			return;
		}

		//--------------------
		// Case 2.2: The user left-clicks in the "UserRect" -> the item should 
		//           be dropped onto the players current location
		//
		if (MouseCursorIsInUserRect(CurPos.x, CurPos.y)) {
			DropHeldItemToTheFloor();
			return;
		}

		//--------------------
		// Case 2.3: The user left-clicks in the weapon's equipment slot
		//
		if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y)) {
			
			// Check if the item can be installed in the weapon slot
			if (!ItemMap[Item_Held_In_Hand->type].item_can_be_installed_in_weapon_slot) {
				append_new_game_message(_("You cannot fight with this!"));
				return;
			}
			
			// Check if the user has enough skill to use the weapon
			if (!HeldItemUsageRequirementsMet()) {
				append_new_game_message(_("You cannot yet fight with this!"));
				return;
			}
	
			// Now a weapon is about to be dropped to the weapons rectangle and obviously
			// the stat requirements for usage are met.  But maybe this is a 2-handed weapon.
			// In this case we need to do some extra check.  If it isn't a 2-handed weapon,
			// then we can just go ahead and equip the item
			if (!ItemMap[Item_Held_In_Hand->type].item_gun_requires_both_hands) {
				DropHeldItemToSlot(&(Me.weapon_item));
				return;
			}
			
			// So, this is a 2-handed weapon. If the shield slot is just empty, 
			// that makes matters a lot simpler, because then we can just drop 
			// this 2-handed weapon to the weapon slot and all is fine, because 
			//no conflicts will result...
			if (Me.shield_item.type == (-1)) {
				DropHeldItemToSlot(&(Me.weapon_item));
				return;
			}
			
			// But if there is something in the shield slot too, then we need to be
			// a bit more sophisticated and either swap the 2-handed item in for just
			// the shield alone, which then will be held OR we need to refuse completely
			// because there might be a weapon AND a shield equipped already.
			if (Me.weapon_item.type == (-1)) {
				// first of all check requirements again but without the shield :
				// virtually remove the shield, compute requirements, if 
				// everything's okay, proceed otherwise we inform the player
				int shield_item_type = Me.shield_item.type;
				Me.shield_item.type = (-1);
				update_all_primary_stats();
				if (HeldItemUsageRequirementsMet()) {
					DropHeldItemToSlot(&(Me.weapon_item));
					Me.shield_item.type = shield_item_type;
					MakeHeldFloorItemOutOf(&(Me.shield_item));
				} else {
					append_new_game_message(
							_("Two-handed weapon requirements not met: shield bonus doesn't count."));
					Me.shield_item.type = shield_item_type;
				}
			} else {
				PlayOnceNeededSoundSample("effects/tux_ingame_comments/ThisItemRequiresBothHands.ogg", 
						FALSE, FALSE);
			}
		}

		//--------------------
		// Case 2.4: The user left-clicks in the shield's equipment slot
		//
		if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y)) {
			
			// Check if the item can be installed in the shield slot
			if (!ItemMap[Item_Held_In_Hand->type].item_can_be_installed_in_shield_slot) {
				append_new_game_message(_("You cannot equip this!"));
				return;
			}
			
			// Check if the user has enough skill to use the shield
			if (!HeldItemUsageRequirementsMet()) {
				append_new_game_message(_("You cannot yet equip this!"));
				return;
			}
			
			// Now if there isn't any weapon equipped right now, the matter
			// is rather simple and we just need to do the normal drop-to-slot-thing.
			if (Me.weapon_item.type == (-1)) {
				DropHeldItemToSlot(&(Me.shield_item));
				return;
			}

			// A shield, when equipped, will push out any 2-handed weapon currently
			// equipped from it's weapon slot.... So first check if a 2-handed
			// weapon is equipped.
			if (!ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
				DropHeldItemToSlot(&(Me.shield_item));
				return;
			}
			
			// There is a 2-handed weapon equipped, so first of all check 
			// requirements again but without the weapon :
			// virtually remove the weapon, compute requirements, if 
			// everything's okay, proceed otherwise we inform the player
			int weapon_item_type = Me.weapon_item.type;
			Me.weapon_item.type = (-1);
			update_all_primary_stats();
			if (HeldItemUsageRequirementsMet()) {
				DropHeldItemToSlot(&(Me.shield_item));
				Me.weapon_item.type = weapon_item_type;
				MakeHeldFloorItemOutOf(&(Me.weapon_item));
			} else {
				append_new_game_message(_
							("Shield requirements not met: two-handed weapon bonus doesn't count."));
				Me.weapon_item.type = weapon_item_type;
			}
		}

		//--------------------
		// Case 2.5: The user left-clicks in an other equipment slot
		//
		itemspec *tocheck = &ItemMap[Item_Held_In_Hand->type];
		struct {
			int btnidx;
			char propcheck;
			item *slot;
		} dropslots[] = { {
		DRIVE_RECT_BUTTON, tocheck->item_can_be_installed_in_drive_slot, &(Me.drive_item)}, {
		ARMOUR_RECT_BUTTON, tocheck->item_can_be_installed_in_armour_slot, &(Me.armour_item)}, {
		HELMET_RECT_BUTTON, tocheck->item_can_be_installed_in_special_slot, &(Me.special_item)},};
		int i;
		for (i = 0; i < sizeof(dropslots) / sizeof(dropslots[0]); i++) {
			if (MouseCursorIsOnButton(dropslots[i].btnidx, CurPos.x, CurPos.y) && dropslots[i].propcheck
			    && HeldItemUsageRequirementsMet()) {
				DropHeldItemToSlot(dropslots[i].slot);
				return;
			}
		}

		//--------------------
		// The left-click did not lead to anything useful
		
		return;
	}

	//--------------------
	// There was no left-click, so we will now analyze right-click, but a right
	// click is no-sense (here) if the Inventory screen is hidden
	//
	if (!GameConfig.Inventory_Visible) {
		return;
	}

	//--------------------
	// Case 3: The user is right-clicking inside the inventory rectangle which 
	// would mean for us that he is applying the item under the mouse button
	//
	if (MouseRightClicked()) {

		if (Me.readied_skill == get_program_index_with_name("Repair equipment")) {
			//--------------------
			// Here we know, that the repair skill is selected, therefore we try to 
			// repair the item currently under the mouse cursor.
			//
			if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
				point Inv_GrabLoc;
				int Grabbed_InvPos;
				
				Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
				Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);

				DebugPrintf(0, "\nTrying to repair item at inv-pos: %d %d.", Inv_GrabLoc.x, Inv_GrabLoc.y);

				Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);
				DebugPrintf(0, "\nTrying to repair inventory entry no.: %d.", Grabbed_InvPos);

				if (Grabbed_InvPos == (-1)) {
					// Nothing grabbed, so we need not do anything more here..
					DebugPrintf(0, "\nRepairing in INVENTORY grid FAILED:  NO ITEM AT THIS POSITION FOUND!");
				} else {
					if (Me.Inventory[Grabbed_InvPos].max_duration != -1)
						HomeMadeItemRepair(&(Me.Inventory[Grabbed_InvPos]));
					else
						ApplyItem(&(Me.Inventory[Grabbed_InvPos]));
				}
			} else {
				int i;
				for (i = 0; i < sizeof(allslots) / sizeof(allslots[0]); i++) {
					if (MouseCursorIsOnButton(allslots[i].buttonidx, CurPos.x, CurPos.y)
					    && allslots[i].slot->type != -1) {
						HomeMadeItemRepair(allslots[i].slot);
						break;
					}
				}
			}
		} else {
			if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
				point Inv_GrabLoc;
				int Grabbed_InvPos;

				Inv_GrabLoc.x = GetInventorySquare_x(CurPos.x);
				Inv_GrabLoc.y = GetInventorySquare_y(CurPos.y);

				Grabbed_InvPos = GetInventoryItemAt(Inv_GrabLoc.x, Inv_GrabLoc.y);

				if (Grabbed_InvPos != -1) {
					//--------------------
					// At this point we know, that we have just applied something from the inventory
					//
					ApplyItem(&(Me.Inventory[Grabbed_InvPos]));
				}
			}
		}
	}
};				// void HandleInventoryScreen ( void );

/**
 *
 *
 */
void raw_move_picked_up_item_to_entry(item * ItemPointer, item * TargetPointer, point Inv_Loc)
{
	/*
	char TempText[1000];

	// Announce that we have taken the item. Pointless for purposes other than debugging.
	// If you pointed and clicked on something, if it vanishes, you picked it up...

	Me.TextVisibleTime = 0;
	sprintf(TempText, _("Item taken: %s."), D_(ItemMap[ItemPointer->type].item_name));
	append_new_game_message(TempText);	// this can be freed/destroyed afterwards.  it's ok.
	Me.TextToBeDisplayed = strdup(TempText);
	*/

	// We add the new item to the inventory
	CopyItem(ItemPointer, TargetPointer, FALSE);
	TargetPointer->inventory_position.x = Inv_Loc.x;
	TargetPointer->inventory_position.y = Inv_Loc.y;

	// We make the sound of an item being taken
	// PlayItemSound( ItemMap[ ItemPointer->type ].sound_number );
	play_item_sound(ItemPointer->type);

	DeleteItem(ItemPointer);
};				// void move_picked_up_item_to_entry ( ItemPointer , TargetPointer )

/**
 *
 */
int place_item_on_this_position_if_you_can(item * ItemPointer, point Inv_Loc, int InvPos)
{
	int item_height;
	int item_width;

	for (item_height = 0; item_height < ItemMap[ItemPointer->type].inv_image.inv_size.y; item_height++) {
		for (item_width = 0; item_width < ItemMap[ItemPointer->type].inv_image.inv_size.x; item_width++) {
			DebugPrintf(1, "\nAddFloorItemDirectlyToInventory:  Checking pos: %d %d ", Inv_Loc.x + item_width,
				    Inv_Loc.y + item_height);
			if (!Inv_Pos_Is_Free(Inv_Loc.x + item_width, Inv_Loc.y + item_height)) {
				Me.Inventory[InvPos].inventory_position.x = -1;
				Me.Inventory[InvPos].inventory_position.y = -1;
				// goto This_Is_No_Possible_Location;
				return (FALSE);
			}
		}
	}
	// if ( !Inv_Pos_Is_Free( Inv_Loc.x , Inv_Loc.y ) ) continue;

	// At this point we know we have reached a position where we can plant this item.
	Me.Inventory[InvPos].inventory_position.x = Inv_Loc.x;
	Me.Inventory[InvPos].inventory_position.y = Inv_Loc.y;
	DebugPrintf(1, "\nAddFloorItemDirectlyToInventory:  FINE INVENTORY POSITION FOUND!!");

	//--------------------
	if ((InvPos >= MAX_ITEMS_IN_INVENTORY - 1) || (Me.Inventory[InvPos].inventory_position.x == (-1))) {
		Me.TextVisibleTime = 0;
		Me.TextToBeDisplayed = _("I can't carry any more.");
		CantCarrySound();
		// can't take any more items,
	} else {
		raw_move_picked_up_item_to_entry(ItemPointer, &(Me.Inventory[InvPos]), Inv_Loc);
	}
	return (TRUE);
};				// int place_item_on_this_position_if_you_can ( ... )

/**
 * This function deals with the case, that WHILE THERE IS NO INVENTORY
 * SCREEN OPEN, the Tux still clicks some items on the floor to pick them
 * up.  So no big visible operation is required, but instead the items
 * picked up should be either auto-equipped, if possible, or they should
 * be put into the inventory items pool OR IN CASE THERE IS NO ROOM ANY
 * MORE the function should also say that and not do much else...
 */
int AddFloorItemDirectlyToInventory(item * ItemPointer)
{
	int InvPos;
	point Inv_Loc = { -1, -1 };
	int TargetItemIndex;

	if (ItemPointer == NULL)
		return -1;

	//--------------------
	// In the special case of money, we add the amount of money to our
	// money counter and eliminate the item on the floor.
	//
	if (MatchItemWithName(ItemPointer->type, "Valuable Circuits")) {
	// Announce that we have taken the money. Pointless for purposes other than debugging.
	// If you pointed and clicked on something, if it vanishes, you picked it up...
	/*
		char tmp[100];
		sprintf(tmp, _("Picked up %d valuable circuits."), ItemPointer->multiplicity);
		append_new_game_message(tmp);
	*/
		play_item_sound(ItemPointer->type);
		Me.Gold += ItemPointer->multiplicity;
		DeleteItem(ItemPointer);
		return 0;
	}
	//--------------------
	// In the special case, that this is an item, that groups together with others
	// of the same type AND we also have as item of this type already in inventory,
	// then we just need to manipulate multiplicity a bit and we're done.  Very easy.
	//
	if (ItemMap[ItemPointer->type].item_group_together_in_inventory) {
		if (CountItemtypeInInventory(ItemPointer->type)) {
			TargetItemIndex = FindFirstInventoryIndexWithItemType(ItemPointer->type);
			Me.Inventory[TargetItemIndex].multiplicity += ItemPointer->multiplicity;
			play_item_sound(ItemPointer->type);
			DeleteItem(ItemPointer);
			return 0;
		}
	}
	//--------------------
	// Maybe the item is of a kind that can be equipped right now.  Then
	// we decide to directly drop it to the corresponding slot.
	//
	if ((Me.weapon_item.type == (-1)) && (ItemMap[ItemPointer->type].item_can_be_installed_in_weapon_slot)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			//--------------------
			// Now we're picking up a weapon while no weapon is equipped.  But still
			// it might be a 2-handed weapon while there is some shield equipped.  Well,
			// when that is the case, we refuse to put it directly to the proper slot, 
			// otherwise we do it.
			//
			if (Me.shield_item.type == (-1)) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.weapon_item), Inv_Loc);
				return 0;
			}
			//--------------------
			// So now we know that some shield item is equipped.  Let's be careful:  2-handed
			// weapons will be rejected from direct addition to the slot.
			//
			if (!ItemMap[ItemPointer->type].item_gun_requires_both_hands) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.weapon_item), Inv_Loc);
				return 0;
			}
		}
	}
	if ((Me.shield_item.type == (-1)) && (ItemMap[ItemPointer->type].item_can_be_installed_in_shield_slot)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			//--------------------
			// Auto-equipping shields can be done.  But only if there isn't a 2-handed
			// weapon equipped already.  Well, in case of no weapon present it's easy:
			//
			if (Me.weapon_item.type == (-1)) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.shield_item), Inv_Loc);
				return 0;
			}
			//--------------------
			// But now we know, that there is some weapon present.  We need to be careful:
			// it might be a 2-handed weapon.
			// 
			if (!ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
				raw_move_picked_up_item_to_entry(ItemPointer, &(Me.shield_item), Inv_Loc);
				return 0;
			}
		}
	}
	if ((Me.armour_item.type == (-1)) && (ItemMap[ItemPointer->type].item_can_be_installed_in_armour_slot)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.armour_item), Inv_Loc);
			return 0;
		}
	}
	if ((Me.drive_item.type == (-1)) && (ItemMap[ItemPointer->type].item_can_be_installed_in_drive_slot)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.drive_item), Inv_Loc);
			return 0;
		}
	}
	if ((Me.special_item.type == (-1)) && (ItemMap[ItemPointer->type].item_can_be_installed_in_special_slot)) {
		if (ItemUsageRequirementsMet(ItemPointer, TRUE)) {
			raw_move_picked_up_item_to_entry(ItemPointer, &(Me.special_item), Inv_Loc);
			return 0;
		}
	}
	// find a free position in the inventory list
	for (InvPos = 0; InvPos < MAX_ITEMS_IN_INVENTORY - 1; InvPos++) {
		if (Me.Inventory[InvPos].type == (-1))
			break;
	}

	//--------------------
	// Maybe the item in question is something, that would best be placed inside
	// the quick inventory.  If that is so, we try to put it there first.  If that
	// isn't possible, it can still be placed somewhere outside of the quick 
	// inventory later.
	//
	if ((ItemMap[ItemPointer->type].inv_image.inv_size.x == 1) &&
	    (ItemMap[ItemPointer->type].inv_image.inv_size.y == 1) && (ItemMap[ItemPointer->type].item_can_be_applied_in_combat)) {
		DebugPrintf(2, "\n\nTrying to place this item inside of the quick inventory first...");
		Inv_Loc.y = InventorySize.y - 1;
		for (Inv_Loc.x = 0; Inv_Loc.x < InventorySize.x - ItemMap[ItemPointer->type].inv_image.inv_size.x + 1; Inv_Loc.x++) {
			if (place_item_on_this_position_if_you_can(ItemPointer, Inv_Loc, InvPos))
				return 0;
		}
	}

	//--------------------
	// find enough free squares in the inventory to fit
	for (Inv_Loc.y = 0; Inv_Loc.y < InventorySize.y - ItemMap[ItemPointer->type].inv_image.inv_size.y + 1; Inv_Loc.y++) {
		for (Inv_Loc.x = 0; Inv_Loc.x < InventorySize.x - ItemMap[ItemPointer->type].inv_image.inv_size.x + 1; Inv_Loc.x++) {
			if (place_item_on_this_position_if_you_can(ItemPointer, Inv_Loc, InvPos))
				return 0;
		}
	}

	if (Me.Inventory[InvPos].inventory_position.x == (-1)) {
		Me.TextVisibleTime = 0;
		Me.TextToBeDisplayed = _("I can't carry any more.");
		CantCarrySound();
		return 1;
	} else {
		raw_move_picked_up_item_to_entry(ItemPointer, &(Me.Inventory[InvPos]), Inv_Loc);
		return 0;
	}

	return 0;
};				// void AddFloorItemDirectlyToInventory( item* ItemPointer )

int item_is_currently_equipped(item * Item)
{
	if ((&(Me.weapon_item) == Item) || (&(Me.drive_item) == Item) || (&(Me.armour_item) == Item)
	    || (&(Me.shield_item) == Item) || (&(Me.special_item) == Item))
		return 1;

	return 0;
}

/**
 * This function reads the descriptions of the different item prefixes
 * and suffixes that are used for magical items.
 */
int Get_Prefixes_Data(char *DataPointer)
{
	char *PrefixPointer;
	char *EndOfPrefixData;

#define PREFIX_SECTION_BEGIN_STRING "*** Start of presuff data section: ***"
#define PREFIX_SECTION_END_STRING "*** End of presuff data section ***"
#define NEW_PREFIX_BEGIN_STRING "** Start of new prefix specification subsection **"
#define NEW_SUFFIX_BEGIN_STRING "** Start of new suffix specification subsection **"

	PrefixPointer = LocateStringInData(DataPointer, PREFIX_SECTION_BEGIN_STRING);
	EndOfPrefixData = LocateStringInData(DataPointer, PREFIX_SECTION_END_STRING);

	int Number_Of_Prefixes = CountStringOccurences(DataPointer, NEW_PREFIX_BEGIN_STRING);
	int Number_Of_Suffixes = CountStringOccurences(DataPointer, NEW_SUFFIX_BEGIN_STRING);

	PrefixList = (item_bonus *) MyMalloc(sizeof(item_bonus) * (Number_Of_Prefixes + 1));
	SuffixList = (item_bonus *) MyMalloc(sizeof(item_bonus) * (Number_Of_Suffixes + 1));

	char *whattogrep = NEW_PREFIX_BEGIN_STRING;
	item_bonus *BonusToFill = PrefixList;
	int i = 0;
	for (i = 0; i < 2; i++) {
		if (i == 1)
			whattogrep = NEW_SUFFIX_BEGIN_STRING;
		while ((PrefixPointer = strstr(PrefixPointer, whattogrep)) != NULL) {
			PrefixPointer++;
			char *EndOfThisPrefix = strstr(PrefixPointer, whattogrep);
			if (EndOfThisPrefix)
				EndOfThisPrefix[0] = 0;

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to dexterity=", "%hd", "0",
						       &BonusToFill->base_bonus_to_dex, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to dexterity modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_dex, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to strength=", "%hd", "0",
						       &BonusToFill->base_bonus_to_str, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to strength modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_str, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to vitality=", "%hd", "0",
						       &BonusToFill->base_bonus_to_vit, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to vitality modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_vit, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to magic=", "%hd", "0",
						       &BonusToFill->base_bonus_to_mag, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to magic modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_mag, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to all attributes=", "%hd", "0",
						       &BonusToFill->base_bonus_to_all_attributes, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to all attributes modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_all_attributes, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to tohit=", "%hd", "0",
						       &BonusToFill->base_bonus_to_tohit, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to tohit modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_tohit, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to armor damage reduction or damage=", "%hd", "0",
						       &BonusToFill->base_bonus_to_damred_or_damage, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to armor damage reduction or damage modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_damred_or_damage, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to life=", "%hd", "0",
						       &BonusToFill->base_bonus_to_life, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to life modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_life, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to mana=", "%hd", "0",
						       &BonusToFill->base_bonus_to_force, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to mana modifier=", "%hd", "0",
						       &BonusToFill->modifier_to_bonus_to_force, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to life recovery=", "%f", "0.000000",
						       &BonusToFill->base_bonus_to_health_recovery, EndOfPrefixData);
			ReadValueFromStringWithDefault(PrefixPointer, "Bonus to mana recovery=", "%f", "0.000000",
						       &BonusToFill->base_bonus_to_cooling_rate, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Price factor=", "%f", "3.000000",
						       &BonusToFill->price_factor, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Light radius bonus=", "%hd", "0",
						       &BonusToFill->light_bonus_value, EndOfPrefixData);

			ReadValueFromStringWithDefault(PrefixPointer, "Level=", "%hd", "1", &BonusToFill->level, EndOfPrefixData);

			BonusToFill->bonus_name = ReadAndMallocStringFromData(PrefixPointer, "Prefix name=_\"", "\"");

			BonusToFill++;
			if (EndOfThisPrefix)
				EndOfThisPrefix[0] = '*';	// We put back the star at its place
		}
		PrefixPointer = LocateStringInData(DataPointer, PREFIX_SECTION_BEGIN_STRING);
		BonusToFill = SuffixList;
	}

	return 0;
}

#undef _items_c
