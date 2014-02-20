//=======================================================================
// common.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "common.h"
#include <sstream>
#include <iomanip>
#include <fnmatch.h>

using std::string;
using namespace Porg;


//
// Format size according to given size unit (bytes, kb, human readable...)
//
string Porg::fmt_size(ulong size, ulong unit /* = HUMAN_READABLE */)
{
	std::ostringstream s;
	
	if (unit == HUMAN_READABLE) {
		if (size < KILOBYTE)
			s << size;
		else if (size < (10 * KILOBYTE))
			s << std::setprecision(2) << static_cast<float>(size) / KILOBYTE << "k";
		else if (size < MEGABYTE)
			s << size / KILOBYTE << "k";
		else if (size < (10 * MEGABYTE))
			s << std::setprecision(2) << static_cast<float>(size) / MEGABYTE << "M";
		else if (size < GIGABYTE)
			s << size / MEGABYTE << "M";
		else
			s << std::setprecision(2) << static_cast<float>(size) / GIGABYTE << "G";
	}
	else
		s << size / unit;
		
	return s.str();
}


//
// Convert date to string
//
string Porg::fmt_date(time_t date, bool print_hour)
{
	struct tm* t;
	char str[32] = "";
	string fmt(string("%x") + (print_hour ? " %H:%M" : ""));

	if (date 
	&& (t = localtime(&date))
	&& strftime(str, sizeof(str) - 1, fmt.c_str(), t))
		return string(str);
	else
		// if date == 0, or an error occurs, return a string
		// of whitespaces with the proper length
		return string(fmt_date(time(0), print_hour).size(), ' ');
}


//
// Check whether a path matches any path in a given colon-separated list 
// of paths.
// Shell-like wildcards in the list are expanded.
//
bool Porg::in_paths(string const& path, string const& list)
{   
	std::istringstream s(list + ":");
	string buf;
	while (getline(s, buf, ':') && buf.size()) {
		if (buf == "/" || !fnmatch(buf.c_str(), path.c_str(), 0) || !path.find(buf + "/"))
			return true;
	}
	return false;
}


Porg::Error::Error(string const& msg, int errno_ /* = 0 */)
:
	std::runtime_error(msg + (errno_ ? (string(": ") + strerror(errno_)) : ""))
{ }

