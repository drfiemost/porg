//=======================================================================
// main.cc
//-----------------------------------------------------------------------
// This file is part of the package grop
// Copyright (C) 2004-2012 David Ricart
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
	Gtk::Main kit(argc, argv);

	try
	{
		Opt::init();
		DB::init();
		MainWindow window;
		Gtk::Main::run(window);
	}
	catch (Porg::Error const& x)
	{
		run_error_dialog(x.what());
	}
}

