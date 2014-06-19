//=======================================================================
// log.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_LOG_H
#define PORG_LOG_H

#include "config.h"
#include <iosfwd>
#include <set>

namespace Porg {

class Log
{
	public:

	static void run();

	protected:

	std::string const		m_pkgname;
	std::set<std::string> 	m_files;
	
	Log();

	void read_files_from_command();
	void exec_command(std::string const&) const;
	void read_files_from_stream(std::istream&);
	void write_files_to_pkg() const;
	void write_files_to_stream(std::ostream&) const;
	void filter_files();

}; 	// class Log

}	// namespace Porg

#endif  // PORG_LOG_H
