//=======================================================================
// main.cc - The main source file.
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "log.h"
#include "db.h"
#include "main.h"

using namespace Porg;
using namespace std;

// Initialization of global vars
namespace Porg
{
	int g_exit_status = EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
	ios::sync_with_stdio(false);

	try 
	{
		Opt::init(argc, argv);

		if (Opt::mode() == MODE_LOG) {
			Log::run();
			exit(g_exit_status);
		}

		DB db;

		if (Opt::mode() == MODE_QUERY || Opt::all_pkgs())
			db.get_all_pkgs();
		else
			db.get_pkgs(Opt::args());

		if (db.empty())
			exit(g_exit_status);

		switch (Opt::mode()) {
			case MODE_CONF_OPTS:	db.print_conf_opts();	break;
			case MODE_INFO:			db.print_info();		break;
			case MODE_LIST_PKGS:	db.list();				break;
			case MODE_LIST_FILES:	db.list_files();		break;
			case MODE_REMOVE:		db.remove();			break;
			case MODE_QUERY:		db.query();				break;
			default: 				assert(0);
		}
	}

	catch (exception const& x) 
	{
		cerr << "porg: " << x.what() << '\n';
		g_exit_status = EXIT_FAILURE;
	}

	exit(g_exit_status);
}

