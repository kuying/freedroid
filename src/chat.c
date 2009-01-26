/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * This file contains all functions dealing with the dialog interface,
 * including blitting the chat protocol to the screen and drawing the
 * right portrait images to the screen.
 */

#define _chat_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "SDL_rotozoom.h"


#define PUSH_ROSTER 2
#define POP_ROSTER 3 

/* This table links dialog file names to the internal
 * chat flags code.
 */
struct { 
    const char *name;
    int person_id;
} dialog_name_to_index[] = { 
    { "614", PERSON_614},
    { "614_cryo", PERSON_614_CRYO},
    { "Bender", PERSON_BENDER},
    { "Benjamin", PERSON_BENJAMIN},
    { "Boris", PERSON_BORIS},
    { "Bruce", PERSON_BRUCE},
    { "Butch", PERSON_BUTCH},
    { "Chandra", PERSON_CHA },
    { "Darwin", PERSON_DARWIN},
    { "Dixon", PERSON_DIXON},
    { "DocMoore", PERSON_DOC_MOORE},
    { "Duncan", PERSON_DUNCAN},
    { "Ewald", PERSON_EWALD},
    { "FirmwareUpdateServer", PERSON_FIRMWARE_SERVER},
    { "Francis", PERSON_FRANCIS},
    { "Jasmine", PERSON_JASMINE},
    { "Kevin", PERSON_KEVIN},
    { "KevinGuard", PERSON_KEVINS_GUARD},
    { "Koan", PERSON_KOAN},
    { "Lina", PERSON_LINA},
    { "Lukas", PERSON_LUKAS},
    { "MER", PERSON_MER},
    { "MSCD", PERSON_MSCD},
    { "MSFacilityGateGuardLeader", PERSON_MS_FACILITY_GATE_GUARD_LEADER},
    { "Melfis", PERSON_MELFIS},
    { "Michelangelo", PERSON_MICHELANGELO},
    { "OldTownGateGuardLeader", PERSON_OLD_TOWN_GATE_GUARD_LEADER},
    { "Pendragon", PERSON_PENDRAGON},
    { "RMS", PERSON_RMS},
    { "SACD", PERSON_SACD},
    { "SADD", PERSON_SADD},
    { "Serge", PERSON_SERGE},
    { "Skippy", PERSON_SKIPPY},
    { "Sorenson", PERSON_SORENSON},
    { "Spencer", PERSON_SPENCER},
    { "StandardBotAfterTakeover", PERSON_STANDARD_BOT_AFTER_TAKEOVER},
    { "StandardMSFacilityGateGuard", PERSON_STANDARD_MS_FACILITY_GATE_GUARD},
    { "StandardOldTownGateGuard", PERSON_STANDARD_OLD_TOWN_GATE_GUARD},
    { "Stone", PERSON_STONE},
    { "Tania", PERSON_TANIA},
    { "TestDroid", PERSON_TEST_DROID},
    { "TutorialTom", PERSON_TUTORIALTOM},
    { "Tybalt", PERSON_TYBALT},
    { "subdlg_", PERSON_SUBDIALOG_DUMMY},
};

char *chat_initialization_code; //first time with a character-code
char *chat_startup_code; //every time we start this dialog-code
EXTERN char *PrefixToFilename[ ENEMY_ROTATION_MODELS_AVAILABLE ];
char* chat_protocol = NULL ;


static void DoChatFromChatRosterData(int ChatPartnerCode , Enemy ChatDroid , int ClearProtocol );

/**
 * This function resets a dialog option to "empty" default values. It does NOT free the strings, this has to be done
 * prior to calling this function when needed.
 */
static void clear_dialog_option(dialogue_option * d)
{
    int i;
    d->option_text = "";
    d->option_sample_file_name = "";

    for (i=0; i < MAX_REPLIES_PER_OPTION; i++) {
	d->reply_sample_list[i] = "";
	d->reply_subtitle_list[i] = "";
    }

    d->always_execute_this_option_prior_to_dialog_start = 0;

    d->lua_code = NULL;
    d->exists = 0;
}

/**
 *
 */
static void delete_one_dialog_option ( int i , int FirstInitialisation )
{
    int j;

    //--------------------
    // If this is not the first initialisation, we have to free the allocated
    // strings first, or we'll be leaking memory otherwise...
    //
    if ( !FirstInitialisation )
    {
	if ( strlen ( ChatRoster[i].option_text ) ) free ( ChatRoster[i].option_text );
	if ( strlen ( ChatRoster[i].option_sample_file_name ) ) free ( ChatRoster[i].option_sample_file_name );
    }

    if(ChatRoster[i].lua_code)
	free(ChatRoster[i].lua_code);
    ChatRoster[i].lua_code = NULL;

    for ( j = 0 ; j < MAX_REPLIES_PER_OPTION ; j++ )
    {
	//--------------------
	// If this is not the first initialisation, we have to free the allocated
	// strings first, or we'll be leaking memory otherwise...
	//
	if ( !FirstInitialisation )
	{
	    if ( strlen ( ChatRoster [ i ] . reply_sample_list [ j ] ) ) 
		free ( ChatRoster [ i ] . reply_sample_list [ j ] );
	    if ( strlen ( ChatRoster [ i ] . reply_subtitle_list [ j ] ) ) 
		free ( ChatRoster [ i ] . reply_subtitle_list [ j ] );
	}
    }
    
    clear_dialog_option(&ChatRoster[i]);
}; // void delete_one_dialog_option ( int i , int FirstInitialisation )

/**
 * This function should init the chat roster with empty values and thereby
 * clean out the remnants of the previous chat dialogue.
 */
static void InitChatRosterForNewDialogue( void )
{
  int i;
  static int FirstInitialisation = TRUE;
  
  for ( i = 0 ; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER ; i ++ )
    {
      delete_one_dialog_option ( i , FirstInitialisation );
    }

  //--------------------
  // Next time, we WILL have to free every used entry before cleaning it
  // out, or we will be leaking memory...
  //
  FirstInitialisation = FALSE ;

}; // void InitChatRosterForNewDialogue( void )

/**
 *
 *
 */
void push_or_pop_chat_roster ( int push_or_pop )
{
    static dialogue_option LocalChatRoster[MAX_DIALOGUE_OPTIONS_IN_ROSTER];
    
    if ( push_or_pop == PUSH_ROSTER )
    {
	memcpy ( LocalChatRoster , ChatRoster , sizeof ( dialogue_option ) * MAX_DIALOGUE_OPTIONS_IN_ROSTER ) ;
	int i;
	for (i = 0; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER; i++)
	    clear_dialog_option(&ChatRoster[i]);
    }
    else if ( push_or_pop == POP_ROSTER )
    {
	memcpy ( ChatRoster , LocalChatRoster , sizeof ( dialogue_option ) * MAX_DIALOGUE_OPTIONS_IN_ROSTER ) ;
    }
    else
    {
	ErrorMessage ( __FUNCTION__  , "\
There was an unrecognized parameter handled to this function." ,
				   PLEASE_INFORM, IS_FATAL );
    }
    
}; // push_or_pop_chat_roster ( int push_or_pop )


/**
 * This function finds the index of the array where the chat flags for
 * this person are stored.  It does this by exploiting on the (unique?)
 * dialog section to use entry of each (friendly) droid.
 */
int ResolveDialogSectionToChatFlagsIndex (const char* SectionName )
{
    int i;
    for ( i=0; i < sizeof(dialog_name_to_index) / sizeof(dialog_name_to_index[0]); i++) {
	if ( !strcmp(SectionName, dialog_name_to_index[i] . name ) )
		return dialog_name_to_index[i] . person_id;
    }

    DebugPrintf ( -1000 , "\n--------------------\nSectionName: %s." , SectionName );
    ErrorMessage ( __FUNCTION__  , "\
There was a dialog section to be used with a droid (name %s), that does not have a \n\
corresponding chat flags array index as defined in chat.c." ,
			       PLEASE_INFORM, IS_FATAL, SectionName );
    return (-1);
    
}; // int ResolveDialogSectionToChatFlagsIndex ( Enemy ChatDroid )


/**
 * This function plants a cookie, i.e. sets a new text string with the
 * purpose of serving as a flag.  These flags can be set/unset from the dialog
 * file and used from within there and they get stored and loaded with
 * every gave via the tux_t structure.
 */
void PlantCookie (const char* CookieString )
{
    int i;
    
    //--------------------
    // First a security check against attempts to plant too long cookies...
    //
    if ( strlen ( CookieString ) >= MAX_COOKIE_LENGTH -1 )
    {
	fprintf( stderr, "\n\nCookieString: %s\n" , CookieString );
	ErrorMessage ( __FUNCTION__  , "\
There was a cookie given that exceeds the maximal length allowed for a\n\
cookie to be set in FreedroidRPG.",
				   PLEASE_INFORM, IS_FATAL );
    }
    
    //--------------------
    // Check if maybe the cookie has already been set.  In this case we would
    // not have to do anything...
    //
    for ( i = 0 ; i < MAX_COOKIES ; i ++ )
    {
	if ( strlen ( Me . cookie_list [ i ] ) <= 0 )
	{
	    if ( !strcmp ( Me . cookie_list [ i ] , CookieString ) )
	    {
		DebugPrintf ( 0 , "\n\nTHAT COOKIE WAS ALREADY SET... DOING NOTHING...\n\n" );
		return;
	    }
	}
    }
    
    //--------------------
    // Now we find a good new and free position for our new cookie...
    //
    for ( i = 0 ; i < MAX_COOKIES ; i ++ )
    {
	if ( strlen ( Me . cookie_list [ i ] ) <= 0 )
	{
	    break;
	}
    }
    
    //--------------------
    // Maybe the position we have found is the last one.  That would mean too
    // many cookies, a case that should never occur in FreedroidRPG and that is
    // a considered a fatal error...
    //
    if ( i >= MAX_COOKIES ) 
    {
	fprintf( stderr, "\n\nCookieString: %s\n" , CookieString );
	ErrorMessage ( __FUNCTION__  , "\
There were no more free positions available to store this cookie.\n\
This should not be possible without a severe bug in FreedroidRPG.",
				   PLEASE_INFORM, IS_FATAL );
    }
    
    //--------------------
    // Now that we know that we have found a good position for storing our
    // new cookie, we can do it.
    //
    strcpy ( Me . cookie_list [ i ] , CookieString );
    DebugPrintf ( 0 , "\n\nNEW COOKIE STORED:  Position=%d Text='%s'.\n\n" , 
		  i , CookieString );
    
    
}; // void PlantCookie ( char* CookieString )


/**
 * This function deletes planted cookie, i.e. delete a text string with the
 * purpose of serving as a flag.  These flags can be set/unset from the dialog
 * file and used from within there and they get stored and loaded with
 * every gave via the tux_t structure.
 */
void DeleteCookie (const char* CookieString)
{
    DebugPrintf ( -4 , "\nDeleting cookie: '%s'." , CookieString );
    
    int i;
    for ( i = 0 ; i < MAX_COOKIES ; i ++ )
	{
	DebugPrintf ( 1 , "\nCookie entry to compare to: %s." , Me . cookie_list [ i ] );
	if ( ! strlen ( Me . cookie_list [ i ] ) ) continue;
	if ( ! strcmp ( Me . cookie_list [ i ] , CookieString ) ) 
		break;
	//--------------------
	// Now some extra safety, cause the ':' termination character might still be on 
	// the cookie or on the comparison string
	//
	if ( strcmp ( Me . cookie_list [ i ] , CookieString ) >= ( ( int ) strlen ( CookieString ) ) ) 
	    break; 
	}
	
    if (i == MAX_COOKIES){
        DebugPrintf ( -4 , "Cookie not found.");
    } else {
	strcpy ( Me . cookie_list [ i ] , "" ) ;
	DebugPrintf ( 1 , "Cookie deleted.");
    }
}; // void DeleteCookie ( char* CookieString )

/**
 * This function should load new chat dialogue information from the 
 * chat info file into the chat roster.
 *
 */
static void LoadDialog ( char* FullPathAndFullFilename )
{
    char *ChatData;
    char *SectionPointer;
    char *EndOfSectionPointer;
    int i , j ;
    char fpath[2048];
    int OptionIndex;
    int NumberOfOptionsInSection;
    int NumberOfReplySubtitles;
    int NumberOfReplySamples;
    
    char* ReplyPointer;
    char* YesNoString;
    
    sprintf(fpath, "%s", FullPathAndFullFilename);
    
    // #define END_OF_DIALOGUE_FILE_STRING "*** End of Dialogue File Information ***"
#define CHAT_CHARACTER_BEGIN_STRING "Beginning of new chat dialog for character=\""
#define CHAT_CHARACTER_END_STRING "End of chat dialog for character"
#define NEW_OPTION_BEGIN_STRING "Nr="
    
    //--------------------
    // At first we read the whole chat file information into memory
    //
    ChatData = ReadAndMallocAndTerminateFile( fpath , CHAT_CHARACTER_END_STRING ) ;
    SectionPointer = ChatData ;
   
    //--------------------
    // Read the initialization code
    //
    if(chat_initialization_code) {
	free(chat_initialization_code);
	chat_initialization_code = NULL;
    }
    chat_initialization_code = ReadAndMallocStringFromDataOptional(SectionPointer, "FirstTime LuaCode={", "}", 0);

    if(chat_startup_code) {
	free(chat_startup_code);
	chat_startup_code = NULL;
    }
    chat_startup_code = ReadAndMallocStringFromDataOptional(SectionPointer, "EveryTime LuaCode={", "}", 0);

    //--------------------
    // At first we go take a look on how many options we have
    // to decode from this section.
    //
    NumberOfOptionsInSection = CountStringOccurences ( SectionPointer , NEW_OPTION_BEGIN_STRING ) ;
    DebugPrintf( CHAT_DEBUG_LEVEL , "\nWe have counted %d Option entries in this section." , NumberOfOptionsInSection ) ;
    
    //--------------------
    // Now we see which option index is assigned to this option.
    // It may happen, that some numbers are OMITTED here!  This
    // should be perfectly ok and allowed as far as the code is
    // concerned in order to give the content writers more freedom.
    //
    for ( i = 0 ; i < NumberOfOptionsInSection; i ++ )
    {
	SectionPointer = LocateStringInData ( SectionPointer, NEW_OPTION_BEGIN_STRING );
	ReadValueFromString( SectionPointer , NEW_OPTION_BEGIN_STRING, "%d" , 
			     &OptionIndex , SectionPointer + strlen(NEW_OPTION_BEGIN_STRING) + 50);

	DebugPrintf( CHAT_DEBUG_LEVEL , "\nFound New Option entry.  Index found is: %d. " , OptionIndex ) ;
	SectionPointer++;

	// Find the end of this dialog option
	EndOfSectionPointer = strstr(SectionPointer, NEW_OPTION_BEGIN_STRING);

	if (EndOfSectionPointer) {//then we are not the last option
	    *EndOfSectionPointer = 0;
	}
	
	// Anything that is loaded into the chat roster doesn't need to be freed,
	// cause this will be done by the next 'InitChatRoster' function anyway.
	//
	ChatRoster[ OptionIndex ] . option_text = 
	    ReadAndMallocStringFromDataOptional ( SectionPointer , "Text=\"" , "\"", 0 ) ;
	if (!ChatRoster[ OptionIndex ] . option_text) {
	    ChatRoster[ OptionIndex ] . option_text = 
		ReadAndMallocStringFromData ( SectionPointer, "Text=_\"" , "\"" ) ;
	}

	DebugPrintf( CHAT_DEBUG_LEVEL , "\nText found : \"%s\"." , ChatRoster[ OptionIndex ] . option_text );

	ChatRoster[ OptionIndex ] . option_sample_file_name = 
	    ReadAndMallocStringFromDataOptional ( SectionPointer , "OptionSample=\"" , "\"", 0 ) ;

	if (!ChatRoster[OptionIndex].option_sample_file_name)
	    ChatRoster[OptionIndex].option_sample_file_name = strdup("Sorry_No_Voice_Sample_Yet_0.wav");

	DebugPrintf( CHAT_DEBUG_LEVEL , "\nOptionSample found : \"%s\"." , ChatRoster[ OptionIndex ] . option_sample_file_name );
	
#define NEW_REPLY_SAMPLE_STRING "ReplySample=\""
#define NEW_REPLY_SUBTITLE_STRING "NPC=_\""
	
	//--------------------
	// We count the number of Subtitle and Sample combinations and then
	// we will read them out
	//
	NumberOfReplySamples = CountStringOccurences ( SectionPointer , NEW_REPLY_SAMPLE_STRING ) ;
	NumberOfReplySubtitles = CountStringOccurences ( SectionPointer , NEW_REPLY_SUBTITLE_STRING ) ;
	if ( NumberOfReplySamples != NumberOfReplySubtitles && NumberOfReplySamples > 0)
	{
	    fprintf( stderr, "\n\nNumberOfReplySamples: %d NumberOfReplySubtitles: %d \n" , NumberOfReplySamples , NumberOfReplySubtitles );
	    fprintf( stderr, "The section in question looks like this: \n%s\n\n" , SectionPointer );
	    ErrorMessage ( __FUNCTION__  , "\
There were an unequal number of reply samples and subtitles specified\n\
within a section of the Freedroid.dialogues file.\n\
This is allowed in Freedroid only if there are no subtitles - ie. either specify\n\
one subtitle per reply, or no subtitle at all.\n\
This is currently not allowed in Freedroid and therefore indicates a\n\
severe error.",
				       PLEASE_INFORM, IS_FATAL );
	}
	
	//--------------------
	// Now that we know exactly how many Sample and Subtitle sections 
	// to read out, we can well start reading exactly that many of them.
	// 
	ReplyPointer = SectionPointer;

	for ( j = 0 ; j < NumberOfReplySubtitles ; j ++ )
	    {
	    ChatRoster[ OptionIndex ] . reply_subtitle_list [ j ] =
		ReadAndMallocStringFromData ( ReplyPointer , NEW_REPLY_SUBTITLE_STRING, "\"" ) ;

	    if (!NumberOfReplySamples)
		ChatRoster[ OptionIndex ] . reply_sample_list [ j ] = strdup("Sorry_No_Voice_Sample_Yet_0.wav");
	    else
		ChatRoster[ OptionIndex ] . reply_sample_list [ j ] =
		    ReadAndMallocStringFromData ( ReplyPointer , NEW_REPLY_SAMPLE_STRING, "\"" ) ;

	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nSubtitle found : \"%s\"." , ChatRoster[ OptionIndex ] . reply_subtitle_list [ j ] );


	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nReplySample found : \"%s\"." , ChatRoster[ OptionIndex ] . reply_sample_list [ j ] );

	    //--------------------
	    // Now we must move the reply pointer to after the previous combination.
	    //
	    ReplyPointer = LocateStringInData ( ReplyPointer, NEW_REPLY_SUBTITLE_STRING );
	    ReplyPointer ++;
	    }
	
	//--------------------
	// Next thing we do will be to get the always-on-startup flag status.
	//
	if ( CountStringOccurences ( SectionPointer , "AlwaysExecuteThisOptionPriorToDialogStart" ) ) {
	    // Now we read in if this item can be used by the influ without help
	    YesNoString = ReadAndMallocStringFromData ( SectionPointer , "AlwaysExecuteThisOptionPriorToDialogStart=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
		{
		ChatRoster[ OptionIndex ] . always_execute_this_option_prior_to_dialog_start = TRUE;
		}
	    else if ( strcmp( YesNoString , "no" ) == 0 )
		{
		ChatRoster[ OptionIndex ] . always_execute_this_option_prior_to_dialog_start = FALSE;
		}
	    else
		{
		ErrorMessage ( __FUNCTION__  , "\
			AlwaysExecuteThisOptionPriorToDialogStart must be yes or no, was %s in file %s. \n",
			PLEASE_INFORM, IS_FATAL, YesNoString, FullPathAndFullFilename );
		}
	    free ( YesNoString ) ;
	}
	else {
	    ChatRoster[ OptionIndex ] . always_execute_this_option_prior_to_dialog_start = FALSE;
	}

	if (strstr(SectionPointer, "LuaCode")) {
	    ChatRoster[OptionIndex] . lua_code = ReadAndMallocStringFromData(SectionPointer, "LuaCode={", "}");
	} else  ChatRoster[OptionIndex] . lua_code = NULL;

	ChatRoster[OptionIndex].exists = 1;
	
	if (EndOfSectionPointer)
	    *EndOfSectionPointer = NEW_OPTION_BEGIN_STRING[0];
    }
    
    //--------------------
    // Now we've got all the information we wanted from the dialogues file.
    // We can now free the loaded file again.  Upon a new character dialogue
    // being initiated, we'll just reload the file.  This is very conveninet,
    // for it allows making and testing changes to the dialogues without even
    // having to restart Freedroid!  Very cool!
    //
    free( ChatData );
    
}; // void LoadDialog ( char* SequenceCode )

/**
 *
 *
 */
void make_sure_chat_portraits_loaded_for_this_droid ( Enemy this_droid )
{
    SDL_Surface* Small_Droid;
    SDL_Surface* Large_Droid;
    char fpath[2048];
    char fname[500];
    int i;
    int model_number;
    static int first_call = TRUE ;
    static int this_type_has_been_loaded [ ENEMY_ROTATION_MODELS_AVAILABLE ] ;
    
    //--------------------
    // We make sure we only load the portrait files once and not
    // every time...
    //
    if ( first_call )
    {
	for ( i = 0 ; i < ENEMY_ROTATION_MODELS_AVAILABLE ; i ++ )
	    this_type_has_been_loaded [ i ] = FALSE ;
    }
    first_call = FALSE ;
    
    //--------------------
    // We look up the model number for this chat partner.
    //
    model_number = this_droid -> type ;
    
    //--------------------
    // We should make sure, that we don't double-load images that we have loaded
    // already, thereby wasting more resources, including OpenGL texture positions.
    //
    if ( this_type_has_been_loaded [ model_number ] )
	return;
    this_type_has_been_loaded [ model_number ] = TRUE ;
    
    //--------------------
    // At first we try to load the image, that is named after this
    // chat section.  If that succeeds, perfect.  If not, we'll revert
    // to a default image.
    //
    strcpy( fname, "droids/" );
    strcat( fname, PrefixToFilename [ model_number ] ) ;
    strcat( fname , "/portrait.png" );
    find_file (fname, GRAPHICS_DIR, fpath, 0);
    DebugPrintf ( 2 , "\nFilename used for portrait: %s." , fpath );
    
    Small_Droid = our_IMG_load_wrapper (fpath) ;
    if ( Small_Droid == NULL )
    {
	strcpy( fname, "droids/" );
	strcat( fname, "DefaultPortrait.png" );
	find_file (fname, GRAPHICS_DIR, fpath, 0);
	Small_Droid = our_IMG_load_wrapper ( fpath ) ;
    }
    if ( Small_Droid == NULL )
    {
	fprintf( stderr, "\n\nfpath: %s \n" , fpath );
	ErrorMessage ( __FUNCTION__  , "\
It wanted to load a small portrait file in order to display it in the \n\
chat interface of Freedroid.  But:  Loading this file has ALSO failed.",
				   PLEASE_INFORM, IS_FATAL );
    }
    
    Large_Droid = zoomSurface( Small_Droid , (float) Droid_Image_Window . w / (float) Small_Droid -> w , 
			       (float) Droid_Image_Window . w / (float) Small_Droid -> w , 0 );
    
    SDL_FreeSurface( Small_Droid );
    
    if ( use_open_gl )
	{
	chat_portrait_of_droid [ model_number ] . surface = SDL_CreateRGBSurface(0, Large_Droid -> w, Large_Droid -> h, 32, rmask, gmask, bmask, amask);
	SDL_SetAlpha(Large_Droid, 0, SDL_ALPHA_OPAQUE);
	our_SDL_blit_surface_wrapper ( Large_Droid, NULL, chat_portrait_of_droid [ model_number ] . surface, NULL );
	SDL_FreeSurface ( Large_Droid );
	}
    else chat_portrait_of_droid [ model_number ] . surface = Large_Droid;
    
    
}; // void make_sure_chat_portraits_loaded_for_this_droid ( Enemy this_droid )


/**
 * This function prepares the chat background window and displays the
 * image of the dialog partner and also sets the right font.
 */
static void PrepareMultipleChoiceDialog ( Enemy ChatDroid , int with_flip )
{
    //--------------------
    // The dialog will always take more than a few seconds to process
    // so we need to prevent framerate distortion...
    //
    Activate_Conservative_Frame_Computation( );
    
    //--------------------
    // We make sure that all the chat portraits we might need are
    // loaded....
    //
    make_sure_chat_portraits_loaded_for_this_droid ( ChatDroid ) ;
    
    //--------------------
    // We select small font for the menu interaction...
    //
    SetCurrentFont( FPS_Display_BFont );
    
    blit_special_background ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE );
    our_SDL_blit_surface_wrapper ( chat_portrait_of_droid [ ChatDroid -> type ] . surface , NULL , 
				   Screen , &Droid_Image_Window );
    
    if ( with_flip ) 
	our_SDL_flip_wrapper( );
        
}; // void PrepareMultipleChoiceDialog ( int Enum )

/**
 * During the Chat with a friendly droid or human, there is a window with
 * the full text transcript of the conversation so far.  This function is
 * here to display said text window and it's content, scrolled to the
 * position desired by the player himself.
 */
void
display_current_chat_protocol ( int background_picture_code , enemy* ChatDroid , int with_update )
{
    const SDL_Rect Subtitle_Window = { .x = CHAT_SUBDIALOG_WINDOW_X,
				       .y = CHAT_SUBDIALOG_WINDOW_Y, 
				       .w = CHAT_SUBDIALOG_WINDOW_W,
				       .h = CHAT_SUBDIALOG_WINDOW_H};
    int lines_needed ;
    int protocol_offset ;
    
#define LINES_IN_PROTOCOL_WINDOW (UNIVERSAL_COORD_H(9))

    SetCurrentFont( FPS_Display_BFont );
    
    lines_needed = GetNumberOfTextLinesNeeded ( chat_protocol , Subtitle_Window , TEXT_STRETCH );
    
    if ( lines_needed <= LINES_IN_PROTOCOL_WINDOW ) 
    {
	//--------------------
	// When there isn't anything to scroll yet, we keep the default
	// position and also the users clicks on up/down button will be
	// reset immediately
	//
	protocol_offset = 0 ;
	chat_protocol_scroll_override_from_user = 0 ;
    }
    else
    {
        // prevent the user from scrolling down too far
	if ( chat_protocol_scroll_override_from_user >= 1 ) 
	    chat_protocol_scroll_override_from_user --;

	protocol_offset = ((int) (  FontHeight(GetCurrentFont()) * TEXT_STRETCH ) *
	     (lines_needed - LINES_IN_PROTOCOL_WINDOW + chat_protocol_scroll_override_from_user)) ;
    }

    //--------------------
    // Prevent the player from scrolling 
    // too high (negative protocol offset)
    //
    if ( protocol_offset < 0 )
    {
	chat_protocol_scroll_override_from_user ++ ;
	protocol_offset = 0 ;
    }
    
    //--------------------
    // Now we need to clear this window, cause there might still be some
    // garbage from the previous subtitle in there...
    //
    PrepareMultipleChoiceDialog ( ChatDroid , FALSE);
    
    //--------------------
    // Now we can display the text and update the screen...
    //
    SDL_SetClipRect( Screen, NULL );
    DisplayText ( chat_protocol , Subtitle_Window.x , Subtitle_Window.y - protocol_offset , &Subtitle_Window , TEXT_STRETCH );
    if ( protocol_offset > 0 ) 
	ShowGenericButtonFromList ( CHAT_PROTOCOL_SCROLL_UP_BUTTON );
    else
	ShowGenericButtonFromList ( CHAT_PROTOCOL_SCROLL_OFF_BUTTON );
    if ( lines_needed <= LINES_IN_PROTOCOL_WINDOW || chat_protocol_scroll_override_from_user >= 0) 
	ShowGenericButtonFromList ( CHAT_PROTOCOL_SCROLL_OFF2_BUTTON );
    else
	ShowGenericButtonFromList ( CHAT_PROTOCOL_SCROLL_DOWN_BUTTON );

    if ( with_update ) 
	our_SDL_flip_wrapper();
    
}; // void display_current_chat_protocol ( int background_picture_code , int with_update )

/**
 * This function should first display a subtitle and then also a sound
 * sample.  It is not very sophisticated or complicated, but nevertheless
 * important, because this combination does indeed occur so often.
 */
void GiveSubtitleNSample(const char* SubtitleText, const char* SampleFilename , enemy* ChatDroid , int with_update )
{
    int do_display = 1;
    int do_wait = 1;

    strcat ( chat_protocol , SubtitleText );
    strcat ( chat_protocol , "\n" );
    
    if (!strcmp(SampleFilename, "NO_WAIT")) {
	do_wait = 0;
    }

    if (do_display)
	display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , with_update );
    
    if (do_wait)
	PlayOnceNeededSoundSample( SampleFilename , do_wait, FALSE );
}; // void GiveSubtitleNSample( char* SubtitleText , char* SampleFilename )

void run_subdialog(const char * tmp_filename)
{
    char fpath[2048];
    char finaldir[50];
    int old_partner_code = chat_control_partner_code;
    
    push_or_pop_chat_roster ( PUSH_ROSTER );

    sprintf(finaldir, "%s", DIALOG_DIR);
    find_file (tmp_filename , finaldir, fpath, 0);

    LoadDialog ( fpath );

    chat_control_partner_code = PERSON_SUBDIALOG_DUMMY;

    // we always initialize subdialogs..
    //
    int i;
    for (i = 0; i < MAX_ANSWERS_PER_PERSON; i ++)
	{
	Me . Chat_Flags [ PERSON_SUBDIALOG_DUMMY ] [ i ] = 0;
	}

    if(chat_initialization_code)
	run_lua(chat_initialization_code);

    DoChatFromChatRosterData( PERSON_SUBDIALOG_DUMMY , chat_control_chat_droid , FALSE );

    push_or_pop_chat_roster ( POP_ROSTER );

    chat_control_partner_code = old_partner_code;
    chat_control_end_dialog = 0;
    chat_control_next_node = -1;
    chat_control_partner_started = 0;
}

/**
 *
 *
 */
static void ProcessThisChatOption ( int MenuSelection , int ChatPartnerCode , Enemy ChatDroid )
{
    int i;
    //reset chat control variables for this option
    chat_control_end_dialog = 0;
    chat_control_next_node = -1; 

    //--------------------
    // Now a menu section has been made.  We do the reaction:
    // say the samples and the replies, later we'll set the new option values
    //
    // But it might be the case that this option is more technical and not accompanied
    // by any reply.  This case must also be caught.
    //
    //printf("Processing option %d with partner %d\n", MenuSelection, ChatPartnerCode);
    if ( strcmp ( ChatRoster [ MenuSelection ] . option_sample_file_name , "NO_SAMPLE_HERE_AND_DONT_WAIT_EITHER" ) )
    {
	strcat ( chat_protocol , "\1TUX: " );
	GiveSubtitleNSample ( L_(ChatRoster [ MenuSelection ] . option_text) ,
			      ChatRoster [ MenuSelection ] . option_sample_file_name , ChatDroid , TRUE ) ;
	strcat ( chat_protocol , "\2" );
    }
    
    //--------------------
    // Now we can proceed to execute
    // the rest of the reply that has been set up for this (the now maybe modified)
    // dialog option.
    //
    for ( i = 0 ; i < MAX_REPLIES_PER_OPTION ; i ++ )
    {
	//--------------------
	// Once we encounter an empty string here, we're done with the reply...
	//
	if ( ! strlen ( ChatRoster [ MenuSelection ] . reply_subtitle_list [ i ] ) ) 
	    break;
	
	GiveSubtitleNSample ( L_(ChatRoster [ MenuSelection ] . reply_subtitle_list [ i ]) ,
			      ChatRoster [ MenuSelection ] . reply_sample_list [ i ] , ChatDroid , TRUE ) ;
    }
    
    if (ChatRoster[MenuSelection].lua_code) {
	run_lua(ChatRoster[MenuSelection].lua_code);
    }
}; // int ProcessThisChatOption ( int MenuSelection , int ChatPartnerCode , Enemy ChatDroid )


/**
 * This is the most important subfunction of the whole chat with friendly
 * droids and characters.  After the pure chat data has been loaded from
 * disk, this function is invoked to handle the actual chat interaction
 * and the dialog flow.
 */
static void DoChatFromChatRosterData( int ChatPartnerCode , Enemy ChatDroid , int clear_protocol )
{
    int i ;
    SDL_Rect Chat_Window;
    char* DialogMenuTexts[ MAX_ANSWERS_PER_PERSON ];
    chat_control_partner_started = (ChatDroid -> will_rush_tux );
    SDL_Event event;

    // Reset chat control variables.
    chat_control_end_dialog = 0;
    chat_control_next_node = -1;

    //--------------------
    // We always should clear the chat protocol.  Only for SUBDIALOGS it is
    // suitable not to clear the chat protocol.
    //
    if ( clear_protocol )
    {
	if ( chat_protocol != NULL ) free ( chat_protocol );
	chat_protocol = MyMalloc ( 500000 ); // enough for any chat...
	strcpy ( chat_protocol , "\2--- " );
	strcat ( chat_protocol , _("Start of Dialog") );
	strcat ( chat_protocol , " ---\n" );
	chat_protocol_scroll_override_from_user = 0 ;
	SetCurrentFont ( FPS_Display_BFont );
    }
    
    display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , TRUE );
    
    Chat_Window . x = 242 ; Chat_Window . y = 100 ; Chat_Window . w = 380; Chat_Window . h = 314 ;
    
    //--------------------
    // We load the option texts into the dialog options variable..
    //
    for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
	if ( strlen ( ChatRoster [ i ] . option_text ) )
	{
	    DialogMenuTexts [ i ] = L_(ChatRoster [ i ] . option_text) ;
	}
    }
    // DialogMenuTexts [ MAX_ANSWERS_PER_PERSON - 1 ] = " END ";

    //--------------------
    // Now we execute all the options that were marked to be executed
    // prior to dialog startup
    //
    // DEPRECATED
    for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
	if ( ChatRoster [ i ] . always_execute_this_option_prior_to_dialog_start )
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nExecuting option no. %d prior to dialog start.\n" , i );
	    ProcessThisChatOption ( i , ChatPartnerCode , ChatDroid );
	    if (chat_control_end_dialog)
		goto wait_click_and_out;
	}
    }
	
    while (1)
	{
	// Now we run the startup code
	if (chat_startup_code && chat_control_next_node == -1) {
	    run_lua(chat_startup_code);
	    free(chat_startup_code);
	    chat_startup_code = NULL; //and free it immediately so as not to run it again in this session
	}

	if (chat_control_next_node == -1)
	    {
	    chat_control_next_node = ChatDoMenuSelectionFlagged ( _("What will you say?") , DialogMenuTexts , Me . Chat_Flags [ ChatPartnerCode ]  , 1 , -1 , FPS_Display_BFont , ChatDroid );
	    //--------------------
	    // We do some correction of the menu selection variable:
	    // The first entry of the menu will give a 1 and so on and therefore
	    // we need to correct this to more C style.
	    //
	    chat_control_next_node --;
	    }
	
	if ( ( chat_control_next_node >= MAX_ANSWERS_PER_PERSON ) || ( chat_control_next_node < 0 ) )
	    {
	    DebugPrintf ( 0 , "%s: Error: chat_control_next_node %i out of range!\n" , __FUNCTION__, chat_control_next_node );
	    chat_control_next_node = END_ANSWER ;
	    }

	ProcessThisChatOption ( chat_control_next_node , ChatPartnerCode , ChatDroid );
	
	if (chat_control_end_dialog)
	    goto wait_click_and_out;
	}
    
wait_click_and_out:   
    while(1) {
	SDL_WaitEvent(&event);
	switch(event.type) {
	    case SDL_KEYDOWN:
	    case SDL_MOUSEBUTTONDOWN:
		return;
	    case SDL_QUIT:
		Terminate(0);
	}
    }
    
}; // void DoChatFromChatRosterData( ... )

/**
 * When the Tux (or rather the player :) ) clicks on a friendly droid,
 * a chat menu will be invoked to do the communication with that friendly
 * character.  However, before the chat menu even starts up, there is a
 * certain time frame still spent in the isometric viewpoint where the
 * two characters (Tux and the chat partner) should turn to each other,
 * so the scene looks a bit more personal and realistic.  This function
 * handles that time interval and the two characters turning to each
 * other.
 */
void
DialogPartnersTurnToEachOther ( Enemy ChatDroid )
{
    int TurningDone = FALSE;
    float AngleInBetween;
    float WaitBeforeTurningTime = 0.00001 ;
    float WaitAfterTurningTime = 0.0001 ;
    int TurningStartTime;
    float OldAngle;
    float RightAngle;
    float TurningDirection;
    
#define TURN_SPEED 900.0

    //--------------------
    // We reset the mouse cursor shape and abort any other
    // mouse global mode operation.
    //
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    
    Activate_Conservative_Frame_Computation();
    
    //--------------------
    // We make sure the one droid in question is in the standing and not
    // in the middle of the walking motion when turning to the chat partner...
    //
    // Calling AnimatEnemies() ONCE for this task seems justified...
    //
    ChatDroid -> speed . x = 0 ;
    ChatDroid -> speed . y = 0 ;
    
    //--------------------
    // At first do some waiting before the turning around starts...
    //
    TurningStartTime = SDL_GetTicks();  TurningDone = FALSE ;
    while ( !TurningDone )
    {
	StartTakingTimeForFPSCalculation();       
	
	AssembleCombatPicture ( SHOW_ITEMS | USE_OWN_MOUSE_CURSOR ); 
	
	our_SDL_flip_wrapper();
	
	if ( ( SDL_GetTicks() - TurningStartTime ) >= 1000.0 * WaitBeforeTurningTime )
	    TurningDone = TRUE;
	
	ComputeFPSForThisFrame();
    }
    
    //--------------------
    // Now we find out what the final target direction of facing should
    // be.
    //
    // For this we use the atan2, which gives angles from -pi to +pi.
    // 
    // Attention must be paid, since 'y' in our coordinates ascends when
    // moving down and descends when moving 'up' on the scren.  So that
    // one sign must be corrected, so that everything is right again.
    //
    RightAngle = ( atan2 ( - ( Me . pos . y - ChatDroid -> pos . y ) ,  
			   + ( Me . pos . x - ChatDroid -> pos . x ) ) * 180.0 / M_PI ) ;
    //
    // Another thing there is, that must also be corrected:  '0' begins
    // with facing 'down' in the current rotation models.  Therefore angle
    // 0 corresponds to that.  We need to shift again...
    //
    RightAngle += 90 ;
    
    //--------------------
    // Now it's time do determine which direction to move, i.e. if to 
    // turn to the left or to turn to the right...  For this purpose
    // we convert the current angle, which is between 270 and -90 degrees
    // to one between -180 and +180 degrees...
    //
    if ( RightAngle > 180.0 ) RightAngle -= 360.0 ; 
    
    // DebugPrintf ( 0 , "\nRightAngle: %f." , RightAngle );
    // DebugPrintf ( 0 , "\nCurrent angle: %f." , ChatDroid -> current_angle );
    
    //--------------------
    // Having done these preparations, it's now easy to determine the right
    // direction of rotation...
    //
    AngleInBetween = RightAngle - ChatDroid -> current_angle ;
    if ( AngleInBetween > 180 ) AngleInBetween -= 360;
    if ( AngleInBetween <= -180 ) AngleInBetween += 360;
    
    if ( AngleInBetween > 0 )
	TurningDirection = +1 ; 
    else 
	TurningDirection = -1 ; 
    
    //--------------------
    // Now we turn and show the image until both chat partners are
    // facing each other, mostly the chat partner is facing the Tux,
    // since the Tux may still turn around to somewhere else all the 
    // while, if the chose so
    //
    TurningStartTime = SDL_GetTicks();  TurningDone = FALSE ;
    while ( !TurningDone )
    {
	StartTakingTimeForFPSCalculation();       
	
	AssembleCombatPicture ( SHOW_ITEMS | USE_OWN_MOUSE_CURSOR ); 
	our_SDL_flip_wrapper();
	
	OldAngle = ChatDroid -> current_angle;
	
	ChatDroid -> current_angle = OldAngle + TurningDirection * Frame_Time() * TURN_SPEED ;
	
	//--------------------
	// In case of positive turning direction, we wait, till our angle is greater
	// than the right angle.
	// Otherwise we wait till our angle is lower than the right angle.
	//
	AngleInBetween = RightAngle - ChatDroid -> current_angle ;
	if ( AngleInBetween > 180 ) AngleInBetween -= 360;
	if ( AngleInBetween <= -180 ) AngleInBetween += 360;
	
	if ( ( TurningDirection > 0 ) && ( AngleInBetween < 0 ) ) TurningDone = TRUE;
	if ( ( TurningDirection < 0 ) && ( AngleInBetween > 0 ) ) TurningDone = TRUE;
	
	ComputeFPSForThisFrame();
    }
    
    //--------------------
    // Now that turning around is basically done, we still wait a few frames
    // until we start the dialog...
    //
    TurningStartTime = SDL_GetTicks();  TurningDone = FALSE ;
    while ( !TurningDone )
    {
	StartTakingTimeForFPSCalculation();       
	
	AssembleCombatPicture ( SHOW_ITEMS | USE_OWN_MOUSE_CURSOR ); 
	our_SDL_flip_wrapper();
	
	if ( ( SDL_GetTicks() - TurningStartTime ) >= 1000.0 * WaitAfterTurningTime )
	    TurningDone = TRUE;
	
	ComputeFPSForThisFrame();
    }
    
}; // void DialogPartnersTurnToEachOther ( Enemy ChatDroid )

/**
 * This is more or less the 'main' function of the chat with friendly 
 * droids and characters.  It is invoked directly from the user interface
 * function as soon as the player requests communication or there is a
 * friendly bot who rushes Tux and opens talk.
 */
void ChatWithFriendlyDroid( enemy * ChatDroid )
{
    int i ;
    SDL_Rect Chat_Window;
    char* DialogMenuTexts[ MAX_ANSWERS_PER_PERSON ];
    int ChatFlagsIndex = (-1);
    char fpath[2048];
    char tmp_filename[5000];
   
    chat_control_chat_droid = ChatDroid; 
    //--------------------
    // Now that we know, that a chat with a friendly droid is planned, the 
    // friendly droid and the Tux should first turn to each other before the
    // real dialog is started...
    //
    DialogPartnersTurnToEachOther ( ChatDroid );
    
    Chat_Window . x = 242 ; Chat_Window . y = 100; Chat_Window . w = 380; Chat_Window . h = 314;
    
    //--------------------
    // First we empty the array of possible answers in the
    // chat interface.
    //
    for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
	DialogMenuTexts [ i ] = "" ;
    }
    
    while ( MouseLeftPressed ( ) || MouseRightPressed() );
    
    //--------------------
    // We clean out the chat roster from any previous use
    //
    InitChatRosterForNewDialogue(  );
    
    ChatFlagsIndex = ResolveDialogSectionToChatFlagsIndex ( ChatDroid -> dialog_section_name ) ;
    
    strcpy ( tmp_filename , ChatDroid -> dialog_section_name );
    strcat ( tmp_filename , ".dialog" );
    char finaldir[50];
    sprintf(finaldir, "%s", DIALOG_DIR);
    find_file (tmp_filename , finaldir, fpath, 0);
    LoadDialog ( fpath );
    
    chat_control_partner_code = ChatFlagsIndex;

    if ( ! Me . chat_character_initialized [ ChatFlagsIndex ] )
	{ // then we must initialize this character
        int i;
	for (i = 0; i < MAX_ANSWERS_PER_PERSON; i ++)
		{
		Me . Chat_Flags [ ChatFlagsIndex ] [ i ] = 0;
		}

	if(chat_initialization_code)
	    run_lua(chat_initialization_code);

        Me . chat_character_initialized [ ChatFlagsIndex ] = 1;
	}

    
    //--------------------
    // Now with the loaded chat data, we can do the real chat now...
    //
    DoChatFromChatRosterData( ChatFlagsIndex , ChatDroid , TRUE );
    
}; // void ChatWithFriendlyDroid( int Enum );



#undef _chat_c
