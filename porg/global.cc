//=======================================================================
// global.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "global.h"
#include <string>

using std::string;


//
// Like libc's realpath(), but it only resolve symlinks in the partial
// directories of the path, thereby retaining symlinks as symlinks.
//
string Porg::clear_path(string const& inpath)
{
	assert(!inpath.empty());

	string path(inpath);

	// absolutize path

	if (path[0] != '/') {
		char cwd[4096];
		if (getcwd(cwd, sizeof(cwd)))
			path.insert(0, string(cwd) + "/");
	}
	// strip trailing slashes
    
	while (path[path.size() - 1] == '/')
		path.erase(path.size() - 1);

	// separate dirname from basename

	string::size_type p = path.rfind('/');
	string base((p == string::npos) ? "" : path.substr(p + 1));
	string dir(path.substr(0, p));

	// get realpath of dirname

	char real[4096];

	if (!::realpath(dir.c_str(), real))
		return path;

	return string(real) + "/" + base;
}

