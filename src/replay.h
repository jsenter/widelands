/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef __S__REPLAY_H
#define __S__REPLAY_H

/**
 * Allow players to watch previous game in a platform-independent way.
 * Also useful as a debugging aid.
 *
 * A game replay consists of a savegame plus a log-file of subsequent
 * playercommands.
 */

#include <string>

#define REPLAY_DIR "replays/"
#define REPLAY_SUFFIX ".wrpl"

class WidelandsStreamRead;
class WidelandsStreamWrite;


/**
 * Read game replays from disk.
 */
class ReplayReader {
public:
	ReplayReader(Game* game, const std::string filename);
	~ReplayReader();

	PlayerCommand* GetPlayerCommand(uint time);
	bool EndOfReplay();

private:
	Game* m_game;
	WidelandsStreamRead* m_cmdlog;

	uint m_replaytime;
};

/**
 * Write game replays to disk.
 */
class ReplayWriter {
public:
	ReplayWriter(Game* game, const std::string filename);
	~ReplayWriter();

	void SendPlayerCommand(PlayerCommand* cmd);

private:
	Game* m_game;
	WidelandsStreamWrite* m_cmdlog;
};



#endif // __S__REPLAYWRITER_H

