//=======================================================================
// main.cc
//-----------------------------------------------------------------------
// This file is part of the package grop
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "db.h"
#include "mainwindow.h"
#include "util.h"
#include "porg/common.h"	// Porg::Error
#include <gtkmm/main.h>

using namespace Grop;


int main(int argc, char* argv[])
{
	// Read configuration file
	Opt::init();

	Glib::OptionContext opt_context;
	Glib::OptionGroup opt_group("grop", "Grop Options:");

	Glib::OptionEntry opt_logdir;
	opt_logdir.set_long_name("logdir");
	opt_logdir.set_short_name('L');
	opt_logdir.set_description("Porg database directory (default is '"
		+ Opt::logdir() + "')");

	std::string logdir;
	opt_group.add_entry_filename(opt_logdir, logdir);
	opt_context.set_main_group(opt_group);

	Gtk::Main kit(argc, argv, opt_context);

	try
	{
		// Open porg database
		DB::init();
		// Run GUI
		MainWindow window;
		Gtk::Main::run(window);
	}
	catch (Porg::Error const& x)
	{
		run_error_dialog(x.what());
	}
}

