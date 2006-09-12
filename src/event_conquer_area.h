/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__EVENT_CONQUER_AREA_H
#define __S__EVENT_CONQUER_AREA_H

#include "event.h"
#include "geometry.h"

class Editor_Game_Base;

/*
 * This event shows a message box
 */
class Event_Conquer_Area : public Event {
   public:
     Event_Conquer_Area();
      ~Event_Conquer_Area();

      // one liner functions
      const char* get_id(void) { return "conquer_area"; }

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
      void Write(Section*, Editor_Game_Base*);
      void Read(Section*, Editor_Game_Base*);

      inline void set_coords(Coords pt) { m_pt=pt; }
      inline Coords get_coords(void) { return m_pt; }
      inline int get_player(void) { return m_player; }
      inline void set_player(int i) { m_player=i; }
      inline int get_area(void) { return m_area; }
      inline void set_area(int i) { m_area=i; }

   private:
      Coords m_pt;
      int m_area;
      int m_player;
};



#endif
