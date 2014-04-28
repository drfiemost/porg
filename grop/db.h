//=======================================================================
// db.h
//-----------------------------------------------------------------------
// This file is part of the package grop
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef GROP_DB_H
#define GROP_DB_H

#include "config.h"
#include <vector>
#include <iosfwd>

namespace Grop {

class Pkg;

class DB
{
	public:

	typedef std::vector<Pkg*>::iterator			pkg_it;
	typedef std::vector<Pkg*>::const_iterator	pkg_cit;
	
	static void init();

	static ulong total_size()			{ return s_total_size; }
	static std::vector<Pkg*>& pkgs()	{ return s_pkgs; }
	static bool initialized()			{ return s_initialized; }
	static int pkg_cnt()				{ return s_pkgs.size(); }

	static void remove_pkg(Pkg*);

	protected:

	DB();
	~DB();

	static std::vector<Pkg*> s_pkgs;
	static ulong s_total_size;
	static bool s_initialized;

};

}	// namespace Grop


#endif  // GROP_DB_H
