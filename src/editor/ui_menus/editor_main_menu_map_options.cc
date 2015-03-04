/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_map_options.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "profile/profile.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"


inline EditorInteractive & MainMenuMapOptions::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}


/**
 * Create all the buttons etc...
*/
MainMenuMapOptions::MainMenuMapOptions(EditorInteractive & parent, bool modal)
	:
	UI::Window
		(&parent, "map_options",
		 20, 20, 350, parent.get_inner_h() - 80,
		 _("Map Options")),
	padding_(4),
	butw_((get_inner_w() - 3 * padding_) / 2),
	buth_(20),
	ok_(
		this, "ok",
		padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("pics/but0.png"),
		_("OK")),
	cancel_(
		this, "cancel",
		butw_ + 2 * padding_, get_inner_h() - padding_ - buth_,
		butw_, buth_,
		g_gr->images().get("pics/but1.png"),
		_("Cancel")),
	modal_(modal) {

	int32_t const offsx   =  5;
	int32_t const offsy   =  5;
	int32_t const spacing =  3;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	UI::Textarea * ta = new UI::Textarea(this, posx, posy - 2, _("Map Name:"));
	m_name =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_name->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Size:"));
	m_size =
		new UI::Textarea
			(this, posx + ta->get_w() + spacing, posy - 2, "512x512");
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Nr Players:"));
	m_nrplayers =
		new UI::Textarea(this, posx + ta->get_w() + spacing, posy - 2, "1");
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Authors:"));
	m_author =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_author->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));
	posy += height + spacing;
	m_descr =
		new UI::MultilineEditbox
			(this,
			 posx, posy,
			 get_inner_w() - spacing - posx, get_inner_h() - 4 * (padding_ + buth_) - posy,
			 parent.egbase().map().get_description());
	m_descr->changed.connect(boost::bind(&MainMenuMapOptions::changed, this));

	UI::Button * btn =
		new UI::Button
			(this, "set_origin",
			 padding_, get_inner_h() - 3 * (padding_ + buth_),
			 get_inner_w() - 2 * padding_, buth_,
			 g_gr->images().get("pics/but0.png"),
			 _("Set origin"),
			 _("Set the position that will have the coordinates (0, 0). This will "
				"be the top-left corner of a generated minimap."));
	btn->sigclicked.connect
		(boost::bind
		 (&EditorInteractive::select_tool, &parent,
		  boost::ref(parent.tools.set_origin), EditorTool::First));

	ok_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_ok, boost::ref(*this)));
	ok_.set_enabled(false);
	cancel_.sigclicked.connect
		(boost::bind(&MainMenuMapOptions::clicked_cancel, boost::ref(*this)));
	update();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void MainMenuMapOptions::update() {
	const Widelands::Map & map = eia().egbase().map();

	m_size     ->set_text((boost::format(_("%1$ix%2$i"))
								  % map.get_width()
								  % map.get_height()).str());
	m_author->set_text(map.get_author());
	m_name  ->set_text(map.get_name());
	m_nrplayers->set_text(std::to_string(static_cast<unsigned int>(map.get_nrplayers())));
	m_descr ->set_text(map.get_description());
}


/**
 * Called when one of the editboxes are changed
*/
void MainMenuMapOptions::changed() {
	ok_.set_enabled(true);
}

void MainMenuMapOptions::clicked_ok() {
	eia().egbase().map().set_name(m_name->text());
	eia().egbase().map().set_author(m_author->text());
	g_options.pull_section("global").set_string("realname", m_author->text());
	eia().egbase().map().set_description(m_descr->get_text());
	if (modal_) {
		end_modal(1);
	} else {
		die();
	}
}

void MainMenuMapOptions::clicked_cancel() {
	if (modal_) {
		end_modal(0);
	} else {
		die();
	}
}

