//=======================================================================
// log.h
//-----------------------------------------------------------------------
// This file is part of the package porg
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

	Log();
	~Log();

	protected:

	std::string				m_tmpfile;
	std::set<std::string>	m_files;
	
	static int const EXIT_FAILURE_EXTERNAL = 2;
	
	void read_files_from_command();
	void read_files_from_stream(std::istream&);
	void write_files_to_pkg() const;
	void write_files_to_stream(std::ostream&) const;
	void filter_files();
	void get_tmpfile();

}; 	// class Log

}	// namespace Porg

#endif  // PORG_LOG_H
