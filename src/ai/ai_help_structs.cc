/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#include "ai/ai_help_structs.h"

#include "base/macros.h"
#include "base/time_string.h"
#include "logic/map.h"
#include "logic/map.h"
#include "logic/player.h"

// couple of constants for calculation of road interconnections
constexpr int kRoadNotFound = -1000;
constexpr int kShortcutWithinSameEconomy = 1000;
constexpr int kRoadToDifferentEconomy = 10000;



namespace Widelands {

// CheckStepRoadAI
CheckStepRoadAI::CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe)
   : player(pl), movecaps(mc), open_end(oe) {
}

bool CheckStepRoadAI::allowed(
   Map& map, FCoords start, FCoords end, int32_t, CheckStep::StepId const id) const {
	uint8_t endcaps = player->get_buildcaps(end);

	// we should not cross fields with road or flags (or any other immovable)
	if ((map.get_immovable(start)) && !(id == CheckStep::stepFirst)) {
		return false;
	}

	// Calculate cost and passability
	if (!(endcaps & movecaps))
		return false;

	// Check for blocking immovables
	if (BaseImmovable const* const imm = map.get_immovable(end))
		if (imm->get_size() >= BaseImmovable::SMALL) {
			if (id != CheckStep::stepLast && !open_end)
				return false;

			if (dynamic_cast<Flag const*>(imm))
				return true;

			if (!dynamic_cast<Road const*>(imm) || !(endcaps & BUILDCAPS_FLAG))
				return false;
		}

	return true;
}

bool CheckStepRoadAI::reachable_dest(const Map& map, const FCoords& dest) const {
	NodeCaps const caps = dest.field->nodecaps();

	if (!(caps & movecaps)) {
		if (!((movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}

// We are looking for fields we can walk on
// and owned by hostile player.
FindNodeEnemy::FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeEnemy::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)
FindNodeEnemiesBuilding::FindNodeEnemiesBuilding(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeEnemiesBuilding::accept(const Map&, const FCoords& fc) const {
	return (fc.field->get_immovable()) && fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player
FindNodeUnowned::FindNodeUnowned(Player* p, Game& g, bool oe)
   : player(p), game(g), only_enemies(oe) {
}

bool FindNodeUnowned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) &&
	       ((fc.field->get_owned_by() == 0) ||
	        player->is_hostile(*game.get_player(fc.field->get_owned_by()))) &&
	       (!only_enemies || (fc.field->get_owned_by() != 0));
}

// Sometimes we need to know how many nodes our allies owns
FindNodeAllyOwned::FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n)
   : player(p), game(g), player_number(n) {
}

bool FindNodeAllyOwned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() != 0) &&
	       (fc.field->get_owned_by() != player_number) &&
	       !player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
FindNodeUnownedMineable::FindNodeUnownedMineable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedMineable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
}

// Unowned but walkable fields nearby
FindNodeUnownedWalkable::FindNodeUnownedWalkable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedWalkable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == 0);
}

// Looking only for mines-capable fields nearby
// of specific type
FindNodeMineable::FindNodeMineable(Game& g, DescriptionIndex r) : game(g), res(r) {
}

bool FindNodeMineable::accept(const Map&, const FCoords& fc) const {

	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_resources() == res);
}

// Fishers and fishbreeders must be built near water
FindNodeWater::FindNodeWater(const World& world) : world_(world) {
}

bool FindNodeWater::accept(const Map& map, const FCoords& coord) const {
	return (world_.terrain_descr(coord.field->terrain_d()).get_is() &
	        TerrainDescription::Is::kWater) ||
	       (world_.terrain_descr(map.get_neighbour(coord, WALK_W).field->terrain_r()).get_is() &
	        TerrainDescription::Is::kWater) ||
	       (world_.terrain_descr(map.get_neighbour(coord, WALK_NW).field->terrain_r()).get_is() &
	        TerrainDescription::Is::kWater);
}

bool FindNodeOpenWater::accept(const Map& /* map */, const FCoords& coord) const {
	return !(coord.field->nodecaps() & MOVECAPS_WALK) && (coord.field->nodecaps() & MOVECAPS_SWIM);
}

// FindNodeWithFlagOrRoad
bool FindNodeWithFlagOrRoad::accept(const Map&, FCoords fc) const {
	if (upcast(PlayerImmovable const, pimm, fc.field->get_immovable()))
		return (dynamic_cast<Flag const*>(pimm) ||
		        (dynamic_cast<Road const*>(pimm) && (fc.field->nodecaps() & BUILDCAPS_FLAG)));
	return false;
}

NearFlag::NearFlag(const Flag& f, int32_t const c, int32_t const d)
   : flag(&f), cost(c), distance(d) {
}

BuildableField::BuildableField(const Widelands::FCoords& fc)
	: coords(fc),
	  field_info_expiration(20000),
	  preferred(false),
	  enemy_nearby(0),
	  enemy_accessible_(false),
	  unowned_land_nearby(0),
	  near_border(false),
	  unowned_mines_spots_nearby(0),
	  trees_nearby(0),
	  // explanation of starting values
	  // this is done to save some work for AI (CPU utilization)
	  // base rules are:
	  // count of rocks can only decrease, so  amount of rocks
	  // is recalculated only when previous count is positive
	  // count of water fields are stable, so if the current count is
	  // non-negative, water is not recaldulated
	  rocks_nearby(1),
	  water_nearby(-1),
	  open_water_nearby(-1),
	  distant_water(0),
	  fish_nearby(-1),
	  critters_nearby(-1),
	  ground_water(1),
	  space_consumers_nearby(0),
	  rangers_nearby(0),
	  area_military_capacity(0),
	  military_loneliness(1000),
	  military_in_constr_nearby(0),
	  area_military_presence(0),
	  military_stationed(0),
	  unconnected_nearby(false),
	  military_unstationed(0),
	  is_portspace(Widelands::ExtendedBool::kUnset),
	  port_nearby(false),
	  portspace_nearby(Widelands::ExtendedBool::kUnset),
	  max_buildcap_nearby(0),
	  last_resources_check_time(0),
	  military_score_(0),
	  inland(false),
	  local_soldier_capacity(0),
      is_militarysite(false) {
}

int32_t BuildableField::own_military_sites_nearby_() {
	return military_stationed + military_unstationed;
}

MineableField::MineableField(const Widelands::FCoords& fc)
   : coords(fc),
     field_info_expiration(20000),
     preferred(false),
     mines_nearby(0),
     same_mine_fields_nearby(0) {
}

EconomyObserver::EconomyObserver(Widelands::Economy& e) : economy(e) {
	dismantle_grace_time = std::numeric_limits<int32_t>::max();
}

int32_t BuildingObserver::total_count() const {
	return cnt_built + cnt_under_construction;
}

Widelands::AiModeBuildings BuildingObserver::aimode_limit_status() {
	if (total_count() > cnt_limit_by_aimode) {
		return Widelands::AiModeBuildings::kLimitExceeded;
	} else if (total_count() == cnt_limit_by_aimode) {
		return Widelands::AiModeBuildings::kOnLimit;
	} else {
		return Widelands::AiModeBuildings::kAnotherAllowed;
	}
}
bool BuildingObserver::buildable(Widelands::Player& p) {
	return is_buildable && p.is_building_type_allowed(id);
}

// Computer player does not get notification messages about enemy militarysites
// and warehouses, so following is collected based on observation
// It is conventient to have some information preserved, like nearby minefields,
// when it was attacked, whether it is warehouse and so on
// Also AI test more such targets when considering attack and calculated score is
// is stored in the observer
EnemySiteObserver::EnemySiteObserver()
   : is_warehouse(false),
     attack_soldiers_strength(0),
     defenders_strength(0),
     stationed_soldiers(0),
     last_time_attackable(std::numeric_limits<uint32_t>::max()),
     last_tested(0),
     score(0),
     mines_nearby(Widelands::ExtendedBool::kUnset),
     no_attack_counter(0) {
}

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
MineTypesObserver::MineTypesObserver() : in_construction(0), finished(0) {
}

ManagementData::ManagementData() {
		scores[0] = 1;
		scores[1] = 1;
		scores[2] = 1;
		review_count = 0;
		last_mutate_time = 0;
		next_neuron_id = 0;
		performance_change = 0;
}

Neuron::Neuron(int8_t w, uint8_t f, uint16_t i) : 
	weight(w),type(f), id(i) {
	assert(type < neuron_curves.size());
	assert(weight >= -100 && weight <= 100);
	lowest_pos = std::numeric_limits<uint8_t>::max();
	highest_pos = std::numeric_limits<uint8_t>::min();
	recalculate();
}


FNeuron::FNeuron(uint8_t c){
	assert (c < 32);
	core = c;
}

bool FNeuron::get_result(const bool bool1, const bool bool2, const bool bool3, const bool bool4, const bool bool5){
	//printf ("returning %lu\n", core[bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5]);
	return core[bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5];
}

uint8_t FNeuron::get_int(){
	return core.to_ulong();
}

void FNeuron::flip_bit(const uint8_t pos){
	assert (pos < f_neuron_bit_size);
	core.flip(pos);
}

void Neuron::set_weight(int8_t w) {
	if (w > 100) {
		weight = 100;
	}else if (w < -100) {
		weight = -100;
	} else {
		weight = w;
	}
}


void Neuron::recalculate() {
	//printf (" type: %d\n", type);
	assert (neuron_curves.size() > type);
	for (int8_t i = 0; i <= 20; i+=1) {
		results[i] = weight * neuron_curves[type][i] / 100;
		//printf (" neuron output: %3d\n", results[i]);
	}
}

int8_t Neuron::get_result(const uint8_t pos){
	assert(pos <= 20);
	return results[pos];
}

int8_t Neuron::get_result_safe(int32_t pos){
	if (pos > highest_pos) {highest_pos = pos;};
	if (pos < lowest_pos) {lowest_pos = pos;};
	if (pos < 0) {
		pos = 0;
	}
	if (pos > 20) {
		pos = 20;
	}
	assert(pos <= 20);
	assert (results[pos] >= -100 && results[pos] <=100);
	return results[pos];
}

void Neuron::set_type(uint8_t new_type) {
	type = new_type;
}

// this randomly sets new values into neurons and AI magic numbers
void ManagementData::mutate(const uint32_t gametime) {

	int16_t probability = -1;
	if (last_mutate_time == 0) {
		probability = get_military_number_at(13) / 8 + 15;
	} else {
		probability = get_military_number_at(14) / 8 + 15;
	}
	assert(probability > 0);
	
	printf ("    ... mutating , time since last mutation: %6d, probability: 1 / %d\n",
	(gametime - last_mutate_time) / 1000 / 60,
	probability);
	last_mutate_time = gametime;

	for (uint16_t i = 0; i < magic_numbers_size; i += 1){
	   	if (std::rand() % probability == 0) {
			// poor man's gausian distribution probability
			int16_t new_value = ((-100 + std::rand() % 200) * 3 -100 + std::rand() % 200) / 4;
			assert (new_value >= -100 && new_value <=100);
			set_military_number_at(i,new_value);
			printf ("      Magic number %d: new value: %4d\n", i, new_value);
		} 
	}

	// Modifying pool of neurons	
	for (auto& item : neuron_pool){
		if (std::rand() % probability == 0) {
			item.set_weight(((-100 + std::rand() % 200) * 3 -100 + std::rand() % 200) / 4);
			pd->neuron_weights[item.get_id()] = item.get_weight();
			assert(neuron_curves.size() > 0);
			item.set_type(std::rand() % neuron_curves.size());
			pd->neuron_functs[item.get_id()] = item.get_type();
			printf ("      Neuron %2d: new weight: %4d, new curve: %d\n", item.get_id(), item.get_weight(), item.get_type());
			item.recalculate();
		} 
	}

	// Modifying pool of f-neurons
	uint16_t pos = 0;	
	for (auto& item : f_neuron_pool){
		if (std::rand() % probability == 0) {
			item.flip_bit(std::rand() % f_neuron_bit_size);
			if (std::rand() % 2 == 0) {
				item.flip_bit(std::rand() % f_neuron_bit_size);
			}

			pd->f_neurons[pos] = item.get_int();
			printf ("      F-Neuron %2d: new value: %4d\n", pos, item.get_int());
		}
		pos += 1; 
	}

	test_consistency();
	dump_data();
}


void ManagementData::review(const uint32_t gametime, PlayerNumber pn, const uint16_t mines,
 const uint32_t strength, const uint32_t strength_delta, const uint32_t land, const uint32_t land_delta) {
	assert(!pd->magic_numbers.empty());
	scores[0] = scores[1];
	scores[1] = scores[2];
	scores[2] = land / 3 + static_cast<int32_t>(land_delta) / 3 + strength + static_cast<int32_t>(strength_delta) + mines * 10;
	assert (scores[2] > -10000 && scores[2] < 100000);
	
	printf (" %d %s: reviewing AI mngm. data, score: %4d ->%4d ->%4d (land:%3d, delta: %3d, strength: %3d, delta:%3d, mines: %3d)\n",
	pn, gamestring_with_leading_zeros(gametime), scores[0], scores[1], scores[2],
	land, land_delta, strength, strength_delta, mines);


	performance_change = (scores[0] != 0) ? scores[2] * 100 / scores[0] : 0;

	//If under 10 seconds we re-initialize
	if (gametime < 25 * 1000){
		printf (" %d - reinitializing DNA\n", pn);
		initialize(pn, true);
		if (pn == 2) { 
			mutate(gametime);
		}
		
		dump_data();
	} else {
		printf ("   still using mutate from %d minutes ago (performance: %3d):\n",
		(gametime - last_mutate_time) / 1000 / 60,
		performance_change); 
		dump_data();
	}
	
	////review min-max values of neurons
	//if (gametime > 120 * 60 * 1000) {
		//for (auto & item : neuron_pool) {
			//if (item.get_highest_pos() > 22 || (item.get_highest_pos() > 0 && item.get_highest_pos() <14) || item.get_lowest_pos() < -2) {
				//printf ("   Neuron %2d: min: %3d, max: %3d\n", item.get_id(), item.get_lowest_pos(), item.get_highest_pos());
			//}
		//}
	//}
	
	review_count += 1;	
}

void ManagementData::initialize( const uint8_t pn, const bool reinitializing) {
	printf (" ... initialize starts %s\n", reinitializing?" * reinitializing *":"");


    //AutoSCore_EForests_   1073
    const std::vector<int16_t> AI_initial_military_numbers_A =
      {-65,  43, -70,  -1, -71,  12,  24,  48,  26, -39,  //AutoContent_01_EForests_
        12,  24, -43,  34, -100, -69,  28,  36, -68,   0,  //AutoContent_02_EForests_
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_03_EForests_
        52,   0,  19, -34, -16, -71,  46, -68, -76, -38,  //AutoContent_04_EForests_
       -22,   0, -17, -62, -53,  35, -43, -29, -22,  75 //AutoContent_05_EForests_
       }
		;
	
	assert(magic_numbers_size == AI_initial_military_numbers_A.size());
	
	const std::vector<int8_t> input_weights_A=
      {-15,  64, -45, -47,  48,  -5,   1,  10,   8, -24,  //AutoContent_06_EForests_
        -2, -29,   0,  71,  49, -68, -30,  16,  67, -68,  //AutoContent_07_EForests_
         7,  -8,  58,  35, -11,  92, -87,  -4,  38,  37,  //AutoContent_08_EForests_
        12,  -2,  52,  89,  40,  35,  -2, -27, -17,  45,  //AutoContent_09_EForests_
         0,  47,  83, -39,  -4,  -6,   3,  35, -16,  50,  //AutoContent_10_EForests_
       -75, -12,  27,  49, -42, -38, -31,   5, -20, -43 //AutoContent_11_EForests_
	}
			;
	const std::vector<int8_t> input_func_A=
      {  3,   3,   3,   4,   0,   1,   1,   3,   3,   0,  //AutoContent_12_EForests_
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_13_EForests_
         2,   0,   0,   2,   1,   3,   1,   2,   3,   3,  //AutoContent_14_EForests_
         1,   3,   4,   4,   0,   3,   1,   1,   2,   4,  //AutoContent_15_EForests_
         0,   2,   0,   0,   3,   4,   3,   3,   1,   0,  //AutoContent_16_EForests_
         0,   2,   2,   4,   1,   1,   2,   2,   3,   0 //AutoContent_17_EForests_
	}
		;
	assert(neuron_pool_size == input_func_A.size());
	assert(neuron_pool_size == input_weights_A.size());

	const std::vector<int16_t> f_neurons_A =
      {  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  //AutoContent_18_EForests_
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7 //AutoContent_19_EForests_
	 };
	assert(f_neuron_pool_size == f_neurons_A.size());

		
    //AutoSCore_CratEr___   860
	const std::vector<int16_t> AI_initial_military_numbers_B =
      {  8,   0,   0,  -1,   0,   0,   0,   0,  80,   0,  //AutoContent_01_CratEr___
        12,  24, -43,  34, -100, -40,  28,  36,  68,   0,  //AutoContent_02_CratEr___
       -45,  26,  32,  45,   0,  27, -83, -57, -33,  25,  //AutoContent_03_CratEr___
       -52,   0,  19, -34, -16, -71,  46,  68, -76, -38,  //AutoContent_04_CratEr___
       -22,   0, -17, -62, -53,  35, -43, -29, -22,  75 //AutoContent_05_CratEr___
}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_B.size());
		
	const std::vector<int8_t> input_weights_B =
      {-15,  64, -45,  80,  48,  -5,   1,  10,   8, -24,  //AutoContent_06_CratEr___
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_07_CratEr___
         7,  -8,  58,  35, -11,  92,  48,  -4,  38,  37,  //AutoContent_08_CratEr___
        12,  -2,  52,  89,  40,  35,  12, -27, -17,  45,  //AutoContent_09_CratEr___
         0,  47, -83, -39,  -4,  -6,  33,  35, -16,  50,  //AutoContent_10_CratEr___
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0 //AutoContent_11_CratEr___
}
	      ;
	
	const std::vector<int8_t> input_func_B = 
      {  3,   3,   3,   2,   0,   1,   1,   3,   3,   0,  //AutoContent_12_CratEr___
         2,   3,   4,   4,   4,   2,   0,   1,   3,   1,  //AutoContent_13_CratEr___
         2,   0,   0,   2,   1,   3,   4,   2,   3,   3,  //AutoContent_14_CratEr___
         0,   0,   0,   0,   0,   0,   2,   0,   0,   0,  //AutoContent_15_CratEr___
         0,   2,   0,   0,   3,   4,   3,   3,   1,   0,  //AutoContent_16_CratEr___
         0,   2,   2,   4,   1,   1,   4,   2,   3,   0 //AutoContent_17_CratEr___
}
		;
		assert(neuron_pool_size == input_func_B.size());
		assert(neuron_pool_size == input_weights_B.size());

      
	const std::vector<int16_t> f_neurons_B =
      {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  //AutoContent_18_CratEr___
         25,   11,    1,   23,   16,    4,   21,    7,   24,    7 //AutoContent_19_CratEr___
	 };
	assert(f_neuron_pool_size == f_neurons_B.size());


    //AutoSCore_4Mount_   588
	const std::vector<int16_t> AI_initial_military_numbers_C =
      {-65,  43, -70,  -1, -71,  12,  24,  48,  26, -39,  //AutoContent_01_4Mount_
        12,  24, -43,  34, -100, -69,  28,  36, -68,   0,  //AutoContent_02_4Mount_
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_03_4Mount_
        52,   0,  19, -34, -16, -71,  46, -68, -76, -38,  //AutoContent_04_4Mount_
       -22,   0, -17, -62, -53,  35, -43, -29, -22,  75 //AutoContent_05_4Mount_
       }

		;
	
		assert(magic_numbers_size == AI_initial_military_numbers_C.size());
	
	const std::vector<int8_t> input_weights_C=
      {-15,  64, -45, -47,  48,  -5,   1,  10,   8, -24,  //AutoContent_06_4Mount_
        -2, -29,   0,  71,  49, -68, -30,  16,  67, -68,  //AutoContent_07_4Mount_
         7,  -8,  58,  35, -11,  92, -87,  -4,  38,  37,  //AutoContent_08_4Mount_
        12,  -2,  52,  89,  40,  35,  -2, -27, -17,  45,  //AutoContent_09_4Mount_
         0,  47,  83, -39,  -4,  -6,   3,  35, -16,  50,  //AutoContent_10_4Mount_
       -75, -12,  27,  49, -42, -38, -31,   5, -20, -43 //AutoContent_11_4Mount_
       }
			;
	const std::vector<int8_t> input_func_C=
      {  3,   3,   3,   4,   0,   1,   1,   3,   3,   0,  //AutoContent_12_4Mount_
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_13_4Mount_
         2,   0,   0,   2,   1,   3,   1,   2,   3,   3,  //AutoContent_14_4Mount_
         1,   3,   4,   4,   0,   3,   1,   1,   2,   4,  //AutoContent_15_4Mount_
         0,   2,   0,   0,   3,   4,   3,   3,   1,   0,  //AutoContent_16_4Mount_
         0,   2,   2,   4,   1,   1,   2,   2,   3,   0 //AutoContent_17_4Mount_
       }
			;
	assert(neuron_pool_size == input_func_C.size());
	assert(neuron_pool_size == input_weights_C.size());
	
	const std::vector<int16_t> f_neurons_C =
      {  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  //AutoContent_18_4Mount_
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7 //AutoContent_19_4Mount_
	 };
	assert(f_neuron_pool_size == f_neurons_C.size());

		
    //AutoSCore_Atol____   638
	const std::vector<int16_t> AI_initial_military_numbers_D =
      {-65,  43, -70,  -1,   0,  12,  24,  48,  26,   0,  //AutoContent_01_Atol____
        12, -30,   5,  34, -100, -69,  28,  36, -68,   0,  //AutoContent_02_Atol____
       -45,   0,  32,   0,   0,   0,   0,   0, -33,   0,  //AutoContent_03_Atol____
        52,   0,  19, -34, -16, -71,  46, -68, -76, -38,  //AutoContent_04_Atol____
       -22, -12, -17, -62, -53,  35, -43, -29, -22,  -3 //AutoContent_05_Atol____
}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_D.size());
		
	const std::vector<int8_t> input_weights_D =
      {-15,  64, -45, -47,  48,  -5,   1,  10,   8, -24,  //AutoContent_06_Atol____
        -2, -29,   0,  71,  49, -68, -30,  16,  67, -68,  //AutoContent_07_Atol____
         7,  -8,  58,  35, -11,  92, -87,  -4,  38,  37,  //AutoContent_08_Atol____
        12,  -2,  52,  27,  40,  35,  -2, -27, -17,  45,  //AutoContent_09_Atol____
         0,  47,  83, -39,  -4,  -6,   3,  35, -16,  50,  //AutoContent_10_Atol____
       -75, -12,  27,  49, -42, -38, -31,   5, -20, -43 //AutoContent_11_Atol____
}
	      ;
	
	const std::vector<int8_t> input_func_D = 
      {  3,   3,   3,   4,   0,   1,   1,   3,   3,   0,  //AutoContent_12_Atol____
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //AutoContent_13_Atol____
         2,   0,   0,   2,   1,   3,   1,   2,   3,   3,  //AutoContent_14_Atol____
         1,   3,   4,   3,   0,   3,   1,   1,   2,   4,  //AutoContent_15_Atol____
         0,   2,   0,   0,   3,   4,   3,   3,   1,   0,  //AutoContent_16_Atol____
         0,   2,   2,   4,   1,   1,   2,   2,   3,   0 //AutoContent_17_Atol____
}
		;
	assert(neuron_pool_size == input_func_D.size());
	assert(neuron_pool_size == input_weights_D.size());

	const std::vector<int16_t> f_neurons_D =
      {  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  //AutoContent_18_Atol____
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7 //AutoContent_19_Atol____
	 };
	assert(f_neuron_pool_size == f_neurons_D.size());


    //Static AI   
	const std::vector<int16_t> AI_initial_military_numbers_static =
      {-65,  43, -70,  -1, -71,  12,  24,  48,  26, -39,  // from EForests_
        12,  24, -43,  34, -100, -69,  28,  36, -68,   0,  // from EForests_
                0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  // from EForests_
       52,   0,  19, -34, -16, -71,  46, -68, -76, -38,  // from EForests_
       -22,   0, -17, -62, -53,  35, -43, -29, -22,  75 //A from EForests_
}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_static.size());
		
	const std::vector<int8_t> input_weights_static =
      {-15,  64, -45, -47,  48,  -5,   1,  10,   8, -24,  // from LRing
       -2, -29,   0,  71,  49, -68,  -30,  16,  67, -68,  // from LRing
         7,  -8,  58,  35, -11,  92,  -87,  -4,  38,  37,  // from LRing
        12,  -2,  52,  89,  40,  35,  -2, -27, -17,  45,  // from LRing
         0,  47, 83, -39,  -4,  -6,  3,  35, -16,  50,   // from LRing
       -75, -12,  27,  49, -42, -38, -31,   5, -20, -43  // from LRing
}
	      ;
	
	const std::vector<int8_t> input_func_static = 
      {  3,   3,   3,   4,   0,   1,   1,   3,   3,   0,  // from 4Mount_
                 0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  // from 4Mount_
         2,   0,   0,   2,   1,   3,   1,   2,   3,   3,  //A from 4Mount_
         1,   3,   4,   4,   0,   3,   1,   1,   2,   4,  // from 4Mount_
         0,   2,   0,   0,   3,   4,   3,   3,   1,   0,  // from 4Mount_
         0,   2,   2,   4,   1,   1,   2,   2,   3,   0 // from 4Mount_

}
		;
	assert(neuron_pool_size == input_func_static.size());
	assert(neuron_pool_size == input_weights_static.size());


	const std::vector<int16_t> f_neurons_static =
      {  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  // from Atol____
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7 //A from Atol____
	 };
	assert(f_neuron_pool_size == f_neurons_static.size());



	for (uint16_t i =  0; i < magic_numbers_size; i = i+1){
		if (std::abs(AI_initial_military_numbers_A[i]) < 45 &&
		std::abs(AI_initial_military_numbers_B[i]) < 45 &&
		std::abs(AI_initial_military_numbers_C[i]) < 45 &&
		std::abs(AI_initial_military_numbers_D[i]) < 45 ) {
			printf ("military number rebalance candidate: %2d: %4d %4d %4d %4d\n",
			i,
			AI_initial_military_numbers_A[i],
			AI_initial_military_numbers_B[i],
			AI_initial_military_numbers_C[i],
			AI_initial_military_numbers_D[i] );
		}
	}
	
	for (uint16_t i =  0; i < neuron_pool_size; i = i+1){
		uint8_t count = 0;
		count  += std::abs(input_weights_A[i]) > 80;
		count  += std::abs(input_weights_B[i]) > 80;
		count  += std::abs(input_weights_C[i]) > 80;
		count  += std::abs(input_weights_D[i]) > 80;	
		if (count >= 2 ) {
			printf ("pool weights rebalance candidate: %2d: %4d %4d %4d %4d\n",
			i,
			input_weights_A[i],
			input_weights_B[i],
			input_weights_C[i],
			input_weights_D[i] );
		}
	}

	printf (" %d: initializing AI's DNA\n", pn);

	// filling vector with zeros
	if (!reinitializing) {
		for (uint16_t i =  0; i < magic_numbers_size; i = i+1){
			pd->magic_numbers.push_back(0);
		}
	}
	assert (pd->magic_numbers.size() == magic_numbers_size);
	
	const uint8_t parent = std::rand() % 4;
	const uint8_t parent2 = std::rand() % 4;
	
	if (pn == 2) {
		printf (" ... DNA initialization (parent: %d, secondary parent: %d)\n", parent, parent2);
	}else {
		printf (" ... DNA initialization\n");
	}
	
	for (uint16_t i = 0; i < magic_numbers_size; i += 1){
		if (pn != 2) {
			set_military_number_at(i,AI_initial_military_numbers_static[i]);
			continue;
			}

		// Child inherites DNA with probability 5:1 from main parent
		const uint8_t dna_donor = (std::rand() % 6 > 0) ? parent : parent2;
		
		switch ( dna_donor ) {
			case 0 : 
				set_military_number_at(i,AI_initial_military_numbers_A[i]);
				break;
			case 1 : 
				set_military_number_at(i,AI_initial_military_numbers_B[i]);
				break;
			case 2 : 
				set_military_number_at(i,AI_initial_military_numbers_C[i]);
				break;
			case 3 : 
				set_military_number_at(i,AI_initial_military_numbers_D[i]);
				break;
			default:
				NEVER_HERE();
			}
		}

	if (reinitializing) {
		neuron_pool.clear();
		reset_neuron_id();
		pd->neuron_weights.clear();
		pd->neuron_functs.clear();
		f_neuron_pool.clear();
		pd->f_neurons.clear();
	}

	//printf (" ... initialize 2, pool size: %lu\n", neuron_pool.size());
	assert(neuron_pool.empty());
	
	for (uint16_t i = 0; i <neuron_pool_size; i += 1){
		if (pn != 2) {
			neuron_pool.push_back(Neuron(input_weights_static[i],input_func_static[i],new_neuron_id()));
			continue;
			}
		switch ( parent ) {
			case 0 : 
				neuron_pool.push_back(Neuron(input_weights_A[i],input_func_A[i],new_neuron_id()));
				break;
			case 1 : 
				neuron_pool.push_back(Neuron(input_weights_B[i],input_func_B[i],new_neuron_id()));
				break;
			case 2 : 
				neuron_pool.push_back(Neuron(input_weights_C[i],input_func_C[i],new_neuron_id()));
				break;
			case 3 : 
				neuron_pool.push_back(Neuron(input_weights_D[i],input_func_D[i],new_neuron_id()));
				break;
			default:
				NEVER_HERE();
		}
	}


	for (uint16_t i = 0; i <f_neuron_pool_size; i += 1){
		if (pn != 2) {
			f_neuron_pool.push_back(FNeuron(f_neurons_static[i]));
			continue;
			}
		switch ( parent ) {
			case 0 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_A[i]));				
				break;
			case 1 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_B[i]));	
				break;
			case 2 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_C[i]));					
				break;
			case 3 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_D[i]));	
				break;
			default:
				NEVER_HERE();
		}
	}
	
	//printf (" ... initialize 2.5, pool size: %lu\n", neuron_pool.size());
	assert(pd->neuron_weights.empty());
	assert(pd->neuron_functs.empty());	
	assert(pd->f_neurons.empty());
		
	for (uint32_t i = 0; i < neuron_pool_size; i = i+1){
		pd->neuron_weights.push_back(neuron_pool[i].get_weight());
		pd->neuron_functs.push_back(neuron_pool[i].get_type());	
	}

	for (uint32_t i = 0; i < f_neuron_pool_size; i = i+1){
		pd->f_neurons.push_back(f_neuron_pool[i].get_int());
	}

	//printf (" ... initialize 3\n");
	
	pd->magic_numbers_size = magic_numbers_size;
	pd->neuron_pool_size = neuron_pool_size;
	pd->f_neuron_pool_size = f_neuron_pool_size;	
	
	test_consistency();
	printf (" %d: DNA initialized\n", pn);
			
}

bool ManagementData::test_consistency() {

	assert (pd->neuron_weights.size() == pd->neuron_pool_size);
	assert (pd->neuron_functs.size() == pd->neuron_pool_size);
	assert (neuron_pool.size() == pd->neuron_pool_size);
	assert (neuron_pool.size() == neuron_pool_size);
	
	assert (pd->magic_numbers_size == magic_numbers_size);			
	assert (pd->magic_numbers.size() == magic_numbers_size);
	
	assert (pd->f_neurons.size() == pd->f_neuron_pool_size);
	assert (f_neuron_pool.size() == pd->f_neuron_pool_size);
	assert (f_neuron_pool.size() == f_neuron_pool_size);	
	return true; //?
}

void ManagementData::dump_data() {
		//dumping new numbers
	printf ("     actual military_numbers (%lu):\n      {", pd->magic_numbers.size());
	uint16_t itemcounter = 1;
	uint16_t line_counter = 1;
	for (const auto& item : pd->magic_numbers) {
		printf ("%3d%s",item,(&item != &pd->magic_numbers.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");
	
	printf ("     actual neuron setup:\n      ");
	printf ("{");
	itemcounter = 1;
	for (auto& item : neuron_pool) {
		printf ("%3d%s",item.get_weight(),(&item != &neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n      {");
	itemcounter = 1;	
	for (auto& item : neuron_pool) {
		printf ("%3d%s",item.get_type(),(&item != &neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");


	printf ("     actual f-neuron setup:\n      ");
	printf ("{");
	itemcounter = 1;
	for (auto& item : f_neuron_pool) {
		printf ("%4u%s",item.get_int(),(&item != &f_neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");


}

int16_t ManagementData::get_military_number_at(uint8_t pos) {
	assert (pos < magic_numbers_size);
	return pd->magic_numbers[pos];
}

void ManagementData::set_military_number_at(const uint8_t pos, const int16_t value) {
	assert (pos < magic_numbers_size);
	assert (pos < pd->magic_numbers.size());
	assert (value >= -100 && value <= 100);
	pd->magic_numbers[pos] = value;
}

uint16_t MineTypesObserver::total_count() const {
	return in_construction + finished;
}

// this is used to count militarysites by their size
MilitarySiteSizeObserver::MilitarySiteSizeObserver() : in_construction(0), finished(0) {
}

// this represents a scheduler task
SchedulerTask::SchedulerTask(const uint32_t time,
                             const Widelands::SchedulerTaskId t,
                             const uint8_t p,
                             const char* d)
   : due_time(time), id(t), priority(p), descr(d) {
}

bool SchedulerTask::operator<(SchedulerTask other) const {
	return priority > other.priority;
}

// List of blocked fields with block time, with some accompanying functions
void BlockedFields::add(Widelands::Coords coords, uint32_t till) {
	const uint32_t hash = coords.hash();
	if (blocked_fields_.count(hash) == 0) {
		blocked_fields_.insert(std::pair<uint32_t, uint32_t>(hash, till));
	} else if (blocked_fields_[hash] < till) {
		blocked_fields_[hash] = till;
	}
	// The third possibility is that a field has been already blocked for longer time than 'till'
}

uint32_t BlockedFields::count() {
	return blocked_fields_.size();
}

void BlockedFields::remove_expired(uint32_t gametime) {
	std::vector<uint32_t> fields_to_remove;
	for (auto field : blocked_fields_) {
		if (field.second < gametime) {
			fields_to_remove.push_back(field.first);
		}
	}
	while (!fields_to_remove.empty()) {
		blocked_fields_.erase(fields_to_remove.back());
		fields_to_remove.pop_back();
	}
}

bool BlockedFields::is_blocked(Coords coords) {
	return (blocked_fields_.count(coords.hash()) != 0);
}

FlagsForRoads::Candidate::Candidate(uint32_t coords, int32_t distance, bool economy)
   : coords_hash(coords), air_distance(distance), different_economy(economy) {
	new_road_possible = false;
	accessed_via_roads = false;
	// Values are only very rough, and are dependant on the map size
	new_road_length = 2 * Widelands::kMapDimensions.at(Widelands::kMapDimensions.size() - 1);
	current_roads_distance = 2 * (Widelands::kMapDimensions.size() - 1);  // must be big enough
	reduction_score = -air_distance;  // allows reasonable ordering from the start
}

bool FlagsForRoads::Candidate::operator<(const Candidate& other) const {
	if (reduction_score == other.reduction_score) {
		return coords_hash < other.coords_hash;
	} else {
		return reduction_score > other.reduction_score;
	}
}

bool FlagsForRoads::Candidate::operator==(const Candidate& other) const {
	return coords_hash == other.coords_hash;
}

void FlagsForRoads::Candidate::calculate_score() {
	if (!new_road_possible) {
		reduction_score = kRoadNotFound - air_distance;  // to have at least some ordering preserved
	} else if (different_economy) {
		reduction_score = kRoadToDifferentEconomy - air_distance - 2 * new_road_length;
	} else if (!accessed_via_roads) {
		if (air_distance + 6 > new_road_length) {
			reduction_score = kShortcutWithinSameEconomy - air_distance - 2 * new_road_length;
		} else {
			reduction_score = kRoadNotFound;
		}
	} else {
		reduction_score = current_roads_distance - 2 * new_road_length;
	}
}

void FlagsForRoads::print() {  // this is for debugging and development purposes
	for (auto& candidate_flag : queue) {
		log("   %starget: %3dx%3d, saving: %5d (%3d), air distance: %3d, new road: %6d, score: %5d "
		    "%s\n",
		    (candidate_flag.reduction_score >= min_reduction && candidate_flag.new_road_possible) ?
		       "+" :
		       " ",
		    Coords::unhash(candidate_flag.coords_hash).x,
		    Coords::unhash(candidate_flag.coords_hash).y,
		    candidate_flag.current_roads_distance - candidate_flag.new_road_length, min_reduction,
		    candidate_flag.air_distance, candidate_flag.new_road_length,
		    candidate_flag.reduction_score,
		    (candidate_flag.new_road_possible) ? ", new road possible" : " ");
	}
}

// Queue is ordered but some target flags are only estimations so we take such a candidate_flag
// first
bool FlagsForRoads::get_best_uncalculated(uint32_t* winner) {
	for (auto& candidate_flag : queue) {
		if (!candidate_flag.new_road_possible) {
			*winner = candidate_flag.coords_hash;
			return true;
		}
	}
	return false;
}

// Road from starting flag to this flag can be built
void FlagsForRoads::road_possible(Widelands::Coords coords, uint32_t distance) {
	// std::set does not allow updating
	Candidate new_candidate_flag = Candidate(0, 0, false);
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == coords.hash()) {
			new_candidate_flag = candidate_flag;
			assert(new_candidate_flag.coords_hash == candidate_flag.coords_hash);
			queue.erase(candidate_flag);
			break;
		}
	}

	new_candidate_flag.new_road_length = distance;
	new_candidate_flag.new_road_possible = true;
	new_candidate_flag.calculate_score();
	queue.insert(new_candidate_flag);
}

// Remove the flag from candidates as interconnecting road is not possible
void FlagsForRoads::road_impossible(Widelands::Coords coords) {
	const uint32_t hash = coords.hash();
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == hash) {
			queue.erase(candidate_flag);
			return;
		}
	}
}

// Updating walking distance over existing roads
// Queue does not allow modifying its members so we erase and then eventually insert modified member
void FlagsForRoads::set_road_distance(Widelands::Coords coords, int32_t distance) {
	const uint32_t hash = coords.hash();
	Candidate new_candidate_flag = Candidate(0, 0, false);
	bool replacing = false;
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == hash) {
			assert(!candidate_flag.different_economy);
			if (distance < candidate_flag.current_roads_distance) {
				new_candidate_flag = candidate_flag;
				queue.erase(candidate_flag);
				replacing = true;
				break;
			}
			break;
		}
	}
	if (replacing) {
		new_candidate_flag.current_roads_distance = distance;
		new_candidate_flag.accessed_via_roads = true;
		new_candidate_flag.calculate_score();
		queue.insert(new_candidate_flag);
	}
}

bool FlagsForRoads::get_winner(uint32_t* winner_hash, uint32_t pos) {
	assert(pos == 1 || pos == 2);
	uint32_t counter = 1;
	// If AI can ask for 2nd position, but there is only one viable candidate
	// we return the first one of course
	bool has_winner = false;
	for (auto candidate_flag : queue) {
		if (candidate_flag.reduction_score < min_reduction || !candidate_flag.new_road_possible) {
			continue;
		}
		assert(candidate_flag.air_distance > 0);
		assert(candidate_flag.reduction_score >= min_reduction);
		assert(candidate_flag.new_road_possible);
		*winner_hash = candidate_flag.coords_hash;
		has_winner = true;

		if (counter == pos) {
			return true;
		} else if (counter < pos) {
			counter += 1;
		} else {
			break;
		}
	}
	if (has_winner) {
		return true;
	}
	return false;
}

// This is an struct that stores strength of players, info on teams and provides some outputs from
// these data
PlayersStrengths::PlayerStat::PlayerStat() : team_number(0), is_enemy(false), players_power(0), old_players_power(0), players_casualities(0) {
}
PlayersStrengths::PlayerStat::PlayerStat(Widelands::TeamNumber tc, bool e, uint32_t pp, uint32_t op, uint32_t cs, uint32_t land, uint32_t oland)
   : team_number(tc), is_enemy(e), players_power(pp),  old_players_power(op),players_casualities(cs), players_land(land), old_players_land(oland)  {
	 last_time_seen = kNever;  
}

// Inserting/updating data
void PlayersStrengths::add(Widelands::PlayerNumber pn, Widelands::PlayerNumber opn, Widelands::TeamNumber en, Widelands::TeamNumber tn, uint32_t pp, uint32_t op,  uint32_t cs, uint32_t land, uint32_t oland) {
	if (all_stats.count(opn) == 0) {
		bool enemy = false;
		if ( pn == opn ) {
			;
		} else if (en == 0 || tn == 0) {
			enemy = true;
		} else if (en != tn) {
			enemy = true;			
		}
		
		printf (" %d PlayersStrengths: player %d is%s enemy\n", pn, opn, (enemy)?"":" not");
		all_stats.insert(std::pair<Widelands::PlayerNumber, PlayerStat>(opn, PlayerStat(tn, enemy, pp, op, cs, land, oland)));
	} else {
		all_stats[opn].players_power = pp;
		all_stats[opn].old_players_power = op;
		all_stats[opn].players_casualities = cs;
		all_stats[opn].players_land = land;
		all_stats[opn].old_players_land = oland;
	}
}

void PlayersStrengths::recalculate_team_power() {
	team_powers.clear();
	for (auto& item : all_stats) {
		if (item.second.team_number > 0) {  // is a member of a team
			if (team_powers.count(item.second.team_number) > 0) {
				team_powers[item.second.team_number] += item.second.players_power;
			} else {
				team_powers[item.second.team_number] = item.second.players_power;
			}
		}
	}
}

bool PlayersStrengths::any_enemy_seen_lately(const uint32_t gametime){
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			return true;
		}  // is a member of a team
	}
	return false;
}

uint8_t PlayersStrengths::enemies_seen_lately_count(const uint32_t gametime){
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			count +=1;
		}  // is a member of a team
	}
	return count;
}

uint32_t PlayersStrengths::enemy_last_seen(){
	uint32_t time = 0;
	for (auto& item : all_stats) {
		if (item.second.last_time_seen == kNever){
			continue;
		}
		if (item.second.is_enemy && item.second.last_time_seen > time) {
			time = item.second.last_time_seen;
		}  
	}
	if (time == 0) {return kNever;}
	return time;
}



void PlayersStrengths::set_last_time_seen(const uint32_t seentime, Widelands::PlayerNumber pn){
	assert(all_stats.count(pn) > 0);
	all_stats[pn].last_time_seen = seentime;
}

bool PlayersStrengths::get_is_enemy(Widelands::PlayerNumber pn){
	assert(all_stats.count(pn) > 0);
	return all_stats[pn].is_enemy;
}

bool PlayersStrengths::player_seen_lately( Widelands::PlayerNumber pn, const uint32_t gametime){
	assert(all_stats.count(pn) > 0);
	if (all_stats[pn].last_time_seen == kNever){
		return false;
	}
	if (all_stats[pn].last_time_seen + static_cast<uint32_t>(2 * 60 * 1000) > gametime){
		return true;
	}
	return false;
}

// This is strength of player
uint32_t PlayersStrengths::get_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_power;
	};
	return 0;
}

// This is land size owned by player
uint32_t PlayersStrengths::get_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_land;
	};
	return 0;
}

uint32_t PlayersStrengths::get_visible_enemies_power(const uint32_t gametime){
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.players_power;
		}
	}
	return pw;
}

uint32_t PlayersStrengths::get_enemies_average_power(){
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_power;
			count += 1;
		}
	}
	if (count > 0) {
		return sum/count;
	}
	return 0;
}

uint32_t PlayersStrengths::get_enemies_average_land(){
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_land;
			count += 1;
		}
	}
	if (count > 0) {
		return sum/count;
	}
	return 0;
}

uint32_t PlayersStrengths::get_old_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old_players_power;
	};
	return 0;
}
uint32_t PlayersStrengths::get_old_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old_players_land;
	};
	return 0;
}


uint32_t PlayersStrengths::get_old_visible_enemies_power(const uint32_t gametime){
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.old_players_power;
		}
	}
	return pw;
}

// This is casualities of player
uint32_t PlayersStrengths::get_player_casualities(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_casualities;
	};
	return 0;
}

// This is strength of player plus third of strength of other members of his team
uint32_t PlayersStrengths::get_modified_player_power(Widelands::PlayerNumber pn) {
	uint32_t result = 0;
	Widelands::TeamNumber team = 0;
	if (all_stats.count(pn) > 0) {
		result = all_stats[pn].players_power;
		team = all_stats[pn].team_number;
	};
	if (team > 0 && team_powers.count(team) > 0) {
		result = result + (team_powers[team] - result) / 3;
	};
	return result;
}

bool PlayersStrengths::players_in_same_team(Widelands::PlayerNumber pl1,
                                            Widelands::PlayerNumber pl2) {
	if (all_stats.count(pl1) > 0 && all_stats.count(pl2) > 0 && pl1 != pl2) {
		// team number 0 = no team
		return all_stats[pl1].team_number > 0 &&
		       all_stats[pl1].team_number == all_stats[pl2].team_number;
	} else {
		return false;
	}
}

bool PlayersStrengths::strong_enough(Widelands::PlayerNumber pl) {
	if (all_stats.count(pl) == 0) {
		return false;
	}
	uint32_t my_strength = all_stats[pl].players_power;
	uint32_t strongest_opponent_strength = 0;
	for (auto item : all_stats) {
		if (!players_in_same_team(item.first, pl) && pl != item.first) {
			if (get_modified_player_power(item.first) > strongest_opponent_strength) {
				strongest_opponent_strength = get_modified_player_power(item.first);
			}
		}
	}
	return my_strength > strongest_opponent_strength + 50;
}

}  // namespace WIdelands
