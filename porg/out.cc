//=======================================================================
// out.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "out.h"
#include "porg/common.h"
#include <string>

using std::string;
using std::cerr;
using namespace Porg;


namespace Porg
{
	int Out::s_verbosity = QUIET;
	int Out::s_screen_width = get_screen_width();
}
	

int Out::get_screen_width()
{
	char* columns = getenv("COLUMNS");
	return columns ? str2num<int>(columns) : DEFAULT_SCREEN_WIDTH;
}


void Out::vrb(string const& msg, int errno_ /* = 0 */)
{
	if (verbose()) {
		if (errno_)
			cerr << "porg: ";
		cerr << msg;
		if (errno_)
			cerr << ": " << strerror(errno_) << '\n';
	}
}


void Out::dbg(string const& msg, bool print_prog_name /* = true */)
{
	if (debug())
		cerr << (print_prog_name ? "porg :: " : "") << msg;
}


void Out::dbg_title(string const& title /* = "" */)
{
	if (!debug())
		return;
	
	string head("porg :: ----");
	cerr << head;
	int cnt = head.size();

	if (title.size()) {
		string str(string("[ ") + title + " ]");
		cerr << str;
		cnt += str.size();
	}
	
	if (s_screen_width > cnt)
		cerr << string(s_screen_width - cnt, '-');
	
	cerr << '\n';
}

