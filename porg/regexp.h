//=======================================================================
// regexp.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_REGEXP_H
#define PORG_REGEXP_H

#include "config.h"
#include <iosfwd>
#include <regex.h>


namespace Porg
{

//XXX: template <int nmatches = 1>
class Regexp
{
	public:

//XXX Create static member func for running regexes "on the fly"

	Regexp(std::string const& exp, int flags = 0);
	~Regexp();

	bool exec(std::string const&);
	std::string match(int = 0);
		
	private:

	//XXX make this with template <int>
	static int const MAX_MATCHES = 8;

	regex_t		m_regex;
	regmatch_t	m_pmatch[MAX_MATCHES];
	std::string	m_str;
	bool		m_matched;
	bool const	m_compiled;
};

}


#endif
