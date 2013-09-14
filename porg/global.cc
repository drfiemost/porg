//=======================================================================
// global.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
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


//
// Like libc's realpath(), but it only resolve symlinks in the partial
// directories of the path, thereby retaining symlinks as symlinks.
//
string Porg::realdir(string const& inpath)
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

	// get real path
	
	p = path.rfind('/');
	string base(path.substr(p + 1)), dir(path.substr(0, p));

	char real[4096];
	if (!realpath(dir.c_str(), real))
		return path;

	return real + string("/") + base;
}


//
// convert a string to lowercase
//
string Porg::to_lower(string const& str)
{
	string low(str);

	for (uint i(0); i < low.size(); ++i)
		low[i] = tolower(low[i]);

	return low;
}

