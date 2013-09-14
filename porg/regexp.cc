//=======================================================================
// regexp.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "regexp.h"
#include <string>

using namespace Porg;
using std::string;


Regexp::Regexp(string const& exp, bool icase /* = false */)
:
	m_ok(!regcomp(	&m_regex, 
					exp.c_str(), 
					REG_NOSUB | REG_EXTENDED | (REG_ICASE & icase)))
{ }


Regexp::~Regexp()
{
	regfree(&m_regex);
}


bool Regexp::run(string const& str)
{
	return m_ok && regexec(&m_regex, str.c_str(), 0, 0, 0) != REG_NOMATCH;
}

