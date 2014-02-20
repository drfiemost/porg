//=======================================================================
// file.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef LIBPORG_FILE_H
#define LIBPORG_FILE_H

#include "config.h"
#include <string>

namespace Porg {

class File
{
	public:

	File(std::string const& name_, long size_, std::string const& ln_name_ = "");
	File(std::string const& name_);

	long size() const					{ return m_size; }
	std::string const& name() const		{ return m_name; }
	std::string const& ln_name() const	{ return m_ln_name; }
	bool is_symlink() const				{ return !m_ln_name.empty(); }

	private:

	// name of file
	std::string m_name;

	// size of file, in bytes
	long m_size;
	
	// if the file is a symlink, name of the file it refers to,
	// or an empty string otherwise
	std::string m_ln_name;	

};	// class File

}	// namespace Porg


#endif  // LIBPORG_FILE_H
