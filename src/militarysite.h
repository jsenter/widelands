/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#ifndef MILITARYSITE_H
#define MILITARYSITE_H

#include "productionsite.h"
#include "requirements.h"
#include "soldiercontrol.h"

namespace Widelands {

class Soldier;

struct MilitarySite_Descr : public ProductionSite_Descr {
	MilitarySite_Descr
		(const Tribe_Descr &, const std::string & militarysite_name);
	virtual ~MilitarySite_Descr();

	virtual void parse(char const * directory, Profile *, EncodeData const *);
	virtual Building * create_object() const;

	virtual bool is_only_production_site() const throw () {return false;}

	virtual uint32_t get_conquers() const {return m_conquer_radius;}
	int32_t get_max_number_of_soldiers () const throw () {return m_num_soldiers;}
	int32_t get_max_number_of_medics   () const throw () {return m_num_medics;}
	int32_t get_heal_per_second        () const throw () {return m_heal_per_second;}
	int32_t get_heal_increase_per_medic() const throw ()
	{return m_heal_incr_per_medic;}

private:
	int32_t m_conquer_radius;
	int32_t m_num_soldiers;
	int32_t m_num_medics;
	int32_t m_heal_per_second;
	int32_t m_heal_incr_per_medic;
};

class MilitarySite : public ProductionSite, public SoldierControl {
	friend struct Map_Buildingdata_Data_Packet;
	MO_DESCR(MilitarySite_Descr);

public:
	MilitarySite(MilitarySite_Descr const &);
	virtual ~MilitarySite();

	virtual int32_t get_building_type() const throw ()
	{return Building::MILITARYSITE;}
	char const * type_name() const throw () {return "militarysite";}
	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint32_t data);
	virtual void remove_worker(Worker*);

	virtual void set_economy(Economy* e);
	virtual bool get_building_work(Game* g, Worker* w, bool success);

	// Begin implementation of SoldierControl
	virtual std::vector<Soldier *> presentSoldiers() const;
	virtual std::vector<Soldier *> stationedSoldiers() const;
	virtual uint32_t soldierCapacity() const;
	virtual void setSoldierCapacity(uint32_t capacity);
	virtual void dropSoldier(Soldier* soldier);
	// End implementation of SoldierControl

	/// This methods are helper for use at configure this site.
	void set_requirements (const Requirements&);
	void  clear_requirements ();
	const Requirements& get_requirements () const {return m_soldier_requirements;}

	/// Testing stuff
	uint32_t nr_attack_soldiers();
	void set_in_battle(bool const in_battle) {m_in_battle = in_battle;};

	void update_soldier_request();

protected:
	void conquer_area(Game &);

	virtual UI::Window * create_options_window
		(Interactive_Player * plr, UI::Window * * registry);

private:
	bool isPresent(Soldier* soldier) const;
	static void request_soldier_callback
		(Game *, Request *, Ware_Index, Worker *, void * data);

private:
	Requirements m_soldier_requirements;
	Request* m_soldier_request;
	bool m_didconquer;
	uint32_t m_capacity;
	bool m_in_battle;

	/**
	 * Next gametime where we should heal something.
	 */
	int32_t m_nexthealtime;
};

};

#endif
