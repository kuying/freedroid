------------------------------------------------------------------------------
-- Defines the list of underlay floor tiles and the list of overlay floor tiles.
--
-- A list can contain a mix of static and animated floor tiles.
--
-- A static floor tile is defined by the name of its image.
--
-- An animated floor tile is defined by a lua array containing:
--  - a frame per second value (float data)
--  - a list of image names
-- Example: { animation_fps = 0.5, filenames = { "iso_img_0.png", "iso_img_1.png", ... } }
------------------------------------------------------------------------------

underlay_floor_tile_list {
	"iso_miscellaneous_floor_0007.png",
	"iso_miscellaneous_floor_0000.png",
	"iso_miscellaneous_floor_0001.png",
	"iso_miscellaneous_floor_0002.png",
	"iso_miscellaneous_floor_0003.png",
	"iso_miscellaneous_floor_0005.png",
	"iso_miscellaneous_floor_0008.png",
	"iso_miscellaneous_floor_0009.png",
	"iso_miscellaneous_floor_0010.png",
	"iso_miscellaneous_floor_0011.png",
	"iso_miscellaneous_floor_0012.png",
	"iso_miscellaneous_floor_0013.png",
	"iso_miscellaneous_floor_0014.png",
	"iso_miscellaneous_floor_0015.png",
	"iso_miscellaneous_floor_0016.png",
	"iso_miscellaneous_floor_0017.png",
	"iso_miscellaneous_floor_0018.png",
	"iso_miscellaneous_floor_0019.png",
	"iso_miscellaneous_floor_0020.png",
	"iso_miscellaneous_floor_0021.png",
	"iso_miscellaneous_floor_0022.png",
	"iso_miscellaneous_floor_0023.png",

	"iso_sidewalk_0000.png",
	"iso_sidewalk_0001.png",
	"iso_sidewalk_0002.png",
	"iso_sidewalk_0003.png",
	"iso_sidewalk_0004.png",
	"iso_sidewalk_0005.png",
	"iso_sidewalk_0006.png",
	"iso_sidewalk_0007.png",
	"iso_sidewalk_0008.png",
	"iso_sidewalk_0009.png",
	"iso_sidewalk_0010.png",
	"iso_sidewalk_0011.png",
	"iso_sidewalk_0012.png",
	"iso_sidewalk_0013.png",
	"iso_sidewalk_0014.png",
	"iso_sidewalk_0015.png",
	"iso_sidewalk_0016.png",
	"iso_sidewalk_0017.png",
	"iso_sidewalk_0018.png",
	"iso_sidewalk_0019.png",
	"iso_sidewalk_0020.png",
	"iso_sidewalk_0021.png",
	"iso_sidewalk_0022.png",
	"iso_sidewalk_0023.png",
	"iso_sidewalk_0024.png",

	"iso_miscellaneous_floor_0004.png",
	"iso_sand_floor_0001.png",
	"iso_sand_floor_0002.png",
	"iso_sand_floor_0003.png",
	"iso_sand_floor_0004.png",
	"iso_sand_floor_0005.png",
	"iso_sand_floor_0006.png",
	"iso_water_0000.png",
	"iso_water_edge_0001.png",
	"iso_water_edge_0002.png",
	"iso_water_edge_0003.png",
	"iso_water_edge_0004.png",
	"iso_water_edge_0005.png",
	"iso_water_edge_0006.png",
	"iso_water_edge_0007.png",
	"iso_water_edge_0008.png",
	"iso_water_edge_0009.png",
	"iso_water_edge_0010.png",
	"iso_water_edge_0011.png",
	"iso_water_edge_0012.png",
	"iso_water_edge_0013.png",
	"iso_water_edge_0014.png",
	"iso_water_sidewalk_01.png",
	"iso_water_sidewalk_02.png",
	"iso_water_sidewalk_03.png",
	"iso_water_sidewalk_04.png",

	"iso_carpet_tile_0001.png",
	"iso_carpet_tile_0002.png",
	"iso_carpet_tile_0003.png",
	"iso_carpet_tile_0004.png",
	"iso_large_square_bbb.png",
	"iso_large_square_brb.png",
	"iso_large_square_brr.png",
	"iso_large_square_gbb.png",
	"iso_large_square_grb.png",
	"iso_large_square_grr.png",
	"iso_large_square_rbb.png",
	"iso_large_square_rrb.png",
	"iso_large_square_rrr.png",
	"iso_mini_square_0001.png",
	"iso_mini_square_0002.png",
	"iso_mini_square_0003.png",
	"iso_mini_square_0004.png",
	"iso_mini_square_0005.png",
	"iso_mini_square_0006.png",
	"iso_mini_square_0007.png",
	"iso_mini_square_0008.png",
	"iso_square_tile_aab.png",
	"iso_square_tile_acb2.png",
	"iso_square_tile_acb.png",
	"iso_square_tile_adb2.png",
	"iso_square_tile_adb.png",
	"iso_square_tile_cab2.png",
	"iso_square_tile_cab.png",
	"iso_square_tile_ccb.png",
	"iso_square_tile_dab2.png",
	"iso_square_tile_dab.png",
	"iso_square_tile_ddb.png",
	"iso_complicated_cmm.png",
	"iso_complicated_cmm2.png",
	"iso_complicated_p4.png",
	"iso_complicated_pmg.png",
	"iso_complicated_pmg2.png",
	"iso_complicated_pmm.png",
	"iso_twosquare_0001.png",
	"iso_twosquare_0002.png",
	"iso_twosquare_0003.png",
	{ filenames = { "iso_water_1_0000.png", "iso_water_1_0001.png", "iso_water_1_0002.png", "iso_water_1_0003.png", "iso_water_1_0004.png", "iso_water_1_0005.png", "iso_water_1_0006.png", "iso_water_1_0007.png", "iso_water_1_0008.png", "iso_water_1_0009.png", "iso_water_1_0010.png", "iso_water_1_0011.png", "iso_water_1_0012.png", "iso_water_1_0013.png", "iso_water_1_0014.png", "iso_water_1_0015.png" },
	  animation_fps = 14 },
	{ filenames = { "disco/iso_0_0000.png", "disco/iso_20_0000.png", "disco/iso_40_0000.png", "disco/iso_60_0000.png", "disco/iso_80_0000.png", "disco/iso_100_0000.png", "disco/iso_120_0000.png", "disco/iso_140_0000.png", "disco/iso_160_0000.png", "disco/iso_180_0000.png", "disco/iso_200_0000.png", "disco/iso_220_0000.png", "disco/iso_240_0000.png", "disco/iso_260_0000.png", "disco/iso_280_0000.png", "disco/iso_300_0000.png", "disco/iso_320_0000.png", "disco/iso_340_0000.png" },
	 animation_fps = 20 }
}

overlay_floor_tile_list {
	"iso_grass_floor_0029.png",
	"iso_grass_floor_0030.png",
	"iso_grass_floor_0031.png",
	"iso_grass_floor_0032.png",
	"iso_grass_floor_0033.png",
	"iso_grass_floor_0034.png",
	"iso_grass_floor_0035.png",
	"iso_grass_floor_0036.png",
	"iso_grass_floor_0037.png",
	"iso_grass_floor_0038.png",
	"iso_grass_floor_0039.png",
	"iso_grass_floor_0040.png",
	"iso_grass_floor_0041.png",
	"iso_grass_floor_0042.png",
	"iso_grass_floor_0043.png",
	"iso_grass_floor_0044.png",
	"iso_grass_floor_0045.png",
	"iso_grass_floor_0046.png",
	"iso_grass_floor_0047.png",
	"iso_grass_floor_0048.png",
	"iso_grass_floor_0049.png",

	"iso_grass_floor_0000.png",
	"iso_grass_floor_0001.png",
	"iso_grass_floor_0002.png",
	"iso_grass_floor_0003.png",
	"iso_grass_floor_0004.png",

	"iso_grass_floor_0024.png",
	"iso_grass_floor_0025.png",
	"iso_grass_floor_0026.png",
	
	"iso_ground_marker_blue_line_ew.png",
	"iso_ground_marker_blue_line_sn.png",
	"iso_ground_marker_blue_line_we.png",
	"iso_ground_marker_blue_line_ns.png",
	"iso_ground_marker_blue_curve_ws.png",
	"iso_ground_marker_blue_curve_nw.png",
	"iso_ground_marker_blue_curve_ne.png",
	"iso_ground_marker_blue_curve_es.png",
	"iso_ground_marker_red_line_ew.png",
	"iso_ground_marker_red_line_sn.png",
	"iso_ground_marker_red_line_we.png",
	"iso_ground_marker_red_line_ns.png",
	"iso_ground_marker_red_curve_ws.png",
	"iso_ground_marker_red_curve_nw.png",
	"iso_ground_marker_red_curve_ne.png",
	"iso_ground_marker_red_curve_es.png",
	"iso_ground_marker_blue_y_nwe.png",
	"iso_ground_marker_blue_y_nws.png",
	"iso_ground_marker_blue_y_nes.png",
	"iso_ground_marker_blue_y_wes.png",
	"iso_ground_marker_blue_x_nwes.png",
	"iso_ground_marker_red_y_nwe.png",
	"iso_ground_marker_red_y_nws.png",
	"iso_ground_marker_red_y_nes.png",
	"iso_ground_marker_red_y_wes.png",
	"iso_ground_marker_red_x_nwes.png"
}
