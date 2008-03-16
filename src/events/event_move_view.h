/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef EVENT_MOVE_VIEW_H
#define EVENT_MOVE_VIEW_H

#include "widelands.h"
#include "event.h"
#include "widelands_geometry.h"

namespace Widelands {

struct Editor_Game_Base;

struct Event_Move_View : public Event {
	Event_Move_View(char const * Name, State const S)
		: Event(Name, S), m_location(Coords::Null()), m_player(1)
	{}

	int32_t option_menu(Editor_Interactive &);

	State run(Game *);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

	void set_location(Coords const c) {m_location = c;}
	Coords location() {return m_location;}

private:
	Coords        m_location;
	Player_Number m_player;
};

};

#endif
