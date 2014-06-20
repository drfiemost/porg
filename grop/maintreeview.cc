//=======================================================================
// maintreeview.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "db.h"
#include "opt.h"
#include "maintreeview.h"

using std::string;
using sigc::mem_fun;
using namespace Grop;


MainTreeView::MainTreeView()
:
	Gtk::TreeView(),
	m_columns(),
	m_model(Gtk::ListStore::create(m_columns))
{
	g_assert(DB::initialized());

	set_rules_hint();
	set_vexpand();

	get_selection()->signal_changed().connect(
		mem_fun(this, &MainTreeView::on_selection_changed));

	add_columns();
	set_columns_visibility();

	fill_model();
	set_model(m_model);
}


void MainTreeView::add_columns()
{
	int id;
	Gtk::CellRenderer* cell;

	append_column("Name", m_columns.m_name);

	id = append_column("Size", m_columns.m_size) - 1;
	cell = get_column_cell_renderer(id);
	cell->set_alignment(1, 0.5);
	get_column(id)->set_cell_data_func(*cell, mem_fun(*this, &MainTreeView::size_cell_func));

	id = append_column("Files", m_columns.m_nfiles) - 1;
	cell = get_column_cell_renderer(id);
	cell->set_alignment(1, 0.5);

	id = append_column("Date", m_columns.m_date) - 1;
	cell = get_column_cell_renderer(id);
	cell->set_alignment(1, 0.5);
	get_column(id)->set_cell_data_func(*cell, mem_fun(*this, &MainTreeView::date_cell_func));

	append_column("Summary", m_columns.m_summary);

	for (int i = 0; i < NCOLS; ++i) {
		get_column(i)->set_resizable();
		get_column(i)->set_sort_column(i);
	}
}


void MainTreeView::reset_opts()
{
	set_columns_visibility();
	m_model->foreach_iter(sigc::mem_fun(this, &MainTreeView::on_refresh_date));
}


bool MainTreeView::on_refresh_date(iterator const& i)
{
	m_model->row_changed(m_model->get_path(i), i);
	return false;
}


void MainTreeView::set_columns_visibility()
{
	for (int i = 0; i < NCOLS; ++i)
		get_column(i)->set_visible(Opt::columns()[i]);
}


void MainTreeView::fill_model()
{
	m_model->clear();

	for (DB::const_iter p = DB::pkgs().begin(); p != DB::pkgs().end(); ++p) {
		iterator i = m_model->append();
		(*i)[m_columns.m_pkg] 		= (*p);
		(*i)[m_columns.m_name] 		= (*p)->name();
		(*i)[m_columns.m_size] 		= (*p)->size();
		(*i)[m_columns.m_nfiles] 	= (*p)->nfiles();
		(*i)[m_columns.m_date] 		= (*p)->date();
		(*i)[m_columns.m_summary] 	= (*p)->summary();
	}
}


void MainTreeView::remove_pkg(Pkg const* const pkg)
{
	Gtk::TreeModel::Children children = m_model->children();

	for (iterator it = children.begin(); it != children.end(); ++it) {
		if ((*it)[m_columns.m_pkg] == pkg) {
			m_model->erase(it);
			break;
		}
	}
}


void MainTreeView::size_cell_func(Gtk::CellRenderer* cell, iterator const& it)
{
	// show sizes in "human readable" format
	static_cast<Gtk::CellRendererText*>(cell)
		->property_text() = Porg::fmt_size((*it)[m_columns.m_size]);
}


void MainTreeView::date_cell_func(Gtk::CellRenderer* cell, iterator const& it)
{
	static_cast<Gtk::CellRendererText*>(cell)
		->property_text() = Porg::fmt_date((*it)[m_columns.m_date], Opt::hour());
}


void MainTreeView::on_selection_changed()
{
	iterator it(get_selection()->get_selected());
	
	if (it)
		signal_pkg_selected.emit((*it)[m_columns.m_pkg]);
	else
		signal_pkg_selected.emit(0);
}


bool MainTreeView::on_button_press_event(GdkEventButton* event)
{
	// call base class, to allow normal handler
	bool handled = Gtk::TreeView::on_button_press_event(event);

	// catch only events within the treeview
	if (event->window == get_bin_window()->gobj()) {
	
		// right click
		if (event->button == 3)
			signal_popup_menu.emit(event);
	
		// double left click
		else if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
			signal_2button_press.emit(event);
	}

	return handled;
}


bool MainTreeView::on_key_press_event(GdkEventKey* event)
{
	// call base class, to allow normal handler
	bool handled = Gtk::TreeView::on_key_press_event(event);
	
	signal_key_press.emit(event);

	return handled;
}

