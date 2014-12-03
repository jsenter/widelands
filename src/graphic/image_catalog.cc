/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/image_catalog.h"

#include <cassert>
#include <map>
#include <string>

#include "base/log.h" // NOCOM
#include "io/filesystem/layered_filesystem.h"

ImageCatalog::ImageCatalog() {
	init();
}

ImageCatalog::~ImageCatalog() {
	entries_.clear();
}

// Register all images here
void ImageCatalog::init()  {
	entries_.clear();
	// ui_basic
	insert(Keys::kButton0, "ui_basic/but0.png");
	insert(Keys::kButton1, "ui_basic/but1.png");
	insert(Keys::kButton2, "ui_basic/but2.png");
	insert(Keys::kButton3, "ui_basic/but3.png");
	insert(Keys::kButton4, "ui_basic/but4.png");
	insert(Keys::kFilesDirectory, "ui_basic/ls_dir.png");
	insert(Keys::kFilesWLMap, "ui_basic/ls_wlmap.png");
	insert(Keys::kFilesS2Map, "ui_basic/ls_s2map.png");
	insert(Keys::kFilesScenario, "ui_basic/ls_wlscenario.png");
	insert(Keys::kScrollbarUp, "ui_basic/scrollbar_up.png");
	insert(Keys::kScrollbarDown, "ui_basic/scrollbar_down.png");
	insert(Keys::kScrollbarLeft, "ui_basic/scrollbar_left.png");
	insert(Keys::kScrollbarRight, "ui_basic/scrollbar_right.png");
	insert(Keys::kScrollbarBackground, "ui_basic/scrollbar_background.png");
	insert(Keys::kSelect, "ui_basic/fsel.png");
	insert(Keys::kCaret, "ui_basic/caret.png");
	insert(Keys::kCheckbox, "ui_basic/checkbox.png");
	insert(Keys::kCheckboxChecked, "ui_basic/checkbox_checked.png");
	insert(Keys::kCheckboxEmpty, "ui_basic/checkbox_empty.png");
	insert(Keys::kCheckboxLight, "ui_basic/checkbox_light.png");
	insert(Keys::kCursor, "ui_basic/cursor.png");
	insert(Keys::kCursor2, "ui_basic/cursor2.png"); // NOCOM unused?
	insert(Keys::kCursorClick, "ui_basic/cursor_click.png");
	insert(Keys::kListFirst, "ui_basic/list_first_entry.png");
	insert(Keys::kListSecond, "ui_basic/list_second_entry.png"); // NOCOM unused?
	insert(Keys::kListThird, "ui_basic/list_third_entry.png"); // NOCOM unused?
	insert(Keys::kListSelected, "ui_basic/list_selected.png");
	insert(Keys::kActionContinue, "ui_basic/continue.png");
	insert(Keys::kActionDifferent, "ui_basic/different.png");
	insert(Keys::kActionStop, "ui_basic/stop.png");


	// wui
	insert(Keys::kWindowBackground, "wui/window_background.png");
	insert(Keys::kWindowBorderTop, "wui/window_top.png");
	insert(Keys::kWindowBorderLeft, "wui/window_left.png");
	insert(Keys::kWindowBorderRight, "wui/window_right.png");
	insert(Keys::kWindowBorderBottom, "wui/window_bottom.png");
	insert(Keys::kButtonMenuOK, "wui/menu_okay.png");
	insert(Keys::kButtonMenuAbort, "wui/menu_abort.png");

	// ui_fsmenu
	insert(Keys::kFullscreen, "ui_fsmenu/ui_fsmenu.jpg");
	insert(Keys::kFullscreenChooseMap, "ui_fsmenu/choosemapmenu.jpg");
	insert(Keys::kFullscreenFileWiew, "ui_fsmenu/fileviewmenu.jpg");
	insert(Keys::kFullscreenInternet, "ui_fsmenu/internetmenu.jpg");
	insert(Keys::kFullscreenLaunchMPG, "ui_fsmenu/launch_mpg_menu.jpg");
	insert(Keys::kFullscreenMain, "ui_fsmenu/mainmenu.jpg");
	insert(Keys::kFullscreenOptions, "ui_fsmenu/optionsmenu.jpg");

	// loadscreens
	insert(Keys::kLoadscreen, "loadscreens/progress.png");
	insert(Keys::kLoadscreenEditor, "loadscreens/editor.jpg");
	insert(Keys::kLoadscreenTips, "loadscreens/tips_bg.png");
	insert(Keys::kLoadscreenSplash, "loadscreens/splash.jpg");

	// editor
	insert(Keys::kEditorRedo, "wui/editor/editor_redo.png");
	insert(Keys::kEditorUndo, "wui/editor/editor_undo.png");
	insert(Keys::kEditorMenuPlayer, "wui/editor/editor_menu_player_menu.png");
	insert(Keys::kEditorMenuToolBob, "wui/editor/editor_menu_tool_place_bob.png");
	insert(Keys::kEditorMenuToolHeight, "wui/editor/editor_menu_tool_change_height.png");
	insert(Keys::kEditorMenuToolImmovable, "wui/editor/editor_menu_tool_place_immovable.png");
	insert(Keys::kEditorMenuToolNoiseHeight, "wui/editor/editor_menu_tool_noise_height.png");
	insert(Keys::kEditorMenuToolPortSpace, "wui/editor/editor_menu_tool_set_port_space.png");
	insert(Keys::kEditorMenuToolResources, "wui/editor/editor_menu_tool_change_resources.png");
	insert(Keys::kEditorMenuTools, "wui/editor/editor_menu_toggle_tool_menu.png");
	insert(Keys::kEditorMenuToolSize, "wui/editor/editor_menu_set_toolsize_menu.png");
	insert(Keys::kEditorMenuToolTerrain, "wui/editor/editor_menu_tool_set_terrain.png");
	insert(Keys::kEditorTerrainDead, "wui/editor/terrain_dead.png");
	insert(Keys::kEditorTerrainDry, "wui/editor/terrain_dry.png");
	insert(Keys::kEditorTerrainGreen, "wui/editor/terrain_green.png");
	insert(Keys::kEditorTerrainMountain, "wui/editor/terrain_mountain.png");
	insert(Keys::kEditorTerrainUnpassable, "wui/editor/terrain_unpassable.png");
	insert(Keys::kEditorTerrainWater, "wui/editor/terrain_water.png");
	insert(Keys::kEditorToolBob, "wui/editor/fsel_editor_place_bob.png");
	insert(Keys::kEditorToolDelete, "wui/editor/fsel_editor_delete.png");
	insert(Keys::kEditorToolHeight, "wui/editor/fsel_editor_set_height.png");
	insert(Keys::kEditorToolHeightDecrease, "wui/editor/fsel_editor_decrease_height.png");
	insert(Keys::kEditorToolHeightIncrease, "wui/editor/fsel_editor_increase_height.png");
	insert(Keys::kEditorToolImmovable, "wui/editor/fsel_editor_place_immovable.png");
	insert(Keys::kEditorToolInfo, "wui/editor/fsel_editor_info.png");
	insert(Keys::kEditorToolNoiseHeight, "wui/editor/fsel_editor_noise_height.png");
	insert(Keys::kEditorToolPortSpaceSet, "wui/editor/fsel_editor_set_port_space.png");
	insert(Keys::kEditorToolPortSpaceSetSet, "wui/editor/fsel_editor_unset_port_space.png");
	insert(Keys::kEditorToolResourcesDecrease, "wui/editor/fsel_editor_decrease_resources.png");
	insert(Keys::kEditorToolResourcesDelete, "wui/editor/fsel_editor_delete.png");
	insert(Keys::kEditorToolResourcesIncrease, "wui/editor/fsel_editor_increase_resources.png");
	insert(Keys::kEditorToolResourcesSet, "wui/editor/fsel_editor_set_resources.png");

	// stats
	insert(Keys::kStatsCasualties, "wui/stats/genstats_casualties.png");
	insert(Keys::kStatsBuildingsLost, "wui/stats/genstats_civil_blds_lost.png");
	insert(Keys::kStatsKills, "wui/stats/genstats_kills.png");
	insert(Keys::kStatsLandsize, "wui/stats/genstats_landsize.png");
	insert(Keys::kStatsMilitaryStrength, "wui/stats/genstats_militarystrength.png");
	insert(Keys::kStatsMilitarySitesDefeated, "wui/stats/genstats_msites_defeated.png");
	insert(Keys::kStatsMilitarySitesLost, "wui/stats/genstats_msites_lost.png");
	insert(Keys::kStatsBuildingsNumber, "wui/stats/genstats_nrbuildings.png");
	insert(Keys::kStatsWaresNumber, "wui/stats/genstats_nrwares.png");
	insert(Keys::kStatsWorkersNumber, "wui/stats/genstats_nrworkers.png");
	insert(Keys::kStatsPoints, "wui/stats/genstats_points.png");
	insert(Keys::kStatsProductivity, "wui/stats/genstats_productivity.png");
	insert(Keys::kStatsTrees, "wui/stats/genstats_trees.png");

	// minimap
	insert(Keys::kMinimapBuildings, "wui/minimap/button_bldns.png");
	insert(Keys::kMinimapFlags, "wui/minimap/button_flags.png");
	insert(Keys::kMinimapOwner, "wui/minimap/button_owner.png");
	insert(Keys::kMinimapRoads, "wui/minimap/button_roads.png");
	insert(Keys::kMinimapTerrain, "wui/minimap/button_terrn.png");
	insert(Keys::kMinimapZoom, "wui/minimap/button_zoom.png");

	// overlays
	insert(Keys::kOverlaysMapSpot, "wui/overlays/map_spot.png");
	insert(Keys::kOverlaysPlotMine, "wui/overlays/mine.png");
	insert(Keys::kOverlaysPlotBig, "wui/overlays/big.png");
	insert(Keys::kOverlaysPlotMedium, "wui/overlays/medium.png");
	insert(Keys::kOverlaysPlotSmall, "wui/overlays/small.png");
	insert(Keys::kOverlaysPlotPort, "wui/overlays/port.png");
	insert(Keys::kOverlaysRoadbuildingLevel, "wui/overlays/roadb_green.png");
	insert(Keys::kOverlaysRoadbuildingSteepAscending, "wui/overlays/roadb_red.png");
	insert(Keys::kOverlaysRoadbuildingSteepDecending, "wui/overlays/roadb_reddown.png");
	insert(Keys::kOverlaysRoadbuildingAscending, "wui/overlays/roadb_yellow.png");
	insert(Keys::kOverlaysRoadbuildingDecending, "wui/overlays/roadb_yellowdown.png");
	insert(Keys::kOverlaysFlag, "wui/overlays/set_flag.png");
	insert(Keys::kOverlaysWorkarea1, "wui/overlays/workarea1.png");
	insert(Keys::kOverlaysWorkarea2, "wui/overlays/workarea2.png");
	insert(Keys::kOverlaysWorkarea3, "wui/overlays/workarea3.png");
	insert(Keys::kOverlaysWorkarea12, "wui/overlays/workarea12.png");
	insert(Keys::kOverlaysWorkarea23, "wui/overlays/workarea23.png");
	insert(Keys::kOverlaysWorkarea123, "wui/overlays/workarea123.png");

	// ship
	insert(Keys::kShipExploreCounterclockwise, "wui/ship/ship_explore_island_ccw.png");
	insert(Keys::kShipExploreClockwise, "wui/ship/ship_explore_island_cw.png");
	insert(Keys::kShipScoutEast, "wui/ship/ship_scout_e.png");
	insert(Keys::kShipScoutNorthEast, "wui/ship/ship_scout_ne.png");
	insert(Keys::kShipScoutNorthWest, "wui/ship/ship_scout_nw.png");
	insert(Keys::kShipScoutSouthEast, "wui/ship/ship_scout_se.png");
	insert(Keys::kShipScoutSouthWest, "wui/ship/ship_scout_sw.png");
	insert(Keys::kShipScoutWest, "wui/ship/ship_scout_w.png");
	insert(Keys::kShipExpeditionCancel, "wui/ship/menu_ship_cancel_expedition.png");
	insert(Keys::kShipDestination, "wui/ship/menu_ship_destination.png");
	insert(Keys::kShipGoto, "wui/ship/menu_ship_goto.png");
	insert(Keys::kShipSink, "wui/ship/menu_ship_sink.png");

	// buildings
	insert(Keys::kDockExpeditionStart, "wui/buildings/start_expedition.png");
	insert(Keys::kDockExpeditionCancel, "wui/buildings/cancel_expedition.png");
	insert(Keys::kBuildingPriorityHigh, "wui/buildings/high_priority_button.png");
	insert(Keys::kBuildingPriorityNormal, "wui/buildings/normal_priority_button.png");
	insert(Keys::kBuildingPriorityLow, "wui/buildings/low_priority_button.png");
	insert(Keys::kBuildingMaxFillIndicator, "wui/buildings/max_fill_indicator.png");
	insert(Keys::kBuildingAttack, "wui/buildings/menu_attack.png");
	insert(Keys::kBuildingBulldoze, "wui/buildings/menu_bld_bulldoze.png");
	insert(Keys::kBuildingDismantle, "wui/buildings/menu_bld_dismantle.png");
	insert(Keys::kBuildingSoldierDrop, "wui/buildings/menu_drop_soldier.png");
	insert(Keys::kBuildingSoldierHeroes, "wui/buildings/prefer_heroes.png");
	insert(Keys::kBuildingSoldierRookies, "wui/buildings/prefer_rookies.png");
	insert(Keys::kBuildingStockPolicyDontStockButton, "wui/buildings/stock_policy_button_dontstock.png");
	insert(Keys::kBuildingStockPolicyNormalButton, "wui/buildings/stock_policy_button_normal.png");
	insert(Keys::kBuildingStockPolicyPreferButton, "wui/buildings/stock_policy_button_prefer.png");
	insert(Keys::kBuildingStockPolicyRemoveButton, "wui/buildings/stock_policy_button_remove.png");
	insert(Keys::kBuildingStockPolicyDontStock, "wui/buildings/stock_policy_dontstock.png");
	insert(Keys::kBuildingStockPolicyPrefer, "wui/buildings/stock_policy_prefer.png");
	insert(Keys::kBuildingStockPolicyRemove, "wui/buildings/stock_policy_remove.png");

	// players
	insert(Keys::kPlayerStartingPosSmall1, "players/fsel_editor_set_player_01_pos.png");
	insert(Keys::kPlayerStartingPosSmall2, "players/fsel_editor_set_player_02_pos.png");
	insert(Keys::kPlayerStartingPosSmall3, "players/fsel_editor_set_player_03_pos.png");
	insert(Keys::kPlayerStartingPosSmall4, "players/fsel_editor_set_player_04_pos.png");
	insert(Keys::kPlayerStartingPosSmall5, "players/fsel_editor_set_player_05_pos.png");
	insert(Keys::kPlayerStartingPosSmall6, "players/fsel_editor_set_player_06_pos.png");
	insert(Keys::kPlayerStartingPosSmall7, "players/fsel_editor_set_player_07_pos.png");
	insert(Keys::kPlayerStartingPosSmall8, "players/fsel_editor_set_player_08_pos.png");
	insert(Keys::kPlayerStartingPosBig1, "players/editor_player_01_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig2, "players/editor_player_02_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig3, "players/editor_player_03_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig4, "players/editor_player_04_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig5, "players/editor_player_05_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig6, "players/editor_player_06_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig7, "players/editor_player_07_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig8, "players/editor_player_08_starting_pos.png");
	insert(Keys::kPlayerFlag1, "players/genstats_enable_plr_01.png");
	insert(Keys::kPlayerFlag2, "players/genstats_enable_plr_02.png");
	insert(Keys::kPlayerFlag3, "players/genstats_enable_plr_03.png");
	insert(Keys::kPlayerFlag4, "players/genstats_enable_plr_04.png");
	insert(Keys::kPlayerFlag5, "players/genstats_enable_plr_05.png");
	insert(Keys::kPlayerFlag6, "players/genstats_enable_plr_06.png");
	insert(Keys::kPlayerFlag7, "players/genstats_enable_plr_07.png");
	insert(Keys::kPlayerFlag8, "players/genstats_enable_plr_08.png");

	// logos
	insert(Keys::kLogoEditor16, "logos/WL-Editor-16.png");
	insert(Keys::kLogoEditor32, "logos/WL-Editor-32.png");
	insert(Keys::kLogoEditor64, "logos/WL-Editor-64.png");
	insert(Keys::kLogoEditor128, "logos/WL-Editor-128.png");
	insert(Keys::kLogoWidelands16, "logos/wl-ico-16.png");
	insert(Keys::kLogoWidelands32, "logos/wl-ico-32.png");
	insert(Keys::kLogoWidelands48, "logos/wl-ico-48.png");
	insert(Keys::kLogoWidelands64, "logos/wl-ico-64.png");
	insert(Keys::kLogoWidelands128, "logos/wl-ico-128.png");
	insert(Keys::kLogoWidelandsLogo, "logos/wl-logo-64.png");
}

void ImageCatalog::insert(Keys key, const std::string& filename) {
	const std::string path = kBaseDir + filename;
	log("NOCOM cataloging image: %s\n",path.c_str());
	assert(!has_key(key));
	assert(g_fs->file_exists(path));
	entries_.emplace(key, path);
}

// NOCOM try to get rid of this.
const std::string& ImageCatalog::filepath(Keys key) const {
	assert(has_key(key));
	return entries_.at(key);
}

bool ImageCatalog::has_key(Keys key) const {
	return entries_.count(key) == 1;
}
