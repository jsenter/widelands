/*
 * Copyright (C) 2003 by the Widelands Development Team
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
// you MUST include ui.h before including this

#ifndef included_ui_tabpanel_h
#define included_ui_tabpanel_h


/*
class TabPanel
--------------
Provides a tab view; every tab is a panel that can contain any number of
sub-panels (such as buttons, other TabPanels, etc..) and an associated
picture.
The picture is displayed as a button the user can click to bring the panel
to the top.

The Panels you add() to the TabPanel must be children of the TabPanel.
*/
class TabPanel : public Panel {
public:
	TabPanel(Panel* parent, int x, int y, uint background);

	void resize();
	
	void set_snapparent(bool snapparent);
	bool get_snapparent() const { return m_snapparent; }
	
	uint add(uint picid, Panel* panel);
	
	void activate(uint idx);
	
private:
	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool handle_mouseclick(uint btn, bool down, int x, int y);
	
	struct Tab {
		uint		picid;
		Panel*	panel;
	};
	
	std::vector<Tab>	m_tabs;
	uint					m_active; // index of the currently active tab
	bool					m_snapparent; // if true, resize parent to fit this panel
	
	int					m_highlight; // index of the highlighted button
	
	uint					m_pic_background; // picture used to draw background
};


#endif // included_ui_tabpanel_h
