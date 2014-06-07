//=======================================================================
// db.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
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
	g_assert(Opt::initialized());

	Opt::check_logdir();
	Glib::Dir dir(Opt::logdir());

	for (Glib::Dir::iterator d = dir.begin(); d != dir.end(); ++d) {
		try 
		{	
			Pkg* pkg = new Pkg(*d);
			s_pkgs.push_back(pkg);
			s_total_size += pkg->size();
		}
		catch (std::exception const& x) 
		{
			g_warning("%s", x.what()); 
		}
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

