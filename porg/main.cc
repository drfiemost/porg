//=======================================================================
// main.cc - The main source file.
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "global.h"
#include "opt.h"
#include "log.h"
#include "pkgset.h"
#include <stdexcept>

using namespace Porg;
using namespace std;


int main(int argc, char* argv[])
{
	ios::sync_with_stdio(false);

	try 
	{
		// read config file and get options from command line
		Opt::init(argc, argv);

		if (Opt::mode() == LOG) {
			Log log;
			exit(g_exit_status);
		}

		PkgSet pset;

		if (Opt::mode() == QUERY || Opt::all_pkgs())
			pset.get_all_pkgs();
		else
			pset.get_pkgs(Opt::args());

		if (pset.empty())
			exit(EXIT_FAILURE);

		switch (Opt::mode()) {
			case UNLOG:			pset.unlog();			break;
			case CONF_OPTS:		pset.print_conf_opts();	break;
			case INFO:			pset.print_info();		break;
			case LIST_PKGS:		pset.list();			break;
			case LIST_FILES:	pset.list_files();		break;
			case QUERY:			pset.query();			break;
			case REMOVE:		pset.remove();			break;
			default: 			assert(0);
		}
	}

	catch (Error const& x) 
	{
		cerr << "porg: " << x.what() << '\n';
		g_exit_status = EXIT_FAILURE;
	}

	exit(g_exit_status);
}

