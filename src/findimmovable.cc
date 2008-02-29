/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "findimmovable.h"

#include "immovable.h"

namespace Widelands {

struct FindImmovableAlwaysTrueImpl {
	bool accept(BaseImmovable *) const {return true;}
};

const FindImmovable& FindImmovableAlwaysTrue()
{
	static FindImmovable alwaystrue = FindImmovableAlwaysTrueImpl();
	return alwaystrue;
}

bool FindImmovableSize::accept(BaseImmovable *imm) const
{
	int32_t size = imm->get_size();
	return (m_min <= size && size <= m_max);
}

bool FindImmovableType::accept(BaseImmovable *imm) const
{
	return (m_type == imm->get_type());
}

bool FindImmovableAttribute::accept(BaseImmovable *imm) const
{
	return imm->has_attribute(m_attrib);
}

bool FindImmovablePlayerImmovable::accept(BaseImmovable* imm) const
{
	return dynamic_cast<PlayerImmovable const *>(imm);
}

}
