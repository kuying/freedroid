---------------------------------------------------------------------
-- This file is part of Freedroid
--
-- Freedroid is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- Freedroid is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Freedroid; see the file COPYING. If not, write to the
-- Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
-- MA 02111-1307 USA
----------------------------------------------------------------------

local Tux = FDrpg.get_tux()

return {
	EveryTime = function()
		play_sound("effects/Menu_Item_Deselected_Sound_0.ogg")
		-- Are we human? CAPTCHA!!!
		number_one=math.random(2,7)
		number_two=math.random(1,number_one-1)
		captcha = number_one - number_two
		if (captcha == 1) then
			captcha = _"one"
		elseif (captcha == 2) then
			captcha = _"two"
		elseif (captcha == 3) then
			captcha = _"three"
		elseif (captcha == 4) then
			captcha = _"four"
		elseif (captcha == 5) then
			captcha = _"five"
		elseif (captcha == 6) then
			captcha = _"six"
		end
		response = user_input_string(string.format(_"CAPTCHA: Please write the lowercase word that answers the following: %d - %d = ?", number_one, number_two))
		if (captcha ~= response) then
			npc_says(_"Non-human detected. Administering paralyzing shock.")
			npc_says(_"NOTE: If you are a human, try again, and make sure you enter a word and not digits.")
			freeze_tux_npc(7)
			Tux:hurt(20)
			Tux:heat(20)
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		else
			npc_says(_"Welcome to MS firmware update server for region #54658.")
			npc_says(_"Please select action")
			show("node1", "node3", "node99")
		end
	end,

	{
		id = "node1",
		text = _"Get current OS version",
		code = function()
			if (not HF_FirmwareUpdateServer_uploaded_faulty_firmware_update) then
				npc_says(_"Currently deployed firmware is v. 5.0.8+worldto-nogpl.")
			else
				npc_says(_"Currently deployed firmware is v. 5.0.8+%swashere+gpl", Tux:get_player_name())
			end
		end,
	},
	{
		id = "node3",
		text = _"Propagate firmware update",
		code = function()
			npc_says(_"Compressing image....")
			npc_says(_"Advertising update to clients...")
			npc_says(_"Waiting for all connections....")
			npc_says("...")
			npc_says("...")
			npc_says(_"Please resolve merge conflicts:")
			if (takeover(get_program("Hacking")+4)) then
				npc_says(_"Conflicts resolved,")
				npc_says(_"Firmware update propagated!")
				HF_FirmwareUpdateServer_uploaded_faulty_firmware_update = true
				Tux:end_quest(_"Propagating a faulty firmware update", _"I managed to upload the faulty firmware update to all bots within transmission range. Victory is mine!")
				kill_faction("ms", "no_respawn")
				hide("node3")
			else
				npc_says(_"Merge failed.")
				npc_says(_"Exiting...")
				play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
				end_dialog()
			end
		end,
	},
	{
		id = "node99",
		text = _"Logout",
		code = function()
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
}