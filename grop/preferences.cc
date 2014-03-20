//=======================================================================
// preferences.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "preferences.h"
#include <gtkmm/frame.h>
#include <gtkmm/stock.h>	// Gtk::RESPONSE_OK
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/grid.h>

using std::string;
using namespace Grop;


Preferences* Preferences::s_prefs = 0;


Preferences::Preferences(Gtk::Window& parent)
:
	Gtk::Dialog("grop :: preferences", parent, true),
	m_buttons(),
	m_button_hour("Show _hour in date", true)
{
	set_border_width(4);
	set_resizable(false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	Gtk::Grid* grid = Gtk::manage(new Gtk::Grid());
	
	string col_names[] = { /*"_Icon",*/ "_Name", "_Size", "_Files", "_Date", "Su_mmary" };

	for (int i = 0; i < MainTreeView::NCOLS; ++i) {
		m_buttons[i].set_label(col_names[i]);
		m_buttons[i].set_use_underline();
		grid->attach(m_buttons[i], 0, i, 1, 1);
	}

	Gtk::Frame* frame = Gtk::manage(new Gtk::Frame(" Visible columns "));
	frame->set_border_width(4);
	frame->add(*grid);

	Gtk::Box* box = get_content_area();
	box->set_spacing(4);
	box->pack_start(*frame);
	box->pack_start(m_button_hour);
	box->pack_start(*(Gtk::manage(new Gtk::Separator())));

	show_all();
}


void Preferences::load_opts()
{
	for (int i = 0; i < MainTreeView::NCOLS; ++i)
		m_buttons[i].set_active(Opt::columns()[i]);

	m_button_hour.set_active(Opt::hour());
}


void Preferences::save_opts()
{
	for (int i = 0; i < MainTreeView::NCOLS; ++i)
		Opt::columns()[i] = m_buttons[i].get_active();

	Opt::hour() = m_button_hour.get_active();
}


int Preferences::instance(Gtk::Window& parent)
{
	if (!s_prefs)
		s_prefs = new Preferences(parent);

	s_prefs->load_opts();
	
	int response = s_prefs->run();
	s_prefs->hide();

	if (response == Gtk::RESPONSE_OK)
		s_prefs->save_opts();

	return response;
}


