/*
 * Copyright (C) 2002 by Widelands Development Team
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
#include "profile.h"
#include "tribedata.h"
#include "helper.h"
#include "tribe.h"
#include "game.h"
#include "player.h"
#include "transport.h"

#include "building_int.h"


#define BUILDING_LEAVE_INTERVAL	1000
#define CARRIER_SPAWN_INTERVAL	2500


/*
==============================================================================

Basic building

==============================================================================
*/

/*
===============
Building_Descr::Building_Descr

Initialize with sane defaults
===============
*/
Building_Descr::Building_Descr(Tribe_Descr *tribe, const char *name)
{
	m_tribe = tribe;
	snprintf(m_name, sizeof(m_name), "%s", name);
	strcpy(m_descname, m_name);
	m_buildable = true;
	m_buildicon = 0;
	m_buildicon_fname = 0;
	m_size = BaseImmovable::SMALL;
	m_mine = false;
}


/*
===============
Building_Descr::~Building_Descr

Cleanup
===============
*/
Building_Descr::~Building_Descr(void)
{
	if (m_buildicon_fname)
		free(m_buildicon_fname);
}

/*
===============
Building_Descr::create

Create a building of this type. Does not perform any sanity checks.
===============
*/
Building *Building_Descr::create(Editor_Game_Base *g, Player *owner, Coords pos, bool construct)
{
	assert(owner);

	Building *b = construct ? create_constructionsite() : create_object();
	b->set_owner(owner);
	b->m_position = pos;
	b->init(g);

	return b;
}

/*
===============
Building_Descr::parse

Parse the basic building settings from the given profile and directory
===============
*/
void Building_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Section *global = prof->get_safe_section("global");
	Section* s;
	const char *string;
	char buf[256];
	char fname[256];

	snprintf(m_descname, sizeof(m_descname), "%s", global->get_safe_string("descname"));

	string = global->get_safe_string("size");
	if (!strcasecmp(string, "small")) {
		m_size = BaseImmovable::SMALL;
	} else if (!strcasecmp(string, "medium")) {
		m_size = BaseImmovable::MEDIUM;
	} else if (!strcasecmp(string, "big")) {
		m_size = BaseImmovable::BIG;
	} else if (!strcasecmp(string, "mine")) {
		m_size = BaseImmovable::SMALL;
		m_mine = true;
	} else
		throw wexception("Section [global], unknown size '%s'. Valid values are small, medium, big, mine",
		                 string);

	// Parse build options
	m_buildable = global->get_bool("buildable", true);

	if (m_buildable)
		{
		// Get build icon
		snprintf(buf, sizeof(buf), "%s_build.bmp", m_name);
		string = global->get_string("buildicon", buf);

		snprintf(fname, sizeof(fname), "%s/%s", directory, string);

		m_buildicon_fname = strdup(fname);

		// Get costs
		s = prof->get_safe_section("buildcost");

		Section::Value* val;

		while((val = s->get_next_val(0)))
			m_buildcost.push_back(CostItem(val->get_name(), val->get_int()));
		}


	// Parse basic animation data
	s = prof->get_section("idle");
	if (!s)
		throw wexception("Missing idle animation");
	m_idle = g_anim.get(directory, s, 0, encdata);
}


/*
===============
Building_Descr::load_graphics

Called whenever building graphics need to be loaded.
===============
*/
void Building_Descr::load_graphics()
{
	if (m_buildicon_fname)
		m_buildicon = g_gr->get_picture(PicMod_Game, m_buildicon_fname, RGBColor(0,0,255));
}

/*
===============
Building_Descr::create_constructionsite

Create a construction site for this type of building
===============
*/
Building* Building_Descr::create_constructionsite()
{
	Building_Descr* descr = m_tribe->get_building_descr(m_tribe->get_building_index("constructionsite"));
	if (!descr)
		throw wexception("Tribe %s has no constructionsite", m_tribe->get_name());

	ConstructionSite* csite = (ConstructionSite*)descr->create_object();

	csite->set_building(this);

	return csite;
}


/*
==============================

Implementation

==============================
*/

Building::Building(Building_Descr *descr)
	: PlayerImmovable(descr)
{
	m_flag = 0;
	m_optionswindow = 0;
}

Building::~Building()
{
	if (m_optionswindow)
		hide_options();
}

/*
===============
Building::get_type
Building::get_size
Building::get_passable
Building::get_base_flag
===============
*/
int Building::get_type()
{
	return BUILDING;
}

int Building::get_size()
{
	return get_descr()->get_size();
}

bool Building::get_passable()
{
	return false;
}

Flag *Building::get_base_flag()
{
	return m_flag;
}


/*
===============
Building::get_playercaps

Return a bitfield of commands the owning player can issue for this building.
The bits are (1 << PCap_XXX).
By default, all buildable buildings can be bulldozed.
===============
*/
uint Building::get_playercaps()
{
	uint caps = 0;

	if (get_descr()->get_buildable())
		caps |= 1 << PCap_Bulldoze;

	return caps;
}


/*
===============
Building::start_animation

Start the given animation
===============
*/
void Building::start_animation(Editor_Game_Base *g, uint anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/*
===============
Building::init

Common building initialization code. You must call this from derived class' init.
===============
*/
void Building::init(Editor_Game_Base* g)
{
	PlayerImmovable::init(g);

	// Set the building onto the map
	Map *map = g->get_map();
	Coords neighb;

	set_position(g, m_position);

	if (get_size() == BIG) {
		map->get_ln(m_position, &neighb);
		set_position(g, neighb);

		map->get_tln(m_position, &neighb);
		set_position(g, neighb);

		map->get_trn(m_position, &neighb);
		set_position(g, neighb);
	}

	// Make sure the flag is there
	BaseImmovable *imm;
	Flag *flag;

	map->get_brn(m_position, &neighb);
	imm = map->get_immovable(neighb);

	if (imm && imm->get_type() == FLAG)
		flag = (Flag *)imm;
	else
		flag = Flag::create(g, get_owner(), neighb);

	m_flag = flag;
	m_flag->attach_building(g, this);

	// Start the animation
	start_animation(g, get_descr()->get_idle_anim());

	m_leave_time = g->get_gametime();
}

/*
===============
Building::cleanup

Cleanup the building
===============
*/
void Building::cleanup(Editor_Game_Base *g)
{
	// Remove from flag
	m_flag->detach_building(g);

	// Unset the building
   unset_position(g, m_position);

	if (get_size() == BIG) {
		Map *map = g->get_map();
		Coords neighb;

		map->get_ln(m_position, &neighb);
		unset_position(g, neighb);

		map->get_tln(m_position, &neighb);
		unset_position(g, neighb);

		map->get_trn(m_position, &neighb);
		unset_position(g, neighb);
	}

	PlayerImmovable::cleanup(g);
}


/*
===============
Building::leave_check_and_wait

Return true if the given worker can leave the building immediately.
Otherwise, return false. The worker's wakeup_leave_building() will be called as
soon as the worker can leave the building.
===============
*/
bool Building::leave_check_and_wait(Game* g, Worker* w)
{
	Map_Object* allow = m_leave_allow.get(g);

	molog("Building::leave_check_and_wait\n");

	if (w == allow) {
		m_leave_allow = 0;
		return true;
	}

	// Check time and queue
	uint time = g->get_gametime();

	if (!m_leave_queue.size())
	{
		if ((int)(time - m_leave_time) >= 0) {
			molog("Building::leave_check_and_wait: Leave now\n");
			m_leave_time = time + BUILDING_LEAVE_INTERVAL;
			return true;
		}

		schedule_act(g, m_leave_time - time);
	}

	molog("Building::leave_check_and_wait: Put on queue\n");

	m_leave_queue.push_back(w);
	return false;
}


/*
===============
Building::act

Advance the leave queue.
===============
*/
void Building::act(Game *g, uint data)
{
	uint time = g->get_gametime();

	if ((int)(time - m_leave_time) >= 0)
	{
		bool wakeup = false;

		// Wake up one worker
		while(m_leave_queue.size())
		{
			Worker* w = (Worker*)m_leave_queue[0].get(g);

			m_leave_queue.erase(m_leave_queue.begin());

			if (!w)
				continue;

			m_leave_allow = w;

			if (w->wakeup_leave_building(g, this)) {
				m_leave_time = time + BUILDING_LEAVE_INTERVAL;
				wakeup = true;
				break;
			}
		}

		if (m_leave_queue.size())
			schedule_act(g, m_leave_time - time);

		if (!wakeup)
			m_leave_time = time; // make sure leave_time doesn't get too far behind
	}

	PlayerImmovable::act(g, data);
}


/*
===============
Building::fetch_from_flag [virtual]

This function is called by our base flag to indicate that some item on the
flag wants to move into this building.
Return true if we can service that request (even if it is delayed), or false
otherwise.
===============
*/
bool Building::fetch_from_flag(Game* g)
{
	molog("TODO: Implement Building::fetch_from_flag\n");

	return false;
}


/*
===============
Building::draw

Draw the building.
===============
*/
void Building::draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos)
{
	if (coords != m_position)
		return; // draw big buildings only once

	dst->drawanim(pos.x, pos.y, m_anim, game->get_gametime() - m_animstart, get_owner()->get_playercolor());

	// door animation?
}


/*
==============================================================================

ConstructionSite BUILDING

==============================================================================
*/

/*
===============
ConstructionSite_Descr::ConstructionSite_Descr
===============
*/
ConstructionSite_Descr::ConstructionSite_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
}


/*
===============
ConstructionSite_Descr::parse

Parse tribe-specific construction site data, such as graphics, worker type,
etc...
===============
*/
void ConstructionSite_Descr::parse(const char* directory, Profile* prof, const EncodeData* encdata)
{
	Building_Descr::parse(directory, prof, encdata);

	// TODO
}


/*
===============
ConstructionSite_Descr::create_object

Allocate a ConstructionSite
===============
*/
Building* ConstructionSite_Descr::create_object()
{
	return new ConstructionSite(this);
}


/*
==============================

IMPLEMENTATION

==============================
*/


/*
===============
ConstructionSite::ConstructionSite

Initialize with default values
===============
*/
ConstructionSite::ConstructionSite(ConstructionSite_Descr* descr)
	: Building(descr)
{
	m_building = 0;

	m_builder = 0;
	m_builder_request = 0;
}


/*
===============
ConstructionSite::~ConstructionSite
===============
*/
ConstructionSite::~ConstructionSite()
{
}


/*
===============
ConstructionSite::get_size

Override: construction size is always the same size as the building
===============
*/
int ConstructionSite::get_size()
{
	return m_building->get_size();
}


/*
===============
ConstructionSite::get_playercaps

Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint ConstructionSite::get_playercaps()
{
	uint caps = Building::get_playercaps();

	caps |= 1 << PCap_Bulldoze;

	return caps;
}


/*
===============
ConstructionSite::set_building

Set the type of building we're going to build
===============
*/
void ConstructionSite::set_building(Building_Descr* descr)
{
	assert(!m_building);

	m_building = descr;
}


/*
===============
ConstructionSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void ConstructionSite::set_economy(Economy* e)
{
	Economy* old = get_economy();
	uint i;

	if (old) {
		for(i = 0; i < m_wares.size(); i++)
			m_wares[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for(i = 0; i < m_wares.size(); i++)
			m_wares[i]->add_to_economy(e);
	}
}


/*
===============
ConstructionSite::init

Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(Editor_Game_Base* g)
{
	Building::init(g);

	if (g->is_game()) {
		uint i;

		// TODO: figure out whether planing is necessary

		// Initialize the wares queues
		const Building_Descr::BuildCost* bc = m_building->get_buildcost();

		m_wares.resize(bc->size());

		for(i = 0; i < bc->size(); i++) {
			WaresQueue* wq = new WaresQueue(this);

			m_wares[i] = wq;

			wq->init((Game*)g, g->get_safe_ware_id((*bc)[i].name.c_str()), (*bc)[i].amount);
		}

		request_builder((Game*)g);
	}
}


/*
===============
ConstructionSite::cleanup

Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(Editor_Game_Base* g)
{
	// Release worker
	if (m_builder_request) {
		get_economy()->remove_request(m_builder_request);
		delete m_builder_request;
		m_builder_request = 0;
	}

	if (m_builder)
		m_builder->stop_job_idleloop((Game*)g);


	// Cleanup the wares queues
	for(uint i = 0; i < m_wares.size(); i++) {
		m_wares[i]->cleanup((Game*)g);
		delete m_wares[i];
	}
	m_wares.clear();

	Building::cleanup(g);

	// TODO: if necessary, warp the building here and make sure the builder gets home safely
}


/*
===============
ConstructionSite::request_builder

Issue a request for the builder.
===============
*/
void ConstructionSite::request_builder(Game* g)
{
	assert(!m_builder && !m_builder_request);

	m_builder_request = new Request(this, g->get_safe_ware_id("builder"),
	                                &ConstructionSite::request_builder_callback, this);
	get_economy()->add_request(m_builder_request);
}


/*
===============
ConstructionSite::request_builder_callback [static]

Called by transfer code when the builder has arrived on site.
===============
*/
void ConstructionSite::request_builder_callback(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	assert(w);

	ConstructionSite* cs = (ConstructionSite*)data;

	cs->m_builder = w;
	w->set_job_idleloop(g, w->get_idle_anim());

	// TODO: Make the builder walk around or whatever

	cs->get_economy()->remove_request(rq);
	delete rq;

	cs->m_builder_request = 0;
}


/*
==============================================================================

Warehouse building

==============================================================================
*/

/*
===============
Warehouse_Descr::Warehouse_Descr

Initialize with sane defaults
===============
*/
Warehouse_Descr::Warehouse_Descr(Tribe_Descr *tribe, const char *name)
	: Building_Descr(tribe, name)
{
	m_subtype = Subtype_Normal;
	m_conquers = 0;
}

/*
===============
Warehouse_Descr::parse

Parse the additional warehouse settings from the given profile and directory
===============
*/
void Warehouse_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	add_attribute(Map_Object::WAREHOUSE);

	Building_Descr::parse(directory, prof, encdata);

	Section *global = prof->get_safe_section("global");
	const char *string;

	string = global->get_safe_string("subtype");
	if (!strcasecmp(string, "HQ")) {
		m_subtype = Subtype_HQ;
	} else if (!strcasecmp(string, "port")) {
		m_subtype = Subtype_Port;
	} else if (!strcasecmp(string, "none")) {
		//
	} else
		throw wexception("Unsupported warehouse subtype '%s'. Possible values: none, HQ, port", string);

	if (m_subtype == Subtype_HQ)
		m_conquers = global->get_int("conquers");
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Warehouse::Warehouse

Initialize a warehouse (zero contents, etc...)
===============
*/
Warehouse::Warehouse(Warehouse_Descr *descr)
	: Building(descr)
{
	m_next_carrier_spawn = 0;
}


/*
===============
Warehouse::Warehouse

Cleanup
===============
*/
Warehouse::~Warehouse()
{
	// During building cleanup, we're removed from the Economy.
	// Therefore, the wares can simply be cleared out. The global inventory
	// will be okay.
	m_wares.clear();
}


/*
===============
Warehouse::init

Conquer the land around the HQ on init.
===============
*/
void Warehouse::init(Editor_Game_Base* gg)
{
   Building::init(gg);

   if (get_descr()->get_subtype() == Warehouse_Descr::Subtype_HQ)
      gg->conquer_area(get_owner()->get_player_number(), m_position, get_descr()->get_conquers());

	if (gg->is_game()) {
      Game* g=static_cast<Game*>(gg);

		m_next_carrier_spawn = schedule_act(g, CARRIER_SPAWN_INTERVAL);
   }
}


/*
===============
Warehouse::cleanup

Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Editor_Game_Base *g)
{
   // TODO: un-conquer the area?
	Building::cleanup(g);
}


/*
===============
Warehouse::act

Act regularly to create carriers. According to intelligence, this is some
highly advanced technology. Not only do the settlers have no problems with
birth control, they don't even need anybody to procreate. They must have
built-in DNA samples in those warehouses. And what the hell are they doing,
killing useless tribesmen! The Borg? Or just like Soilent Green?
Or maybe I should just stop writing comments that late at night ;-)
===============
*/
void Warehouse::act(Game *g, uint data)
{
	if (g->get_gametime() - m_next_carrier_spawn >= 0)
	{
		int id = g->get_safe_ware_id("carrier");
		int stock = m_wares.stock(id);
		int tdelta = CARRIER_SPAWN_INTERVAL;

		if (stock < 100) {
			tdelta -= 4*(100 - stock);
			create_wares(id, 1);
		} else if (stock > 100) {
			tdelta -= 4*(stock - 100);
			if (tdelta < 10)
				tdelta = 10;
			destroy_wares(id, 1);
		}

		m_next_carrier_spawn = schedule_act(g, tdelta);
	}

	Building::act(g, data);
}


/*
===============
Warehouse::set_economy

Transfer our registration to the new economy.
===============
*/
void Warehouse::set_economy(Economy *e)
{
	Economy *old = get_economy();

	if (old == e)
		return;

	if (old)
		old->remove_warehouse(this);

	Building::set_economy(e);

	if (e)
		e->add_warehouse(this);
}

/*
===============
Warehouse::create_wares

Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::create_wares(int id, int count)
{
	m_wares.add(id, count);

	assert(get_economy());

	get_economy()->add_wares(id, count);
}

/*
===============
Warehouse::destroy_wares

Magically destroy wares.
===============
*/
void Warehouse::destroy_wares(int id, int count)
{
	m_wares.remove(id, count);

	assert(get_economy());

	get_economy()->remove_wares(id, count);
}


/*
===============
Warehouse::fetch_from_flag

Launch a carrier to fetch an item from our flag.
===============
*/
bool Warehouse::fetch_from_flag(Game* g)
{
	int carrierid;
	Worker* worker;

	carrierid = g->get_ware_id("carrier");

	if (!m_wares.stock(carrierid)) // yep, let's cheat
		create_wares(carrierid, 1);

	worker = launch_worker(g, carrierid);
	worker->set_job_fetchfromflag(g);

	return true;
}


/*
===============
Warehouse::launch_worker

Start a worker of a given type. The worker will be assigned a job by the caller.
===============
*/
Worker *Warehouse::launch_worker(Game *g, int ware)
{
	assert(m_wares.stock(ware));

	Ware_Descr *waredescr;
	Worker_Descr *workerdescr;
	Worker *worker;

	waredescr = g->get_ware_description(ware);
	assert(waredescr->is_worker());

	workerdescr = ((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	m_wares.remove(ware, 1);
	get_economy()->remove_wares(ware, 1); // re-added by the worker himself

	return worker;
}


/*
===============
Warehouse::incorporate_worker

This is the opposite of launch_worker: destroy the worker and add the
appropriate ware to our warelist
===============
*/
void Warehouse::incorporate_worker(Game *g, Worker *w)
{
	int ware = w->get_ware_id();
	WareInstance* item = w->fetch_carried_item(g); // rescue an item

	w->remove(g);

	m_wares.add(ware, 1);
	get_economy()->add_wares(ware, 1);

	get_economy()->match_requests(this, ware);

	if (item)
		incorporate_item(g, item);
}


/*
===============
Warehouse::launch_item

Create an instance of a ware, and make sure it gets carried out of the warehouse.
===============
*/
WareInstance* Warehouse::launch_item(Game* g, int ware)
{
	WareInstance* item;
	int carrierid;
	Ware_Descr* waredescr;
	Worker_Descr* workerdescr;
	Worker* worker;

	// Create a carrier
	carrierid = g->get_ware_id("carrier");
	waredescr = g->get_ware_description(carrierid);
	workerdescr = ((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	// Yup, this is cheating.
	if (m_wares.stock(carrierid)) {
		m_wares.remove(carrierid, 1);
		get_economy()->remove_wares(carrierid, 1);
	}

	// Create the item
	item = new WareInstance(ware);
	item->init(g);

	m_wares.remove(ware, 1);
	get_economy()->remove_wares(ware, 1); // re-added by the item itself

	// Setup the carrier
	worker->set_job_dropoff(g, item);

	return item;
}


/*
===============
Warehouse::incorporate_item

Swallow the item, adding it to out inventory.
===============
*/
void Warehouse::incorporate_item(Game* g, WareInstance* item)
{
	int ware = item->get_ware();

	item->destroy(g);

	m_wares.add(ware, 1);
	get_economy()->add_wares(ware, 1);

	get_economy()->match_requests(this, ware);
}


/*
===============
Warehouse_Descr::create_object
===============
*/
Building *Warehouse_Descr::create_object()
{
	return new Warehouse(this);
}


/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

ProductionSite_Descr::ProductionSite_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
}


/*
===============
ProductionSite_Descr::parse

Parse the additional information necessary for production buildings
===============
*/
void ProductionSite_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Building_Descr::parse(directory, prof, encdata);

	// TODO
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
ProductionSite::ProductionSite
===============
*/
ProductionSite::ProductionSite(ProductionSite_Descr* descr)
	: Building(descr)
{
}


/*
===============
ProductionSite::~ProductionSite
===============
*/
ProductionSite::~ProductionSite()
{
}


/*
===============
ProductionSite::init

Initialize the production site.
===============
*/
void ProductionSite::init(Editor_Game_Base *g)
{
	Building::init(g);

	// TODO: request worker
}


/*
===============
ProductionSite::cleanup

Cleanup after a production site is removed
===============
*/
void ProductionSite::cleanup(Editor_Game_Base *g)
{
	// TODO: release worker, wares

	Building::cleanup(g);
}


/*
===============
ProductionSite_Descr::create_object

Create a new building of this type
===============
*/
Building* ProductionSite_Descr::create_object()
{
	return new ProductionSite(this);
}


/*
==============================================================================

Building_Descr Factory

==============================================================================
*/

/*
===============
Building_Descr::create_from_dir

Open the appropriate configuration file and check if a building description
is there.

May return 0.
===============
*/
Building_Descr *Building_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory,
		                                          const EncodeData *encdata)
{
	const char *name;
	
	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');
	
	if (backslash && (!slash || backslash > slash))
		slash = backslash;
	
	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Building_Descr *descr = 0;
	char fname[256];
	
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	if (!g_fs->FileExists(fname))
		return 0;
	
	try
	{
		Profile prof(fname, "global"); // section-less file
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");
		
		if (!strcasecmp(type, "warehouse"))
			descr = new Warehouse_Descr(tribe, name);
		else if (!strcasecmp(type, "production"))
			descr = new ProductionSite_Descr(tribe, name);
		else if (!strcasecmp(type, "construction"))
			descr = new ConstructionSite_Descr(tribe, name);
		else
			throw wexception("Unknown building type '%s'", type);
		
		descr->parse(directory, &prof, encdata);
		prof.check_used();
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading building %s: %s", name, e.what());
	}
	catch(...) {
		if (descr)
			delete descr;
		throw;
	}
	
	return descr;
}



#if 0
// 
// Need List
// 
int NeedWares_List::read(FileRead* f)
{
   nneeds = f->Signed16();
   if(!nneeds) {
      // we're done, this guy is for free
      return RET_OK;
   }
   

   list=(List*) malloc(sizeof(List)*nneeds);

   int i;
   for(i=0; i< nneeds; i++) {
      list[i].count = f->Unsigned16();
      list[i].index = f->Unsigned16();
      list[i].stock = f->Unsigned16();
   }

   return RET_OK;
}

// 
// Down here: Descriptions

int Has_Needs_Building_Descr::read(FileRead *f)
{
   uchar temp;
   temp = f->Unsigned8();
   needs_or=temp ? true : false;

   // read needs
   needs.read(f);
   
   return RET_OK;
}

int Has_Products_Building_Descr::read(FileRead *f)
{
   uchar temp;
	temp = f->Unsigned8();
	products_or = temp ? true : false;

   // read products
   products.read(f);

   return RET_OK;
}

int Has_Is_A_Building_Descr::read(FileRead *f)
{
	is_a = f->Unsigned16();

	return RET_OK;
}

int Buildable_Building_Descr::read(FileRead *f)
{
	memcpy(category, f->Data(sizeof(category)), sizeof(category));
	build_time = f->Unsigned16();;

	// read cost
	cost.read(f);

	build.set_dimensions(idle.get_w(), idle.get_h());
	build.set_hotspot(idle.get_hsx(), idle.get_hsy());
	
	build.read(f);
	
	return RET_OK;
}

int Working_Building_Descr::read(FileRead *f)
{
   working.set_dimensions(idle.get_w(), idle.get_h());
   working.set_hotspot(idle.get_w(), idle.get_h());
   
   working.read(f);

   return RET_OK;
}

int Boring_Building_Descr::read(FileRead *f)
{
   // nothing to do
   return RET_OK;
}


// 
// DOWN HERE: The real buildings, no abstractions
// 
/*
==============================================================================

Dig Building

==============================================================================
*/

int Dig_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   memcpy(resource, f->Data(sizeof(resource)), sizeof(resource));

   return RET_OK;
}
Map_Object *Dig_Building_Descr::create_object()
{
   cerr << "Dig_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}

/*
==============================================================================

Search Building

==============================================================================
*/

int Search_Building_Descr::read(FileRead *f)
{
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // read our own stuff
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   nbobs = f->Unsigned16();
   bobs = (char*)malloc(nbobs*30);
	memcpy(bobs, f->Data(nbobs*30), nbobs*30);

   return RET_OK;
}
Map_Object *Search_Building_Descr::create_object()
{
   cerr << "Search_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}

/*
==============================================================================

Plant Building

==============================================================================
*/

int Plant_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // read our own stuff
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   nbobs = f->Unsigned16();
   bobs = (char*)malloc(nbobs*30);
   memcpy(bobs, f->Data(nbobs*30), nbobs*30);

   return RET_OK;
}
Map_Object *Plant_Building_Descr::create_object()
{
   cerr << "Plant_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Grow Building

==============================================================================
*/

int Grow_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own stuff
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   memcpy(plant_bob, f->Data(sizeof(plant_bob)), sizeof(plant_bob));
	memcpy(search_bob, f->Data(sizeof(search_bob)), sizeof(search_bob));

   return RET_OK;
}
Map_Object *Grow_Building_Descr::create_object()
{
   cerr << "Grow_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Sitter Building

==============================================================================
*/

int Sit_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // our stuff 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   uchar temp; 
   temp = f->Unsigned8();
   order_worker=temp ? true : false;

   return RET_OK;
}
Map_Object *Sit_Building_Descr::create_object()
{
   cerr << "Sit_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}


int Sit_Building_Produ_Worker_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   prod_worker = f->Unsigned16();

   return RET_OK;
}
Map_Object *Sit_Building_Produ_Worker_Descr::create_object()
{
   cerr << "Sit_Building_Produ_Worker_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Science Building

==============================================================================
*/

int Science_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   cerr << "Science_Building_Descr::read() TODO!" << endl;

   return RET_OK;
}
Map_Object *Science_Building_Descr::create_object()
{
   cerr << "Science_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Military Building

==============================================================================
*/

int Military_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff
   beds = f->Unsigned16();
   conquers = f->Unsigned16();
   idle_time = f->Unsigned16();
   nupgr = f->Unsigned16();

   return RET_OK;
}
Map_Object *Military_Building_Descr::create_object()
{
   cerr << "Military_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Cannon

==============================================================================
*/

int Cannon_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   uchar temp;
   // own stuff 
   idle_time = f->Unsigned16();
   projectile_speed = f->Unsigned16();
   temp = f->Unsigned8();
   fires_balistic=temp ? true : false;
   worker = f->Unsigned16();
   //                         // width and height ob projectile bob
   ushort wproj, hproj;
   wproj = f->Unsigned16();
   hproj = f->Unsigned16();

   projectile.set_dimensions(wproj, hproj);
   projectile.set_hotspot(wproj/2, hproj/2);

   fire_ne.set_dimensions(idle.get_w(), idle.get_h());
   fire_ne.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_e.set_dimensions(idle.get_w(), idle.get_h());
   fire_e.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_se.set_dimensions(idle.get_w(), idle.get_h());
   fire_se.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_sw.set_dimensions(idle.get_w(), idle.get_h());
   fire_sw.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_w.set_dimensions(idle.get_w(), idle.get_h());
   fire_w.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_nw.set_dimensions(idle.get_w(), idle.get_h());
   fire_nw.set_hotspot(idle.get_hsx(), idle.get_hsy());

   projectile.read(f);
   fire_ne.read(f);
   fire_e.read(f);
   fire_se.read(f);
   fire_sw.read(f);
   fire_w.read(f);
   fire_nw.read(f);

   return RET_OK;
}
Map_Object *Cannon_Descr::create_object()
{
   cerr << "Cannon_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}

/*
==============================================================================

Store

==============================================================================
*/

int Store_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
Map_Object *Store_Descr::create_object()
{
   cerr << "Store_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Dockyard

==============================================================================
*/

int Dockyard_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();

   return RET_OK;
}
Map_Object *Dockyard_Descr::create_object()
{
   cerr << "Dockyard_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Port

==============================================================================
*/

int Port_Descr::read(FileRead *f) {

   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
Map_Object *Port_Descr::create_object()
{
   cerr << "Port_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


#endif
