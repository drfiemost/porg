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
#include <gtkmm/main.h>


int main(int argc, char* argv[])
{
	Grop::Opt::init();
	Gtk::Main kit(argc, argv, Grop::Opt::context());

	try
	{
		Grop::DB::init();
		Grop::MainWindow window;
		kit.run(window);
	}
	catch (std::exception const& x)
	{
		Grop::run_error_dialog(x.what());
	}
	catch (Glib::Exception const& x)
	{
		Grop::run_error_dialog(x.what());
	}
}

