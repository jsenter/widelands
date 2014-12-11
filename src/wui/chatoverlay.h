/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef WL_WUI_CHATOVERLAY_H
#define WL_WUI_CHATOVERLAY_H

#include <memory>

#include "ui_basic/panel.h"

struct ChatProvider;

/**
 * The overlay that displays all new chat messages for some timeout on the main window.
 *
 * \see GameChatPanel, GameChatMenu
 */
struct ChatOverlay : public UI::Panel {
	ChatOverlay(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);
	~ChatOverlay();

	void set_chat_provider(ChatProvider &);
	void draw(RenderTarget &) override;
	void think() override;

	// Check is position and size is still correct.
	void recompute();

private:
	struct Impl;
	std::unique_ptr<Impl> m;
};

#endif  // end of include guard: WL_WUI_CHATOVERLAY_H
