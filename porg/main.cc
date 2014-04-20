//=======================================================================
// main.cc - The main source file.
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "log.h"
#include "pkgset.h"
#include <stdexcept>

using namespace Porg;
using namespace std;


int main(int argc, char* argv[])
{
	int exit_status = EXIT_SUCCESS;

	ios::sync_with_stdio(false);

	try 
	{
		// read config file and get options from command line
		Opt::init(argc, argv);

		if (Opt::mode() == MODE_LOG) {
			Log log;
			exit(EXIT_SUCCESS);
		}

		PkgSet pset;

		if (Opt::mode() == MODE_QUERY || Opt::all_pkgs())
			pset.get_all_pkgs();
		else
			exit_status = pset.get_pkgs(Opt::args());

		if (pset.empty())
			exit(exit_status);

		switch (Opt::mode()) {
			case MODE_CONF_OPTS:	pset.print_conf_opts();		break;
			case MODE_INFO:			pset.print_info();			break;
			case MODE_LIST_PKGS:	pset.list();				break;
			case MODE_LIST_FILES:	pset.list_files();			break;
			case MODE_REMOVE:		pset.remove();				break;
			case MODE_QUERY:		exit_status = pset.query();	break;
			default: 				assert(0);
		}
	}

	catch (Error const& x) 
	{
		cerr << "porg: " << x.what() << '\n';
		exit(EXIT_FAILURE);
	}

	exit(exit_status);
}

