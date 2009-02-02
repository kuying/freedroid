/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2007 Arthur Huillet
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
 * This file contains all functions for the heart of the level editor.
 */

#define _leveleditor_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "leveleditor.h"
#include "leveleditor_validator.h"

#include "leveleditor_actions.h"
#include "leveleditor_display.h"
#include "leveleditor_grass_actions.h"
#include "leveleditor_input.h"
#include "leveleditor_map.h"
#include "leveleditor_menu.h"

int OriginWaypoint = (-1);

char VanishingMessage[10000]="";
float VanishingMessageEndDate = 0;
int FirstBlock = 0 ;
int *object_list;
int number_of_walls [ NUMBER_OF_LEVEL_EDITOR_GROUPS ] ;
int level_editor_done = FALSE;

LIST_HEAD (quickbar_entries);


/**
 * Return the X coordinate of the block we are on.
 */
int EditX(void) 
{
    int BlockX = rintf ( Me . pos . x - 0.5 );
    if (BlockX < 0) {
	BlockX = 0 ;
	Me . pos . x = 0.51 ;
	}
    return BlockX;
}

/**
 * Return the Y coordinate of the block we are on.
 */
int EditY(void)
{
    int BlockY = rintf ( Me . pos . y - 0.5 );
    if (BlockY < 0) {
	BlockY = 0 ;
	Me . pos . y = 0.51 ;
	}
    return BlockY;
}

/**
 * Return a pointer to the level we are currently editing.
 */
level *EditLevel(void)
{
    return CURLEVEL();
}

iso_image *leveleditor_get_object_image(enum leveleditor_object_type type, int * array, int idx)
{
    switch(type) {
	case OBJECT_FLOOR:
	    return &(floor_iso_images[array[idx]]);
	case OBJECT_OBSTACLE:
	    return &(obstacle_map[array[idx]].image);
	case OBJECT_NPC:
	case OBJECT_ANY:
	    ErrorMessage(__FUNCTION__, "Abstract object type %d for leveleditor not supported.\n", PLEASE_INFORM, IS_FATAL, type);
	    break;
    }

    return NULL;
}

/**
 *
 *
 */
void level_editor_cycle_marked_obstacle()
{
    int current_mark_index ;
    int j;

    if ( level_editor_marked_obstacle != NULL )
    {
	//--------------------
	// See if this floor tile has some other obstacles glued to it as well
	//
	if ( EditLevel() -> map [ EditY() ] [ EditX() ] . obstacles_glued_to_here [ 1 ] != (-1) )
	{
	    //--------------------
	    // Find out which one of these is currently marked
	    //
	    current_mark_index = (-1);
	    for ( j = 0 ; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; j ++ )
	    {
		if ( level_editor_marked_obstacle == & ( EditLevel() -> obstacle_list [ EditLevel() -> map [ EditY() ] [ EditX() ] . obstacles_glued_to_here [ j ] ] ) )
		    current_mark_index = j ;
	    }
	    
	    if ( current_mark_index != (-1) ) 
	    {
		if ( EditLevel() -> map [ EditY() ] [ EditX() ] . obstacles_glued_to_here [ current_mark_index + 1 ] != (-1) )
		    level_editor_marked_obstacle = & ( EditLevel() -> obstacle_list [ EditLevel() -> map [ EditY() ] [ EditX() ] . obstacles_glued_to_here [ current_mark_index + 1 ] ] ) ;
		else
		    level_editor_marked_obstacle = & ( EditLevel() -> obstacle_list [ EditLevel() -> map [ EditY() ] [ EditX() ] . obstacles_glued_to_here [ 0 ] ] ) ;
	    }

	}
    }

}; // void level_editor_cycle_marked_obstacle()



/* ------------------
 * Quickbar functions
 * ------------------
 */
struct quickbar_entry *
quickbar_getentry ( int id )
{
    int i = 0;
    struct list_head *node;
    list_for_each (node, &quickbar_entries) {
	if (id == i) {
	    struct quickbar_entry *entry = list_entry (node, struct quickbar_entry, node);
	    return entry;
	}
	i ++;
    }
    return NULL;
}

iso_image * quickbar_getimage ( int selected_index , int *placing_floor ) 
{
/*    struct quickbar_entry *entry = quickbar_getentry ( selected_index );
    if (!entry) 
	return NULL;
    if (entry->obstacle_type == LEVEL_EDITOR_SELECTION_FLOOR) {
	*placing_floor = TRUE;
	return &floor_iso_images  [ entry->id ];
    } else {
	return &obstacle_map [wall_indices [ entry -> obstacle_type ] [ entry->id ] ] . image;
    }*/
}

/**
 *  @fn void quickbar_additem (struct quickbar_entry *entry)
 * 
 *  @brief Inserts an item in a sorted list 
 */
void
quickbar_additem (struct quickbar_entry *entry)
{
    struct quickbar_entry *tmp1, *tmp2;
    struct quickbar_entry *smallest, *biggest;
    struct list_head *node;
    /* The smallest element (if the list is non-empty) is the last element */
    smallest = list_entry(quickbar_entries.prev, struct quickbar_entry, node);
    /* Biggest one */ 
    biggest = list_entry(quickbar_entries.next, struct quickbar_entry, node);

    /* If the list is empty or if the entry we want to insert is smaller than 
     * the smallest element, just insert the entry */
    if ((list_empty(&quickbar_entries)) ||
	    (entry->used < smallest->used)) {
	list_add_tail(&entry->node, &quickbar_entries);
    /* If it's bigger than the biggest one, let it be the first */
    } else if (entry->used > biggest->used) {
	list_add(&entry->node, &quickbar_entries);
    } else {
	/* We know the element is between two entries, so let's find the place */
	list_for_each (node, &quickbar_entries) {
	    tmp1 = list_entry (node, struct quickbar_entry, node);
	    tmp2 = list_entry (node->next, struct quickbar_entry, node);
	    if (tmp1->used >= entry->used && entry->used >= tmp2->used) {
		list_add (&entry->node, &tmp1->node);
		break;
	    }
	}
    }

    int i = 0;
    list_for_each (node, &quickbar_entries) i++;
    number_of_walls [ LEVEL_EDITOR_SELECTION_QUICK ] = i;
}

void
quickbar_use (int obstacle, int id)
{
    struct list_head *node;
    struct quickbar_entry *entry = NULL;;
    list_for_each (node, &quickbar_entries) {
	entry = list_entry (node, struct quickbar_entry, node);
	if (entry->id == id && entry->obstacle_type == obstacle)  {
	    break;
	}
    }
    if (entry && node != &quickbar_entries) {
	entry->used ++;
	list_del (&entry->node);
	quickbar_additem (entry);
    } else {
	entry = MyMalloc (sizeof *entry);
	entry->obstacle_type = obstacle;
	entry->id = id;
	entry->used = 1;
	quickbar_additem (entry);
    }
}

/*
void
quickbar_click (level *level, int id, leveleditor_state *cur_state)
{
    struct quickbar_entry *entry = quickbar_getentry ( id );
    if ( entry ) {
	switch ( entry->obstacle_type )
	{
	    case LEVEL_EDITOR_SELECTION_FLOOR:
		cur_state->r_tile_used = entry->id;
		start_rectangle_mode(cur_state, TRUE);
		break;
	    case LEVEL_EDITOR_SELECTION_WALLS:
		cur_state->l_selected_mode = entry->obstacle_type;
		cur_state->l_id = entry->id;
		start_line_mode(cur_state, TRUE);
		break;
	    default:
	    action_create_obstacle_user (level, 
		    cur_state->TargetSquare . x, cur_state->TargetSquare . y, 
		    wall_indices [ entry -> obstacle_type ] [ entry -> id ]);
	}
	entry->used ++;
    }
}    
*/
/**
 *
 *
 */
void
close_all_chests_on_level ( int l_num ) 
{
  int i;

  for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
      switch ( curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type )
	{
	case ISO_H_CHEST_OPEN:
	  curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type = ISO_H_CHEST_CLOSED ;
	  break;
	case ISO_V_CHEST_OPEN:
	  curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type = ISO_V_CHEST_CLOSED ;
	  break;
	default:
	  break;
	}
    }

}; // void close_all_chests_on_level ( int l_num ) 

/**
 * This function should associate the current mouse position with an
 * index in the level editor item drop screen.
 * (-1) is returned when cursor is not on any item in the item drop grid.
 */
int
level_editor_item_drop_index ( int row_len , int line_len )
{
    if ( ( GetMousePos_x ( )  > UNIVERSAL_COORD_W(55) ) && ( GetMousePos_x ( )  < UNIVERSAL_COORD_W(55 + 64 * line_len) ) &&
	 ( GetMousePos_y ( )  > UNIVERSAL_COORD_H(32) ) && ( GetMousePos_y ( )  < UNIVERSAL_COORD_H(32 + 66 * row_len)))
	{
	    return (   ( GetMousePos_x()  - UNIVERSAL_COORD_W(55) ) / ( 64 * GameConfig . screen_width / 640 ) + 
		     ( ( GetMousePos_y()  - UNIVERSAL_COORD_H(32) ) / ( 66 * GameConfig . screen_height / 480 ) ) * line_len ) ;
	}

    //--------------------
    // If no level editor item grid index was found under the current
    // mouse cursor position, we just return (-1) to indicate that.
    //
    return ( -1 ) ;
    
}; // int level_editor_item_drop_index ( void )

/**
 * This function drops an item onto the floor.  It works with a selection
 * of item images and clicking with the mouse on an item image or on one
 * of the buttons presented to the person editing the level.
 */
void ItemDropFromLevelEditor( void )
{
    int SelectionDone = FALSE;
    int NewItemCode = ( -1 );
    int i;
    int j;
    item temp_item;
    int row_len = 5 ;
    int line_len = 8 ; 
    int our_multiplicity = 1 ;
    int item_group = 0 ; 
    static int previous_mouse_position_index = (-1) ;
    static int previous_suffix_selected = (-1) ;
    static int previous_prefix_selected = (-1) ;
    game_status = INSIDE_MENU;
    
    while ( MouseLeftPressed() ) SDL_Delay(1);
    
    while ( !SelectionDone )
    {
	save_mouse_state();
	
	our_SDL_fill_rect_wrapper ( Screen , NULL , 0 );
	
	for ( j = 0 ; j < row_len ; j ++ )
	{
	    for ( i = 0 ; i < line_len ; i ++ ) 
	    {
		temp_item . type = i + j * line_len + item_group * line_len * row_len ;
		if ( temp_item.type >= Number_Of_Item_Types )  continue; //temp_item.type = 1 ;
		ShowRescaledItem ( i , 32 + (64*GameConfig.screen_height/480+2) * j, & ( temp_item ) );
	    }
	}
	
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_PREFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_PREFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_SUFFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_SUFFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON );

	if ( MouseCursorIsOnButton ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON ,
                                             GetMousePos_x()  , GetMousePos_y()  ) )	    
	    PutStringFont ( Screen , FPS_Display_BFont , 20 , 440 * GameConfig . screen_height / 480 , _("Cancel item drop")) ;
	if ( level_editor_item_drop_index ( row_len , line_len ) != (-1) )
	{
	    previous_mouse_position_index = level_editor_item_drop_index ( row_len , line_len ) + 
		item_group * line_len * row_len ;
	    if ( previous_mouse_position_index >= Number_Of_Item_Types ) 
	    {
		previous_mouse_position_index = Number_Of_Item_Types - 1 ;
	    }
	    else PutStringFont ( Screen , FPS_Display_BFont , 20 , 440 * GameConfig . screen_height / 480 , D_(ItemMap [ previous_mouse_position_index ] . item_name )) ;
	}

	if ( previous_prefix_selected != (-1) )
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 370 * GameConfig . screen_height / 480, 
			    PrefixList [ previous_prefix_selected ] . bonus_name ) ;
	}
	else
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 370 * GameConfig . screen_height / 480, 
			    _("NO PREFIX" )) ;
	}

	if ( previous_suffix_selected != (-1) )
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 410 * GameConfig . screen_height / 480 , 
			    SuffixList [ previous_suffix_selected ] . bonus_name ) ;
	}
	else
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 410 * GameConfig . screen_height / 480 , 
			    _("NO SUFFIX" )) ;
	}
	
	our_SDL_flip_wrapper();
	
	if ( EscapePressed() )
	{ //Pressing escape cancels the dropping
	    while ( EscapePressed() );
	    return ;
	}

	if ( MouseLeftClicked())
	{
	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
	    if ( (item_group + 1 ) * line_len * row_len < Number_Of_Item_Types )
		item_group ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( item_group > 0 ) item_group -- ;
	    }

	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_PREFIX_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
		if ( PrefixList[ previous_prefix_selected + 1 ] . bonus_name != NULL )
		    previous_prefix_selected ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_PREFIX_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( previous_prefix_selected > (-1) )
		    previous_prefix_selected -- ;
	    }

	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_SUFFIX_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
		if ( SuffixList [ previous_suffix_selected + 1 ] . bonus_name != NULL )
		    previous_suffix_selected ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_SUFFIX_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( previous_suffix_selected > (-1) )
		    previous_suffix_selected -- ;
	    }
	    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON ,
					 GetMousePos_x()  , GetMousePos_y()  ) )
	    {
		return ;
	    }
	    else if ( level_editor_item_drop_index ( row_len , line_len ) != (-1) )
	    {
		NewItemCode = level_editor_item_drop_index ( row_len , line_len ) + item_group * line_len * row_len ;
		if ( NewItemCode < 0 ) NewItemCode = 0 ; // just if the mouse has moved away in that little time...
		if ( NewItemCode < Number_Of_Item_Types )
			SelectionDone = TRUE ;
	    }
	}
    }
    
    if ( NewItemCode >= Number_Of_Item_Types ) 
    {
	NewItemCode=0;
    }
    
    
    if ( ItemMap [ NewItemCode ] . item_group_together_in_inventory )
    {
	our_multiplicity = do_graphical_number_selection_in_range ( 1 , (!MatchItemWithName(NewItemCode, "Cyberbucks")) ? 100 : 1000, 1 );
	if ( our_multiplicity == 0 ) our_multiplicity = 1;
    }
    DropItemAt( NewItemCode , Me . pos . z , rintf( Me.pos.x ) , rintf( Me.pos.y ) , 
		previous_prefix_selected , previous_suffix_selected , our_multiplicity );
    
    while ( MouseLeftPressed() ) SDL_Delay(1);

    save_mouse_state();
    game_status = INSIDE_LVLEDITOR;
}; // void ItemDropFromLevelEditor( void )

/**
 * Run several validations
 */
void LevelValidation()
{
	int is_invalid = FALSE;

	SDL_Rect BackgroundRect = { UNIVERSAL_COORD_W(20), UNIVERSAL_COORD_H(20), UNIVERSAL_COORD_W(600), UNIVERSAL_COORD_H(440) };
	SDL_Rect ReportRect     = { UNIVERSAL_COORD_W(30), UNIVERSAL_COORD_H(30), UNIVERSAL_COORD_W(580), UNIVERSAL_COORD_H(420) };

	BFont_Info* current_font = GetCurrentFont();
	int raw_height = FontHeight( current_font );
	int max_raws = (ReportRect.h / raw_height) - 4; // 4 lines are reserved for header and footer 
	int column_width = TextWidth( "Level 000: empty" );

	AssembleCombatPicture ( ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS );
	ShadowingRectangle ( Screen, BackgroundRect );

	//--------------------
	// Title
	//
	CenteredPutString( Screen, ReportRect.y, "Level Validation tests - Summary\n" );

	//--------------------
	// Loop on each level
	//
	int l;
	int col_pos = 0;
	int raw_pos = 0;
	
	for ( l = 0; l < curShip.num_levels; ++l )
	{
		level_validator_ctx ValidatorCtx = { &ReportRect, curShip.AllLevels[l] };

		// Compute raw and column position, when a new column of text starts
		if ( (l % max_raws) == 0 )
		{
			col_pos = ReportRect.x + (l/max_raws) * column_width;
			raw_pos = ReportRect.y + 2 * raw_height; // 2 lines are reserved for the header
			SetTextCursor( col_pos, raw_pos);
		}
		
		if ( curShip.AllLevels[l] == NULL )
		{
			// Empty level
			char txt[40];
			sprintf(txt, "%s %3d: \2empty\n", "Level", l );
			DisplayText( txt, col_pos, -1, &ReportRect, 1.0 );			
			SetCurrentFont( current_font ); // Reset font
		}
		else
		{
		// Loop on each validation function
		int v = 0;
		level_validator one_validator;
		int level_is_invalid = FALSE;

		while ( (one_validator = level_validators[v++]) != NULL ) level_is_invalid |= one_validator(&ValidatorCtx);

		// Display report
		char txt[40];
		sprintf(txt, "%s %3d: %s\n", "Level", l, (level_is_invalid)?"\1fail":"pass" );
		DisplayText( txt, col_pos, -1, &ReportRect, 1.0 );
		SetCurrentFont( current_font ); // Reset font in case of the red "fail" was displayed

		// Set global is_invalid flag
		is_invalid |= level_is_invalid;
	}
	}

	//--------------------
	// This was it.  We can say so and return.
	//
	if ( is_invalid ) CenteredPutString( Screen, ReportRect.y + ReportRect.h - 2.0*raw_height, "\1Some tests were invalid. See the report in the console\3" );

	CenteredPutString( Screen, ReportRect.y + ReportRect.h - raw_height, "--- End of List --- Press Space to return tolevel *Editor ---" );

	our_SDL_flip_wrapper();

} // LevelValidation( int levelnum )

void TestMap ( void )  /* Keeps World map in a clean state */
{
	if (game_root_mode == ROOT_IS_GAME) /*don't allow map testing if root mode is GAME*/
		return;
	SaveGame();
	Game();
	LoadGame();
	return;
} // TestMap ( void )


/**
 * There is a 'help' screen for the level editor too.  This help screen
 * is presented as a scrolling text, giving a short introduction and also
 * explaining the keymap to the level editor.  The info for this scrolling
 * text is all in a title file in the maps dir, much like the initial
 * scrolling text at any new game startup.
 */
void 
ShowLevelEditorKeymap ( void )
{
    PlayATitleFile ( "level_editor_help.title" );
}; // void ShowLevelEditorKeymap ( void )

/**
 * The levels in Freedroid may be connected into one big map by simply
 * 'gluing' then together, i.e. we define some interface areas to the
 * sides of a map and when the Tux passes these areas, he'll be silently
 * put into another map without much fuss.  This operation is performed
 * silently and the two maps must be synchronized in this interface area
 * so the map change doesn't become apparend to the player.  Part of this
 * synchronisation, namely copying the map tiles to the other map, is 
 * done automatically, but some inconsistencies like non-matching map
 * sizes or non-symmetric jump directions (i.e. not back and forth but
 * back and forth-to-somewhere else) are not resolved automatically.
 * Instead, a report on inconsistencies will be created and the person
 * editing the map can then resolve the inconsistencies manually in one
 * fashion or the other.
 */
void ReportInconsistenciesForLevel ( int LevelNum )
{
    int TargetLevel;
    SDL_Rect ReportRect;
    
    ReportRect.x = 20;
    ReportRect.y = 20;
    ReportRect.w = 600;
    ReportRect.h = 440;
    
    AssembleCombatPicture ( ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS );
    
    DisplayText ( _("\nThe list of inconsistencies of the jump interfaces for this level:\n\n") ,
		  ReportRect.x, ReportRect.y + FontHeight ( GetCurrentFont () ) , &ReportRect , 1.0 );
    
    //--------------------
    // First we test for inconsistencies of back-forth ways, i.e. if the transit
    // in one direction will lead back in the right direction when returning.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_south != LevelNum )
	{
	    DisplayText ( _("BACK-FORTH-MISMATCH: North doesn't lead back here (yet)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_north != LevelNum )
	{
	    DisplayText ( _("BACK-FORTH-MISMATCH: South doesn't lead back here (yet)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_west != LevelNum )
	{
	    DisplayText ( _("BACK-FORTH-MISMATCH: East doesn't lead back here (yet)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_east != LevelNum )
	{
	    DisplayText ( _("BACK-FORTH-MISMATCH: West doesn't lead back here (yet)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    DisplayText ( _("\nNO OTHER BACK-FORTH-MISMATCH ERRORS other than those listed above\n\n") ,
		  -1 , -1 , &ReportRect , 1.0 );
    
    //--------------------
    // Now we test for inconsistencies of interface sizes, i.e. if the interface source level
    // has an interface as large as the target interface level.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_south != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_north )
	{
	    DisplayText ( _("INTERFACE SIZE MISMATCH: North doesn't lead so same-sized interface level!!!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_north != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_south )
	{
	    DisplayText ( _("INTERFACE SIZE MISMATCH: South doesn't lead so same-sized interface level!!!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_west != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_east )
	{
	    DisplayText ( _("INTERFACE SIZE MISMATCH: East doesn't lead so same-sized interface level!!!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_east != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_west )
	{
	    DisplayText ( _("INTERFACE SIZE MISMATCH: West doesn't lead so same-sized interface level!!!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    
    //--------------------
    // Now we test for inconsistencies of level sizes, i.e. if the interface source level
    // has the same relevant dimension like the target interface level.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> xlen != curShip.AllLevels [ LevelNum ] -> xlen )
	{
	    DisplayText ( _("LEVEL DIMENSION MISMATCH: North doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> xlen != curShip.AllLevels [ LevelNum ] -> xlen )
	{
	    DisplayText ( _("LEVEL DIMENSION MISMATCH: South doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> ylen != curShip.AllLevels [ LevelNum ] -> ylen )
	{
	    DisplayText ( _("LEVEL DIMENSION MISMATCH: East doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> ylen != curShip.AllLevels [ LevelNum ] -> ylen )
	{
	    DisplayText ( _("LEVEL DIMENSION MISMATCH: West doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n") ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    
    //--------------------
    // This was it.  We can say so and return.
    //
    DisplayText ( _("\n\n--- End of List --- Press Space to return to menu ---\n") ,
		  -1 , -1 , &ReportRect , 1.0 );
    
    our_SDL_flip_wrapper();
    
}; // void ReportInconsistenciesForLevel ( int LevelNum )

/**
 * If we want to synchronize two levels, we need to remove the old obstacles
 * before we can add new ones.  Else the place might get too crowded with
 * obstacles. :)
 */
void
delete_all_obstacles_in_area (level *TargetLevel , float start_x , float start_y , float area_width , float area_height )
{
    int i;
    
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( TargetLevel -> obstacle_list [ i ] . type <= (-1) ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . x < start_x ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . y < start_y ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . x > start_x + area_width ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . y > start_y + area_height ) continue;
	action_remove_obstacle ( TargetLevel , & ( TargetLevel -> obstacle_list [ i ] ) );
	i--; // this is so that this obstacle will be processed AGAIN, since deleting might
	// have moved a different obstacle to this list position.
    }
}; // void delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , 0 , TargetLevel->ylen-AreaHeight , AreaWidth , AreaHeight )

/**
 * After exporting a level, there might be some old corpses of 
 * descriptions that were deleted when the target level was partly cleared
 * out and overwritten with the new obstacles that brought their own new
 * obstacle descriptions.
 *
 * In this function, we try to clean out those old corpses to avoid 
 * cluttering in the map file.
 */
void
eliminate_dead_obstacle_descriptions (level *target_level )
{
    int i;
    int is_in_use;
    int desc_index;

    //--------------------
    // We proceed through the list of known descriptions.  Some of them
    // might not be in use, but still hold a non-null content string.
    // Such instances will be eliminated.
    //
    for ( desc_index = 0 ; desc_index < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL ; desc_index ++ )
    {
	//--------------------
	// Maybe the description in question is an empty index anyway.
	// Then of course there is no need to eliminate anything and
	// we can proceed right away.
	//
	if ( target_level -> obstacle_description_list [ desc_index ] == NULL ) continue;

	//--------------------
	// So now we've encountered some string.  Let's see if it's really
	// in use.  For that, we need to proceed through all the obstacles
	// of this level and see if one of them has a description index 
	// pointing to this description string.
	//
	is_in_use = FALSE ;
	for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
	{
	    if ( target_level -> obstacle_list [ i ] . description_index == desc_index )
	    {
		DebugPrintf ( 1 , "\nvoid eliminate_dead_obstacle_descriptions(...):  This descriptions seems to be in use still." );
		is_in_use = TRUE ;
		break;
	    }
	}
	
	if ( is_in_use ) continue;
	target_level -> obstacle_description_list [ desc_index ] = NULL ;
	DebugPrintf ( 1 , "\nNOTE: void eliminate_dead_obstacle_descriptions(...):  dead description found.  Eliminated." );
    }
	
}; // void eliminate_dead_obstacle_descriptions (level *target_level )

/**
 * This function should allow for conveninet duplication of obstacles from
 * one map to the other.  It assumes, that the target area has been cleaned
 * out of obstacles already.
 */
void
duplicate_all_obstacles_in_area (level *source_level ,
				  float source_start_x , float source_start_y , 
				  float source_area_width , float source_area_height ,
				 level *target_level ,
				  float target_start_x , float target_start_y )
{
    int i;
    obstacle* new_obstacle;

    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( source_level -> obstacle_list [ i ] . type <= (-1) ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . x < source_start_x ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . y < source_start_y ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . x > source_start_x + source_area_width ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . y > source_start_y + source_area_height ) continue;
	
	new_obstacle = 
	    action_create_obstacle ( target_level , 
			   target_start_x  + source_level -> obstacle_list [ i ] . pos . x - source_start_x ,
			   target_start_y  + source_level -> obstacle_list [ i ] . pos . y - source_start_y ,
			   source_level -> obstacle_list [ i ] . type );
	
	//--------------------
	// Maybe the source obstacle had a label attached to it?  Then
	// We should also duplicate the obstacle label.  Otherwise it
	// might get overwritten when exporting in the other direction.
	//
	if ( source_level -> obstacle_list [ i ] . name_index != (-1) )
	{
	    action_change_obstacle_label_user ( 
		target_level , new_obstacle , 
		source_level -> obstacle_name_list [ source_level -> obstacle_list [ i ] . name_index ] );
	    DebugPrintf ( 1 , "\nNOTE: void duplicate_all_obstacles_in_area(...):  obstacle name was exported:  %s." ,
			  source_level -> obstacle_name_list [ source_level -> obstacle_list [ i ] . name_index ] );
	}

	//--------------------
	// Maybe the source obstacle had a description attached to it?  Then
	// We should also duplicate the obstacle description.  Otherwise it
	// might get overwritten when exporting in the other direction.
	//
	if ( source_level -> obstacle_list [ i ] . description_index != (-1) )
	{
	    action_change_obstacle_description (target_level, new_obstacle, 
		source_level -> obstacle_description_list [ source_level -> obstacle_list [ i ] . description_index ] );
	    DebugPrintf ( -1 , "\nNOTE:  obstacle description was exported:  %s." ,
			  source_level -> obstacle_description_list [ source_level -> obstacle_list [ i ] . description_index ] );
	}
	
	//action_remove_obstacle ( source_level , & ( source_level -> obstacle_list [ i ] ) );
	// i--; // this is so that this obstacle will be processed AGAIN, since deleting might
	// // have moved a different obstacle to this list position.
    }

    eliminate_dead_obstacle_descriptions ( target_level );
    
}; // void duplicate_all_obstacles_in_area ( ... )

/**
 *
 *
 */      
void 
floor_copy ( map_tile* target_pointer , map_tile* source_pointer , int amount )
{
    int i;
    
    for ( i = 0 ; i < amount ; i ++ )
    {
	target_pointer -> floor_value = source_pointer -> floor_value ;
	target_pointer ++ ;
	source_pointer ++ ;
    }
}; // void floor_copy ( map_tile* target_pointer , map_tile* source_pointer , int amount )

/**
 * This function should create a completely new level into the existing
 * ship structure that we already have.  The new level will be rather
 * small and simple.
 */
void CreateNewMapLevel( int level_num )
{
   level *NewLevel;
    int i, k, l ;
    
    //--------------------
    // Get the memory for one level 
    //
    NewLevel = (Level) MyMalloc ( sizeof ( level ) );
    
    DebugPrintf (0, "\n-----------------------------------------------------------------");
    DebugPrintf (0, "\nStarting to create and add a completely new level to the ship.");
    
    //--------------------
    // Now we proceed in the order of the struct 'level' in the
    // struct.h file so that we can easily verify if we've handled
    // all the data structure or left something out which could
    // be terrible!
    //
    NewLevel -> levelnum = level_num ;
    NewLevel -> xlen = 90 ;
    NewLevel -> ylen = 90 ;
    NewLevel -> light_radius_bonus = 1 ;
    NewLevel -> minimum_light_value = 13 ;
    NewLevel -> Levelname = "New level just created" ;
    NewLevel -> Background_Song_Name = "TheBeginning.ogg" ;
    //--------------------
    // Now we initialize the statement array with 'empty' values
    //
    for ( i = 0 ; i < MAX_STATEMENTS_PER_LEVEL ; i ++ )
    {
	NewLevel -> StatementList [ i ] . x = ( -1 ) ;
	NewLevel -> StatementList [ i ] . y = ( -1 ) ;
	NewLevel -> StatementList [ i ] . Statement_Text = "No Statement loaded." ;
    }
    //--------------------
    // Now we initialize the obstacle name list with 'empty' values
    //
    for ( i = 0 ; i < MAX_OBSTACLE_NAMES_PER_LEVEL ; i ++ )
    {
	NewLevel -> obstacle_name_list [ i ] = NULL ;
    }
    //--------------------
    // Now we initialize the obstacle description list with 'empty' values
    //
    for ( i = 0 ; i < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL ; i ++ )
    {
	NewLevel -> obstacle_description_list [ i ] = NULL ;
    }
    //--------------------
    // First we initialize the floor with 'empty' values
    //
    for ( i = 0 ; i < NewLevel -> ylen ; i ++ )
    {
	NewLevel -> map [ i ] = MyMalloc ( NewLevel -> xlen * sizeof ( map_tile ) ) ;
	for ( k = 0 ; k < NewLevel -> xlen ; k ++ )
	{
	    NewLevel -> map [ i ] [ k ] . floor_value = ISO_FLOOR_SAND ;
	    for ( l = 0 ; l < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; l ++ )
	    {
		NewLevel -> map [ i ] [ k ] . obstacles_glued_to_here [ l ] = (-1) ;
	    }
	}
    }
    //--------------------
    // Now we initialize the level jump interface variables with 'empty' values
    //
    NewLevel->jump_target_north = (-1) ;
    NewLevel->jump_target_south = (-1) ;
    NewLevel->jump_target_east = (-1) ;
    NewLevel->jump_target_west = (-1) ;
    NewLevel->jump_threshold_north = (-1) ;
    NewLevel->jump_threshold_south = (-1) ;
    NewLevel->jump_threshold_east = (-1) ;
    NewLevel->jump_threshold_west = (-1) ;
    //--------------------
    // Now we initialize the map obstacles with 'empty' information
    //
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	NewLevel -> obstacle_list [ i ] . type = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . x = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . y = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . name_index = ( -1 ) ;
    }
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	NewLevel -> obstacle_list [ i ] . type = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . x = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . y = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . name_index = ( -1 ) ;
    }
    //--------------------
    // This should initialize the lists with the refreshed and other
    // animated map tiles...
    //
    GetAnimatedMapTiles () ;
    //--------------------
    // Now we initialize the map labels array with 'empty' information
    //
    for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
	NewLevel -> labels [ i ] . pos . x = ( -1 ) ;
	NewLevel -> labels [ i ] . pos . y = ( -1 ) ;
	NewLevel -> labels [ i ] . label_name = "no_label_defined" ;
    }
    //--------------------
    // Now we add empty waypoint information...
    //
    NewLevel -> num_waypoints = 0 ;
    for ( i = 0 ; i < MAXWAYPOINTS ; i++ )
    {
	NewLevel -> AllWaypoints [ i ] . x = 0 ;
	NewLevel -> AllWaypoints [ i ] . y = 0 ;
	
	for ( k = 0 ; k < MAX_WP_CONNECTIONS ; k++ )
	{
	    NewLevel -> AllWaypoints [ i ] . connections [ k ] = -1 ;
	}
    }
    //--------------------
    // First we initialize the items arrays with 'empty' information
    //
    for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
    {
	NewLevel -> ItemList [ i ] . pos.x = ( -1 ) ;
	NewLevel -> ItemList [ i ] . pos.y = ( -1 ) ;
	NewLevel -> ItemList [ i ] . type = ( -1 ) ;
	NewLevel -> ItemList [ i ] . currently_held_in_hand = FALSE;
	
    }
    //--------------------
    // Now we initialize the chest items arrays with 'empty' information
    //
    for ( i = 0 ; i < MAX_CHEST_ITEMS_PER_LEVEL ; i ++ )
    {
	NewLevel -> ChestItemList [ i ] . pos . x = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . pos . y = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . type = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . currently_held_in_hand = FALSE ;
    }
    
    curShip . AllLevels [ level_num ] = NewLevel ;
    
    glue_obstacles_to_floor_tiles_for_level ( level_num );
    
}; // void CreateNewMapLevel( int )


/**
 *
 *
 */
int marked_obstacle_is_glued_to_here (level *EditLevel , float x , float y )
{
    int j;
    int current_mark_index = (-1);
    
    if ( level_editor_marked_obstacle == NULL ) return ( FALSE );
    
    for ( j = 0 ; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; j ++ )
    {
	if ( level_editor_marked_obstacle == & ( EditLevel -> obstacle_list [ EditLevel -> map [ (int)y ] [ (int)x ] . obstacles_glued_to_here [ j ] ] ) )
	current_mark_index = j ;
    }
    
    if ( current_mark_index != (-1) ) return ( TRUE );
    return ( FALSE );
    
}; // int marked_obstacle_is_glued_to_here ( Me . pos . x , Me . pos . y )


/**
 * This function should assign a new individual obstacle description to a 
 * single instance of a given obstacle type on a given level.  
 *
 * New indices must be found and the user must be queried for his input 
 * about the desired description text for the obstacle.
 *
 */
/**
 * Begins a new line of walls
 */
/*
void start_line_mode(leveleditor_state *cur_state, int already_defined)
{
    // Initialize a line
    INIT_LIST_HEAD(&(cur_state->l_elements.list));
    cur_state->mode = LINE_MODE;
    cur_state->l_direction = UNDEFINED;
    // If the tile is not already defined (ie. if the function is not 
    // called from the quickbar_click function)
    if (! already_defined)
    {
	cur_state->l_selected_mode = GameConfig . level_editor_edit_mode;
	cur_state->l_id = Highlight;
    }

    
    cur_state->l_elements.position.x = (int)cur_state->TargetSquare.x +
	((obstacle_map [ wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] ] . flags & IS_HORIZONTAL) ? 0.5 : 0);

    cur_state->l_elements.position.y = (int)cur_state->TargetSquare.y + 
	((obstacle_map [ wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] ] . flags & IS_HORIZONTAL) ? 0 : 0.5);

    cur_state->l_elements.address = action_create_obstacle_user ( EditLevel() , 
	    cur_state->l_elements.position.x , cur_state->l_elements.position.y , 
	    wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] );
	    
}
*/

/**
 * This function handles the line mode; adds a wall, or starts line mode
 **/
/*
void handle_line_mode(leveleditor_state *cur_state)
{ 
    if(cur_state->mode != LINE_MODE) {
	start_line_mode(cur_state, FALSE);	
    } else {
	line_element *wall;
	int actual_direction;
	float distance;
	moderately_finepoint pos_last;
	moderately_finepoint offset;
	int direction_is_possible;

	wall = list_entry((cur_state->l_elements).list.prev, line_element, list);
	pos_last = wall->position;
	distance = calc_euklid_distance(pos_last.x, pos_last.y ,
		cur_state->TargetSquare.x ,
		cur_state->TargetSquare.y );

	// Let's calculate the difference of position since last time
	offset.x = pos_last.x - cur_state->TargetSquare.x;
	offset.y = pos_last.y - cur_state->TargetSquare.y;

	// Then we want to find out in which direction the mouse has moved
	// since the last time, and compute the distance relatively to the axis
	if (fabsf(offset.y) > fabsf(offset.x))
	    {
	    if (offset.y > 0)
		{
		actual_direction = NORTH;
		}
	    else
		{
		actual_direction = SOUTH;
		}
	    distance = fabsf(cur_state->TargetSquare.y - pos_last.y);
	    }
	else
	    {
	    if (offset.x > 0) {
		actual_direction = WEST;
	    }
	    else
		{
		actual_direction = EAST;
		}
	    distance = fabsf(cur_state->TargetSquare.x - pos_last.x);
	    }

	// Are we going in a direction possible with that wall?

	if ( obstacle_map [ wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] ] . flags & IS_HORIZONTAL )
	    {
	    direction_is_possible =  (actual_direction == WEST) || (actual_direction == EAST);
	    } 
	else if ( obstacle_map [ wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] ] . flags & IS_VERTICAL ) 
	    {
	    direction_is_possible = (actual_direction == NORTH) || (actual_direction == SOUTH);
	    }
	else 
	    {
	    direction_is_possible = FALSE;
	    }

	// If the mouse is far away enoug
	if ((distance > 1) && (direction_is_possible) &&
		((cur_state->l_direction == actual_direction) || 
		 (cur_state->l_direction == UNDEFINED))) 
	    {
	    wall = malloc(sizeof(line_element));

	    // Then we calculate the position of the next wall
	    wall->position = pos_last;
	    switch(actual_direction) {
		case NORTH:
		    wall->position.y --;
		    break;
		case SOUTH:
		    wall->position.y ++; 
		    break;
		case EAST:
		    wall->position.x ++;
		    break;
		case WEST:
		    wall->position.x --;
		    break;
		default:
		    break;
	    }
	    // And add the wall, to the linked list and to the map
	    //
	    
	    list_add_tail(&(wall->list), &(cur_state->l_elements.list));
	    wall->address = action_create_obstacle_user ( EditLevel() ,
		    wall->position.x , wall->position.y ,
		    wall_indices [ cur_state->l_selected_mode ] [ cur_state->l_id ] );

	    // If the direction is unknown (ie. we only have one wall), 
	    // let's define it
	    if (cur_state->l_direction == UNDEFINED)
		{
		cur_state->l_direction = actual_direction;
		}
	    }
	if ( (cur_state->l_direction == (- actual_direction)) && 
		(!list_empty(&(cur_state->l_elements.list))) )
	    {
	    // Looks like the user wants to go back, so let's remove the line wall
	    wall = list_entry(cur_state->l_elements.list.prev, line_element, list);
	    action_remove_obstacle_user(EditLevel(), wall->address);
	    list_del(cur_state->l_elements.list.prev);
	    free(wall);
	    if(list_empty(&(cur_state->l_elements.list)))
		{
		cur_state->l_direction = UNDEFINED;
		}
	    }
    }
}; // void handle_line_mode(line_element *wall_line, moderately_finepoint TargetSquare, int *direction)
*/

/*
void end_line_mode(leveleditor_state *cur_state, int place_line)
{
    line_element *tmp;
    int list_length = 1;  

    cur_state->mode = NORMAL_MODE;

    // Remove the linked list
    while(!(list_empty(&(cur_state->l_elements.list))))
    {
	tmp = list_entry(cur_state->l_elements.list.prev, line_element, list);
	free(tmp);
	if(!place_line)
	    action_remove_obstacle(EditLevel(), cur_state->l_elements.address);
	list_del(cur_state->l_elements.list.prev);
	list_length++;
    }
    if(!place_line)
	action_remove_obstacle(EditLevel(), cur_state->l_elements.address);

    // Remove the sentinel
    list_del(&(cur_state->l_elements.list));

    if (place_line)
	action_push(ACT_MULTIPLE_FLOOR_SETS, list_length);

}; // void end_line_mode(line_element *wall_line, int place_line)

*/

/*
void start_rectangle_mode ( leveleditor_state *cur_state , int already_defined )
{
    // Start actual mode
    cur_state->mode = RECTANGLE_MODE;

    // Starting values
    cur_state->r_start.x = (int)cur_state->TargetSquare.x;
    cur_state->r_start.y = (int)cur_state->TargetSquare.y;
    cur_state->r_len_x = 0;
    cur_state->r_len_y = 0;

    // The tile we'll use 
    if (! already_defined)
	cur_state->r_tile_used = selected_tile_nb;

    // Place the first tile 
    action_set_floor ( EditLevel(), cur_state->r_start.x, cur_state->r_start.y, cur_state->r_tile_used );
    action_push ( ACT_MULTIPLE_FLOOR_SETS, 1);

} // void start_rectangle_mode ( leveleditor_state cur_state , int already_defined )
*/
/*
void handle_rectangle_mode ( leveleditor_state *cur_state )
{
    int i, j;
    int changed_tiles = 0;
    // If there is something to change
    if (calc_euklid_distance(cur_state->TargetSquare.x, cur_state->TargetSquare.y,
		cur_state->r_start.x + cur_state->r_len_x,
		cur_state->r_start.y + cur_state->r_len_y) > 0.5)
    {
	// Redefine the rectangle dimensions
	cur_state->r_len_x = (int)cur_state->TargetSquare.x - cur_state->r_start.x; 
	cur_state->r_step_x = (cur_state->r_len_x > 0 ? 1 : -1);
	cur_state->r_len_y = (int)cur_state->TargetSquare.y - cur_state->r_start.y;
	cur_state->r_step_y = (cur_state->r_len_y > 0 ? 1 : -1);

	// Undo previous rectangle
	level_editor_action_undo ();

	// Then redo a correct one
	for (i = cur_state->r_start.x;
		i != cur_state->r_start.x + cur_state->r_len_x + cur_state->r_step_x;
		i += cur_state->r_step_x)
	{
	    for (j = cur_state->r_start.y;
		    j != cur_state->r_start.y + cur_state->r_len_y + cur_state->r_step_y;
		    j += cur_state->r_step_y)
	    {
		action_set_floor ( EditLevel(), i, j, cur_state->r_tile_used );
		changed_tiles++;
	    }
	}
	action_push ( ACT_MULTIPLE_FLOOR_SETS, changed_tiles);
    }
} // void handle_rectangle_mode (whole_rectangle *rectangle, moderately_finepoint TargetSquare)
*/

/*
void end_rectangle_mode( leveleditor_state *cur_state, int place_rectangle)
{
    cur_state->mode = NORMAL_MODE;
    if ( ! place_rectangle )
	level_editor_action_undo ();
}
*/

static void leveleditor_init() 
{
/*    cur_state = MyMalloc(sizeof(leveleditor_state));
    cur_state->mode = NORMAL_MODE;

    // This is only here to shutup a warning
    cur_state->c_last_right_click.x = 0;
    cur_state->c_last_right_click.y = 0;
*/
    level_editor_done = FALSE;

    //--------------------
    // We set the Tux position to something 'round'.
    //
    Me . pos . x = rintf ( Me . pos . x ) + 0.5 ;
    Me . pos . y = rintf ( Me . pos . y ) + 0.5 ;

    //--------------------
    // We disable all the 'screens' so that we have full view on the
    // map for the purpose of level editing.
    //
    GameConfig.Inventory_Visible = FALSE;
    GameConfig.CharacterScreen_Visible = FALSE;
    GameConfig.SkillScreen_Visible = FALSE;

    strcpy ( VanishingMessage , "" );
    VanishingMessageEndDate = 0 ;

    //--------------------
    // For drawing new waypoints, we init this.
    //
    OriginWaypoint = (-1);

    leveleditor_init_widgets();
}

static void leveleditor_cleanup()
{
    level_editor_marked_obstacle = NULL ; 

    Activate_Conservative_Frame_Computation();
    action_freestack ( ) ;
}

/**
 * This function provides thelevel *Editor integrated into
 * freedroid.  Actually this function is a submenu of the big
 * Escape Menu.  In here you can edit the level and, upon pressing
 * escape, you can enter a new submenu where you can save the level,
 * change level name and quit from level editing.
 */
void LevelEditor()
{
    leveleditor_init();

    while ( !level_editor_done ) {
	game_status = INSIDE_LVLEDITOR;

	if ( ! GameConfig . hog_CPU ) 
	    SDL_Delay (1);

	leveleditor_process_input();
	
	leveleditor_display();	    

    }
    

    leveleditor_cleanup();
}; // void LevelEditor ( void )

#undef _leveleditor_c
