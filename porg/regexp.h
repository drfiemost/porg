//=======================================================================
// regexp.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_REGEXP_H
#define PORG_REGEXP_H

#include "config.h"
#include <iosfwd>
#include <regex.h>


namespace Porg
{

class Regexp
{
	public:

	Regexp(std::string const& exp, bool icase = false);
	~Regexp();

	bool run(std::string const& str);
		
	private:

	regex_t	m_regex;
	bool	m_ok;
};

}


#endif
