/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#define _widgets_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * @struct image_resource
 * Used by widget_load_image_resource to store images.
 */
struct image_resource {
	char *name;
	struct image img;
	struct list_head node;
};

LIST_HEAD(widget_list);
LIST_HEAD(image_resource_list);

/**
 * This function used to store images used by the widget system.
 */
struct image *widget_load_image_resource(char *name, int use_offset_file) 
{
	struct image_resource *res;
	
	// Check if image is already loaded.
	list_for_each_entry(res, &image_resource_list, node) {
		if (!strcmp(name, res->name))
			return &res->img;
	}

	// Image not found, allocate memory and load it from its file.
	res = MyMalloc(sizeof(struct image_resource));
	load_image(&res->img, name, use_offset_file);
	res->name = name;
	list_add(&res->node, &image_resource_list);
	return &res->img;
}

/**
 * This function pushes events to the currently active top level containers. 
 *
 * NOTE: EVENT_UPDATE events are passed to both active and inactive top level containers.
 */
void handle_widget_event(SDL_Event *event)
{
	struct widget *w;
	int is_update_event = 0;
	if (event->type == SDL_USEREVENT && event->user.code == EVENT_UPDATE)
		is_update_event = 1;
	list_for_each_entry(w, &widget_list, node) {
		if (is_update_event || w->enabled)
			w->handle_event(w, event);
	}
}

void widget_set_rect(struct widget *w, int x, int y, int width, int height)
{
	w->rect.x = x;
	w->rect.y = y;
	w->rect.w = width;
	w->rect.h = height;
}

static struct {
	string text;		/**< Tooltip text to be displayed. */
	SDL_Rect widget_rect;	/**< Tooltip owner rectangle. */
} tooltip;	/**< Contains informations about the current tooltip to be displayed.*/

/**
 * This function changes the tooltip text to be displayed
 * by display_tooltips.
 * Set the tooltip to NULL to stop displaying it.
 * @param new_tooltip The new tooltip to be displayed.
 * @param widget_rect Rectangle of the tooltip owner.
 */
void widget_set_tooltip(string new_tooltip, SDL_Rect *widget_rect)
{
	tooltip.text = new_tooltip;
	if (widget_rect)
		tooltip.widget_rect = *widget_rect;
}

/**
 * This function displays tooltips when hovering widgets for a certain amount
 * of time. Widget can set their tooltip to be displayed by calling widget_set_tooltip.
 */
static void display_tooltips()
{
	static float time_spent_on_button = 0;
	static float previous_function_call_time = 0;
	int centered = 1;
	SDL_Rect tooltip_rect;

	// Update the timer.
	time_spent_on_button += SDL_GetTicks() - previous_function_call_time;
	previous_function_call_time = SDL_GetTicks();

	if (!tooltip.text) {
		// No tooltip is set, meaning the mouse is not hovering any widget with tooltip.
		time_spent_on_button = 0;
		return;
	}

	// Level editor specific options.
	if (game_status == INSIDE_LVLEDITOR) {
		if (!GameConfig.show_lvledit_tooltips)
			return;

		if (time_spent_on_button <= 1200)
			return;
		
		centered = 0;	// Editor tooltips are not centered.
	}
	
	// Temporary copy required for not altering the tooltip string.
	char buffer[strlen(tooltip.text) + 1];
	strcpy(buffer, tooltip.text);

	// Set the correct font before computing text width.
	SetCurrentFont(FPS_Display_BFont);

	// Tooltip width is given by the longest line in the tooltip, with a maximum of 400 pixels
	// after which linebreaks are automatically added.
	tooltip_rect.w = longest_line_width(buffer) + 2 * TEXT_BANNER_HORIZONTAL_MARGIN;
	if (tooltip_rect.w > 400)
		tooltip_rect.w = 400;	

	// Compute height
	int lines_in_text = get_lines_needed(tooltip.text, tooltip_rect, LINE_HEIGHT_FACTOR);
	tooltip_rect.h = lines_in_text * FontHeight(FPS_Display_BFont);

	int center_x = tooltip.widget_rect.x + tooltip.widget_rect.w / 2;	
	int center_y = tooltip.widget_rect.y + tooltip.widget_rect.h / 2;	

	// The tooltip is positioned to the left or to the right (whichever is closer 
	// to the screen's center) of the widget's center.
	if (center_x < GameConfig.screen_width / 2)
		tooltip_rect.x = center_x;
	else
		tooltip_rect.x = center_x - tooltip_rect.w;

	// The tooltip is positioned above or under the widget (whichever is closer 
	// to the screen's center). A small offset is added for aesthetic reasons.
	if (center_y < GameConfig.screen_height / 2)
		tooltip_rect.y = tooltip.widget_rect.y + tooltip.widget_rect.h + 4;
	else
		tooltip_rect.y = tooltip.widget_rect.y - tooltip_rect.h - 4;
		

	display_tooltip(tooltip.text, centered, tooltip_rect);
}

/**
 * This functions pushes an EVENT_UPDATE event through the widget system.
 * This should be called once every few frames to update widgets' state.
 *
 * NOTE: Update events are handled by disabled widgets but are not sent to
 * children widgets. This allows for inactive widgets to become active on
 * update events. 
 */
void update_widgets()
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = EVENT_UPDATE;
	handle_widget_event(&event);
}

/**
 * This function displays the currently active top level widget groups.
 */
void display_widgets() 
{
	struct widget *w;
	list_for_each_entry(w, &widget_list, node) {
		if (w->enabled && w->display)
			w->display(w);
	}
	display_tooltips();
}
