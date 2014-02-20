//=======================================================================
// opt.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_OPT_H
#define PORG_OPT_H

#include "config.h"
#include "porg/common.h"
#include "porg/porgrc.h"
#include <vector>


namespace Porg {


enum Mode {
   	MODE_LIST_PKGS,
   	MODE_LIST_FILES,
   	MODE_QUERY,
   	MODE_INFO,
   	MODE_CONF_OPTS,
   	MODE_LOG,
   	MODE_REMOVE,
	MODE_UNLOG,
   	NMODES
};


class Opt : public Porgrc
{
	public:

	static void init(int argc, char* argv[]);

	static bool all_pkgs()			{ return s_all_pkgs; }
	static bool print_sizes()		{ return s_print_sizes; }
	static bool print_files()		{ return s_print_files; }
	static bool print_nfiles()		{ return s_print_nfiles; }
	static bool print_totals()		{ return s_print_totals; }
	static bool print_symlinks()	{ return s_print_symlinks; }
	static bool print_no_pkg_name()	{ return s_print_no_pkg_name; }
	static bool remove_batch()		{ return s_remove_batch; }
	static bool log_append()		{ return s_log_append; }
	static bool log_ignore_errors()	{ return s_log_ignore_errors; }
	static bool log_missing()		{ return s_log_missing; }
	static bool reverse_sort() 		{ return s_reverse_sort; }
	static bool print_date() 		{ return s_print_date; }
	static bool print_hour() 		{ return s_print_hour; }
	static sort_t sort_type()		{ return s_sort_type; }
	static int size_unit()			{ return s_size_unit; }
	static Mode mode()				{ return s_mode; };
	static std::string const& log_pkg_name()		{ return s_log_pkg_name; }
	static std::vector<std::string> const& args()	{ return s_args; }
	
	protected:

	Opt(int argc, char* argv[]);

	static void set_mode(Mode m, char optchar);

	static bool s_all_pkgs;
	static bool s_print_sizes;
	static bool s_print_files;
	static bool s_print_nfiles;
	static bool s_print_totals;
	static bool s_print_symlinks;
	static bool s_print_no_pkg_name;
	static bool s_remove_batch;
	static bool s_log_append;
	static bool s_log_ignore_errors;
	static bool s_log_missing;
	static bool s_reverse_sort;
	static bool s_print_date;
	static bool s_print_hour;
	static sort_t	s_sort_type;
	static int s_size_unit;
	static std::string s_log_pkg_name;
	static Mode s_mode;
	static std::vector<std::string> s_args;

};	// class Opt

}	// namespace Porg


#endif  // PORG_OPT_H
