/*
 * Copyright (C) 2004 by the Widelands Development Team
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

#include "fullscreen_menu_netsetup.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "ui_table.h"
#include "constants.h"
#include "network_lan_promotion.h"
#include "network.h"


Fullscreen_Menu_NetSetup::Fullscreen_Menu_NetSetup ()
	:Fullscreen_Menu_Base("singleplmenu.jpg") // change this
{
	discovery=new LAN_Game_Finder();
	discovery->set_callback (discovery_callback, this);
	
	// Text
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 140, "Begin Network Game", Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 60, 170, 174, 24, 1, JOINGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title("Join a Game");

	b = new UIButton(this, 60, 210, 174, 24, 1, HOSTGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title("Host a New Game");

	b = new UIButton(this, 60, 250, 174, 24, 1, INTERNETGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title("Play in Internet");

	b = new UIButton(this, 60, 290, 174, 24, 0, CANCEL);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title("Back");
	
	// Hostname
	hostname=new UIEdit_Box(this, 288, 170, 174, 24, 2, 0);
	hostname->set_text("localhost");	
	
	// List of open games in local network
	opengames=new UITable(this, 288, 210, 320, 128);
	opengames->add_column ("Host", UITable::STRING, 128);
	opengames->add_column ("Map", UITable::STRING, 128);
	opengames->add_column ("State", UITable::STRING, 64);
	opengames->selected.set (this, &Fullscreen_Menu_NetSetup::game_selected);
}

Fullscreen_Menu_NetSetup::~Fullscreen_Menu_NetSetup ()
{
	delete discovery;
}

void Fullscreen_Menu_NetSetup::think ()
{
	Fullscreen_Menu_Base::think ();
	
	discovery->run ();
}

bool Fullscreen_Menu_NetSetup::get_host_address (ulong& addr, ushort& port)
{
	const char* host=hostname->get_text();
	
	int i;
	for (i=0;i<opengames->get_nr_entries();i++) {
	    LAN_Open_Game* game=(LAN_Open_Game*) (opengames->get_entry(i)->get_user_data());
	    
	    if (!strcmp(game->info.hostname, host)) {
		addr=game->address;
		port=game->port;
		return true;
	    }
	}
	
	hostent* he=gethostbyname(host);
	if (he==0)
	    return false;
	
	addr=((in_addr*) (he->h_addr_list[0]))->s_addr;
	port=htons(WIDELANDS_PORT);
	
	return true;
}

void Fullscreen_Menu_NetSetup::game_selected (int sel)
{
	LAN_Open_Game* game=(LAN_Open_Game*) (opengames->get_selection());
	
	hostname->set_text (game->info.hostname);
}

void Fullscreen_Menu_NetSetup::update_game_info (UITable_Entry* entry, const LAN_Game_Info& info)
{
	entry->set_string (0, info.hostname);
	entry->set_string (1, info.map);
	
	switch (info.state) {
	    case LAN_GAME_OPEN:
		entry->set_string (2, "Open");
		break;
	    case LAN_GAME_CLOSED:
		entry->set_string (2, "Closed");
		break;
	    default:
		entry->set_string (2, "Unknown");
		break;
	}
}

void Fullscreen_Menu_NetSetup::game_opened (const LAN_Open_Game* game)
{
	update_game_info (new UITable_Entry(opengames, (void*) game), game->info);
}

void Fullscreen_Menu_NetSetup::game_closed (const LAN_Open_Game* game)
{
}

void Fullscreen_Menu_NetSetup::game_updated (const LAN_Open_Game* game)
{
	UITable_Entry* entry=opengames->find_entry(game);
	
	if (entry!=0)
	    update_game_info (entry, game->info);
}

void Fullscreen_Menu_NetSetup::discovery_callback (int type, const LAN_Open_Game* game, void* userdata)
{
    switch (type) {
	case LAN_Game_Finder::GameOpened:
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_opened (game);
	    break;
	case LAN_Game_Finder::GameClosed:
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_closed (game);
	    break;
	case LAN_Game_Finder::GameUpdated:
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_updated (game);
	    break;
	default:
	    abort ();
    }
}

