/* 
 *
 *   Copyright (c) 2009-2011 Arthur Huillet
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

#define _leveleditor_tile_lists_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_object_lists.h"

static int floor_tiles_array[] = {

	ISO_SIDEWALK_0,

	ISO_SIDEWALK_7,
	ISO_SIDEWALK_5,
	ISO_SIDEWALK_6,
	ISO_SIDEWALK_8,

	ISO_SIDEWALK_11,
	ISO_SIDEWALK_9,
	ISO_SIDEWALK_10,
	ISO_SIDEWALK_12,

	ISO_SIDEWALK_3,
	ISO_SIDEWALK_1,
	ISO_SIDEWALK_2,
	ISO_SIDEWALK_4,

	ISO_SIDEWALK_18,
	ISO_SIDEWALK_19,
	ISO_SIDEWALK_20,
	ISO_SIDEWALK_17,

	ISO_SIDEWALK_15,
	ISO_SIDEWALK_16,
	ISO_SIDEWALK_13,
	ISO_SIDEWALK_14,

	ISO_SIDEWALK_21,
	ISO_SIDEWALK_22,
	ISO_SIDEWALK_23,
	ISO_SIDEWALK_24,
	ISO_WATER_SIDEWALK_01,
	ISO_WATER_SIDEWALK_02,
	ISO_WATER_SIDEWALK_03,
	ISO_WATER_SIDEWALK_04,

	ISO_FLOOR_STONE_FLOOR,
	ISO_FLOOR_STONE_FLOOR_WITH_DOT,
	ISO_FLOOR_STONE_FLOOR_WITH_GRATE,

	ISO_MISCELLANEOUS_FLOOR_14,
	ISO_MISCELLANEOUS_FLOOR_10,
	ISO_MISCELLANEOUS_FLOOR_11,
	ISO_MISCELLANEOUS_FLOOR_12,
	ISO_MISCELLANEOUS_FLOOR_13,

	ISO_MISCELLANEOUS_FLOOR_23,
	ISO_MISCELLANEOUS_FLOOR_17,
	ISO_MISCELLANEOUS_FLOOR_18,
	ISO_MISCELLANEOUS_FLOOR_15,
	ISO_MISCELLANEOUS_FLOOR_16,

	ISO_WATER,
	ISO_WATER_EDGE_1,
	ISO_WATER_EDGE_2,
	ISO_WATER_EDGE_3,
	ISO_WATER_EDGE_4,
	ISO_WATER_EDGE_5,
	ISO_WATER_EDGE_6,
	ISO_WATER_EDGE_7,
	ISO_WATER_EDGE_8,
	ISO_WATER_EDGE_9,
	ISO_WATER_EDGE_10,
	ISO_WATER_EDGE_11,
	ISO_WATER_EDGE_12,
	ISO_WATER_EDGE_13,
	ISO_WATER_EDGE_14,

	ISO_MISCELLANEOUS_FLOOR_9,
	ISO_COMPLETELY_DARK,
	ISO_FLOOR_ERROR_TILE,
	ISO_RED_WAREHOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_21,
	ISO_MISCELLANEOUS_FLOOR_22,
	ISO_FLOOR_HOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_19,
	ISO_MISCELLANEOUS_FLOOR_20,

	ISO_SAND_FLOOR_4,
	ISO_SAND_FLOOR_5,
	ISO_SAND_FLOOR_6,
	ISO_FLOOR_SAND,
	ISO_SAND_FLOOR_1,
	ISO_SAND_FLOOR_2,
	ISO_SAND_FLOOR_3,

	ISO_FLOOR_SAND_WITH_GRASS_14,
	ISO_FLOOR_SAND_WITH_GRASS_15,
	ISO_FLOOR_SAND_WITH_GRASS_16,
	ISO_FLOOR_SAND_WITH_GRASS_17,

	ISO_FLOOR_SAND_WITH_GRASS_1,
	ISO_FLOOR_SAND_WITH_GRASS_2,
	ISO_FLOOR_SAND_WITH_GRASS_3,
	ISO_FLOOR_SAND_WITH_GRASS_4,
	ISO_FLOOR_SAND_WITH_GRASS_5,
	ISO_FLOOR_SAND_WITH_GRASS_25,
	ISO_FLOOR_SAND_WITH_GRASS_26,
	ISO_FLOOR_SAND_WITH_GRASS_27,

	ISO_FLOOR_SAND_WITH_GRASS_20,
	ISO_FLOOR_SAND_WITH_GRASS_19,
	ISO_FLOOR_SAND_WITH_GRASS_18,
	ISO_FLOOR_SAND_WITH_GRASS_21,

	ISO_FLOOR_SAND_WITH_GRASS_6,
	ISO_FLOOR_SAND_WITH_GRASS_7,
	ISO_FLOOR_SAND_WITH_GRASS_8,
	ISO_FLOOR_SAND_WITH_GRASS_9,
	ISO_FLOOR_SAND_WITH_GRASS_10,
	ISO_FLOOR_SAND_WITH_GRASS_11,
	ISO_FLOOR_SAND_WITH_GRASS_12,
	ISO_FLOOR_SAND_WITH_GRASS_13,

	ISO_FLOOR_SAND_WITH_GRASS_22,
	ISO_FLOOR_SAND_WITH_GRASS_23,
	ISO_FLOOR_SAND_WITH_GRASS_24,
	ISO_FLOOR_SAND_WITH_GRASS_28,
	ISO_FLOOR_SAND_WITH_GRASS_29,

	ISO_CARPET_TILE_0001,
	ISO_CARPET_TILE_0002,
	ISO_CARPET_TILE_0003,
	ISO_CARPET_TILE_0004,
	ISO_LARGE_SQUARE_BBB,
	ISO_LARGE_SQUARE_BRB,
	ISO_LARGE_SQUARE_BRR,
	ISO_LARGE_SQUARE_GBB,
	ISO_LARGE_SQUARE_GRB,
	ISO_LARGE_SQUARE_GRR,
	ISO_LARGE_SQUARE_RBB,
	ISO_LARGE_SQUARE_RRB,
	ISO_LARGE_SQUARE_RRR,
	ISO_MINI_SQUARE_0001,
	ISO_MINI_SQUARE_0002,
	ISO_MINI_SQUARE_0003,
	ISO_MINI_SQUARE_0004,
	ISO_MINI_SQUARE_0005,
	ISO_MINI_SQUARE_0006,
	ISO_MINI_SQUARE_0007,
	ISO_MINI_SQUARE_0008,
	ISO_SQUARE_TILE_AAB,
	ISO_SQUARE_TILE_ACB2,
	ISO_SQUARE_TILE_ACB,
	ISO_SQUARE_TILE_ADB2,
	ISO_SQUARE_TILE_ADB,
	ISO_SQUARE_TILE_CAB2,
	ISO_SQUARE_TILE_CAB,
	ISO_SQUARE_TILE_CCB,
	ISO_SQUARE_TILE_DAB2,
	ISO_SQUARE_TILE_DAB,
	ISO_SQUARE_TILE_DDB,

	ISO_COMPLICATED_CMM,
	ISO_COMPLICATED_CMM2,
	ISO_COMPLICATED_P4,
	ISO_COMPLICATED_PMG,
	ISO_COMPLICATED_PMG2,
	ISO_COMPLICATED_PMM,

	ISO_TWOSQUARE_0001,
	ISO_TWOSQUARE_0002,
	ISO_TWOSQUARE_0003,
	-1
};

static int wall_tiles_array[] = {
	ISO_TRANSP_FOR_WATER,

	ISO_OUTER_DOOR_V_00,
	ISO_OUTER_DOOR_H_00,
	ISO_OUTER_DOOR_V_LOCKED,
	ISO_OUTER_DOOR_H_LOCKED,
	ISO_OUTER_DOOR_V_OFFLINE,
	ISO_OUTER_DOOR_H_OFFLINE,
	ISO_OUTER_WALL_W1,
	ISO_OUTER_WALL_N1,
	ISO_OUTER_WALL_W2,
	ISO_OUTER_WALL_N2,
	ISO_OUTER_WALL_W3,
	ISO_OUTER_WALL_N3,
	ISO_OUTER_WALL_E1,
	ISO_OUTER_WALL_S1,
	ISO_OUTER_WALL_E2,
	ISO_OUTER_WALL_S2,
	ISO_OUTER_WALL_E3,
	ISO_OUTER_WALL_S3,
	ISO_OUTER_WALL_CORNER_NW,
	ISO_OUTER_WALL_CORNER_NE,
	ISO_OUTER_WALL_CORNER_SE,
	ISO_OUTER_WALL_CORNER_SW,
	ISO_OUTER_WALL_SMALL_CORNER_1,
	ISO_OUTER_WALL_SMALL_CORNER_4,
	ISO_OUTER_WALL_SMALL_CORNER_3,
	ISO_OUTER_WALL_SMALL_CORNER_2,
	ISO_V_DOOR_000_OPEN,
	ISO_H_DOOR_000_OPEN,
	ISO_DV_DOOR_000_OPEN,
	ISO_DH_DOOR_000_OPEN,
	ISO_V_DOOR_LOCKED,
	ISO_H_DOOR_LOCKED,
	ISO_DV_DOOR_LOCKED,
	ISO_DH_DOOR_LOCKED,
	ISO_V_WALL,
	ISO_H_WALL,
	ISO_V_WALL_WITH_DOT,
	ISO_H_WALL_WITH_DOT,
	ISO_GREY_WALL_END_W,
	ISO_GREY_WALL_END_N,
	ISO_GREY_WALL_END_E,
	ISO_GREY_WALL_END_S,
	ISO_ROOM_WALL_V_RED,
	ISO_ROOM_WALL_H_RED,
	ISO_RED_WALL_WINDOW_1,
	ISO_RED_WALL_WINDOW_2,
	ISO_ROOM_WALL_V_GREEN,
	ISO_ROOM_WALL_H_GREEN,
	ISO_CYAN_WALL_WINDOW_1,
	ISO_CYAN_WALL_WINDOW_2,
	ISO_LIGHT_GREEN_WALL_1,
	ISO_LIGHT_GREEN_WALL_2,
	ISO_FLOWER_WALL_WINDOW_1,
	ISO_FLOWER_WALL_WINDOW_2,
	ISO_FUNKY_WALL_1,
	ISO_FUNKY_WALL_2,
	ISO_FUNKY_WALL_3,
	ISO_FUNKY_WALL_4,
	ISO_FUNKY_WALL_WINDOW_1,
	ISO_FUNKY_WALL_WINDOW_2,
	ISO_CAVE_WALL_V,
	ISO_CAVE_WALL_H,
	ISO_CAVE_CORNER_NW,
	ISO_CAVE_CORNER_NE,
	ISO_CAVE_CORNER_SE,
	ISO_CAVE_CORNER_SW,
	ISO_CAVE_WALL_END_E,
	ISO_CAVE_WALL_END_S,
	ISO_CAVE_WALL_END_W,
	ISO_CAVE_WALL_END_N,
	ISO_V_WOOD_FENCE,
	ISO_H_WOOD_FENCE,
	ISO_V_MESH_FENCE,
	ISO_H_MESH_FENCE,
	ISO_V_WIRE_FENCE,
	ISO_H_WIRE_FENCE,
	ISO_GLASS_WALL_1,
	ISO_BROKEN_GLASS_WALL_1,
	ISO_GLASS_WALL_2,
	ISO_BROKEN_GLASS_WALL_2,
	ISO_THICK_WALL_V,
	ISO_THICK_WALL_H,
	ISO_THICK_WALL_CORNER_NW,
	ISO_THICK_WALL_CORNER_NE,
	ISO_THICK_WALL_CORNER_SE,
	ISO_THICK_WALL_CORNER_SW,
	ISO_THICK_WALL_T_W,
	ISO_THICK_WALL_T_N,
	ISO_THICK_WALL_T_E,
	ISO_THICK_WALL_T_S,
	ISO_BRICK_WALL_H,
	ISO_BRICK_WALL_V,
	ISO_BRICK_WALL_EH,
	ISO_BRICK_WALL_EV,
	ISO_BRICK_WALL_END,
	ISO_BRICK_WALL_CORNER_NW,
	ISO_BRICK_WALL_CORNER_NE,
	ISO_BRICK_WALL_CORNER_SE,
	ISO_BRICK_WALL_CORNER_SW,
	ISO_BRICK_WALL_JUNCTION_4,
	ISO_BRICK_WALL_JUNCTION_1,
	ISO_BRICK_WALL_JUNCTION_2,
	ISO_BRICK_WALL_JUNCTION_3,
	ISO_BRICK_WALL_CRACKED_1,
	ISO_BRICK_WALL_CRACKED_2,
	ISO_BRICK_WALL_RUBBLE_1,
	ISO_BRICK_WALL_RUBBLE_2,
	ISO_BRICK_WALL_CABLES_V,
	ISO_BRICK_WALL_CABLES_H,
	ISO_BRICK_WALL_CABLES_CORNER_SE,
	ISO_BRICK_WALL_CABLES_CORNER_NW,
	ISO_BRICK_WALL_CABLES_CORNER_SW,
	ISO_BRICK_WALL_CABLES_CORNER_NE,
	ISO_RED_FENCE_V,
	ISO_RED_FENCE_H,
	ISO_V_CURTAIN,
	ISO_H_CURTAIN,
	ISO_GREY_WALL_CORNER_1,
	ISO_GREY_WALL_CORNER_2,
	ISO_GREY_WALL_CORNER_3,
	ISO_GREY_WALL_CORNER_4,
	ISO_V_DENSE_FENCE,
	ISO_H_DENSE_FENCE,

	-1
};

static int machinery_tiles_array[] = {
	ISO_ENHANCER_RU,
	ISO_ENHANCER_LU,
	ISO_ENHANCER_RD,
	ISO_ENHANCER_LD,

	ISO_TELEPORTER_3,

	ISO_REFRESH_3,

	ISO_AUTOGUN_W,
	ISO_AUTOGUN_N,
	ISO_AUTOGUN_E,
	ISO_AUTOGUN_S,

	ISO_DIS_AUTOGUN_W,
	ISO_DIS_AUTOGUN_N,
	ISO_DIS_AUTOGUN_E,
	ISO_DIS_AUTOGUN_S,

	ISO_COOKING_POT,

	ISO_CONSOLE_S,
	ISO_CONSOLE_E,
	ISO_CONSOLE_N,
	ISO_CONSOLE_W,
	
	ISO_WALL_TERMINAL_S,
	ISO_WALL_TERMINAL_E,
	ISO_WALL_TERMINAL_N,
	ISO_WALL_TERMINAL_W,

	ISO_TV_PILLAR_W,
	ISO_TV_PILLAR_N,
	ISO_TV_PILLAR_E,
	ISO_TV_PILLAR_S,

	ISO_PROJECTOR_N,
	ISO_PROJECTOR_E,
	ISO_PROJECTOR_S,
	ISO_PROJECTOR_W,

	ISO_SIGN_1,
	ISO_SIGN_2,
	ISO_SIGN_3,

	ISO_EXIT_2,
	ISO_EXIT_1,
	ISO_EXIT_4,
	ISO_EXIT_3,
	ISO_EXIT_5,
	ISO_EXIT_6,

	ISO_LADDER_1,
	ISO_LADDER_2,

	ISO_SOLAR_PANEL_BROKEN,
	ISO_SOLAR_PANEL_E,
	ISO_BOTLINE_01_N,
	ISO_BOTLINE_01_E,
	ISO_BOTLINE_02_E,
	ISO_BOTLINE_02_N,
	ISO_BOTLINE_02_W,
	ISO_BOTLINE_02_S,
	ISO_BOTLINE_06_W,
	ISO_BOTLINE_06_N,
	ISO_BOTLINE_06_E,
	ISO_BOTLINE_06_S,
	ISO_BOTLINE_07_W,
	ISO_BOTLINE_07_N,
	ISO_BOTLINE_07_E,
	ISO_BOTLINE_07_S,
	ISO_BOTLINE_08_W,
	ISO_BOTLINE_08_N,
	ISO_BOTLINE_08_E,
	ISO_BOTLINE_08_S,

	ISO_FREIGHTER_RAILWAY_01_N,
	ISO_FREIGHTER_RAILWAY_01_E,
	ISO_FREIGHTER_RAILWAY_02_S,
	ISO_FREIGHTER_RAILWAY_02_E,
	ISO_FREIGHTER_RAILWAY_02_N,
	ISO_FREIGHTER_RAILWAY_02_W,

	ISO_TURBINES_SMALL_W,
	ISO_TURBINES_SMALL_N,
	ISO_TURBINES_SMALL_E,
	ISO_TURBINES_SMALL_S,

	ISO_REACTOR_S,
	ISO_REACTOR_E,
	ISO_REACTOR_N,
	ISO_REACTOR_W,

	ISO_SECURITY_GATE_GREEN_E,
	ISO_SECURITY_GATE_GREEN_S,
	ISO_SECURITY_GATE_RED_E,
	ISO_SECURITY_GATE_RED_S,
	ISO_SECURITY_GATE_OPEN_E,
	ISO_SECURITY_GATE_OPEN_S,
	ISO_SECURITY_GATE_CLOSED_E,
	ISO_SECURITY_GATE_CLOSED_S,

	-1,
};

static int furniture_tiles_array[] = {
	ISO_LAMP_N,
	ISO_LAMP_E,
	ISO_LAMP_S,
	ISO_LAMP_W,

	ISO_N_TOILET_SMALL,
	ISO_E_TOILET_SMALL,

	ISO_E_TOILET_WHITE_SMALL,
	ISO_N_TOILET_WHITE_SMALL,
	ISO_W_TOILET_WHITE_SMALL,
	ISO_S_TOILET_WHITE_SMALL,

	ISO_N_TOILET_BIG,
	ISO_E_TOILET_BIG,
	ISO_S_TOILET_BIG,
	ISO_W_TOILET_BIG,

	ISO_N_CHAIR,
	ISO_E_CHAIR,
	ISO_S_CHAIR,
	ISO_W_CHAIR,

	ISO_N_DESK,
	ISO_E_DESK,
	ISO_S_DESK,
	ISO_W_DESK,

	ISO_N_SCHOOL_CHAIR,
	ISO_E_SCHOOL_CHAIR,
	ISO_S_SCHOOL_CHAIR,
	ISO_W_SCHOOL_CHAIR,

	ISO_DESKCHAIR_1,
	ISO_DESKCHAIR_2,
	ISO_DESKCHAIR_3,

	ISO_N_BED,
	ISO_E_BED,
	ISO_S_BED,
	ISO_W_BED,

	ISO_N_FULL_PARK_BENCH,
	ISO_E_FULL_PARK_BENCH,
	ISO_S_FULL_PARK_BENCH,
	ISO_W_FULL_PARK_BENCH,

	ISO_H_BATHTUB,
	ISO_V_BATHTUB,
	ISO_3_BATHTUB,
	ISO_4_BATHTUB,

	ISO_H_WASHTUB,
	ISO_V_WASHTUB,

	ISO_BASIN_1,
	ISO_BASIN_2,
	ISO_BASIN_3,
	ISO_BASIN_4,

	ISO_W_SOFA,
	ISO_S_SOFA,
	ISO_N_SOFA,
	ISO_E_SOFA,

	ISO_TABLE_OVAL_1,
	ISO_TABLE_OVAL_2,
	ISO_TABLE_GLASS_1,
	ISO_TABLE_GLASS_2,

	ISO_SHOP_FURNITURE_1,
	ISO_SHOP_FURNITURE_2,
	ISO_SHOP_FURNITURE_3,
	ISO_SHOP_FURNITURE_4,
	ISO_SHOP_FURNITURE_5,
	ISO_SHOP_FURNITURE_6,

	ISO_LIBRARY_FURNITURE_1,
	ISO_LIBRARY_FURNITURE_2,

	ISO_YELLOW_CHAIR_N,
	ISO_YELLOW_CHAIR_E,
	ISO_YELLOW_CHAIR_S,
	ISO_YELLOW_CHAIR_W,

	ISO_RED_CHAIR_N,
	ISO_RED_CHAIR_E,
	ISO_RED_CHAIR_S,
	ISO_RED_CHAIR_W,

	ISO_SOFFA_1,
	ISO_SOFFA_2,
	ISO_SOFFA_3,
	ISO_SOFFA_4,

	ISO_SOFFA_CORNER_1,
	ISO_SOFFA_CORNER_2,
	ISO_SOFFA_CORNER_3,
	ISO_SOFFA_CORNER_4,

	ISO_SOFFA_CORNER_PLANT_1,
	ISO_SOFFA_CORNER_PLANT_2,
	ISO_SOFFA_CORNER_PLANT_3,
	ISO_SOFFA_CORNER_PLANT_4,

	ISO_CONFERENCE_TABLE_N,
	ISO_CONFERENCE_TABLE_E,
	ISO_CONFERENCE_TABLE_S,
	ISO_CONFERENCE_TABLE_W,

	ISO_PROJECTOR_SCREEN_N,
	ISO_PROJECTOR_SCREEN_E,
	ISO_PROJECTOR_SCREEN_S,
	ISO_PROJECTOR_SCREEN_W,

	ISO_BED_1,
	ISO_BED_2,
	ISO_BED_3,
	ISO_BED_4,
	ISO_BED_5,
	ISO_BED_6,
	ISO_BED_7,
	ISO_BED_8,

	ISO_SHELF_FULL_V,
	ISO_SHELF_FULL_H,
	ISO_SHELF_EMPTY_V,
	ISO_SHELF_EMPTY_H,

	ISO_SHELF_SMALL_FULL_V,
	ISO_SHELF_SMALL_FULL_H,
	ISO_SHELF_SMALL_EMPTY_V,
	ISO_SHELF_SMALL_EMPTY_H,

	ISO_RESTAURANT_SHELVES_1,
	ISO_RESTAURANT_SHELVES_2,
	ISO_RESTAURANT_SHELVES_3,
	ISO_RESTAURANT_SHELVES_4,
	ISO_RESTAURANT_SHELVES_5,
	ISO_RESTAURANT_SHELVES_6,
	ISO_RESTAURANT_SHELVES_7,
	ISO_RESTAURANT_SHELVES_8,
	ISO_RESTAURANT_SHELVES_9,
	ISO_RESTAURANT_SHELVES_10,

	ISO_COUNTER_MIDDLE_1,
	ISO_COUNTER_MIDDLE_2,
	ISO_COUNTER_MIDDLE_3,
	ISO_COUNTER_MIDDLE_4,

	ISO_COUNTER_CORNER_ROUND_1,
	ISO_COUNTER_CORNER_ROUND_2,
	ISO_COUNTER_CORNER_ROUND_3,
	ISO_COUNTER_CORNER_ROUND_4,
	ISO_COUNTER_CORNER_SHARP_1,
	ISO_COUNTER_CORNER_SHARP_2,
	ISO_COUNTER_CORNER_SHARP_3,
	ISO_COUNTER_CORNER_SHARP_4,

	ISO_BAR_TABLE,

	ISO_RESTAURANT_DESK_1,
	ISO_RESTAURANT_DESK_2,

	ISO_RESTAURANT_BIGSHELF_1,
	ISO_RESTAURANT_BIGSHELF_2,

	ISO_PILLAR_TALL,

	ISO_PILLAR_SHORT,

	ISO_V_CURTAIN,
	ISO_H_CURTAIN,

	-1,
};

static int container_tiles_array[] = {
	ISO_H_CHEST_CLOSED,
	ISO_H_CHEST_OPEN,

	ISO_V_CHEST_CLOSED,
	ISO_V_CHEST_OPEN,

	ISO_W_CHEST2_CLOSED,
	ISO_W_CHEST2_OPEN,
	ISO_E_CHEST2_CLOSED,
	ISO_E_CHEST2_OPEN,

	ISO_S_CHEST2_CLOSED,
	ISO_S_CHEST2_OPEN,
	ISO_N_CHEST2_CLOSED,
	ISO_N_CHEST2_OPEN,

	ISO_BARREL_1,
	ISO_BARREL_2,
	ISO_BARREL_3,
	ISO_BARREL_4,
	ISO_BARREL_5,

	ISO_WEAPON_CRATE,

	-1,
};

static int plant_tiles_array[] = {
	ISO_TREE_1,
	ISO_TREE_2,
	ISO_TREE_3,
	ISO_TREE_5,
	ISO_TREE_4,

	ISO_ROCKS_N_PLANTS_1,
	ISO_ROCKS_N_PLANTS_8,
	ISO_ROCKS_N_PLANTS_5,
	ISO_ROCKS_N_PLANTS_6,
	ISO_ROCKS_N_PLANTS_7,

	ISO_ROCKS_N_PLANTS_3,
	ISO_ROCKS_N_PLANTS_2,
	ISO_ROCKS_N_PLANTS_4,

	ISO_CRYSTALS_1,
	ISO_CRYSTALS_3,
	ISO_CRYSTALS_4,
	ISO_CRYSTALS_6,
	ISO_CRYSTALS_2,
	ISO_CRYSTALS_5,

	-1,
};

static int misc_tiles_array[] = {
	ISO_DEFAULT_DEAD_BODY_0_1,

	ISO_DEFAULT_DEAD_BODY_1_1,
	ISO_DEFAULT_DEAD_BODY_2_1,
	ISO_DEFAULT_DEAD_BODY_3_1,
	ISO_DEFAULT_DEAD_BODY_4_1,
	ISO_DEFAULT_DEAD_BODY_5_1,
	ISO_DEFAULT_DEAD_BODY_6_1,
	ISO_DEFAULT_DEAD_BODY_7_1,
	ISO_DEFAULT_DEAD_BODY_0_2,

	ISO_WRECKED_CAR_1,
	ISO_WRECKED_CAR_2,
	ISO_WRECKED_CAR_3,
	ISO_WRECKED_CAR_4,
	
	ISO_CRUSHED_476,

	-1
};

static int waypoint_array[] = {
	0,			//random spawn
	1,			//no random spawn 
	-1
};
	
int *floor_tiles_list = floor_tiles_array;
int *wall_tiles_list = wall_tiles_array;
int *machinery_tiles_list = machinery_tiles_array;
int *furniture_tiles_list = furniture_tiles_array;
int *container_tiles_list = container_tiles_array;
int *plant_tiles_list = plant_tiles_array;
int *misc_tiles_list = misc_tiles_array;
int *waypoint_list = waypoint_array;

int *sidewalk_floor_list;
int *water_floor_list;
int *grass_floor_list;
int *square_floor_list;
int *other_floor_list;
	
int *melee_items_list;
int *gun_items_list;
int *defense_items_list;
int *spell_items_list;
int *other_items_list;
int *all_items_list;


static void build_floor_tile_lists(void)
{
	int i;
	int sidewalk = 0;
	int water    = 0;
	int grass    = 0;
	int square   = 0;
	int other    = 0;

	free(sidewalk_floor_list); //Sidewalk Tiles
	free(water_floor_list);    //Water Tiles
	free(grass_floor_list);    //Grass Tiles
	free(square_floor_list);   //Square Tiles - Geometric Patterned
	free(other_floor_list);    //OTHER: Dirt, Sand, Carpet, Misc, etc.

	sidewalk_floor_list = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	water_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	grass_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	square_floor_list   = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	other_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));

	for (i = 0; i < ALL_ISOMETRIC_FLOOR_TILES; i++) {
		if (strstr(floor_tile_filenames[i], "sidewalk")) {
			sidewalk_floor_list[sidewalk] = i;
			sidewalk++;
			if (strstr(floor_tile_filenames[i], "water")) { //Water-Sidewalk tiles should be in both
				water_floor_list[water] = i;
				water++;
			}
		} else if (strstr(floor_tile_filenames[i], "water")) {
			water_floor_list[water] = i;
			water++;
		} else if (strstr(floor_tile_filenames[i], "grass")) {
			grass_floor_list[grass] = i;
			grass++;
		} else if (strstr(floor_tile_filenames[i], "square")) {
			square_floor_list[square] = i;
			square++;
		} else {
			other_floor_list[other] = i;
			other++;
		}
	}

	sidewalk_floor_list[sidewalk] = -1;
	water_floor_list[water]       = -1;
	grass_floor_list[grass]       = -1;
	square_floor_list[square]     = -1;
	other_floor_list[other]       = -1;

}

static void build_item_lists(void)
{
	int i;
	int melee   = 0;
	int guns    = 0;
	int defense = 0;
	int spell   = 0;
	int other       = 0;

	free(melee_items_list);  //MELEE WEAPONS
	free(gun_items_list);    //GUNS
	free(defense_items_list);//ARMOR, SHIELDS, & SHOES
	free(spell_items_list);  //SPELL-LIKE: Grenades, Spell Books, Pills, & Potions
	free(other_items_list);  //OTHER: Ammo, Circuts, Plot Items, etc.
	free(all_items_list);    //EVERYTHING

	melee_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	gun_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	defense_items_list = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	spell_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	other_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	all_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));

	for (i = 1; i < Number_Of_Item_Types; i++) { //Start at 1 to skip the CTD item
		if (ItemMap[i].item_weapon_is_melee) {
			melee_items_list[melee] = i;
			melee++;
		} else if (ItemMap[i].item_can_be_installed_in_weapon_slot) {
			gun_items_list[guns] = i;
			guns++;
		} else if (ItemMap[i].item_can_be_installed_in_drive_slot || ItemMap[i].item_can_be_installed_in_armour_slot || 
				ItemMap[i].item_can_be_installed_in_shield_slot || ItemMap[i].item_can_be_installed_in_special_slot) {
			defense_items_list[defense] = i;
			defense++;
		} else if (ItemMap[i].item_combat_use_description) {
			spell_items_list[spell] = i;
			spell++;
		} else {
			other_items_list[other] = i;
			other++;
		}
		all_items_list[i] = i;
	}
	melee_items_list[melee]     = -1;
	gun_items_list[guns]        = -1;
	defense_items_list[defense] = -1;
	spell_items_list[spell]     = -1;
	other_items_list[other]     = -1;
	all_items_list[i]           = -1;

}

/**
 * This function builds all the lists of objects belonging to the various categories.
 */
void build_leveleditor_tile_lists(void)
{
	build_floor_tile_lists();
	build_item_lists();
}
