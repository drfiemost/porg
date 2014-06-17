//=======================================================================
// pkg.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_PKG_H
#define PORG_PKG_H

#include "config.h"
#include "porg/basepkg.h"
#include <iosfwd>
#include <set>


namespace Porg
{

class DB;

class Pkg : public BasePkg
{
	friend class Info;

	public:

	Pkg(std::string const& name_);
	Pkg(std::string const& name_, std::set<std::string> const& files);
	
	void unlog() const;
	void remove(DB const&);
	void print_conf_opts(bool print_pkg_name) const;
	void print_info() const;
	void print_info_dbg() const;
	void list(int size_w, int nfiles_w) const;
	void list_files(int size_w);
	void append(std::set<std::string> const& files);
	void write_log() const;

	protected:

	std::string format_description() const;
	void add_file(std::string const& path);
	std::string description_str(bool debug = false) const;

};	// class Pkg

}	// namespace Porg


#endif  // PORG_PKG_H

