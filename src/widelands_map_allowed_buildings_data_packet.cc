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

#include "editorinteractive.h"
#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "widelands_map_allowed_buildings_data_packet.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Allowed_Buildings_Data_Packet::~Widelands_Map_Allowed_Buildings_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Allowed_Buildings_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // First of all: if we shouldn't skip, all buildings default to false in the game (!not editor)
      if(!skip && egbase->is_game()) {
         int i=0;
         for(i=1; i<=egbase->get_map()->get_nrplayers(); i++) {
            Player* plr=egbase->get_player(i);
            if(!plr) continue;
            Tribe_Descr* t=plr->get_tribe();
            
            int b;
            for(b=0; b<t->get_nrbuildings(); b++) {
               plr->allow_building(b, false);
            }
         }
      }

      // Now read all players and buildings
      int i=0;
      for(i=1; i<=egbase->get_map()->get_nrplayers(); i++) {
         Player* plr=egbase->get_safe_player(i);
         if(!plr) continue; // skip this player, is data can not be saved
         Tribe_Descr* t;
         
         assert(plr);
         t=plr->get_tribe();

         // Read number of buildings
         ushort nr_buildings=fr->Unsigned16();

         // Write for all buildings if it is enabled
         int b;
         for(b=0; b<nr_buildings; b++) {
            const char* name=fr->CString();
            bool allowed=fr->Unsigned8();
            
            if(!skip) {
               int index=t->get_building_index(name);
               if(index==-1) 
                  throw wexception("Unknown building found in map (Allowed_Buildings_Data): %s is not in tribe %s", name, t->get_name());

               plr->allow_building(index, allowed);
            }
         }
      }

      // DONE
      return;
   } else {
      throw wexception("Unknown version %i Allowed_Building_Data_Packet in map!\n", packet_version);
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Allowed_Buildings_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_ALLOWED_BUILDINGS);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   int i=0;
   for(i=1; i<=egbase->get_map()->get_nrplayers(); i++) {
      Player* plr=egbase->get_player(i);
      if(!plr) continue; // skip this player, is data can not be saved
      Tribe_Descr* t;
      if(plr) 
         t=plr->get_tribe();
      else 
         t=egbase->get_tribe(egbase->get_map()->get_scenario_player_tribe(i).c_str());

      // Write out number of buildings
      fw->Unsigned16(t->get_nrbuildings());

      // Write for all buildings if it is enabled
      int b;
      for(b=0; b<t->get_nrbuildings(); b++) {
         Building_Descr* building=t->get_building_descr(b);
         std::string name=building->get_name();
         fw->CString(name.c_str());
         if(plr)
            fw->Unsigned8(plr->is_building_allowed(b));
         else 
            fw->Unsigned8(true);  // All known buildings are allowed
      }
   }
   // Done
}
