//=======================================================================
// file.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "file.h"
#include "common.h"

using std::string;


Porg::File::File(string const& name_, long size_, string const& ln_name_ /* = "" */)
:
	m_name(name_),
	m_size(size_),
	m_ln_name(ln_name_)
{ }


Porg::File::File(string const& name_)
:
	m_name(name_),
	m_size(UNKNOWN_SIZE),
	m_ln_name()
{
	struct stat s;

	if (lstat(m_name.c_str(), &s) < 0)
		return;

	else if (S_ISDIR(s.st_mode))
		throw Error(m_name + ": Is a directory");

	else if (S_ISLNK(s.st_mode)) {
		char ln[4096];
		int cnt = readlink(m_name.c_str(), ln, sizeof(ln) - 1);
		if (cnt > 0) {
			ln[cnt] = 0;
			m_ln_name = ln;
		}
	}

	m_size = s.st_size;
}

