/* 
 *
 *   Copyright (c) 2011 Catalin Badea
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

#define _game_ui_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * This macro is used for creating callbacks that updates
 * a widget flag using an external variable.
 *
 * @param widget_type the type of the widget for which the callback is set.
 * @param flag the widget flag to be updated.
 * @param data the external data used for updating.
 */
#define WIDGET_UPDATE_FLAG_ON_DATA(widget_type, flag, data) \
({ \
  void anonymous_func(struct widget *w) \
  { \
    widget_type(w)->flag = data; \
  } \
  anonymous_func; \
})

static struct widget_group *game_widget_group = NULL;

/** Toggle the skills screen when the current skill button is pressed. */
static void current_skill_button_click(struct widget_button *w)
{
	toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_SKILLS);
}

/** Computes the tooltip text displayed when hovering the current skill button. */
static char *get_current_skill_button_tooltip()
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	autostr_printf(buffer, _("%s\nHeat: %d\nRevision: %d\n"), SpellSkillMap[Me.readied_skill].name, calculate_program_heat_cost(Me.readied_skill), Me.skill_level[Me.readied_skill]);

	return buffer->value;
}

/** Update the skill button image to match the selected skill. */
static void current_skill_button_update(struct widget *w)
{
	spell_skill_spec *spec = &SpellSkillMap[Me.readied_skill];
	load_skill_icon_if_needed(spec);
	WIDGET_BUTTON(w)->image[0][DEFAULT] = &spec->icon_surface;
}

/** Update the weapon button image to match the wielded weapon. */
static void current_weapon_button_update(struct widget *w)
{
	if (Me.weapon_item.type == -1 || &Me.weapon_item == item_held_in_hand)
		WIDGET_BUTTON(w)->image[0][DEFAULT] = NULL;
	else
		WIDGET_BUTTON(w)->image[0][DEFAULT] = get_item_inventory_image(Me.weapon_item.type);
}

/** Computes the tooltip text displayed when hovering the current weapon button. */
static char *get_current_weapon_button_tooltip()
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	if (Me.weapon_item.type == -1 || &Me.weapon_item == item_held_in_hand)
		return NULL;

	autostr_printf(buffer, "");
	append_item_description(buffer, &(Me.weapon_item));

	return buffer->value;
}

/** Refresh the ammo indicator text widget. */
static void current_ammo_update(struct widget *w)
{
	struct auto_string *text = WIDGET_TEXT(w)->text;
	autostr_printf(text, "");

	if (Me.weapon_item.type == -1 || &Me.weapon_item == item_held_in_hand)
		return;
	if (!ItemMap[Me.weapon_item.type].item_gun_use_ammunition)
		return;

	if (Me.busy_type == WEAPON_RELOAD)
		autostr_printf(text, _("reloading"));
	else if (!Me.weapon_item.ammo_clip)
		autostr_printf(text, _("%s EMPTY"), font_switchto_red);
	else
		autostr_printf(text, "%5d / %2d", Me.weapon_item.ammo_clip, ItemMap[Me.weapon_item.type].item_gun_ammo_clip_size);
}

/** Activate the quest log interface. */
static void toggle_log_screen(struct widget_button *w)
{
	// The quest browser interface runs in a separate loop, preventing widgets from
	// receiving events. Because of this, the toggle_log button doesn't receive
	// the MOUSE_LEAVE event, once the quest browser is open. Thus, we have to reset the state of the
	// toggle_log button 'by hand'. Note: this is a hack, until the quest browser is reimplemented.
	w->state = DEFAULT;
	quest_browser_interface();
}

/** Toggle inventory screen on/off. */
static void toggle_inventory_screen(struct widget_button *w)
{
	toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_INVENTORY);
}

/** Toggle character screen on/off. */
static void toggle_character_screen(struct widget_button *w)
{
	toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_CHARACTER);
}

/** Toggle the skill screen when the skill button is clicked. */
static void toggle_skill_screen(struct widget_button *w)
{
	toggle_game_config_screen_visibility(GAME_CONFIG_SCREEN_VISIBLE_SKILLS);
}

/**
 * This function draws a vertical status bar representing the amount of running power
 * currently available to the player.
 */
static void stamina_bar_display(struct widget *w)
{
	static Uint32 normal_fill_color = 0;
	static Uint32 empty_color = 0;
	static Uint32 rest_fill_color = 0;
	static Uint32 infinite_fill_color = 0;

	// Initialize drawing colors.
	if (!normal_fill_color) {
		empty_color = SDL_MapRGBA(Screen->format, 20, 20, 20, 80);
		normal_fill_color = SDL_MapRGBA(Screen->format, 255, 255, 0, 80);
		rest_fill_color = SDL_MapRGBA(Screen->format, 255, 20, 20, 80);
		infinite_fill_color = SDL_MapRGBA(Screen->format, 255, 255, 255, 80);
	}

	float max_value;
	float current_value;
	Uint32 filled_color;

	// Set color and fill ratio values
	if (curShip.AllLevels[Me.pos.z]->infinite_running_on_this_level) {
		max_value = 2.0;
		current_value = 2.0;
		filled_color = infinite_fill_color;
	} else {
		max_value = Me.max_running_power;
		current_value = Me.running_power;

		if (Me.running_must_rest)
			filled_color = rest_fill_color;
		else
			filled_color = normal_fill_color;
	}

	// Draw the status bar.
	blit_vertical_status_bar(max_value, current_value, filled_color, empty_color, w->rect.x, w->rect.y, w->rect.w, w->rect.h);

	if (GameConfig.cheat_running_stamina)
		PutStringFont(Screen, Messagestat_BFont, w->rect.x, w->rect.y, "C");
}

/** Computes the tooltip text displayed when hovering the stamina bar. */
static char *get_stamina_bar_tooltip()
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	autostr_printf(buffer, "%s\n%s%d/%d\n", _("RUN"), Me.running_power / Me.max_running_power <= 0.1 ? font_switchto_red : "", (int)Me.running_power, (int)Me.max_running_power);

	return buffer->value;
}

/** This function displays the experience status bar. */
static void experience_bar_display(struct widget *w)
{
	static Uint32 fill_color = 0;
	static Uint32 empty_color = 0;
	int exp_range = Me.ExpRequired - Me.ExpRequired_previously;
	int exp_achieved = Me.Experience - Me.ExpRequired_previously;

	// Avoid arithmetic exceptions.
	if (Me.ExpRequired <= 1)
		return;
	if ((Me.Experience > Me.ExpRequired) || (exp_range <= 1) || (exp_achieved < 1))
		return;

	// Initialize the status bar colors.
	if (!fill_color) {
		empty_color = SDL_MapRGBA(Screen->format, 50, 50, 50, 80);
		fill_color = SDL_MapRGBA(Screen->format, 255, 120, 120, 80);
	}

	blit_vertical_status_bar(exp_range, exp_achieved, fill_color, empty_color, w->rect.x, w->rect.y, w->rect.w, w->rect.h);
}

/** Computes the tooltip text displayed when hovering the experience bar. */
static char *get_experience_bar_tooltip(struct widget *w)
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	autostr_printf(buffer, "%s\n%d/%d\n", _("XP"), Me.Experience, Me.ExpRequired);

	return buffer->value;
}

/** This function displays the health status bar. */
static void health_bar_display(struct widget *w)
{
	static Uint32 fill_color = 0;
	static Uint32 empty_color = 0;

	// Initialize the colors used for drawing.
	if (!fill_color) {
		fill_color = SDL_MapRGBA(Screen->format, 255, 0, 0, 0);
		empty_color = SDL_MapRGBA(Screen->format, 20, 0, 0, 0);
	}

	blit_vertical_status_bar(Me.maxenergy, Me.energy, fill_color, empty_color, w->rect.x, w->rect.y, w->rect.w, w->rect.h);

	if (Me.god_mode)
		PutStringFont(Screen, Messagestat_BFont,  w->rect.x, w->rect.y, "C");
}

/** Computes the tooltip text displayed when hovering the health bar. */
static char* get_health_bar_tooltip(struct widget *w)
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	autostr_printf(buffer, "%s\n%s%d/%d\n", _("Health"), Me.energy / Me.maxenergy <= 0.1 ? font_switchto_red : "", (int)Me.energy, (int)Me.maxenergy);

	return buffer->value;
}

/** This function displays the temperature status bar. */
static void heat_bar_display(struct widget *w)
{
	static Uint32 empty_color = 0;

	// Initialize drawing color.
	if (!empty_color)
		empty_color = SDL_MapRGBA(Screen->format, 0, 0, 55, 0);

	// Compute temperature ratio.
	int temp_ratio = Me.max_temperature ? (100 * Me.temperature) / Me.max_temperature : 100;
	if (temp_ratio > 100)
		temp_ratio = 100;

	// Compute fill color using the temperature ratio.
	int red;
	int green;
	int blue;
	if (temp_ratio < 25) {
		red = 0;
		green = 2.55 * 4 * temp_ratio;
		blue = 255;
	} else if (temp_ratio < 50) {
		red = 0;
		green = 255;
		blue = 255 - (2.55 * 4 * (temp_ratio - 25));
	} else if (temp_ratio < 75) {
		red = 2.4 * 4 * (temp_ratio - 50);
		green = 255;
		blue = 0;
	} else {
		red = 255;
		green = 255 - (1.8 * 4 * (temp_ratio - 75));
		blue = 0;
	}

	// Make the bar blink if Tux is overheating.
	int add = 0;
	if (Me.temperature > Me.max_temperature) {
		// Use game date to modify the filling color.
		if ((int)(Me.current_game_date) % 2)
			add = 255 - (Me.current_game_date - (int)(Me.current_game_date)) * 255;
		else
			add = (Me.current_game_date - (int)(Me.current_game_date)) * 255;

		blue += add;
		green += add;

		// Make sure color values don't exceed 255.
		blue = blue < 255 ? blue : 255;
		green = green < 255 ? green : 255;
	}

	Uint32 fill_color = SDL_MapRGBA(Screen->format, red, green, blue, 0);

	// If the current temperature is higher than the max temperature, the status bar will exceed its normal rectangle.
	int temperature = Me.temperature > Me.max_temperature ? Me.max_temperature : Me.temperature;

	// Display the status bar.
	blit_vertical_status_bar(Me.max_temperature, temperature, fill_color, empty_color, w->rect.x, w->rect.y, w->rect.w, w->rect.h);
}

/** Computes the tooltip text displayed when hovering the heat bar. */
static char *get_heat_bar_tooltip()
{
	static struct auto_string *buffer = NULL;
	if (!buffer)
		buffer = alloc_autostr(64);

	autostr_printf(buffer, "%s\n%s%d/%d\n", _("Temperature"), Me.temperature / Me.max_temperature >= 0.9 ? font_switchto_red : "", (int)Me.temperature, (int)Me.max_temperature);

	return buffer->value;
}

/**
 * This function builds the hud bar widgets.
 */
static struct widget_group *create_hud_bar()
{
	struct widget_group *hud_bar = widget_group_create();
	widget_set_rect(WIDGET(hud_bar), 0, GameConfig.screen_height - 97, GameConfig.screen_width, 97);

	// hud bar background
	struct widget_background *panel = widget_background_create();
	WIDGET(panel)->rect = WIDGET(hud_bar)->rect;
	widget_group_add(hud_bar, WIDGET(panel));

	// Fixed size tiles
	struct image *img_1 = widget_load_image_resource("widgets/hud_background_1.png", 0);
	int left_panel_x = WIDGET(panel)->rect.x;
	widget_background_add(panel, img_1, left_panel_x, WIDGET(panel)->rect.y, img_1->w, img_1->h);

	struct image *img_3 = widget_load_image_resource("widgets/hud_background_3.png", 0);
	int center_panel_x = WIDGET(panel)->rect.x + (WIDGET(panel)->rect.w - img_3->w) / 2;
	widget_background_add(panel, img_3, center_panel_x, WIDGET(panel)->rect.y, img_3->w, img_3->h);

	struct image *img_5 = widget_load_image_resource("widgets/hud_background_5.png", 0);
	int right_panel_x = WIDGET(panel)->rect.x + WIDGET(panel)->rect.w - img_5->w;
	widget_background_add(panel, img_5, right_panel_x, WIDGET(panel)->rect.y, img_5->w, img_5->h);

	// Scalable tiles.
	// These tiles will expand to fill the gap between the 3 fixed tiles.
	struct image *img_2 = widget_load_image_resource("widgets/hud_background_2.png", 0);
	int left_scaling_panel_x = left_panel_x + img_1->w;
	int fill = center_panel_x - left_scaling_panel_x;
	widget_background_add(panel, img_2, left_scaling_panel_x, WIDGET(panel)->rect.y, fill, img_2->h);

	struct image *img_4 = widget_load_image_resource("widgets/hud_background_4.png", 0);
	int right_scaling_panel_x = center_panel_x + img_3->w;
	fill = right_panel_x - right_scaling_panel_x;
	widget_background_add(panel, img_4, right_scaling_panel_x, WIDGET(panel)->rect.y, fill, img_4->h);

	struct {
		char *image[2][3];
		SDL_Rect rect;
		void (*display)(struct widget *);
		void (*activate_button)(struct widget_button *);
		char *(*get_tooltip_text)();
		void (*update)(struct widget *);
	} b[] = {
		// Current skill button
		{
			{{NULL}},
			{right_panel_x + 78, WIDGET(panel)->rect.y + 17, 64, 64},
			NULL,
			current_skill_button_click,
			get_current_skill_button_tooltip,
			current_skill_button_update
		},
		// Current weapon button
		{
			{{NULL}},
			{left_panel_x + 40, WIDGET(panel)->rect.y + 25, 56, 56},
			NULL,
			(void *)TuxReloadWeapon,
			get_current_weapon_button_tooltip,
			current_weapon_button_update
		},
		// Log button
		{
			{{"mouse_buttons/log_button.png", "mouse_buttons/log_button_yellow.png", NULL},
			{"mouse_buttons/log_button_red.png","mouse_buttons/log_button_yellow.png", NULL}},
			{center_panel_x + 55, WIDGET(panel)->rect.y + 49, 32, 19},
			NULL,
			toggle_log_screen,
			NULL,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, Me.quest_browser_changed)
		},
		// Inventory button
		{
			{{"mouse_buttons/inv_button.png", "mouse_buttons/inv_button_yellow.png", NULL},
			{NULL}},
			{center_panel_x + 99, WIDGET(panel)->rect.y + 49, 32, 19},
			NULL,
			toggle_inventory_screen,
			NULL,
			NULL
		},
		// Character button
		{
			{{"mouse_buttons/cha_button.png", "mouse_buttons/cha_button_yellow.png", NULL},
			{"mouse_buttons/cha_button_red.png", "mouse_buttons/cha_button_yellow.png",NULL}},
			{center_panel_x + 150, WIDGET(panel)->rect.y + 49, 32, 19},
			NULL,
			toggle_character_screen,
			NULL,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active,(Me.points_to_distribute > 0))
		},
		// Skill button
		{
			{{"mouse_buttons/ski_button.png", "mouse_buttons/ski_button_yellow.png", NULL},
			{NULL}},
			{center_panel_x + 192, WIDGET(panel)->rect.y + 49, 32, 19},
			NULL,
			toggle_skill_screen,
			NULL,
			NULL
		},
		// Stamina bar
		{
			{{NULL}},
			{left_panel_x + 6, WIDGET(panel)->rect.y + 11, 6, 50},
			stamina_bar_display,
			NULL,
			get_stamina_bar_tooltip,
			NULL
		},
		// Experience bar
		{
			{{NULL}},
			{left_panel_x + 19, WIDGET(panel)->rect.y + 11, 6, 50},
			experience_bar_display,
			NULL,
			get_experience_bar_tooltip,
			NULL
		},
		// Health bar
		{
			{{NULL}},
			{right_panel_x + 154, WIDGET(panel)->rect.y + 11, 6, 50},
			health_bar_display,
			NULL,
			get_health_bar_tooltip,
			NULL
		},
		// Heat bar
		{
			{{NULL}},
			{right_panel_x + 169, WIDGET(panel)->rect.y + 11, 6, 50},
			heat_bar_display,
			NULL,
			get_heat_bar_tooltip, NULL
		},
	};

	int i, j, k;
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		struct widget_button * wb = widget_button_create();

		// Load button images
		for (j = 0; j < 2; j++)
			for (k = 0; k < 3; k++)
				if (b[i].image[j][k])
					wb->image[j][k] = widget_load_image_resource(b[i].image[j][k], 0);

		widget_set_rect(WIDGET(wb), b[i].rect.x, b[i].rect.y, b[i].rect.w, b[i].rect.h);

		if (b[i].display)
			WIDGET(wb)->display = b[i].display;

		wb->activate_button = b[i].activate_button;
		wb->tooltip.get_text = b[i].get_tooltip_text;
		WIDGET(wb)->update = b[i].update;
		widget_group_add(hud_bar, WIDGET(wb));
	}

	// Ammo indicator
	struct widget_text *ammo = widget_text_create();
	widget_set_rect(WIDGET(ammo), left_panel_x + 36, WIDGET(panel)->rect.y + 77, 100, 22);
	ammo->font = FPS_Display_BFont;
	WIDGET(ammo)->update = current_ammo_update;

	// Message log.
	init_message_log();
	widget_set_rect(WIDGET(message_log), 120,  WIDGET(panel)->rect.y + 7, 415, 46);
	message_log->font = Messagevar_BFont;
	widget_group_add(hud_bar, WIDGET(message_log));

widget_group_add(hud_bar, WIDGET(ammo));

	return hud_bar;
}

/**
 * This function returns the game top level widget and creates it if necessary.
 */
struct widget_group *get_game_ui()
{
	if (game_widget_group)
		// Widgets already initialized.
		return game_widget_group;	

	game_widget_group = widget_group_create();
	widget_set_rect(WIDGET(game_widget_group), 0, 0, GameConfig.screen_width, GameConfig.screen_height);

	// Create map widget.
	// This widget is required to know when a mouse event should be handled
	// by AnalyzePlayerMouseClicks.
	game_map = widget_button_create();
	widget_set_rect(WIDGET(game_map), 0, 0, GameConfig.screen_width, GameConfig.screen_height - 98); 
	widget_group_add(game_widget_group, WIDGET(game_map));

	// Create the hud bar.
	struct widget_group *hud_bar = create_hud_bar(); 
	widget_group_add(game_widget_group, WIDGET(hud_bar));

	struct {
		SDL_Rect rect;
		void (*display)(struct widget *w);
		void (*update)(struct widget *w);
	} panels[] = {
		{
			{0, 0, 320, 480},
			(void *)show_inventory_screen,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, GameConfig.Inventory_Visible)
		},
		{
			{0, 0, 320, 480},
			NULL,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, GameConfig.skill_explanation_screen_visible)
		},
		{
			{GameConfig.screen_width - 320, 0, 320, 480},
			(void *)ShowCharacterScreen,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, GameConfig.CharacterScreen_Visible)
		},
		{
			{GameConfig.screen_width - 320, 0, 320, 480},
			(void *)ShowSkillsScreen,
			WIDGET_UPDATE_FLAG_ON_DATA(WIDGET, enabled, GameConfig.SkillScreen_Visible)
		}
	};

	int i;
	for (i = 0; i < sizeof(panels) / sizeof(panels[0]); i++) {
		struct widget_group *w = widget_group_create();
		widget_set_rect(WIDGET(w), panels[i].rect.x, panels[i].rect.y, panels[i].rect.w, panels[i].rect.h);
		WIDGET(w)->display = panels[i].display;
		WIDGET(w)->update = panels[i].update;
		widget_group_add(game_widget_group, WIDGET(w));
	}

	return game_widget_group;
}
