/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "ware_statistics_menu.h"
#include "ware_statistics_common.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "logic/player.h"
#include "graphic/rendertarget.h"
#include "logic/tribe.h"
#include "logic/warelist.h"
#include "plot_area.h"
#include "differential_plot_area.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/wsm_checkbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/slider.h"


#define MIN_WARES_PER_LINE 7
#define MAX_WARES_PER_LINE 11

#define PLOT_HEIGHT 100

//TODO place holder, need to be changed
static const char pic_tab_production[] = "pics/menu_tab_wares.png";
static const char pic_tab_economy[] = "pics/menu_tab_wares.png";

Ware_Statistics_Menu::Ware_Statistics_Menu
	(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&parent, "ware_statistics", &registry, 400, 270, _("Ware Statistics")),
m_parent(&parent)
{
	set_cache(false);

	//  First, we must decide about the size.
	uint8_t const nr_wares = parent.get_player()->tribe().get_nrwares().value();
	uint32_t wares_per_row = MIN_WARES_PER_LINE;
	while (nr_wares % wares_per_row && wares_per_row <= MAX_WARES_PER_LINE)
		++wares_per_row;
	const uint32_t nr_rows =
		nr_wares / wares_per_row + (nr_wares % wares_per_row ? 1 : 0);

#define spacing 5
	Point const offs(spacing, 30);
	Point       pos (offs);

	set_inner_size
		(10,
		 (offs.y + spacing + PLOT_HEIGHT + spacing +
		  nr_rows * (WARE_MENU_PIC_HEIGHT + spacing) + 100));


	//create a tabbed environment for the different plots
	UI::Tab_Panel * tabs =
		 new UI::Tab_Panel
			 (this, spacing, 0, g_gr->get_picture(PicMod_UI, "pics/but1.png"));


	m_plot_production =
		new WUIPlot_Area
			(tabs,
			 0, 0, 0, 0);
	m_plot_production->set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot_production->set_plotmode(WUIPlot_Area::PLOTMODE_RELATIVE);

	tabs->add
		("production", g_gr->get_picture(PicMod_UI, pic_tab_production),
			m_plot_production, _("Production"));

	m_plot_consumption =
		new DifferentialPlot_Area(tabs, 0, 0, 0, 0);
	m_plot_consumption->set_sample_rate(STATISTICS_SAMPLE_TIME);
	m_plot_consumption->set_plotmode(WUIPlot_Area::PLOTMODE_RELATIVE);

	tabs->add
		("consumption", g_gr->get_picture(PicMod_UI, pic_tab_production),
			m_plot_consumption, _("Consumption"));

	//add buttons for all wares below the tabbed environment
	//and register the statistic data
	Widelands::Ware_Index::value_t cur_ware = 0;
	int32_t dposy    = 0;
	pos.y += PLOT_HEIGHT + 2 * spacing;
	Widelands::Tribe_Descr const & tribe = parent.get_player()->tribe();
	for (uint32_t y = 0; y < nr_rows; ++y) {
		pos.x = spacing;
		for
			(uint32_t x = 0;
			 x < wares_per_row and cur_ware < nr_wares;
			 ++x, ++cur_ware)
		{
			Widelands::Item_Ware_Descr const & ware =
				*tribe.get_ware_descr(Widelands::Ware_Index(cur_ware));
			WSM_Checkbox & cb =
				*new WSM_Checkbox
					(this, pos, cur_ware, ware.icon(), colors[cur_ware]);
			cb.set_tooltip(ware.descname().c_str());
			cb.changedtoid.set(this, &Ware_Statistics_Menu::cb_changed_to);
			pos.x += cb.get_w() + spacing;
			dposy = cb.get_h() + spacing;
			set_inner_size
				(spacing + (cb.get_w() + spacing) * wares_per_row, get_inner_h());

			//register data
			m_plot_production->register_plot_data
				(cur_ware,
				 parent.get_player()->get_ware_production_statistics
				 	(Widelands::Ware_Index(cur_ware)),
				 colors[cur_ware]);

			m_plot_consumption->register_plot_data
				(cur_ware,
				 parent.get_player()->get_ware_production_statistics
				 	(Widelands::Ware_Index(cur_ware)),
				 colors[cur_ware]);

			m_plot_consumption->register_negative_plot_data
				(cur_ware,
				 parent.get_player()->get_ware_consumption_statistics
				 	(Widelands::Ware_Index(cur_ware)));
		}
		pos.y += dposy;
	}

	//set height of Tab_Panel to height of the plot + height of the tabs
	tabs->set_size(get_inner_w() - 2 * spacing, PLOT_HEIGHT + offs.y + spacing);
	tabs->activate(0);

	pos.x  = spacing;
	pos.y += spacing + spacing;

	new WUIPlot_Area_Slider
		(this, *m_plot_production,
		 pos.x, pos.y, get_inner_w() - 2 * spacing, 45,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"));

	pos.y += 45 + spacing;

	set_inner_size(get_inner_w(), pos.y);
}

/**
 * Callback for the ware buttons. Change the state of all ware statistics
 * simultaneously.
 */
void Ware_Statistics_Menu::cb_changed_to(int32_t const id, bool const what) {
	m_plot_production->show_plot(id, what);
	m_plot_consumption->show_plot(id, what);
}

/**
 * Callback for the time buttons. Change the time axis of all ware
 * statistics simultaneously.
 */
void Ware_Statistics_Menu::set_time(WUIPlot_Area::TIME timescale) {
	m_plot_production->set_time(timescale);
	m_plot_consumption->set_time(timescale);
}
