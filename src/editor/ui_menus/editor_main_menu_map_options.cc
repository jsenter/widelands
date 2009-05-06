/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "editor_main_menu_map_options.h"

#include "editor/editorinteractive.h"
#include "i18n.h"
#include "map.h"
#include "profile/profile.h"

#include "ui/ui_basic/ui_textarea.h"
#include "ui/ui_basic/ui_multilinetextarea.h"
#include "ui/ui_basic/ui_multilineeditbox.h"
#include "ui/ui_basic/ui_editbox.h"

#include <cstdio>

inline Editor_Interactive & Main_Menu_Map_Options::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Main_Menu_Map_Options::Enable_Set_Origin_Tool_Button::
Enable_Set_Origin_Tool_Button(Main_Menu_Map_Options & parent)
	:
	UI::Button
		(&parent,
		 5, parent.get_inner_h() - 25, parent.get_inner_w() - 10, 20,
		 0,
		 _("Set origin"),
		 _
		 	("Set the position that will have the coordinates (0, 0). This will "
		 	 "be the top-left corner of a generated minimap."))
{}


/**
 * Create all the buttons etc...
*/
Main_Menu_Map_Options::Main_Menu_Map_Options(Editor_Interactive & parent)
	:
	UI::Window
		(&parent,
		 (parent.get_w() - 200) / 2, (parent.get_h() - 300) / 2, 200, 305,
		 _("Map Options")),
	m_enable_set_origin_tool(*this)
{

	int32_t const offsx   =  5;
	int32_t const offsy   =  5;
	int32_t const spacing =  3;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	UI::Textarea * ta =
		new UI::Textarea(this, posx, posy - 2, _("Map Name:"), Align_Left);
	m_name =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20,
			 1, 0);
	m_name->changedid.set(this, &Main_Menu_Map_Options::changed);
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Size:"));
	m_size =
		new UI::Textarea
			(this, posx + ta->get_w() + spacing, posy - 2, "512x512", Align_Left);
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Nr Players:"));
	m_nrplayers =
		new UI::Textarea
			(this, posx + ta->get_w() + spacing, posy - 2, "1", Align_Left);
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("World:"));
	m_world =
		new UI::Textarea
			(this,
			 posx + ta->get_w() + spacing, posy - 2,
			 "\"Greenland\"", Align_Left);
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Author:"), Align_Left);
	m_author =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20, 1, 1);
	m_author->changedid.set(this, &Main_Menu_Map_Options::changed);
	posy += height + spacing;
	m_descr = new UI::Multiline_Editbox
		(this,
		 posx, posy,
		 get_inner_w() - spacing - posx, get_inner_h() - 25 - spacing - posy,
		 _("Nothing defined!"));
	m_descr->changed.set(this, &Main_Menu_Map_Options::editbox_changed);
	update();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void Main_Menu_Map_Options::update() {
	Widelands::Map const & map = eia().egbase().map();

	char buf[200];
	sprintf(buf, "%ix%i", map.get_width(), map.get_height());
	m_size->set_text(buf);
	m_author->setText(map.get_author());
	m_name  ->setText(map.get_name());
	sprintf(buf, "%i", map.get_nrplayers());
	m_nrplayers->set_text(buf);
	m_world ->set_text(map.world().get_name());
	m_descr ->set_text(map.get_description());
}


/**
 * Called when one of the editboxes are changed
*/
void Main_Menu_Map_Options::changed(int32_t const id) {
	if        (id == 0) {
		eia().egbase().map().set_name(m_name->text().c_str());
	} else if (id == 1) {
		eia().egbase().map().set_author(m_author->text().c_str());
		g_options.pull_section("global").set_string
			("realname", m_author->text());
	}
	update();
}

/**
 * Called when the editbox has changed
 */
void Main_Menu_Map_Options::editbox_changed() {
	eia().egbase().map().set_description(m_descr->get_text().c_str());
}


void Main_Menu_Map_Options::Enable_Set_Origin_Tool_Button::clicked() const {
	Editor_Interactive & eia =
		dynamic_cast<Main_Menu_Map_Options &>(*get_parent()).eia();
	eia.select_tool(eia.tools.set_origin, Editor_Tool::First);
}
