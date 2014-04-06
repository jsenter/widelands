/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef ONE_WORLD_LEGACY_CONVERSION_H
#define ONE_WORLD_LEGACY_CONVERSION_H

#include <map>
#include <string>

#include <boost/noncopyable.hpp>

class OneWorldLegacyLookupTable : boost::noncopyable {
public:
	OneWorldLegacyLookupTable();

	// Looks up the new name for the 'resource'. They were all the same for all
	// former worlds.
	std::string lookup_resource(const std::string& resource) const;

	// Looks up the new name for the 'terrain' from the former 'world'.
	std::string lookup_terrain(const std::string& world, const std::string& terrain) const;

	// Looks up the new name for the 'critter' from the former 'world'.
	std::string lookup_critter(const std::string& world, const std::string& critter) const;

	// Looks up the new name for the 'immovable' from the former 'world'.
	std::string lookup_immovable(const std::string& world, const std::string& immovable) const;

private:
	const std::map<std::string, std::string> resources_;
	const std::map<std::string, std::map<std::string, std::string>> terrains_;
	const std::map<std::string, std::map<std::string, std::string>> critters_;
	const std::map<std::string, std::map<std::string, std::string>> immovables_;
};

#endif /* end of include guard: ONE_WORLD_LEGACY_CONVERSION_H */
