/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "game_io/game_interactive_player_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "wui/interactive_player.h"
#include "wui/mapview.h"

namespace Widelands {

namespace {

constexpr uint16_t kCurrentPacketVersion = 4;

void load_landmarks_pre_zoom(FileRead* fr, InteractiveBase* ibase) {
	size_t no_of_landmarks = fr->unsigned_8();
	for (size_t i = 0; i < no_of_landmarks; ++i) {
		uint8_t set = fr->unsigned_8();
		MapView::View view = {Vector2f(fr->signed_32(), fr->signed_32()), 1.f};
		if (set > 0) {
			ibase->set_landmark(i, view);
		}
	}
}

}  // namespace

void GameInteractivePlayerPacket::read(FileSystem& fs, Game& game, MapObjectLoader*) {
	try {
		FileRead fr;
		fr.open(fs, "binary/interactive_player");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 2 && packet_version <= kCurrentPacketVersion) {
			PlayerNumber player_number = fr.unsigned_8();
			if (!(0 < player_number && player_number <= game.map().get_nrplayers())) {
				throw GameDataError("Invalid player number: %i.", player_number);
			}

			if (!game.get_player(player_number)) {
				// This happens if the player, that saved the game, was a spectator
				// and the slot for player 1 was not used in the game.
				// So now we try to create an InteractivePlayer object for another
				// player instead.
				const PlayerNumber max = game.map().get_nrplayers();
				for (player_number = 1; player_number <= max; ++player_number)
					if (game.get_player(player_number))
						break;
				if (player_number > max)
					throw GameDataError("The game has no players!");
			}
			Vector2f center_map_pixel = Vector2f::zero();
			if (packet_version <= 3) {
				center_map_pixel.x = fr.unsigned_16();
				center_map_pixel.y = fr.unsigned_16();
			} else {
				center_map_pixel.x = fr.float_32();
				center_map_pixel.y = fr.float_32();
			}

			uint32_t const display_flags = fr.unsigned_32();

			if (InteractiveBase* const ibase = game.get_ibase()) {
				ibase->map_view()->scroll_to_map_pixel(center_map_pixel, MapView::Transition::Jump);

				uint32_t const loaded_df =
				   InteractiveBase::dfShowCensus | InteractiveBase::dfShowStatistics;
				uint32_t const olddf = ibase->get_display_flags();
				uint32_t const realdf = (olddf & ~loaded_df) | (display_flags & loaded_df);
				ibase->set_display_flags(realdf);
			}
			if (InteractivePlayer* const ipl = game.get_ipl()) {
				ipl->set_player_number(player_number);
			}

			// Map landmarks
			if (InteractiveBase* const ibase = game.get_ibase()) {
				if (packet_version == 3) {
					load_landmarks_pre_zoom(&fr, ibase);
				} else if (packet_version >= 4) {
					size_t no_of_landmarks = fr.unsigned_8();
					for (size_t i = 0; i < no_of_landmarks; ++i) {
						uint8_t set = fr.unsigned_8();
						const float x = fr.float_32();
						const float y = fr.float_32();
						const float zoom = fr.float_32();
						MapView::View view = {Vector2f(x, y), zoom};
						if (set > 0) {
							ibase->set_landmark(i, view);
						}
					}
				}
			}
		} else {
			throw UnhandledVersionError(
			   "GameInteractivePlayerPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("interactive player: %s", e.what());
	}
}

/*
 * Write Function
 */
void GameInteractivePlayerPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	InteractiveBase* const ibase = game.get_ibase();
	InteractivePlayer* const iplayer = game.get_ipl();

	// Player number
	fw.unsigned_8(iplayer ? iplayer->player_number() : 1);

	if (ibase != nullptr) {
		const Vector2f center = ibase->map_view()->view_area().rect().center();
		fw.float_32(center.x);
		fw.float_32(center.y);
	} else {
		fw.float_32(0.f);
		fw.float_32(0.f);
	}

	// Display flags
	fw.unsigned_32(ibase ? ibase->get_display_flags() : 0);

	// Map landmarks
	const std::vector<QuickNavigation::Landmark>& landmarks = ibase->landmarks();
	fw.unsigned_8(landmarks.size());
	for (const QuickNavigation::Landmark& landmark : landmarks) {
		fw.unsigned_8(landmark.set ? 1 : 0);
		fw.float_32(landmark.view.viewpoint.x);
		fw.float_32(landmark.view.viewpoint.y);
		fw.float_32(landmark.view.zoom);
	}

	fw.write(fs, "binary/interactive_player");
}
}
