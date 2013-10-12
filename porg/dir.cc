//=======================================================================
// dir.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porg/common.h"	// Error
#include "dir.h"

using std::string;
using namespace Porg;


Dir::Dir(string const& path)
:
	m_dir(opendir(path.c_str())),
	m_dirent(NULL)
{
	if (!m_dir)
		throw Error("opendir(\"" + path + "\")", errno);
}


Dir::~Dir()
{
	if (m_dir)
		closedir(m_dir);
}


bool Dir::read(string &name)
{
	if (!(m_dirent = readdir(m_dir)))
		return false;
	
	name = m_dirent->d_name;

	// skip hidden files and special files '.' and '..'
	if (name.at(0) == '.')
		return read(name);

	return true;
}


void Dir::rewind()
{
	rewinddir(m_dir);
}

