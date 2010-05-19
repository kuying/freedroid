/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

/*
 * This file has been checked for remaining german comments. However, if you still
 * find some, please either translate them to english or tell me where you found them.
 * Thanks a lot, Johannes Prix.
 */

// This stuff is nescessary, so that global.h and proto.h know that they have been
// included this time from the file map.h
#ifndef _map_h
#define _map_h

/* some defines */

#define CONNECTION_STRING	"c: "
#define WHITE_SPACE 		" \t"
#define MAP_BEGIN_STRING	"beginning_of_map"
#define WP_BEGIN_STRING		"wp"
#define LEVEL_END_STRING	"end_of_level"
#define WP_END_STRING		LEVEL_END_STRING
#define LEVEL_HEADER_LEVELNUMBER "Levelnumber:"

#define LEVEL_NAME_STRING "Name of this level=_\""
#define LEVEL_NAME_STRING_LEGACY "Name of this level="
#define BACKGROUND_SONG_NAME_STRING "BgSong="
#define MAP_END_STRING "/pmapinfolvl"

#define ITEMS_SECTION_BEGIN_STRING "piteminfolvl"
#define ITEMS_SECTION_END_STRING "/piteminfolvl"
#define ITEM_NAME_STRING "it: n=\""
#define ITEM_POS_X_STRING " X="
#define ITEM_POS_Y_STRING " Y="
#define ITEM_DAMRED_BONUS_STRING " DAMRED="
#define ITEM_DAMAGE_STRING " Dmg="
#define ITEM_DAMAGE_MODIFIER_STRING " DmgMod="
#define ITEM_MAX_DURATION_STRING " MDur="
#define ITEM_CUR_DURATION_STRING " CDur="
#define ITEM_AMMO_CLIP_STRING " AClip="
#define ITEM_MULTIPLICITY_STRING " Multi="
#define ITEM_PREFIX_CODE_STRING " Prefix="
#define ITEM_SUFFIX_CODE_STRING " Suffix="
#define ITEM_BONUS_TO_STR_STRING " StrBon="
#define ITEM_BONUS_TO_DEX_STRING " StrDex="
#define ITEM_BONUS_TO_VIT_STRING " StrVit="
#define ITEM_BONUS_TO_MAG_STRING " StrMag="
#define ITEM_BONUS_TO_ALLATT_STRING " StrAllAtt="
#define ITEM_BONUS_TO_LIFE_STRING " StrLife="
#define ITEM_BONUS_TO_FORCE_STRING " StrForce="
#define ITEM_BONUS_TO_HEALTH_RECOVERY_STRING " StrHealthRecovery="
#define ITEM_BONUS_TO_MANA_RECOVERY_STRING " StrManaRecovery="
#define ITEM_BONUS_TO_TOHIT_STRING " StrToHit="
#define ITEM_BONUS_TO_DAMREDDAM_STRING " StrDAMREDDam="
#define ITEM_BONUS_TO_RESELE_STRING " ResEle="
#define ITEM_BONUS_TO_RESFIR_STRING " ResFir="
#define ITEM_IS_IDENTIFIED_STRING " IsIdent="

#define OBSTACLE_EXTENSIONS_BEGIN_STRING "obstacleextensions"
#define OBSTACLE_EXTENSIONS_END_STRING "/obstacleextensions"

#define STATEMENT_BEGIN_STRING "pstmtinfolvl"
#define STATEMENT_END_STRING "/pstmtinfolvl"
#define X_POSITION_OF_STATEMENT_STRING "X="
#define Y_POSITION_OF_STATEMENT_STRING "Y="
#define STATEMENT_ITSELF_ANNOUNCE_STRING "Msg=\""

#define MAP_LABEL_BEGIN_STRING "plabelinfolvl"
#define MAP_LABEL_END_STRING "/plabelinfolvl"
#define X_POSITION_OF_LABEL_STRING "x="
#define Y_POSITION_OF_LABEL_STRING "y="
#define LABEL_ITSELF_ANNOUNCE_STRING "name=\""

#define OBSTACLE_LABEL_BEGIN_STRING "pobstaclelabelinfolvl"
#define OBSTACLE_LABEL_END_STRING "/pobstaclelabelinfolvl"
#define OBSTACLE_LABEL_ANNOUNCE_STRING "obstacle_label_name=\""
#define OBSTACLE_LABEL_STATES_STRING "states=\""
#define INDEX_OF_OBSTACLE_NAME "obstacle_label.index="

#define BIG_MAP_INSERT_SECTION_BEGIN_STRING "Start of big graphics insert information for this level"
#define BIG_MAP_INSERT_SECTION_END_STRING "End of big graphics insert information for this level"
#define POSITION_X_OF_BIG_MAP_INSERT_STRING "BigGraphicsInsertPosX="
#define POSITION_Y_OF_BIG_MAP_INSERT_STRING "BigGraphicsInsertPosY="
#define BIG_MAP_INSERT_TYPE_STRING "BigGraphicsInsertType="

#define SPECIAL_FORCE_INDICATION_STRING "SForce: T="

#define OBSTACLE_DATA_BEGIN_STRING "obsdata"
#define OBSTACLE_DATA_END_STRING "/obsdata"
#define OBSTACLE_TYPE_STRING "t"
#define OBSTACLE_X_POSITION_STRING "x"
#define OBSTACLE_Y_POSITION_STRING "y"

//--------------------
// How big is the margin the walls have as far as collisions are concerned.
//
#define WALLPASS (4.0/64.0)

//--------------------
// How big is the margin in console tiles
//
#define CONSOLEPASS_X 	((INITIAL_BLOCK_WIDTH/2 + 4)/64.0)
#define CONSOLEPASS_Y 	((INITIAL_BLOCK_HEIGHT/2 + 4)/64.0)

//--------------------
// How big is the margin of doors and the passable area in front of doors
//
#define TUERBREITE	(6/64.0)
#define V_RANDSPACE		WALLPASS
#define V_RANDBREITE		(5/64.0)
#define H_RANDSPACE		WALLPASS
#define H_RANDBREITE		(5/64.0)

enum _colornames {
	PD_RED,
	PD_YELLOW,
	PD_GREEN,
	PD_GRAY,
	PD_BLUE,
	PD_GREENBLUE,
	PD_DARK
};

#endif
