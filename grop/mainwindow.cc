//=======================================================================
// mainwindow.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "db.h"
#include "pkg.h"
#include "porg/file.h"
#include "mainwindow.h"
#include "properties.h"
#include "preferences.h"
#include "porgball.h"
#include "removepkg.h"
#include "util.h"	// run_*_dialog()
#include "find.h"
#include <gtkmm/grid.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/scrolledwindow.h>
#include <sstream>


using std::string;
using Gtk::Action;
using namespace Grop;
using namespace sigc;


MainWindow::MainWindow()
:
	Gtk::Window(),
	m_treeview(),
	m_statusbar(),
	m_uimanager(Gtk::UIManager::create()),
	m_action_group(Gtk::ActionGroup::create()),
	m_action_find(Action::create("FindFile", Gtk::Stock::FIND)),
	m_action_properties(Action::create("Properties", Gtk::Stock::PROPERTIES)),
	m_action_porgball(Action::create("Porgball", "Create porgball")),
	m_action_remove(Action::create("RemovePkg", Gtk::Stock::REMOVE)),
	m_action_unlog(Action::create("UnlogPkg", "Unlog")),
	m_popup_menu(0),
	m_selected_pkg(0)
{
	// Opt should have been initialized before
	g_assert(Opt::initialized());
	set_default_size(Opt::width(), Opt::height());
	move(Opt::xpos(), Opt::ypos());

	set_border_width(4);

	try 
	{ 
		set_icon_from_file(DATADIR "/pixmaps/grop.png"); 
	}
	catch (Glib::Error& x) 
	{ 
		g_warning("%s", x.what().c_str()); 
	}

	build_menu_bar();
	set_actions_sensitivity();

	m_treeview.signal_popup_menu.connect(mem_fun(this, &MainWindow::on_popup_menu));
	m_treeview.signal_2button_press.connect(mem_fun(this, &MainWindow::on_2button_press));
	m_treeview.signal_key_press.connect(mem_fun(this, &MainWindow::on_key_press));
	m_treeview.signal_pkg_selected.connect(mem_fun(this, &MainWindow::on_pkg_selected));

	m_statusbar.set_vexpand(false);
	update_statusbar();

	Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window->add(m_treeview);

	Gtk::Grid* grid = Gtk::manage(new Gtk::Grid());
	grid->attach(*(m_uimanager->get_widget("/MenuBar")), 0, 0, 1, 1);
	grid->attach(*scrolled_window, 0, 1, 1, 1);
	grid->attach(m_statusbar, 0, 2, 1, 1);
	add(*grid);

	show_all();
}


MainWindow::~MainWindow()
{
	// save window geometry
	int w, h, x, y;
	get_size(w, h);
	get_position(x, y);	//XXX does not work
	Opt::set_whxy(w, h, x, y);
}


void MainWindow::update_statusbar()
{
	std::ostringstream os;
	os 	<< "  " << DB::pkg_cnt() << " packages | " 
		<< Porg::fmt_size(DB::total_size(), Porg::HUMAN_READABLE);
	m_statusbar.push(os.str());
}


void MainWindow::build_menu_bar()
{
	m_action_group->add(Action::create("MenuFile", "_File"));
	m_action_group->add(Action::create("Quit", Gtk::Stock::QUIT), 
		mem_fun(this, &MainWindow::hide));

	m_action_group->add(Action::create("MenuEdit", "_Edit"));
	m_action_group->add(m_action_find, mem_fun(this, &MainWindow::on_find_file));
	m_action_group->add(Action::create("Preferences", Gtk::Stock::PREFERENCES), 
		mem_fun(this, &MainWindow::on_preferences));

	m_action_group->add(Action::create("MenuPackage", "_Package"));
	m_action_group->add(m_action_properties, mem_fun(this, &MainWindow::on_properties));
	m_action_group->add(m_action_porgball, mem_fun(this, &MainWindow::on_porgball));
	m_action_group->add(m_action_remove, mem_fun(this, &MainWindow::on_remove));
	m_action_group->add(m_action_unlog, mem_fun(*this, &MainWindow::on_unlog));

	m_action_group->add(Action::create("MenuHelp", "_Help"));
	m_action_group->add(Action::create("About", Gtk::Stock::ABOUT), 
		mem_fun(this, &MainWindow::on_about));

	m_uimanager->insert_action_group(m_action_group);
	add_accel_group(m_uimanager->get_accel_group());
	
	m_uimanager->add_ui_from_string(
		"<ui>"
		"	<menubar name='MenuBar'>"
		"		<menu action='MenuFile'>"
		"			<menuitem action='Quit'/>"
		"		</menu>"
		"		<menu action='MenuEdit'>"
		"			<menuitem action='FindFile'/>"
		"			<menuitem action='Preferences'/>"
		"		</menu>"
		"		<menu action='MenuPackage'>"
		"			<menuitem action='Properties'/>"
		"			<menuitem action='Porgball'/>"
		"			<separator/>"
		"			<menuitem action='RemovePkg'/>"
		"			<menuitem action='UnlogPkg'/>"
		"		</menu>"
		"		<menu action='MenuHelp'>"
		"			<menuitem action='About'/>"
		"		</menu>"
		"	</menubar>"
		"	<popup name='PopupMenu'>"
		"		<menuitem action='Properties'/>"
		"		<menuitem action='Porgball'/>"
		"		<separator/>"
		"		<menuitem action='RemovePkg'/>"
		"		<menuitem action='UnlogPkg'/>"
		"	</popup>"
		"</ui>");

	m_popup_menu = dynamic_cast<Gtk::Menu*>(m_uimanager->get_widget("/PopupMenu"));
	g_assert(m_popup_menu != 0);
}


void MainWindow::on_popup_menu(GdkEventButton* event)
{
	if (m_selected_pkg && m_popup_menu)
		m_popup_menu->popup(event->button, event->time);
}


void MainWindow::on_2button_press(GdkEventButton*)
{
	if (m_selected_pkg)
		on_properties();
}


void MainWindow::on_key_press(GdkEventKey* event)
{
	if (!m_selected_pkg)
		return;

	switch (event->keyval) {
		case GDK_KEY_Delete:
		case GDK_KEY_BackSpace:
			on_unlog();
			break;
		case GDK_KEY_Return:
			on_properties();
			break;
		case GDK_KEY_Menu:
			if (m_popup_menu)
				m_popup_menu->popup(0, event->time);
	}
}


void MainWindow::on_pkg_selected(Pkg* pkg)
{
	m_selected_pkg = pkg;
	set_actions_sensitivity();
}


void MainWindow::set_actions_sensitivity()
{
	// set allowed actions

	m_action_find->set_sensitive(DB::pkg_cnt() > 0);
	m_action_properties->set_sensitive(m_selected_pkg);
	m_action_porgball->set_sensitive(m_selected_pkg);
	m_action_remove->set_sensitive(m_selected_pkg && Opt::logdir_writable());
	m_action_unlog->set_sensitive(m_selected_pkg && Opt::logdir_writable());
}


void MainWindow::on_about()
{
	Gtk::AboutDialog dialog;

	dialog.set_transient_for(*this);
	dialog.set_name("grop");
	dialog.set_logo_icon_name("grop");
	dialog.set_version(PACKAGE_VERSION);
	dialog.set_comments("Graphic interface of porg,\nthe source code PACkage Organizer");
	dialog.set_authors(std::vector<Glib::ustring>(1, "David Ricart " PACKAGE_BUGREPORT));
	dialog.set_copyright("Copyright (C) 2004-2012 David Ricart");
	dialog.set_website("http://porg.sourceforge.net");

	dialog.run();
}


void MainWindow::on_preferences()
{
	if (Preferences::instance(*this) == Gtk::RESPONSE_OK)
		m_treeview.set_opts();
}


void MainWindow::on_find_file()
{
	Find::instance(*this);
}


void MainWindow::on_properties()
{
	if (m_selected_pkg)
		Properties::instance(*m_selected_pkg, *this);
}


void MainWindow::on_porgball()
{
	if (m_selected_pkg)
		Porgball::instance(*m_selected_pkg, *this);
}


void MainWindow::on_unlog()
{
	if (!(Opt::logdir_writable() && m_selected_pkg))
		return;

	if (run_question_dialog("Remove package '" + m_selected_pkg->name() + "' from database ?", this))
		unlog_pkg(m_selected_pkg);
}


void MainWindow::on_remove()
{
	if (!(Opt::logdir_writable() && m_selected_pkg))
		return;

	if (!run_question_dialog("Remove package '" + m_selected_pkg->name() + "' ?", this))
		return;

	if (RemovePkg::instance(*m_selected_pkg, *this))
		unlog_pkg(m_selected_pkg);
}


void MainWindow::unlog_pkg(Pkg* pkg)
{
	g_assert(pkg != 0);

	try
	{
		DB::remove_pkg(pkg);
		m_treeview.remove_pkg(pkg);
		update_statusbar();
	}
	catch (Porg::Error const& x)
	{
		run_error_dialog(x.what(), this);
	}
}

