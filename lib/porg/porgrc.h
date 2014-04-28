//=======================================================================
// porgrc.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef LIBPORG_PORGRC_H
#define LIBPORG_PORGRC_H

#include "config.h"
#include <string>

namespace Porg {

class Porgrc
{
	public:
	
	static std::string const& logdir()	{ return s_logdir; }
	static std::string const& include()	{ return s_include; }
	static std::string const& exclude()	{ return s_exclude; }
	static std::string const& remove_skip()	{ return s_remove_skip; }
	
	static bool logdir_writable();

	protected:

	Porgrc();
	virtual ~Porgrc() { };

	static std::string s_logdir;
	static std::string s_include;
	static std::string s_exclude;
	static std::string s_remove_skip;

};	// class Porgrc

}	// namespace Porg

#endif  // LIBPORG_PORGRC_H

