//=======================================================================
// db.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "pkg.h"
#include "db.h"
#include <glibmm/fileutils.h>	// Dir

using std::string;

using namespace Grop;


ulong 				DB::s_total_size = 0;
std::vector<Pkg*> 	DB::s_pkgs;
bool				DB::s_initialized = false;


DB::DB()
{
	g_assert(s_initialized == false);
	// Opt should have been initialized before
	g_assert(Opt::initialized());

	try
	{
		Glib::Dir dir(Opt::logdir());
	
		for (Glib::Dir::iterator d = dir.begin(); d != dir.end(); ++d) {
			try 
			{	
				Pkg* pkg = new Pkg(*d);
				s_pkgs.push_back(pkg);
				s_total_size += pkg->size();
			}
			catch (Porg::Error const& x) 
			{
				g_warning("%s", x.what()); 
			}
		}
	}
	catch (Glib::Error const& x)
	{
		// fatal error opening database dir (abort app.)
		throw Porg::Error(x.what());
	}

	s_initialized = true;
}


DB::~DB()
{
	for (pkg_it p(s_pkgs.begin()); p != s_pkgs.end(); delete *p++) ;
}


void DB::init()
{
	static DB db;
}


Pkg* DB::find_pkg(string const& name)
{
	for (pkg_cit p(s_pkgs.begin()); p != s_pkgs.end(); ++p) {
		if ((*p)->name() == name)
			return *p;
	}
	return 0;
}


void DB::remove_pkg(Pkg* pkg)
{
	g_assert(pkg != 0);

	pkg->unlog();

	for (pkg_it p(s_pkgs.begin()); p != s_pkgs.end(); ++p) {
		if (*p == pkg) {
			s_total_size -= pkg->size();
			s_pkgs.erase(p);
			break;
		}
	}
			
	delete pkg;
}

