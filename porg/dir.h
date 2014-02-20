//=======================================================================
// dir.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef LIBPORG_DIR_H
#define LIBPORG_DIR_H

#include "config.h"
#include <dirent.h>
#include <iosfwd>


namespace Porg {

class Dir
{
	public:

	Dir(std::string const&);
	~Dir();

	bool read(std::string&);
	void rewind();

	private:

	DIR* m_dir;
	struct dirent* m_dirent;
	
};	// class Dir

}	// namespace Porg


#endif	// LIBPORG_DIR_H
