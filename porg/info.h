//=======================================================================
// info.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_INFO_H
#define PORG_INFO_H

#include "config.h"
#include <vector>
#include <set>


namespace Porg {

class Pkg;

class Info
{
	public:

	Info(Pkg*);

	private:

	class Define
	{
		public:

		Define(std::string const& var, std::string const& val);
	
		void resolve(std::string& str) const;
	
		private:
		
		std::string m_var;
		std::string m_val;

	};	// class Info::Define

	std::vector<Define> 	m_defs;
	Pkg*					m_pkg;

	void get_icon_path();
	void get_info_spec();
	void get_info_desktop();
	void get_info_config_log();
	void get_defs_spec(std::string const&);
	void get_spec_desc(std::string const&);
	std::string resolve_defines(std::string const&) const;
	bool get_var(std::string const&, std::string const&, std::string&, bool = true) const;
	std::string search_file(std::string const&) const;

};	// class Info

}	// namespace Porg


#endif	// PORG_INFO_H
