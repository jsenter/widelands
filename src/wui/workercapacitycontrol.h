/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WUI_WORKERCAPACITYCONTROL_H
#define WL_WUI_WORKERCAPACITYCONTROL_H

#include "logic/widelands.h"

class InteractiveGameBase;

namespace UI {
class Panel;
}

namespace Widelands {
class Building;
class WorkersQueue;
}

UI::Panel* create_worker_capacity_control(UI::Panel& parent,
                                          InteractiveGameBase& igb,
                                          Widelands::Building& building,
                                          Widelands::DescriptionIndex index,
                                          Widelands::WorkersQueue& workers);

#endif  // end of include guard: WL_WUI_WORKERCAPACITYCONTROL_H
