//=======================================================================
// porgrc.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porgrc.h"
#include "regexp.h"
#include <wordexp.h>
#include <fstream>

using std::string;
using namespace Porg;

static string sh_expand(string const&);


namespace Porg
{
	string Porgrc::s_logdir = LOGDIR;
	string Porgrc::s_include = "/";
	string Porgrc::s_exclude = EXCLUDE;
	string Porgrc::s_remove_skip = string();
}


Porgrc::Porgrc()
{
	std::ifstream f(PORGRC);
	if (!f)
		return;

	Regexp re("^([A-Z_]+)=(.*)$");

	for (string buf, opt, val; getline(f, buf); ) {
		if (re.exec(buf)) {
			opt = re.match(1);
			val = re.match(2);
			if (opt == "LOGDIR")
				s_logdir = sh_expand(val);
			else if (opt == "INCLUDE")
				s_include = val;
			else if (opt == "EXCLUDE")
   				s_exclude = val;
			else if (opt == "REMOVE_SKIP")
   				s_remove_skip = val;
		}
	}
}


bool Porgrc::logdir_writable()
{
	return !access(s_logdir.c_str(), W_OK);
}


static string sh_expand(string const& str)
{
	wordexp_t p;

	if (0 != wordexp(str.c_str(), &p, WRDE_NOCMD))
		return str;
 
 	string ret = p.we_wordv[0];
	for (uint i = 1; i < p.we_wordc; ++i)
		ret += string(" ") + p.we_wordv[i];

	wordfree(&p);
  	return ret;
}

