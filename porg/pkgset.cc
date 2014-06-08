//=======================================================================
// pkgset.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porg/file.h"
#include "pkgset.h"
#include "util.h"
#include "opt.h"
#include "out.h"
#include "pkg.h"
#include <algorithm>
#include <iomanip>

using std::cout;
using std::vector;
using std::setw;
using std::string;
using std::max;
using namespace Porg;

static int get_digits(long);
static int get_width(long);
static bool match_pkg(string const&, string const&);


PkgSet::PkgSet()
:
	vector<Pkg*>(),
	m_total_size(0),
	m_total_files(0)
{ }


PkgSet::~PkgSet()
{
	for (iterator p(begin()); p != end(); delete *p++) ;
}


//
// get all packages logged in database
//
void PkgSet::get_all_pkgs()
{
	Dir dir(Opt::logdir());

	for (string name; dir.read(name); add_pkg(name)) ;

	std::sort(begin(), end(), Sorter());

	if (empty())
		Out::vrb("porg: No packages logged in '" + Opt::logdir() + "'\n");
}


//
// Search the database for packages matching any of the strings in args given
// by the command line
//
int PkgSet::get_pkgs(vector<string> const& args)
{
	int ret = EXIT_SUCCESS;
	
	Dir dir(Opt::logdir());

	for (uint i = 0; i < args.size(); ++i, dir.rewind()) {
		
		bool found = false;

		for (string name; dir.read(name); ) {
			if (match_pkg(args[i], name) && add_pkg(name))
				found = true;
		}

		if (!found) {
			Out::vrb("porg: " + args[i] + ": Package not logged\n");
			ret = EXIT_FAILURE;
		}
	}

	std::sort(begin(), end(), Sorter());

	return ret;
}


bool PkgSet::add_pkg(string const& name)
{
	try 
	{
		Pkg* pkg = new Pkg(name);
		push_back(pkg);
		m_total_size += pkg->size();
		m_total_files += pkg->nfiles();
		return true;
	}
	catch (...) 
	{ 
		return false; 
	}
}


//
// get widths for printing pkg sizes and number of files
//
void PkgSet::get_pkg_list_widths(int& size_w, int& nfiles_w)
{
	size_w = Opt::print_totals() ? get_width(m_total_size) : 0;
	long max_nfiles(Opt::print_totals() ? m_total_files : 0);
	
	for (iterator p(begin()); p != end(); ++p) {
		size_w = max(size_w, get_width((*p)->size()));
		if (!Opt::print_totals())
			max_nfiles = max(max_nfiles, (*p)->nfiles());
	}

	nfiles_w = get_digits(max_nfiles);
}


//
// get width for printing file sizes
//
int PkgSet::get_file_size_width()
{
	int size_w = Opt::print_totals() ? get_width(m_total_size) : 0;

	for (iterator p(begin()); p != end(); ++p) {
		for (Pkg::file_it f((*p)->files().begin()); f != (*p)->files().end(); ++f)
			size_w = max(size_w, get_width((*f)->size()));
	}

	return size_w;
}


void PkgSet::get_files()
{
	for (iterator p(begin()); p != end(); (*p++)->get_files()) ;
}


void PkgSet::print_conf_opts() const
{
	for (const_iterator p(begin()); p != end(); ++p) {
		if (size() > 1)
			cout << (*p)->name() << ":\n";
		cout << (*p)->conf_opts() << '\n';
		if (!(*p)->conf_opts().empty() && size() > 1 && p != end() - 1)
			cout << '\n';
	}
}


int PkgSet::query()
{
	int ret = EXIT_FAILURE;

	get_files();

	for (uint i(0); i < Opt::args().size(); ++i) {
		
		string path(clear_path(Opt::args()[i]));
		cout << path << ':';
		
		for (const_iterator p(begin()); p != end(); ++p) {
			if ((*p)->has_file(path)) {
				ret = EXIT_SUCCESS;
				cout << "  " << (*p)->name();
			}
		}
		
		cout << '\n';
	}
	
	return ret;
}


void PkgSet::print_info() const
{
	for (const_iterator p(begin()); p != end(); (*p++)->print_info()) ;
}


void PkgSet::remove()
{
	if (Opt::remove_unlog()) {
		for (iterator p(begin()); p != end(); (*p++)->unlog()) ;
		return;
	}
	
	get_files();

	PkgSet all;
	all.get_all_pkgs();
	all.get_files();

	for (iterator p(begin()); p != end(); ++p) {
		if ((*p)->remove(all))
			all.del_pkg((*p)->name());
	}
}


void PkgSet::del_pkg(string const& name)
{
	for (iterator p(begin()); p != end(); ++p) {
		if ((*p)->name() == name) {
			erase(p);
			break;
		}
	}
}


void PkgSet::list()
{
	// sort list of packages
	
	std::sort(begin(), end(), Sorter(Opt::sort_type()));
	if (Opt::reverse_sort())
		std::reverse(begin(), end());

	// get widths for printing pkg sizes and number of files
	
	int size_w, nfiles_w;
	get_pkg_list_widths(size_w, nfiles_w);

	// list packages
	
	for (iterator p(begin()); p != end(); (*p++)->list(size_w, nfiles_w)) ;

	// print totals, if needed
	
	if (Opt::print_totals()) {
		
		cout << '\n';

		if (Opt::print_sizes())
			cout << setw(size_w) << fmt_size(m_total_size) << "  ";
		
		if (Opt::print_nfiles())
			cout << setw(nfiles_w) << m_total_files << "  ";

		if (Opt::print_date())
			cout << fmt_date(0, Opt::print_hour()) << "  ";
		
		cout << "TOTAL\n";
	}
}


void PkgSet::list_files()
{
	get_files();

	int size_w(get_file_size_width());

	for (iterator p(begin()); p != end(); ++p) {
		(*p)->list_files(size_w);
		if (size() > 1 && p != end() - 1)
			cout << '\n';
	}

	if (Opt::print_totals())
		cout << '\n' << setw(size_w) << fmt_size(m_total_size) << "  TOTAL\n";
}


//----------------//
// PkgSet::Sorter //
//----------------//


PkgSet::Sorter::Sorter(sort_t const& t /* = SORT_BY_NAME */)
:
	m_sort_func()
{
	switch (t) {
		case SORT_BY_SIZE: 		m_sort_func = &Sorter::sort_by_size; 	break;
		case SORT_BY_NFILES:	m_sort_func = &Sorter::sort_by_nfiles;	break;
		case SORT_BY_DATE: 		m_sort_func = &Sorter::sort_by_date; 	break;
		default: 				m_sort_func = &Sorter::sort_by_name;
	}
}


bool PkgSet::Sorter::operator()(Pkg* left, Pkg* right) const
{
	return (this->*m_sort_func)(right, left);
}

bool PkgSet::Sorter::sort_by_name(Pkg* left, Pkg* right) const
{
	return left->name() > right->name();
}

bool PkgSet::Sorter::sort_by_size(Pkg* left, Pkg* right) const
{
	return left->size() < right->size();
}

bool PkgSet::Sorter::sort_by_nfiles(Pkg* left, Pkg* right) const
{
	return left->nfiles() < right->nfiles();
}

inline bool PkgSet::Sorter::sort_by_date(Pkg* left, Pkg* right) const
{
	return left->date() > right->date();
}


//-------------------//
// static free funcs //
//-------------------//


//
// Return the number of digits of a number
//
static int get_digits(long n)
{
	int ret;
	for (ret = 0; n; n /= 10, ret++) ;
	return ret;
}


inline static int get_width(long size)
{
	return fmt_size(size).size();
}


static bool match_pkg(string const& str, string const& pkg)
{
	if (Opt::exact_version())
		return str == pkg;

	string str_base = Pkg::get_base(str);
	string pkg_base = Pkg::get_base(pkg);
	string str_version = Pkg::get_version(str);
	string pkg_version = Pkg::get_version(pkg);
	
	if (pkg_base != str_base)
		return false;

	else if (str_version.empty() || str_version == pkg_version)
		return true;

	else if (str_version.compare(0, str_version.size(), 
		pkg_version.c_str(), str_version.size()))
		return false;

	return ispunct(pkg_version[str_version.size()]);
}

