/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include "interactive_player.h"
#include "cmd_queue.h"
#include "editorinteractive.h"
#include "fieldaction.h"
#include "game_debug_ui.h"
#include "map.h"
#include "player.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "ui_box.h"
#include "ui_button.h"
#include "ui_icongrid.h"
#include "ui_tabpanel.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "watchwindow.h"
#include "error.h"

class Building_Descr;

/*
==============================================================================

BuildGrid IMPLEMENTATION

==============================================================================
*/

#define BG_CELL_WIDTH		34 // extents of one cell
#define BG_CELL_HEIGHT		34


// The BuildGrid presents a selection of buildable buildings
class BuildGrid : public UIIcon_Grid {
public:
	BuildGrid(UIPanel* parent, Tribe_Descr* tribe, int x, int y, int cols);

	UISignal1<int> buildclicked;

	void add(int id);

private:
	void clickslot(int idx);

private:
	Tribe_Descr*		m_tribe;
};


/*
===============
BuildGrid::BuildGrid

Initialize the grid
===============
*/
BuildGrid::BuildGrid(UIPanel* parent, Tribe_Descr* tribe, int x, int y, int cols)
	: UIIcon_Grid(parent, x, y, BG_CELL_WIDTH, BG_CELL_HEIGHT, Grid_Horizontal, cols)
{
	m_tribe = tribe;

	clicked.set(this, &BuildGrid::clickslot);
}


/*
===============
BuildGrid::add

Add a new building to the list of buildable buildings
===============
*/
void BuildGrid::add(int id)
{
	Building_Descr* descr = m_tribe->get_building_descr(id);
	uint picid = descr->get_buildicon();

	UIIcon_Grid::add(picid, (void*)id, descr->get_descname());
}


/*
===============
BuildGrid::clickslot [private]

The icon with the given index has been clicked. Figure out which building it
belongs to and trigger signal buildclicked.
===============
*/
void BuildGrid::clickslot(int idx)
{
	int id = (int)get_data(idx);

	buildclicked.call(id);
}



/*
==============================================================================

FieldActionWindow IMPLEMENTATION

==============================================================================
*/
class FieldActionWindow : public UIUniqueWindow {
public:
	FieldActionWindow(Interactive_Base *iabase, Player* plr, UIUniqueWindowRegistry *registry);
	~FieldActionWindow();

	void init();
	void add_buttons_auto();
	void add_buttons_build(int buildcaps);
	void add_buttons_road(bool flag);
   void add_buttons_attack();

   // Action handlers
	void act_watch();
	void act_show_census();
	void act_show_statistics();
	void act_debug();
	void act_buildflag();
	void act_ripflag();
	void act_buildroad();
	void act_abort_buildroad();
	void act_removeroad();
	void act_build(int idx);
	void act_geologist();
   void act_attack();         /// Launch the attack
   void act_attack_more();    /// Increase the number of soldiers to be launched
   void act_attack_less();    /// Decrease the number of soldiers to be launched
   void act_attack_strong();  /// Prepare to launch strongest soldiers
   void act_attack_weak();    /// Prepare to launch weakest soldiers

private:
   void add_tab(const char* picname, UIPanel* panel);
   void add_button(UIBox* box, const char* picname, void (FieldActionWindow::*fn)());
	void okdialog();

	Interactive_Base    *m_iabase;
   Player              *m_plr;
	Map				     *m_map;

	FCoords		m_field;

	UITab_Panel*	m_tabpanel;
	bool			m_fastclick; // if true, put the mouse over first button in first tab
   
   /// Variables to use with attack dialog
   UITextarea* m_text_attackers;
   uint     m_attackers;      // 0 - Number of available soldiers.
   int      m_attackers_type; // STRONGEST - WEAKEST ...
};

static const char* const pic_tab_buildroad = "pics/menu_tab_buildroad.png";
static const char* const pic_tab_watch = "pics/menu_tab_watch.png";
static const char* const pic_tab_buildhouse[3] = {
	"pics/menu_tab_buildsmall.png",
	"pics/menu_tab_buildmedium.png",
	"pics/menu_tab_buildbig.png"
};
static const char* const pic_tab_buildmine = "pics/menu_tab_buildmine.png";

static const char* const pic_buildroad = "pics/menu_build_way.png";
static const char* const pic_remroad = "pics/menu_rem_way.png";
static const char* const pic_buildflag = "pics/menu_build_flag.png";
static const char* const pic_ripflag = "pics/menu_rip_flag.png";
static const char* const pic_watchfield = "pics/menu_watch_field.png";
static const char* const pic_showcensus = "pics/menu_show_census.png";
static const char* const pic_showstatistics = "pics/menu_show_statistics.png";
static const char* const pic_debug = "pics/menu_debug.png";
static const char* const pic_abort = "pics/menu_abort.png";
static const char* const pic_geologist = "pics/menu_geologist.png";

/// TESTING STUFF
static const char* const pic_tab_attack    = "pics/menu_tab_attack.png";
static const char* const pic_attack_more   = "pics/attack_add_soldier.png";
static const char* const pic_attack_less   = "pics/attack_sub_soldier.png";
static const char* const pic_attack_strong = "pics/attack_strongest.png";
static const char* const pic_attack_weak   = "pics/attack_weakest.png";
static const char* const pic_attack        = "pics/menu_attack.png";


/*
===============
FieldActionWindow::FieldActionWindow

Initialize a field action window, creating the appropriate buttons.
===============
*/
FieldActionWindow::FieldActionWindow(Interactive_Base *iabase, Player* plr, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(iabase, registry, 68, 34, "Action")
{
	// Hooks into the game classes
   m_iabase = iabase;
	m_plr = plr;
	m_map = iabase->get_egbase()->get_map();

	Field *f = m_map->get_field(iabase->get_fieldsel_pos());
	m_field = FCoords(iabase->get_fieldsel_pos(), f);

	iabase->set_fieldsel_freeze(true);

	//
	m_tabpanel = new UITab_Panel(this, 0, 0, 1);
	m_tabpanel->set_snapparent(true);
   m_text_attackers = 0;

	m_fastclick = true;
}

/*
===============
FieldActionWindow::~FieldActionWindow

Free allocated resources, remove from registry.
===============
*/
FieldActionWindow::~FieldActionWindow()
{
	m_iabase->set_fieldsel_freeze(false);
   if (m_text_attackers)
   {
      delete m_text_attackers;
      m_text_attackers = 0;
   }
}


/*
===============
FieldActionWindow::init

Initialize after buttons have been registered.
This mainly deals with mouse placement
===============
*/
void FieldActionWindow::init()
{
	m_tabpanel->resize();

	center_to_parent(); // override UIUniqueWindow position

	// Move the window away from the current mouse position, i.e.
	// where the field is, to allow better view
	int mousex = get_mouse_x();
	int mousey = get_mouse_y();

	if (mousex >= 0 && mousex < get_w() &&
	    mousey >= 0 && mousey < get_h()) {
		if (mousey < get_h()/2)
			set_pos(get_x(), get_y()+get_h());
		else
			set_pos(get_x(), get_y()-get_h());
	}

	// Now force the mouse onto the first button
	// TODO: should be on first tab button if we're building
	set_mouse_pos(17, m_fastclick ? 51 : 17);
}


/*
===============
FieldActionWindow::add_buttons_auto

Add the buttons you normally get when clicking on a field.
===============
*/
void FieldActionWindow::add_buttons_auto()
{
	UIBox* buildbox = 0;
	UIBox* watchbox;

	watchbox = new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);

	// Add road-building actions
	if (m_field.field->get_owned_by() == m_plr->get_player_number()) {
	
		BaseImmovable *imm = m_map->get_immovable(m_field);

		// The box with road-building buttons
		buildbox = new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);

		if (imm && imm->get_type() == Map_Object::FLAG)
		{
			// Add flag actions
			Flag *flag = (Flag*)imm;

			add_button(buildbox, pic_buildroad, &FieldActionWindow::act_buildroad);

			Building *building = flag->get_building();

			if (!building || building->get_playercaps() & (1 << Building::PCap_Bulldoze))
				add_button(buildbox, pic_ripflag, &FieldActionWindow::act_ripflag);

         if(m_iabase->get_egbase()->is_game()) // No geologist in editor
            add_button(buildbox, pic_geologist, &FieldActionWindow::act_geologist);
		}
		else
		{
			int buildcaps = m_plr->get_buildcaps(m_field);

			// Add house building
			if ((buildcaps & BUILDCAPS_SIZEMASK) || (buildcaps & BUILDCAPS_MINE))
				add_buttons_build(buildcaps);

			// Add build actions
			if (buildcaps & BUILDCAPS_FLAG)
				add_button(buildbox, pic_buildflag, &FieldActionWindow::act_buildflag);

			if (imm && imm->get_type() == Map_Object::ROAD)
				add_button(buildbox, pic_remroad, &FieldActionWindow::act_removeroad);
		}
	}
      // There goes actions that can be done to non-owner fields ;)
   else
   {
      add_buttons_attack ();
/*      BaseImmovable *imm = m_map->get_immovable(m_field);
      // The box with road-building buttons
      buildbox = new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);

      if (imm && imm->get_type() == Map_Object::FLAG)
      {
         //Add flag actions
         Flag *flag = (Flag*)imm;

         Building *building = flag->get_building();
         if (building && 
               m_iabase->get_egbase()->is_game() &&
               ((building->get_building_type() == Building::MILITARYSITE) ||
                (building->get_building_type() == Building::WAREHOUSE))
            )
            add_button(buildbox, pic_attack, &FieldActionWindow::act_attack);
      }*/
   }

	// Watch actions, only when game (no use in editor)
   // same for statistics. census is ok
   if(m_iabase->get_egbase()->is_game()) {
      add_button(watchbox, pic_watchfield, &FieldActionWindow::act_watch);
      add_button(watchbox, pic_showstatistics, &FieldActionWindow::act_show_statistics);
   }
	add_button(watchbox, pic_showcensus, &FieldActionWindow::act_show_census);

	if (m_iabase->get_display_flag(Interactive_Base::dfDebug))
		add_button(watchbox, pic_debug, &FieldActionWindow::act_debug);

	// Add tabs
	if (buildbox && buildbox->get_nritems())
		{
		buildbox->resize();
		add_tab(pic_tab_buildroad, buildbox);
		}

	watchbox->resize();
	add_tab(pic_tab_watch, watchbox);
}

void FieldActionWindow::add_buttons_attack ()
{
   UIBox* attackbox = 0;

      // Add attack button
   if (m_field.field->get_owned_by() != m_plr->get_player_number()) 
   {
      
      BaseImmovable *imm = m_map->get_immovable(m_field);
      
         // The box with attack buttons
      attackbox = new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);

      if (imm && imm->get_type() == Map_Object::BUILDING)
      {
            //Add flag actions
         Flag *flag = (Flag*)((Building*)imm)->get_base_flag();

         Building *building = flag->get_building();
         if (building && 
               m_iabase->get_egbase()->is_game() &&
               ((building->get_building_type() == Building::MILITARYSITE) ||
                (building->get_building_type() == Building::WAREHOUSE))
            )
         {
            m_attackers = 0;
            m_attackers_type = STRONGEST;
            add_button(attackbox, pic_attack_less, &FieldActionWindow::act_attack_less);
            
            m_text_attackers = new UITextarea(attackbox, 90, 0, "000/000", Align_Center);
            attackbox->add(m_text_attackers, UIBox::AlignTop);
            
            add_button(attackbox, pic_attack_more, &FieldActionWindow::act_attack_more);
            
            add_button(attackbox, pic_attack_strong, &FieldActionWindow::act_attack_strong);
            add_button(attackbox, pic_attack_weak,   &FieldActionWindow::act_attack_weak);
            
            add_button(attackbox, pic_attack, &FieldActionWindow::act_attack);
            act_attack_more();
         }
      }
   }

      // Add tab
   if (attackbox && attackbox->get_nritems())
   {
      attackbox->resize();
      add_tab(pic_tab_attack, attackbox);
   }
}

/*
===============
FieldActionWindow::add_buttons_build

Add buttons for house building.
===============
*/
void FieldActionWindow::add_buttons_build(int buildcaps)
{
	BuildGrid* bbg_house[3] = { 0, 0, 0 };
	BuildGrid* bbg_mine = 0;

	Tribe_Descr* tribe = m_plr->get_tribe();

	m_fastclick = false;

	for(int id = 0; id < tribe->get_nrbuildings(); id++)
		{
		Building_Descr* descr = tribe->get_building_descr(id);
		BuildGrid** ppgrid;

		// Some buildings cannot be built (i.e. construction site, HQ)
      // and not allowed buildings. The rules are different in editor
      // and game: enhanced buildings _are_ buildable in the editor
      if(m_iabase->get_egbase()->is_game()) {
         if (!descr->get_buildable() || !m_plr->is_building_allowed(id))
            continue;
      } else {
         if(!descr->get_buildable() && !descr->get_enhanced_building()) continue;
      }

		// Figure out if we can build it here, and in which tab it belongs
		if (descr->get_ismine())
			{
			if (!(buildcaps & BUILDCAPS_MINE))
				continue;

			ppgrid = &bbg_mine;
			}
		else
			{
			int size = descr->get_size() - BaseImmovable::SMALL;

			if ((buildcaps & BUILDCAPS_SIZEMASK) < (size+1))
				continue;

			ppgrid = &bbg_house[size];
			}

		// Allocate the tab's grid if necessary
		if (!*ppgrid)
			{
			*ppgrid = new BuildGrid(m_tabpanel, tribe, 0, 0, 5);
			(*ppgrid)->buildclicked.set(this, &FieldActionWindow::act_build);
			}

		// Add it to the grid
		(*ppgrid)->add(id);
		}

	// Add all necessary tabs
	for(int i = 0; i < 3; i++)
		if (bbg_house[i])
			add_tab(pic_tab_buildhouse[i], bbg_house[i]);

	if (bbg_mine)
		add_tab(pic_tab_buildmine, bbg_mine);
}


/*
===============
FieldActionWindow::add_buttons_road

Buttons used during road building: Set flag here and Abort
===============
*/
void FieldActionWindow::add_buttons_road(bool flag)
{
	UIBox* buildbox = new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);

	if (flag)
		add_button(buildbox, pic_buildflag, &FieldActionWindow::act_buildflag);

	add_button(buildbox, pic_abort, &FieldActionWindow::act_abort_buildroad);

	// Add the box as tab
	buildbox->resize();
	add_tab(pic_tab_buildroad, buildbox);
}


/*
===============
FieldActionWindow::add_tab

Convenience function: Adds a new tab to the main tab panel
===============
*/
void FieldActionWindow::add_tab(const char* picname, UIPanel* panel)
{
	m_tabpanel->add(g_gr->get_picture(PicMod_Game, picname, true), panel);
}


/*
===============
FieldActionWindow::add_button
===============
*/
void FieldActionWindow::add_button(UIBox* box, const char* picname, void (FieldActionWindow::*fn)())
{
	UIButton *b = new UIButton(box, 0, 0, 34, 34, 2);
	b->clicked.set(this, fn);
	b->set_pic(g_gr->get_picture(PicMod_Game, picname, true));

	box->add(b, UIBox::AlignTop);
}

/*
===============
FieldActionWindow::okdialog

Call this from the button handlers.
It resets the mouse to its original position and closes the window
===============
*/
void FieldActionWindow::okdialog()
{
   m_iabase->warp_mouse_to_field(m_field);
	die();
}

/*
===============
FieldActionWindow::act_watch()

Open a watch window for the given field and delete self.
===============
*/
void FieldActionWindow::act_watch()
{
	assert(m_iabase->get_egbase()->is_game());
   
   show_watch_window(static_cast<Interactive_Player*>(m_iabase), m_field);
	okdialog();
}


/*
===============
FieldActionWindow::act_show_census
FieldActionWindow::act_show_statistics

Toggle display of census and statistics for buildings, respectively.
===============
*/
void FieldActionWindow::act_show_census()
{
	m_iabase->set_display_flag(Interactive_Base::dfShowCensus,
		!m_iabase->get_display_flag(Interactive_Base::dfShowCensus));
	okdialog();
}

void FieldActionWindow::act_show_statistics()
{
	m_iabase->set_display_flag(Interactive_Base::dfShowStatistics,
		!m_iabase->get_display_flag(Interactive_Base::dfShowStatistics));
	okdialog();
}


/*
===============
FieldActionWindow::act_debug

Show a debug widow for this field.
===============
*/
void FieldActionWindow::act_debug()
{
	show_field_debug(m_iabase, m_field);
}


/*
===============
FieldActionWindow::act_buildflag

Build a flag at this field
===============
*/
void FieldActionWindow::act_buildflag()
{
	Editor_Game_Base* egbase=m_iabase->get_egbase();

   if(egbase->is_game()) { 
      // Game: send command
      Game *g = static_cast<Game*>(egbase); 
      g->send_player_build_flag (m_plr->get_player_number(), m_field);
   } else {
      // Editor: Just plain build this flag
      m_plr->build_flag(m_field);
   }
   if (m_iabase->is_building_road())
      m_iabase->finish_build_road();
	okdialog();
}

/*
===============
FieldActionWindow::act_ripflag

Remove the flag at this field
===============
*/
void FieldActionWindow::act_ripflag()
{
   BaseImmovable* imm = m_iabase->get_egbase()->get_map()->get_immovable(m_field);
   Flag* flag;
   Building* building;

   okdialog();

   if (!imm || imm->get_type() != Map_Object::FLAG)
      return;

   flag = (Flag*)imm;
   building = flag->get_building();

   if (building)
   {
      if (!(building->get_playercaps() & (1 << Building::PCap_Bulldoze)))
         return;

      show_bulldoze_confirm(m_iabase, building, flag);
   }
   else
   {
      if(m_iabase->get_egbase()->is_game()) {
         // Game
         Game *g = static_cast<Game*>(m_iabase->get_egbase()); 
	 g->send_player_bulldoze (flag);
      } else {
         // Editor
         imm->remove(m_iabase->get_egbase());
      }
   }
}


/*
===============
FieldActionWindow::act_buildroad

Start road building.
===============
*/
void FieldActionWindow::act_buildroad()
{
   m_iabase->start_build_road(m_field, m_plr->get_player_number());
   okdialog();
}

/*
===============
FieldActionWindow::act_abort_buildroad

Abort building a road.
===============
*/
void FieldActionWindow::act_abort_buildroad()
{
   if (!m_iabase->is_building_road())
      return;

   m_iabase->abort_build_road();
   okdialog();
}

/*
===============
FieldActionWindow::act_removeroad

Remove the road at the given field
===============
*/
void FieldActionWindow::act_removeroad()
{
   BaseImmovable *imm = m_iabase->get_egbase()->get_map()->get_immovable(m_field);

   if (imm && imm->get_type() == Map_Object::ROAD)
      if(m_iabase->get_egbase()->is_game()) {
         // Game
         Game *g = static_cast<Game*>(m_iabase->get_egbase()); 
         g->send_player_bulldoze (static_cast<PlayerImmovable*>(imm));
      } else {
         Road* road=static_cast<Road*>(imm);
         Player* plr=road->get_owner();
         plr->bulldoze(road);
      }
   okdialog();
}


/*
===============
FieldActionWindow::act_build

Start construction of the building with the give description index
===============
*/
void FieldActionWindow::act_build(int idx)
{
   if(m_iabase->get_egbase()->is_game()) {
      // Game
      Interactive_Player* m_player=static_cast<Interactive_Player*>(m_iabase);
      Game *g = m_player->get_game();
      g->send_player_build (m_player->get_player_number(), m_field, idx);
   } else {
      // Editor
      m_iabase->get_egbase()->warp_building(m_field, m_plr->get_player_number(), idx);
      static_cast<Editor_Interactive*>(m_iabase)->reference_player_tribe(m_plr->get_player_number(), m_plr->get_tribe());
   }
   okdialog();
}


/*
===============
FieldActionWindow::act_geologist

Call a geologist on this flag.
===============
*/
void FieldActionWindow::act_geologist()
{
   assert(m_iabase->get_egbase()->is_game());

   Interactive_Player* m_player=static_cast<Interactive_Player*>(m_iabase);
   Game* g = m_player->get_game();
   BaseImmovable *imm = g->get_map()->get_immovable(m_field);

   if (imm && imm->get_type() == Map_Object::FLAG)
	g->send_player_flagaction (static_cast<Flag*>(imm), FLAGACTION_GEOLOGIST);

   okdialog();
}

/**
 * Here there are a problem: the sender of an event is allways the owner of were is done this even.
 * But for attacks, the owner of an event is the player who start an attack, so is needed to get an
 * extra parameter to the send_player_enemyflagaction, the player number
 *
 */
void FieldActionWindow::act_attack ()
{
   assert(m_iabase->get_egbase()->is_game());
   Interactive_Player* m_player=static_cast<Interactive_Player*>(m_iabase);
   Game* g = m_player->get_game();
   BaseImmovable *imm = g->get_map()->get_immovable(m_field);

   log( "imm: %p, imm->get_type(): %i, m_attackers: %i\n", 
         imm, imm->get_type(), m_attackers);

   // Before doing this is needed to show a window to configure the attack.
   if (imm && imm->get_type() >= Map_Object::BUILDING && m_attackers > 0)
      g->send_player_enemyflagaction (
         static_cast<PlayerImmovable*>(imm)->get_base_flag(), 
         ENEMYFLAGACTION_ATTACK, 
         m_player->get_player_number(),
         m_attackers,  // Number of soldiers
         m_attackers_type); // Type of soldiers

   okdialog();
}
   
void FieldActionWindow::act_attack_more()
{
   // TODO: Recalculate the number of available soldiers
   char buf[20];
   uint available = 99;
   
   if (m_attackers < available)
      m_attackers ++;
   else
      m_attackers = available;

   sprintf(buf, "%d/%d", m_attackers, available);
   m_text_attackers->set_text (buf);
}

void FieldActionWindow::act_attack_less()
{
   // TODO: Recalculate the number of available soldiers
   char buf[20];
   uint available = 99;
   
   if (m_attackers > 0)
      m_attackers --;
   else
      m_attackers = 0;
            
   sprintf(buf, "%d/%d", m_attackers, available);
   m_text_attackers->set_text (buf);
}

void FieldActionWindow::act_attack_strong()
{
   m_attackers_type = STRONGEST;
}


void FieldActionWindow::act_attack_weak()
{
   m_attackers_type = WEAKEST;
}

/*
===============
show_field_action

Perform a field action (other than building options).
Bring up a field action window or continue road building.
===============
*/
void show_field_action(Interactive_Base *iabase, Player* player, UIUniqueWindowRegistry *registry)
{
	FieldActionWindow *faw;

	// Force closing of old fieldaction windows. This is necessary because
	// show_field_action() does not always open a FieldActionWindow (e.g.
	// connecting the road we are building to an existing flag)
	if (registry->window) {
		delete registry->window;
		registry->window = 0;
	}

	if (!iabase->is_building_road()) {
		faw = new FieldActionWindow(iabase, player, registry);
		faw->add_buttons_auto();
		faw->init();
		return;
	}

	// we're building a road right now
	Map *map = iabase->get_egbase()->get_map();
	Coords target = iabase->get_fieldsel_pos();

	// if user clicked on the same field again, build a flag
	if (target == iabase->get_build_road_end()) {
		faw = new FieldActionWindow(iabase, player, registry);

		bool flag = false;
		if (target != iabase->get_build_road_start() &&
		    player->get_buildcaps(target) & BUILDCAPS_FLAG)
			flag = true;
		faw->add_buttons_road(flag);
		faw->init();
		return;
	}

	// append or take away from the road
	if (!iabase->append_build_road(target)) {
		faw = new FieldActionWindow(iabase, player, registry);
		faw->add_buttons_road(false);
		faw->init();
		return;
	}

	// did he click on a flag or a road where a flag can be built?
	BaseImmovable *imm = map->get_immovable(target);

	if (imm) {
		switch(imm->get_type()) {
		case Map_Object::ROAD:
			if (!(player->get_buildcaps(target) & BUILDCAPS_FLAG))
				break;
			if(iabase->get_egbase()->is_game())
				static_cast<Interactive_Player*>(iabase)->get_game()->
				    send_player_build_flag(player->get_player_number(), target);

		case Map_Object::FLAG:
			iabase->finish_build_road();
			break;
		}
	}
}
