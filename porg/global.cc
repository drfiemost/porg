//=======================================================================
// global.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "global.h"
#include <string>

using std::string;

// initialization of globals
namespace Porg
{
	int g_exit_status = EXIT_SUCCESS;
}


string Porg::clear_path(string const& inpath)
{
	string path(inpath);
	
	// strip trailing and consecutive slashes
    
	string::size_type p;
    while ((p = path.find("//")) != string::npos)
        path.erase(p, 1);

    if ((p = path.find_last_not_of("/")) != string::npos)
        path.erase(++p);
	
	// absolutize path
	
	if (path[0] != '/') {
		char cwd[4096];
		if (getcwd(cwd, sizeof(cwd)))
			path.insert(0, string(cwd) + '/');
	}

	return path;
}

