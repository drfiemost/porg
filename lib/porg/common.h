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

	extern std::string fmt_size(ulong size);
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
