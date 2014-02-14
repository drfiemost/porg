//=======================================================================
// porgrc.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porgrc.h"
#include <fstream>

using std::string;
using namespace Porg;

static string expand_var(string const& str, string const& var);


namespace Porg
{
	string Porgrc::s_logdir = LOGDIR;
	string Porgrc::s_include = INCLUDE;
	string Porgrc::s_exclude = EXCLUDE;
	string Porgrc::s_remove_skip = "";
}

			
Porgrc::Porgrc()
{
	std::ifstream f(PORGRC);
	if (!f)
		return;

	string buf;

	while (getline(f, buf)) {

		string::size_type p(buf.find("="));

		if (p == string::npos || buf.at(0) == '#')
   			continue;
		
		string val(buf.substr(p + 1));
		buf.erase(p);
   
		if (buf == "LOGDIR")
   			s_logdir = expand_var(val, "HOME");
		else if (buf == "INCLUDE")
   			s_include = expand_var(val, "HOME");
		else if (buf == "EXCLUDE")
   			s_exclude = expand_var(val, "HOME");
		else if (buf == "REMOVE_SKIP")
   			s_remove_skip = expand_var(val, "HOME");
	}
}


bool Porgrc::logdir_writable()
{
	return !access(s_logdir.c_str(), W_OK);
}


//
// Expand environment variable var in str.
// var may be given as "$var" or "${var}".
//
static string expand_var(string const& str, string const& var)
{
	string ret(str);
	string::size_type p;

	if (char* val = getenv(var.c_str())) {
		
		// expand occurrences of "$var"
		for (p = 0; (p = ret.find(string("$") + var, p)) != string::npos; )
			ret.replace(p, var.length() + 1, val);
		
		// expand occurrences of "${var}"
		for (p = 0; (p = ret.find(string("${") + var + string("}"), p)) != string::npos; )
			ret.replace(p, var.length() + 3, val);
    }

	return ret;
}

