//=======================================================================
// find.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "find.h"
#include "pkg.h"
#include "db.h"
#include <gtkmm/grid.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>
#include <glibmm/miscutils.h>	// Glib::get_home_dir()

using sigc::mem_fun;
using namespace Grop;

Find* Find::s_find = 0;


Find::Find(Gtk::Window& parent)
:
	Gtk::Dialog("grop :: find", parent, true),
	m_entry(),
	m_treeview()
{
	set_border_width(8);
	set_default_size(200, 200);
	
	Gtk::Button* button_browse = Gtk::manage(new Gtk::Button("_Browse", true));
	button_browse->signal_clicked().connect(mem_fun(*this, &Find::browse));

	Gtk::Grid* grid = Gtk::manage(new Gtk::Grid());
	grid->set_column_spacing(get_border_width());
	grid->attach(m_entry, 0, 0, 1, 1);
	grid->attach(*button_browse, 1, 0, 1, 1);

	Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window->add(m_treeview);

	Gtk::Box* box = get_content_area();
	box->set_spacing(8);
	box->pack_start(*grid, Gtk::PACK_SHRINK);
	box->pack_start(*scrolled_window, Gtk::PACK_EXPAND_WIDGET);

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	add_button(Gtk::Stock::FIND, Gtk::RESPONSE_APPLY);

	set_default_response(Gtk::RESPONSE_APPLY);
	m_entry.set_activates_default();
	m_entry.set_hexpand();

	get_action_area()->set_layout(Gtk::BUTTONBOX_EDGE);

	show_all();
}


void Find::instance(Gtk::Window& parent)
{
	if (!s_find)
		s_find = new Find(parent);

	s_find->m_entry.set_text("");
	s_find->reset_treeview();
	s_find->run();
}


void Find::on_response(int id)
{
	if (id == Gtk::RESPONSE_APPLY)
		find();
	else
		hide();
}


Gtk::TreeModel::iterator Find::reset_treeview()
{
	m_treeview.m_model->clear();
	return m_treeview.m_model->append();
}


void Find::find()
{
	Gtk::TreeModel::iterator it = reset_treeview();
	Glib::ustring path(m_entry.get_text());

	if (path.empty())
		return;
	
	(*it)[m_treeview.m_columns.m_name] = "(file not found)";

	if (path[0] != '/')
		return;
	
	int cnt = 0;

	for (DB::pkg_it p = DB::pkgs().begin(); p != DB::pkgs().end(); ++p) {
		if ((*p)->has_file(path)) {
			if (cnt++ > 0)
				it = m_treeview.m_model->append();
			// XXX show package files on double click:
			// (*it)[m_treeview.m_columns.m_pkg] = *p;
			(*it)[m_treeview.m_columns.m_name] = (*p)->name();
		}
	}
}


void Find::browse()
{
	Gtk::FileChooserDialog dialog(*this, "Select file");

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	
	dialog.set_show_hidden();
	dialog.set_default_size(450, 350);

	std::string path(m_entry.get_text());

	dialog.set_filename(path[0] == '/' ? path : Glib::get_home_dir());
	
	if (dialog.run() == Gtk::RESPONSE_OK) {
		m_entry.set_text(dialog.get_filename());
		m_entry.set_position(-1);
	}
}


//--------------------//
// Find::PkgsTreeView //
//--------------------//


Find::PkgsTreeView::PkgsTreeView()
:
	m_columns(),
	m_model(Gtk::ListStore::create(m_columns))
{
	set_model(m_model);
	set_headers_visible(false);
	append_column("", m_columns.m_name);
}

