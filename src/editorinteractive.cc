/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "editorinteractive.h"
#include "e_ui.h"
#include "options.h"
#include "editor.h"
#include "map.h"
#include "player.h"
#include "minimap.h"
#include "editor_tools.h"

/*
=============================

class Editor_Tool_Menu

This class is the tool selection window/menu. 
Here, you can select the tool you wish to use the next time

=============================
*/

class Editor_Tool_Menu : public Window {
   public:
      Editor_Tool_Menu(Editor_Interactive*, UniqueWindow*, Editor_Interactive::Editor_Tools*);
      virtual ~Editor_Tool_Menu();

   private:
      Editor_Interactive::Editor_Tools* m_tools;
      UniqueWindow* m_registry;
      Editor_Interactive* m_parent;

      void changed_to_function(int);
};

/*
===============
Editor_Tool_Menu::Editor_Tool_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Menu::Editor_Tool_Menu(Editor_Interactive *parent, UniqueWindow *registry, Editor_Interactive::Editor_Tools* tools)
	: Window(parent, (parent->get_w()-102)/2, (parent->get_h()-136)/2, 350, 200, "Tool Menu")
{
	m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
   m_tools=tools;

   Radiogroup* r=new Radiogroup();
   r->changedto.set(this, &Editor_Tool_Menu::changed_to_function);

   int y = 20;
   uint i;
   for(i = 0; i < m_tools->tools.size(); i++, y+= 25) {
      char buf[32];
      r->add_button(this, 100, y);
      sprintf(buf, "%s", m_tools->tools[i]->get_name());
      new Textarea(this, 125, y+10, buf, Align_VCenter);

   }
   r->set_state(m_tools->current_tool);

}

/*
===============
Editor_Tool_Menu::~Editor_Tool_Menu

Unregister from the registry pointer
===============
*/
Editor_Tool_Menu::~Editor_Tool_Menu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}

/*
===========
Editor_Tool_Menu::changed_to_function()

called when the listselect changes
===========
*/
void Editor_Tool_Menu::changed_to_function(int n) {
   m_tools->current_tool=n;
   // TODO: call some kind of 'you've been selected' function
}

/**********************************************
 *
 * class EditorInteractive
 *
 **********************************************/

/*
==========
Editor_Interactive::Editor_Interactive()

construct editor sourroundings
==========
*/
Editor_Interactive::Editor_Interactive(Editor *e) : Interactive_Base(e) {
   m_editor = e;

   // The mapview. watch the map!!!
   Map_View* mm;
   mm = new Map_View(this, 0, 0, get_w(), get_h(), this);
   mm->warpview.set(this, &Editor_Interactive::mainview_move);
   mm->fieldclicked.set(this, &Editor_Interactive::field_clicked);
   set_mapview(mm);
     
   // The panel. Tools, infos and gimmicks
   m_panel = new ToolPanel(this, 0, get_h()-PANEL_HEIGHT, get_w(), PANEL_HEIGHT);
		

   // user interface buttons
   int x = (get_w() - (4*34)) >> 1;
   int y = get_h() - 34;
   Button *b;

   MiniMapView* minimapview;
	minimapview= new MiniMapView(m_panel, m_panel->get_w()-PANEL_HEIGHT, 0, this, PANEL_HEIGHT, PANEL_HEIGHT);
   minimapview->warpview.set(this, &Editor_Interactive::minimap_warp);
   set_minimapview(minimapview);

   // temp (should be Main menu)
   b = new Button(this, x, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::exit_game_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.bmp", RGBColor(0,0,255)));
   // temp

   b = new Button(this, x+34, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::tool_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/editor_menu_toggle_tool_menu.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+68, y, 34, 34, 2);
   //      b->clicked.set(this, &Interactive_Player::minimap_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+102, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::toggle_buildhelp);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.bmp", RGBColor(0,0,255)));

   // Init Tools
   tools.current_tool=0;
   tools.tools.push_back(new Editor_Info_Tool());
   tools.tools.push_back(new Editor_Increase_Height_Tool());
}

/****************************************
 * Editor_Interactive::~EditorInteractive()
 *
 * cleanup
 */
Editor_Interactive::~Editor_Interactive() {
   while(tools.tools.size()) {
      delete tools.tools.back();
      tools.tools.pop_back();
   }
}

/*
===============
Editor_Interactive::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Editor_Interactive::start()
{
   int mapw;
	int maph;

   m_maprenderinfo.egbase = m_editor; 
	m_maprenderinfo.map = m_editor->get_map();
	m_maprenderinfo.visibility = 0; 
	m_maprenderinfo.show_buildhelp = true;
	
	mapw = m_maprenderinfo.map->get_width();
	maph = m_maprenderinfo.map->get_height();
	m_maprenderinfo.overlay_basic = (uchar*)malloc(mapw*maph);
	m_maprenderinfo.overlay_roads = (uchar*)malloc(mapw*maph);
	memset(m_maprenderinfo.overlay_roads, 0, mapw*maph);
	
	for(int y = 0; y < maph; y++)
		for(int x = 0; x < mapw; x++) {
			FCoords coords(x, y, m_maprenderinfo.map->get_field(x,y));
			
			recalc_overlay(coords);
		}
}

/*
===============
Editor_Interactive::recalc_overlay

Recalculate build help and borders for the given field
===============
*/
void Editor_Interactive::recalc_overlay(FCoords fc)
{
   Map* map = m_maprenderinfo.map;

   // Only do recalcs after maprenderinfo has been setup
   if (!map)
      return;

   uchar code = 0;
   int owner = fc.field->get_owned_by();

   // A border is on every field that is owned by a player and has
   // neighbouring fields that are not owned by that player
   for(int dir = 1; dir <= 6; dir++) {
      FCoords neighb;

      map->get_neighbour(fc, dir, &neighb);

      if (neighb.field->get_owned_by() != owner)
         code = Overlay_Frontier_Base + owner;
   }

   int buildcaps=fc.field->get_caps();
   if(owner) {
      // Determine the buildhelp icon for that field		
      buildcaps = m_editor->get_player(owner)->get_buildcaps(fc);
   }	

   if (buildcaps & BUILDCAPS_MINE)
      code = Overlay_Build_Mine;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG)
      code = Overlay_Build_Big;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_MEDIUM)
      code = Overlay_Build_Medium;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_SMALL)
      code = Overlay_Build_Small;
   else if (buildcaps & BUILDCAPS_FLAG)
      code = Overlay_Build_Flag;

   m_maprenderinfo.overlay_basic[fc.y*map->get_width() + fc.x] = code;
}

  
/** Editor_Interactive::exit_game_btn(void *a)
 *
 * Handle exit button
 */
void Editor_Interactive::exit_game_btn()
{
	end_modal(0);
}

/*
===========
Editor_Interactive::field_clicked()

This functions is called, when a field is clicked. it mainly calls
the function of the currently selected tool
===========
*/
void Editor_Interactive::field_clicked() {
   Map* m=get_map();
   tools.tools[tools.current_tool]->handle_click(&m_maprenderinfo.fieldsel, m->get_field(m_maprenderinfo.fieldsel), m, this);
}

/*
===========
Editor_Interactive::toggle_buildhelp()

toggles the buildhelp on the map
===========
*/
void Editor_Interactive::toggle_buildhelp(void)
{
	m_maprenderinfo.show_buildhelp = !m_maprenderinfo.show_buildhelp;
}

/*
===============
Editor_Interactive::tool_menu_btn

Bring up or close the main menu
===============
*/
void Editor_Interactive::tool_menu_btn()
{
	if (m_toolmenu.window)
		delete m_toolmenu.window;
	else
		new Editor_Tool_Menu(this, &m_toolmenu, &tools);
}
