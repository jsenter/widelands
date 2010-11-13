/*
 * Copyright (C) 2002-2003, 2006-2010 by the Widelands Development Team
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

#include "immovable.h"

#include "editor_game_base.h"
#include "game_data_error.h"
#include "field.h"
#include "game.h"
#include "helper.h"
#include "immovable_program.h"
#include "player.h"
#include "map.h"
#include "mapfringeregion.h"
#include "profile/profile.h"
#include "graphic/animation_gfx.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "worker.h"

#include "upcast.h"

#include "container_iterate.h"

#include <cstdio>

#include <config.h>

#ifndef HAVE_VARARRAY
#include <climits>
#endif
namespace Widelands {

BaseImmovable::BaseImmovable(const Map_Object_Descr & mo_descr) :
Map_Object(&mo_descr)
{}


static std::string const base_immovable_name = "unknown";
std::string const & BaseImmovable::name() const throw () {
	return base_immovable_name;
}

/**
 * Associate the given field with this immovable. Recalculate if necessary.
 *
 * Only call this during init.
*/
void BaseImmovable::set_position(Editor_Game_Base & egbase, Coords const c)
{
	assert(c);

	Map & map = egbase.map();
	FCoords f = map.get_fcoords(c);
	if (f.field->immovable && f.field->immovable != this) {
		assert(f.field->immovable->get_size() == NONE);

		f.field->immovable->cleanup(egbase);
		delete f.field->immovable;
	}

	f.field->immovable = this;

	if (get_size() >= SMALL)
		map.recalc_for_field_area(Area<FCoords>(f, 2));
}

/**
 * Remove the link to the given field.
 *
 * Only call this during cleanup.
*/
void BaseImmovable::unset_position(Editor_Game_Base & egbase, Coords const c)
{
	Map & map = egbase.map();
	FCoords const f = map.get_fcoords(c);

	assert(f.field->immovable == this);

	f.field->immovable = 0;
	egbase.inform_players_about_immovable(f.field - &map[0], 0);

	if (get_size() >= SMALL)
		map.recalc_for_field_area(Area<FCoords>(f, 2));
}


/*
==============================================================================

ImmovableProgram IMPLEMENTATION

==============================================================================
*/


ImmovableProgram::ImmovableProgram
	(std::string    const & directory,
	 Profile              & prof,
	 std::string    const & _name,
	 Immovable_Descr      & immovable)
	: m_name(_name)
{
	Section & program_s = prof.get_safe_section(_name.c_str());
	while (Section::Value * const v = program_s.get_next_val()) {
		Action * action;
		if      (not strcmp(v->get_name(), "animate"))
			action = new ActAnimate  (v->get_string(), immovable, directory, prof);
		else if (not strcmp(v->get_name(), "transform"))
			action = new ActTransform(v->get_string(), immovable);
		else if (not strcmp(v->get_name(), "grow"))
			action = new ActGrow(v->get_string(), immovable);
		else if (not strcmp(v->get_name(), "remove"))
			action = new ActRemove   (v->get_string(), immovable);
		else if (not strcmp(v->get_name(), "seed"))
			action = new ActSeed     (v->get_string(), immovable);
		else if (not strcmp(v->get_name(), "playFX"))
			action = new ActPlayFX   (v->get_string(), immovable);
		else if (not strcmp(v->get_name(), "construction"))
			action = new ActConstruction(v->get_string(), immovable, directory, prof);
		else
			throw game_data_error
				(_("unknown command type \"%s\""), v->get_name());
		m_actions.push_back(action);
	}
	if (m_actions.empty())
		throw game_data_error(_("no actions"));
}


/*
==============================================================================

Immovable_Descr IMPLEMENTATION

==============================================================================
*/

/**
 * Parse an immovable from its conf file.
 *
 * Section [global]:
 * picture (default = $NAME_00.bmp): name of picture used in editor
 * size = none|small|medium|big (default = none): influences build options
 *
 * Section [program] (optional)
 * step = animation [animation name] [duration]
 *        transform [immovable name]
 *
 * Default:
 * 0=animation idle -1
*/
Immovable_Descr::Immovable_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 World const & world, Tribe_Descr const * const owner_tribe)
:
	Map_Object_Descr(_name, _descname),
	m_picture       (directory + global_s.get_string("picture", "menu.png")),
	m_size          (BaseImmovable::NONE),
	m_owner_tribe   (owner_tribe)
{

	if (char const * const string = global_s.get_string("size"))
		try {
			if      (!strcasecmp(string, "small"))
				m_size = BaseImmovable::SMALL;
			else if (!strcasecmp(string, "medium"))
				m_size = BaseImmovable::MEDIUM;
			else if (!strcasecmp(string, "big"))
				m_size = BaseImmovable::BIG;
			else
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 "{\"small\"|\"medium\"|\"big\"}", string);
		} catch (_wexception const & e) {
			throw game_data_error("size: %s", e.what());
		}


	//  parse attributes
	while (Section::Value const * const v = global_s.get_next_val("attrib")) {
		uint32_t attrib = get_attribute_id(v->get_string());
		if (attrib < Map_Object::HIGHEST_FIXED_ATTRIBUTE)
			if (attrib != Map_Object::RESI)
				throw game_data_error(_("bad attribute \"%s\""), v->get_string());
		add_attribute(attrib);
	}


	//  parse the programs
	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string program_name = v->get_string();
		std::transform
			(program_name.begin(), program_name.end(), program_name.begin(),
			 tolower);
		ImmovableProgram * program = 0;
		try {
			if (m_programs.count(program_name))
				throw game_data_error(_("this program has already been declared"));
			m_programs[program_name.c_str()] =
				new ImmovableProgram(directory, prof, program_name, *this);
		} catch (std::exception const & e) {
			delete program;
			throw game_data_error
				(_("program %s: %s"), program_name.c_str(), e.what());
		}
	}

	if (m_programs.find("program") == m_programs.end()) { //  default program
		char parameters[] = "idle";
		m_programs["program"] =
			new ImmovableProgram
				("program",
				 new ImmovableProgram::ActAnimate
				 	(parameters, *this, directory, prof));
	}

	uint8_t * it = m_terrain_affinity;
	memset(it, 0, sizeof(m_terrain_affinity));
	if
		(Section * const terrain_affinity_s =
		 	prof.get_section("terrain affinity"))
	{
		memset(it, 0, sizeof(m_terrain_affinity));
		for
			(wl_index_range<Terrain_Index> i(0, world.get_nr_terrains());
			 i;
			 ++i, ++it)
		{
			char const * const terrain_type_name =
				world.get_ter(i.current).name().c_str();
			try {
				uint32_t const value =
					terrain_affinity_s->get_natural(terrain_type_name, 0);
				if ((*it = value) != value)
					throw game_data_error
						(_("expected 0 .. 255 but found %u"), value);
				if (terrain_affinity_s->get_next_val(terrain_type_name))
					throw game_data_error(_("duplicated"));
			} catch (_wexception const & e) {
				throw game_data_error
					("[terrain affinity] %s: %s", terrain_type_name, e.what());
			}
		}
		if (owner_tribe) {
			//  Tribe immovables may have entries for other worlds.
			while (Section::Value * const v = terrain_affinity_s->get_next_val())
				try {
					uint32_t const value = v->get_natural();
					if ((*it = value) != value)
						throw game_data_error
							(_("expected 0 .. 255 but found %u"), value);
					if (terrain_affinity_s->get_next_val(v->get_name()))
						throw game_data_error(_("duplicated"));
				} catch (_wexception const & e) {
					throw game_data_error
						(_("[terrain affinity] \"%s\" (not in current world): %s"),
						 v->get_name(), e.what());
				}
		}
	} else
		memset(it, 255, sizeof(m_terrain_affinity));

	if (owner_tribe) {
		if (Section * buildcost_s = prof.get_section("buildcost"))
			m_buildcost.parse(*owner_tribe, *buildcost_s);
	}
}


/**
 * Cleanup
*/
Immovable_Descr::~Immovable_Descr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Find the program of the given name.
*/
ImmovableProgram const * Immovable_Descr::get_program
	(std::string const & programname) const
{
	Programs::const_iterator const it = m_programs.find(programname);

	if (it == m_programs.end())
		throw game_data_error
			(_("immovable %s has no program \"%s\""),
			 name().c_str(), programname.c_str());

	return it->second;
}


/**
 * Create an immovable of this type
*/
Immovable & Immovable_Descr::create
	(Editor_Game_Base & egbase, Coords const coords) const
{
	assert(this);
	Immovable & result = *new Immovable(*this);
	result.m_position = coords;
	result.init(egbase);
	return result;
}


/*
==============================

IMPLEMENTATION

==============================
*/

Immovable::Immovable(const Immovable_Descr & imm_descr) :
BaseImmovable (imm_descr),
m_owner(0),
m_anim        (0),
m_anim_construction_total(0),
m_anim_construction_done(0),
m_program     (0),
m_program_ptr (0),
m_program_step(0),
m_action_data(0),
m_reserved_by_worker(false)
{}

Immovable::~Immovable()
{
	delete m_action_data;
	m_action_data = 0;
}

int32_t Immovable::get_type() const throw ()
{
	return IMMOVABLE;
}

BaseImmovable::PositionList Immovable::get_positions
	(const Editor_Game_Base &) const throw ()
{
	PositionList rv;

	rv.push_back(m_position);
	return rv;
}

int32_t Immovable::get_size() const throw ()
{
	return descr().get_size();
}

bool Immovable::get_passable() const throw ()
{
	return descr().get_size() < BIG;
}


std::string const & Immovable::name() const throw () {return descr().name();}

void Immovable::set_owner(Player* player)
{
	m_owner = player;
}

void Immovable::start_animation
	(Editor_Game_Base const & egbase, uint32_t const anim)
{
	m_anim      = anim;
	m_animstart = egbase.get_gametime();
	m_anim_construction_done = m_anim_construction_total = 0;
}


void Immovable::increment_program_pointer()
{
	m_program_ptr = (m_program_ptr + 1) % m_program->size();
	delete m_action_data;
	m_action_data = 0;
}


/**
 * Actually initialize the immovable.
*/
void Immovable::init(Editor_Game_Base & egbase)
{
	BaseImmovable::init(egbase);

	set_position(egbase, m_position);

	//  Set animation data according to current program state.
	ImmovableProgram const * prog = m_program;
	if (!prog)
		prog = descr().get_program("program");
	if
		(upcast
		 	(ImmovableProgram::ActAnimate const,
		 	 act_animate,
		 	 &(*prog)[m_program_ptr]))
		start_animation(egbase, act_animate->animation());

	if (upcast(Game, game, &egbase))
		switch_program(*game, "program");
}


/**
 * Cleanup before destruction
*/
void Immovable::cleanup(Editor_Game_Base & egbase)
{
	unset_position(egbase, m_position);

	BaseImmovable::cleanup(egbase);
}


/**
 * Switch the currently running program.
*/
void Immovable::switch_program(Game & game, std::string const & programname)
{
	m_program = descr().get_program(programname);
	m_program_ptr = 0;
	m_program_step = 0;
	delete m_action_data;
	m_action_data = 0;
	schedule_act(game, 1);
}

uint32_t Immovable_Descr::terrain_suitability
	(FCoords const f, Map const & map) const
{
	uint32_t result = 0;
	//  Neighbours
	FCoords const tr = map.tr_n(f);
	FCoords const tl = map.tl_n(f);
	FCoords const  l = map. l_n(f);

	result += m_terrain_affinity[tr.field->terrain_d()];
	result += m_terrain_affinity[tl.field->terrain_r()];
	result += m_terrain_affinity[tl.field->terrain_d()];
	result += m_terrain_affinity [l.field->terrain_r()];
	result += m_terrain_affinity [f.field->terrain_d()];
	result += m_terrain_affinity [f.field->terrain_r()];

	return result;
}


/**
 * Run program timer.
*/
void Immovable::act(Game & game, uint32_t const data)
{
	BaseImmovable::act(game, data);

	if (m_program_step <= game.get_gametime())
		//  Might delete itself!
		(*m_program)[m_program_ptr].execute(game, *this);
}


void Immovable::draw
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 FCoords,
	 Point                    const pos)
{
	if (m_anim) {
		if (!m_anim_construction_total)
			dst.drawanim(pos, m_anim, game.get_gametime() - m_animstart, 0);
		else
			draw_construction(game, dst, pos);
	}
}

void Immovable::draw_construction(const Editor_Game_Base& game, RenderTarget& dst, const Point pos)
{
	const ImmovableProgram::ActConstruction * constructionact = 0;
	if (m_program_ptr < m_program->size())
		constructionact = dynamic_cast<const ImmovableProgram::ActConstruction *>
			(&(*m_program)[m_program_ptr]);

	const int32_t steptime = constructionact ? constructionact->buildtime() : 5000;
	uint32_t total = m_anim_construction_total * steptime;
	uint32_t done = 0;

	if (m_anim_construction_done > 0) {
		done = steptime * (m_anim_construction_done - 1);
		done += std::min(steptime, game.get_gametime() - m_animstart);
	}

	if (done > total)
		done = total;

	const AnimationGfx::Index nr_frames = g_gr->nr_frames(m_anim);
	uint32_t frametime = g_anim.get_animation(m_anim)->frametime;
	uint32_t units_per_frame = (total + nr_frames - 1) / nr_frames;
	const AnimationGfx::Index current_frame = done / units_per_frame;
	uint32_t curw, curh;
	g_gr->get_animation_size(m_anim, current_frame * frametime, curw, curh);

	uint32_t lines = ((done % units_per_frame) * curh) / units_per_frame;

	if (current_frame > 0) {
		// Not the first pic, so draw the previous one in the back
		dst.drawanim(pos, m_anim, (current_frame-1) * frametime, get_owner());
	}

	assert(lines <= curh);
	dst.drawanimrect
		(pos, m_anim, current_frame * frametime, get_owner(), Rect(Point(0, curh - lines), curw, lines));
}


/**
 * Returns whether this immovable was reserved by a worker.
 */
bool Immovable::is_reserved_by_worker() const
{
	return m_reserved_by_worker;
}

/**
 * Change whether this immovable is marked as reserved by a worker.
 */
void Immovable::set_reserved_by_worker(bool reserve)
{
	m_reserved_by_worker = reserve;
}

/**
 * Set the current action's data to \p data.
 *
 * \warning \p data must not be equal to the currently set data, but it may be 0.
 */
void Immovable::set_action_data(ImmovableActionData* data)
{
	delete m_action_data;
	m_action_data = data;
}


/*
==============================

Load/save support

==============================
*/

#define IMMOVABLE_SAVEGAME_VERSION 5

void Immovable::Loader::load(FileRead & fr, uint8_t const version)
{
	BaseImmovable::Loader::load(fr);

	Immovable & imm = ref_cast<Immovable, Map_Object>(*get_object());

	if (version >= 5) {
		Player_Number pn = fr.Unsigned8();
		if (pn && pn <= MAX_PLAYERS) {
			Player * plr = egbase().get_player(pn);
			if (!plr)
				throw game_data_error("Immovable::load: player %u does not exist", pn);
			imm.set_owner(plr);
		}
	}

	// Position
	imm.m_position = fr.Coords32(egbase().map().extent());
	imm.set_position(egbase(), imm.m_position);

	// Animation
	char const * const animname = fr.CString();
	try {
		imm.m_anim = imm.descr().get_animation(animname);
	} catch (Map_Object_Descr::Animation_Nonexistent const &) {
		imm.m_anim = imm.descr().main_animation();
		log
			("Warning: Animation \"%s\" not found, using animation %s).\n",
			 animname, imm.descr().get_animation_name(imm.m_anim).c_str());
	}
	imm.m_animstart = fr.Signed32();
	if (version >= 4) {
		imm.m_anim_construction_total = fr.Unsigned32();
		if (imm.m_anim_construction_total)
			imm.m_anim_construction_done = fr.Unsigned32();
	}

	{ //  program
		std::string program_name;
		if (1 == version) {
			program_name = fr.Unsigned8() ? fr.CString() : "program";
			std::transform
				(program_name.begin(), program_name.end(), program_name.begin(),
				 tolower);
		} else {
			program_name = fr.CString();
			if (program_name.empty())
				program_name = "program";
		}
		imm.m_program = imm.descr().get_program(program_name);
	}
	imm.m_program_ptr = fr.Unsigned32();

	if (!imm.m_program) {
		imm.m_program_ptr = 0;
	} else {
		if (imm.m_program_ptr >= imm.m_program->size()) {
			// Try to not fail if the program of some immovable has changed
			// significantly.
			// Note that in some cases, the immovable may end up broken despite
			// the fixup, but there isn't really anything we can do against that.
			log
				("Warning: Immovable '%s', size of program '%s' seems to have "
				 "changed.\n",
				 imm.descr().name().c_str(), imm.m_program->name().c_str());
			imm.m_program_ptr = 0;
		}
	}

	imm.m_program_step = fr.Signed32();

	if (version >= 3)
		imm.m_reserved_by_worker = fr.Unsigned8();

	if (version >= 4) {
		std::string dataname = fr.CString();
		if (!dataname.empty()) {
			imm.set_action_data(ImmovableActionData::load(fr, imm, dataname));
		}
	}
}

void Immovable::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();
}

void Immovable::Loader::load_finish()
{
	BaseImmovable::Loader::load_finish();

	Immovable & imm = dynamic_cast<Immovable &>(*get_object());
	if (upcast(Game, game, &egbase()))
		imm.schedule_act(*game, 1);

	egbase().inform_players_about_immovable
		(Map::get_index(imm.m_position, egbase().map().get_width()),
		 &imm.descr());
}

void Immovable::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	// This is in front because it is required to obtain the description
	// necessary to create the Immovable
	fw.Unsigned8(header_Immovable);
	fw.Unsigned8(IMMOVABLE_SAVEGAME_VERSION);

	if (const Tribe_Descr * const tribe = get_owner_tribe())
		fw.String(tribe->name());
	else
		fw.CString("world");

	fw.String(name());

	// The main loading data follows
	BaseImmovable::save(egbase, mos, fw);

	fw.Player_Number8(get_owner() ? get_owner()->player_number() : 0);
	fw.Coords32(m_position);

	// Animations
	fw.String(descr().get_animation_name(m_anim));
	fw.Signed32(m_animstart);
	fw.Unsigned32(m_anim_construction_total);
	if (m_anim_construction_total)
		fw.Unsigned32(m_anim_construction_done);

	// Program Stuff
	fw.String(m_program ? m_program->name() : "");

	fw.Unsigned32(m_program_ptr);
	fw.Signed32(m_program_step);

	fw.Unsigned8(m_reserved_by_worker);

	if (m_action_data) {
		fw.CString(m_action_data->name());
		m_action_data->save(fw, *this);
	} else {
		fw.CString("");
	}
}

Map_Object::Loader * Immovable::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version and version <= IMMOVABLE_SAVEGAME_VERSION) {

			char const * const owner = fr.CString ();
			char const * const name  = fr.CString ();
			Immovable * imm = 0;

			if (strcmp(owner, "world")) { //  It is a tribe immovable.
				egbase.manually_load_tribe(owner);

				if (Tribe_Descr const * const tribe = egbase.get_tribe(owner)) {
					std::string effective_name = name;
					const std::vector<std::string> & compat =
						tribe->compatibility_immovable(name);

					if (compat.size() >= 1) {
						if (compat[0] == "replace") {
							if (compat.size() != 2)
								throw game_data_error
									("incomplete compatibility_immovable replace for %s",
									 name);

							effective_name = compat[1];
						} else
							throw game_data_error
								("bad compatibility_immovable code %s for %s",
								 compat[0].c_str(), name);
					}

					int32_t const idx = tribe->get_immovable_index(effective_name);
					if (idx != -1)
						imm = new Immovable(*tribe->get_immovable_descr(idx));
					else
						throw game_data_error
							(_("tribe %s does not define immovable type \"%s\""),
							 owner, effective_name.c_str());
				} else
					throw wexception(_("unknown tribe %s"), owner);
			} else { //  world immovable
				World const & world = egbase.map().world();
				int32_t const idx = world.get_immovable_index(name);
				if (idx == -1)
					throw wexception
						(_("world does not define immovable type \"%s\""), name);

				imm = new Immovable(*world.get_immovable_descr(idx));
			}

			loader->init(egbase, mol, *imm);
			loader->load(fr, version);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception(_("immovable type %s"), e.what());
	}

	return loader.release();
}


ImmovableProgram::Action::~Action() {}


ImmovableProgram::ActAnimate::ActAnimate
	(char * parameters, Immovable_Descr & descr,
	 std::string const & directory, Profile & prof)
{
	try {
		bool reached_end;
		char * const animation_name = match(parameters, reached_end);
		if (descr.is_animation_known(animation_name))
			m_id = descr.get_animation(animation_name);
		else {
			m_id =
				g_anim.get
					(directory.c_str(),
					 prof.get_safe_section(animation_name),
					 0);

			descr.add_animation(animation_name, m_id);
		}
		if (not reached_end) { //  The next parameter is the duration.
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp or value <= 0)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("duration in ms"), parameters);
			m_duration = value;
		} else
			m_duration = 0; //  forever
	} catch (_wexception const & e) {
		throw game_data_error("animate: %s", e.what());
	}
}


/// Use convolutuion to make the animation time a random variable with binomial
/// distribution and the configured time as the expected value.
void ImmovableProgram::ActAnimate::execute
	(Game & game, Immovable & immovable) const
{
	immovable.start_animation(game, m_id);
	immovable.program_step
		(game,
		 m_duration ?
		 1 + game.logic_rand() % m_duration + game.logic_rand() % m_duration
		 :
		 0);
}


ImmovableProgram::ActPlayFX::ActPlayFX
	(char * parameters, Immovable_Descr const &)
{
	try {
		bool reached_end;
		name = match(parameters, reached_end);

		if (not reached_end) {
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			priority = value;
			if (*endp or priority != value)
				throw game_data_error
					(_("expected %s but found \"%s\""), _("priority"), parameters);
		} else
			priority = 127;
	} catch (_wexception const & e) {
		throw game_data_error("playFX: %s", e.what());
	}
}

/** Demand from the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played
 * is decided only by the sound server*/
void ImmovableProgram::ActPlayFX::execute
	(Game & game, Immovable & immovable) const
{
	g_sound_handler.play_fx(name, immovable.get_position(), priority);
	immovable.program_step(game);
}


ImmovableProgram::ActTransform::ActTransform
	(char * parameters, Immovable_Descr & descr)
{
	try {
		tribe = true;
		bob = false;
		probability = 0;

		std::vector<std::string> params = split_string(parameters, " ");
		for (uint32_t i = 0; i < params.size(); ++i) {
			if (params[i] == "bob")
				bob = true;
			else if (params[i] == "immovable")
				bob = false;
			else if (params[i][0] >= '0' && params[i][0] <= '9') {
				long int const value = atoi(params[i].c_str());
				if (value < 1 or 254 < value)
					throw game_data_error
						(_
							("expected probability in range [1, 254] but found "
							"\"%s\""),
						 params[i].c_str());
				probability = value;
			} else {
				std::vector<std::string> segments = split_string(params[i], ":");

				if (segments.size() > 2)
					throw game_data_error("object type has more than 2 segments");
				if (segments.size() == 2) {
					if (segments[0] == "world")
						tribe = false;
					else if (segments[0] == "tribe") {
						if (!descr.get_owner_tribe())
							throw game_data_error("scope \"tribe\", but have no owner tribe");
						tribe = true;
					} else
						throw game_data_error
							(_
								("unknown scope \"%s\" given for target type (must be "
								 "\"world\" or \"tribe\")"),
							 parameters);

					type_name = segments[1];
				} else {
					type_name = segments[0];
				}
			}
		}
		if (type_name == descr.name())
			throw game_data_error(_("illegal transformation to the same type"));
	} catch (_wexception const & e) {
		throw game_data_error("transform: %s", e.what());
	}
}

void ImmovableProgram::ActTransform::execute
	(Game & game, Immovable & immovable) const
{
	if (probability == 0 or game.logic_rand() % 256 < probability) {
		Player * player = immovable.get_owner();
		Coords const c = immovable.get_position();
		Tribe_Descr const * const owner_tribe =
			tribe ? immovable.descr().get_owner_tribe() : 0;
		immovable.remove(game); //  Now immovable is a dangling reference!

		if (bob) {
			Bob & bob = game.create_bob(c, type_name, owner_tribe);
			if (player)
				bob.set_owner(player);
		} else {
			Immovable & imm = game.create_immovable(c, type_name, owner_tribe);
			if (player)
				imm.set_owner(player);
		}
	} else
		immovable.program_step(game);
}


ImmovableProgram::ActGrow::ActGrow
	(char * parameters, Immovable_Descr & descr)
{
	try {
		tribe = true;
		for (char * p = parameters;;)
			switch (*p) {
			case ':': {
				*p = '\0';
				++p;
				Tribe_Descr const * const owner_tribe = descr.get_owner_tribe();
				if (not owner_tribe)
					throw game_data_error
						(_
						 	("immovable type not in tribe but target type has scope "
						 	 "(\"%s\")"),
						 parameters);
				else if (strcmp(parameters, "world"))
					throw game_data_error
						(_
						 	("scope \"%s\" given for target type (must be "
						 	 "\"world\")"),
						 parameters);
				tribe = false;
				parameters = p;
				break;
			}
			case '\0':
				goto end;
			default:
				++p;
			}
	end:
		type_name = parameters;
	} catch (_wexception const & e) {
		throw game_data_error("grow: %s", e.what());
	}
}

void ImmovableProgram::ActGrow::execute
	(Game & game, Immovable & immovable) const
{
	Map             const & map   = game     .map  ();
	Immovable_Descr const & descr = immovable.descr();
	FCoords const f = map.get_fcoords(immovable.get_position());
	if (game.logic_rand() % (6 * 255) < descr.terrain_suitability(f, map)) {
		Tribe_Descr const * const owner_tribe =
			tribe ? immovable.descr().get_owner_tribe() : 0;
		immovable.remove(game); //  Now immovable is a dangling reference!
		game.create_immovable(f, type_name, owner_tribe);
	} else
		immovable.program_step(game);
}


/**
 * remove
*/
ImmovableProgram::ActRemove::ActRemove(char * parameters, Immovable_Descr &)
{
	try {
		if (*parameters) {
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp or value < 1 or 254 < value)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("probability in range [1, 254]"), parameters);
			probability = value;
		} else
			probability = 0;
	} catch (_wexception const & e) {
		throw game_data_error("remove: %s", e.what());
	}
}

void ImmovableProgram::ActRemove::execute
	(Game & game, Immovable & immovable) const
{
	if (probability == 0 or game.logic_rand() % 256 < probability)
		immovable.remove(game); //  Now immovable is a dangling reference!
	else
		immovable.program_step(game);
}


ImmovableProgram::ActSeed::ActSeed(char * parameters, Immovable_Descr & descr)
{
	try {
		tribe = true;
		probability = 0;
		for (char * p = parameters;;)
			switch (*p) {
			case ':': {
				*p = '\0';
				++p;
				Tribe_Descr const * const owner_tribe = descr.get_owner_tribe();
				if (not owner_tribe)
					throw game_data_error
						(_
						 	("immovable type not in tribe but target type has scope "
						 	 "(\"%s\")"),
						 parameters);
				else if (strcmp(parameters, "world"))
					throw game_data_error
						(_
						 	("scope \"%s\" given for target type (must be "
						 	 "\"world\")"),
						 parameters);
				tribe = false;
				parameters = p;
				break;
			}
			case ' ': {
				*p = '\0';
				++p;
				char * endp;
				long int const value = strtol(p, &endp, 0);
				if (*endp or value < 1 or 254 < value)
					throw game_data_error
						(_
						 	("expected probability in range [1, 254] but found "
						 	 "\"%s\""),
						 p);
				probability = value;
			//  fallthrough
			}
			case '\0':
				goto end;
			default:
				++p;
			}
	end:
		type_name = parameters;
	} catch (_wexception const & e) {
		throw game_data_error("seed: %s", e.what());
	}
}

void ImmovableProgram::ActSeed::execute
	(Game & game, Immovable & immovable) const
{
	Immovable_Descr const & descr = immovable.descr();
	Map const & map = game.map();
	if
		(game.logic_rand() % (6 * 256)
		 <
		 descr.terrain_suitability
		 	(map.get_fcoords(immovable.get_position()), map))
	{
		MapFringeRegion<> mr(map, Area<>(immovable.get_position(), 0));
		uint32_t fringe_size = 0;
		do {
			mr.extend(map);
			fringe_size += 6;
		} while (game.logic_rand() % 256 < probability);
		for (uint32_t n = game.logic_rand() % fringe_size; n; --n)
			mr.advance(map);
		FCoords const f = map.get_fcoords(mr.location());
		if
			(not f.field->get_immovable()        and
			 f.field->nodecaps() & MOVECAPS_WALK and
			 game.logic_rand() % (6 * 256) < descr.terrain_suitability(f, map))
			game.create_immovable
				(mr.location(),
				 type_name,
				 tribe ? immovable.descr().get_owner_tribe() : 0);
	}

	immovable.program_step(game);
}

ImmovableProgram::ActConstruction::ActConstruction
	(char * parameters, Immovable_Descr & descr, std::string const & directory, Profile & prof)
{
	try {
		if (!descr.get_owner_tribe())
			throw game_data_error("only usable for tribe immovable");

		std::vector<std::string> params = split_string(parameters, " ");

		if (params.size() != 3)
			throw game_data_error("usage: animation-name buildtime decaytime");

		m_buildtime = atoi(params[1].c_str());
		m_decaytime = atoi(params[2].c_str());

		std::string animation_name = params[0];
		if (descr.is_animation_known(animation_name))
			m_animid = descr.get_animation(animation_name);
		else {
			m_animid =
				g_anim.get
					(directory.c_str(),
					 prof.get_safe_section(animation_name),
					 0);

			descr.add_animation(animation_name, m_animid);
		}
	} catch (const _wexception & e) {
		throw game_data_error("construction: %s", e.what());
	}
}

#define CONSTRUCTION_DATA_VERSION 1

struct ActConstructionData : ImmovableActionData {
	const char * name() const {return "construction";}
	void save(FileWrite & fw, Immovable & imm) {
		fw.Unsigned8(CONSTRUCTION_DATA_VERSION);
		delivered.save(fw, *imm.descr().get_owner_tribe());
	}

	static ActConstructionData * load(FileRead & fr, Immovable & imm) {
		ActConstructionData * d = new ActConstructionData;

		try {
			uint8_t version = fr.Unsigned8();
			if (version == CONSTRUCTION_DATA_VERSION) {
				d->delivered.load(fr, *imm.descr().get_owner_tribe());

				return d;
			} else
				throw game_data_error("unknown version %u", version);
		} catch(const _wexception & e) {
			delete d;
			throw game_data_error("ActConstructionData: %s", e.what());
		}
	}

	Buildcost delivered;
};

void ImmovableProgram::ActConstruction::execute(Game & g, Immovable & imm) const
{
	ActConstructionData * d = imm.get_action_data<ActConstructionData>();
	if (!d) {
		// First execution
		d = new ActConstructionData;
		imm.set_action_data(d);

		imm.start_animation(g, m_animid);
		imm.m_anim_construction_total = imm.descr().buildcost().total();
	} else {
		// Perhaps we are called due to the construction timeout of the last construction step
		Buildcost remaining;
		imm.construct_remaining_buildcost(g, &remaining);
		if (remaining.empty()) {
			imm.program_step(g);
			return;
		}

		// Otherwise, this is a decay timeout
		uint32_t totaldelivered = 0;
		for (Buildcost::const_iterator it = d->delivered.begin(); it != d->delivered.end(); ++it)
			totaldelivered += it->second;

		if (!totaldelivered) {
			imm.remove(g);
			return;
		}

		uint32_t randdecay = g.logic_rand() % totaldelivered;
		for (Buildcost::iterator it = d->delivered.begin(); it != d->delivered.end(); ++it) {
			if (randdecay < it->second) {
				it->second--;
				break;
			}

			randdecay -= it->second;
		}

		imm.m_anim_construction_done = d->delivered.total();
	}

	imm.m_program_step = imm.schedule_act(g, m_decaytime);
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * compute the remaining buildcost.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_remaining_buildcost(Game & game, Buildcost * buildcost)
{
	ActConstructionData * d = get_action_data<ActConstructionData>();
	if (!d)
		return false;

	const Buildcost & total = descr().buildcost();
	for (Buildcost::const_iterator it = total.begin(); it != total.end(); ++it) {
		uint32_t delivered = d->delivered[it->first];
		if (delivered < it->second)
			(*buildcost)[it->first] = it->second - delivered;
	}

	return true;
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * consume the given ware type as delivered.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_ware_item(Game & game, Ware_Index index)
{
	ActConstructionData * d = get_action_data<ActConstructionData>();
	if (!d)
		return false;

	molog("construct_ware_item: index %u", index.value());

	Buildcost::iterator it = d->delivered.find(index);
	if (it != d->delivered.end())
		it->second++;
	else
		d->delivered[index] = 1;

	m_anim_construction_done = d->delivered.total();
	m_animstart = game.get_gametime();

	molog("construct_ware_item: total %u delivered: %u", index.value(), d->delivered[index]);

	Buildcost remaining;
	construct_remaining_buildcost(game, &remaining);

	const ImmovableProgram::ActConstruction * act =
		dynamic_cast<const ImmovableProgram::ActConstruction *>(&(*m_program)[m_program_ptr]);
	assert(act != 0);

	if (remaining.empty()) {
		// Wait for the last building animation to finish.
		m_program_step = schedule_act(game, act->buildtime());
	} else {
		m_program_step = schedule_act(game, act->decaytime());
	}

	return true;
}


ImmovableActionData * ImmovableActionData::load(FileRead & fr, Immovable & imm, const std::string & name)
{
	if (name == "construction")
		return ActConstructionData::load(fr, imm);
	else {
		log("ImmovableActionData::load: type %s not known", name.c_str());
		return 0;
	}
}


/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize
*/
PlayerImmovable::PlayerImmovable(const Map_Object_Descr & mo_descr) :
	BaseImmovable(mo_descr), m_owner(0), m_economy(0)
{}

/**
 * Cleanup
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log
			("PlayerImmovable::~PlayerImmovable: %lu workers left!\n",
			 static_cast<long unsigned int>(m_workers.size()));
}

/**
 * Change the economy, transfer the workers
*/
void PlayerImmovable::set_economy(Economy * const e)
{
	if (m_economy == e)
		return;

	for (uint32_t i = 0; i < m_workers.size(); ++i)
		m_workers[i]->set_economy(e);

	m_economy = e;
}

/**
 * Associate the given worker with this immovable.
 * The worker will be transferred along to another economy, and it will be
 * released when the immovable is destroyed.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::add_worker(Worker & w)
{
	m_workers.push_back(&w);
}

/**
 * Disassociate the given worker with this building.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::remove_worker(Worker & w)
{
	container_iterate(Workers, m_workers, i)
		if (*i.current == &w) {
			*i.current = *(i.get_end() - 1);
			return m_workers.pop_back();
		}

	throw wexception("PlayerImmovable::remove_worker: not in list");
}


/**
 * Set the immovable's owner. Currently, it can only be set once.
*/
void PlayerImmovable::set_owner(Player * const new_owner) {
	m_owner = new_owner;

	m_owner->egbase().receive(NoteImmovable(this, GAIN));
}

/**
 * Initialize the immovable.
*/
void PlayerImmovable::init(Editor_Game_Base & egbase)
{
	BaseImmovable::init(egbase);
}

/**
 * Release workers
*/
void PlayerImmovable::cleanup(Editor_Game_Base & egbase)
{
	while (m_workers.size())
		m_workers[0]->set_location(0);

	if (m_owner)
		m_owner->egbase().receive(NoteImmovable(this, LOSE));

	BaseImmovable::cleanup(egbase);
}

/**
 * We are the destination of the given ware's transfer, which is not associated
 * with any request.
 */
void PlayerImmovable::receive_ware(Game &, Ware_Index ware)
{
	throw wexception
		("MO(%u): Received a ware(%u), do not know what to do with it",
		 serial(), ware.value());
}

/**
 * We are the destination of the given worker's transfer, which is not
 * associated with any request.
 */
void PlayerImmovable::receive_worker(Game &, Worker & worker)
{
	throw wexception
		("MO(%u): Received a worker(%u), do not know what to do with it",
		 serial(), worker.serial());
}


/**
 * Dump general information
 */
void PlayerImmovable::log_general_info(Editor_Game_Base const & egbase)
{
	BaseImmovable::log_general_info(egbase);

	molog("this: %p\n", this);
	molog("m_owner: %p\n", m_owner);
	molog("* player nr: %i\n", m_owner->player_number());
	molog("m_economy: %p\n", m_economy);
}

}
