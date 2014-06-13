//=======================================================================
// common.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef LIBPORG_COMMON_H
#define LIBPORG_COMMON_H

#include "config.h"
#include <stdexcept>
#include <iosfwd>
#include <sstream>


namespace Porg
{
	typedef enum {
		SORT_BY_NAME,
		SORT_BY_SIZE,
		SORT_BY_NFILES,
		SORT_BY_DATE
	} sort_t;

	long const UNKNOWN_SIZE = -1;
	long const KILOBYTE = 1024;
	long const MEGABYTE = 1048576;
	long const GIGABYTE = 1073741824;

	class Error : public std::runtime_error
	{
		public: Error(std::string const& msg, int errno_ = 0);
	};

	// A safer std::{i,o}fstream
	template <typename T>	// T = std::{i,o}fstream
	class FileStream : public T
	{
		public:
		FileStream(std::string const& path) : T(path.c_str())
		{
			if (!this)
				throw Error(path, errno);
			this->exceptions(std::ios::badbit);
		}
	};

	extern std::string fmt_size(long size);
	extern std::string fmt_date(time_t date, bool print_hour);
	extern std::string strip_trailing(std::string const&, char);
	extern std::string strip_repeated(std::string const&, char);
	extern bool in_paths(std::string const&, std::string const&);

	// convert string to numeric
	template <typename T>	// T = {int,long,unsigned,...}
	T str2num(std::string const& s)
	{
		std::istringstream is(s);
		T t;
		is >> t;
		return t;
	}

}		// namespace Porg

#endif  // LIBPORG_COMMON_H
