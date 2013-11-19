//=======================================================================
// pkgset.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_PKGSET_H
#define PORG_PKGSET_H

#include "config.h"
#include "porg/common.h"	// sort_t
#include <vector>


namespace Porg {

class Pkg;

class PkgSet : public std::vector<Pkg*>
{
	public:

	PkgSet();
	~PkgSet();

	void get_pkgs(std::vector<std::string> const& args);
	void get_all_pkgs();

	Pkg* find_pkg(std::string const& name) const;
	void list();
	void list_files();
	void unlog() const;
	void print_conf_opts() const;
	void query();
	void remove();
	void print_info() const;

	protected:

	void get_files();
	void get_pkg_list_widths(int& size_w, int& nfiles_w);
	int get_file_size_width();
	bool add_pkg(std::string const& name);
	void del_pkg(std::string const& name);

	class Sorter
	{
		public:

		Sorter(sort_t const& = SORT_BY_NAME);
		bool operator()(Pkg* left, Pkg* right) const;

		private:

		bool (Sorter::*m_sort_func)(Pkg*, Pkg*) const;
		bool sort_by_name(Pkg* left, Pkg* right) const;
		bool sort_by_size(Pkg* left, Pkg* right) const;
		bool sort_by_nfiles(Pkg* left, Pkg* right) const;
		bool sort_by_date(Pkg* left, Pkg* right) const;

	};	// class PkgSet::Sorter

	ulong	m_total_size;
	ulong	m_total_files;

};		// class PkgSet

}		// namespace Porg

#endif  // PORG_PKGSET_H
