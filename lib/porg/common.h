//=======================================================================
// common.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
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
	ulong const HUMAN_READABLE	= 0;
	ulong const BYTE			= 1;
	ulong const KILOBYTE		= 1024;
	ulong const MEGABYTE		= 1048576;
	ulong const GIGABYTE		= 1073741824;

	typedef enum {
		SORT_BY_NAME	= 0,
		SORT_BY_SIZE	= 1,
		SORT_BY_NFILES	= 2,
		SORT_BY_DATE	= 3
	} sort_t;


	class Error : public std::runtime_error
	{
		public: Error(std::string const& msg, int errno_ = 0);
	};

	// A safer std::{i,o}fstream
	template<typename T>	// T = std::{i,o}fstream
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

	extern std::string fmt_size(ulong size, ulong unit = HUMAN_READABLE);
	extern std::string fmt_date(time_t date, bool print_hour);
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
