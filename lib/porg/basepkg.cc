//=======================================================================
// basepkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "basepkg.h"
#include "baseopt.h"
#include "file.h"
#include "rexp.h"
#include <fstream>
#include <algorithm>
#include <sstream>

using std::string;
using namespace Porg;

template<typename T> static T str2num(string const&);


BasePkg::BasePkg(string const& name_, bool logged /* = true */)
:
	m_files(),
	m_name(name_),
	m_log(BaseOpt::logdir() + "/" + name_),
	m_base_name(get_base(name_)),
	m_version(get_version(name_)),
	m_date(time(0)),
	m_size(0),
	m_nfiles(0),
	m_icon_path(),
	m_url(),
	m_license(),
	m_summary(),
	m_description(),
	m_conf_opts(),
	m_author(),
	m_bug_report()
{
	// logged is false for packages that are being installed and are
	// not registered in the database yet.
	if (!logged)
		return;
	
	// open log file
	
	FileStream<std::ifstream> f(m_log);
	string buf;

	// read '#!porg' header or die
	
	if (!(getline(f, buf) && buf.find("#!porg") == 0))
		throw Error(m_log + ": '#!porg' header missing");

	//
	// Read info header.
	// Each line in the header has the form '#<char>:<value>', where <char> is
	// a single character defining the info field, and <value> is its value.
	//
	
	while (getline(f, buf) && buf[0] == '#') {

		if (buf.size() < 3) {
			assert(buf.size() > 2);
			continue;
		}

		string val(buf.substr(3));

		switch (buf[1]) {

			case CODE_DATE: 		m_date = str2num<int>(val);		break;
			case CODE_SIZE:			m_size = str2num<ulong>(val);	break;
			case CODE_NFILES:		m_nfiles = str2num<ulong>(val);	break;
			case CODE_CONF_OPTS:	m_conf_opts = val; 				break;
			case CODE_ICON_PATH:	m_icon_path = val;				break;
			case CODE_SUMMARY: 		m_summary = val; 				break;
			case CODE_URL: 			m_url = val; 					break;
			case CODE_LICENSE: 		m_license = val; 				break;
			case CODE_AUTHOR: 		m_author = val;					break;
			case CODE_BUG_REPORT:	m_bug_report = val;				break;
			case CODE_DESCRIPTION:
				if (!m_description.empty())
					m_description += "\n";
				m_description += val;
				break;
		}
	}
}


BasePkg::~BasePkg()
{
	for (iter f(m_files.begin()); f != m_files.end(); delete *f++) ;
}


void BasePkg::unlog() const
{
	if (unlink(m_log.c_str()) != 0 && errno != ENOENT)
		throw Error("unlink(" + m_log + ")", errno);
}


void BasePkg::get_files()
{
	assert(m_files.empty());
	if (!m_files.empty())
		return;

	ulong fsize;

	FileStream<std::ifstream> f(m_log);
	
	Rexp re("^(/.+)\\|([0-9]+)\\|(.*)$");

	for (string buf; getline(f, buf); ) {

		assert(buf[0] == '/' || buf[0] == '#');

		if (re.exec(buf)) {
			fsize = str2num<ulong>(re.match(2));
			m_files.push_back(new File(re.match(1), fsize, re.match(3)));
		}
	}

	sort_files();
}


bool BasePkg::find_file(File* file) const
{
	assert(file != 0);
	return std::binary_search(m_files.begin(), m_files.end(), file, Sorter());
}


bool BasePkg::find_file(string const& path) const
{
	File file(path, 0);
	return find_file(&file);
}


void BasePkg::sort_files(	sort_t type,	// = SORT_BY_NAME
							bool reverse)	// = false
{
	std::sort(m_files.begin(), m_files.end(), Sorter(type));
	if (reverse)
		std::reverse(m_files.begin(), m_files.end());
}


string BasePkg::get_base(string const& name)
{
	for (string::size_type i = 1; i < name.size(); ++i) {
		if (isdigit(name.at(i)) && name.at(i - 1) == '-')
			return name.substr(0, i - 1);
	}
	return name;
}


string BasePkg::get_version(string const& name)
{
	for (string::size_type i = 1; i < name.size(); ++i) {
		if (isdigit(name.at(i)) && name.at(i - 1) == '-')
			return name.substr(i);
	}
	return "";
}


// convert string to numeric
template <typename T>	// T = {int,long,unsigned,...}
T str2num(std::string const& s)
{
	std::istringstream is(s);
	T t;
	is >> t;
	return t;
}


//-----------------//
// BasePkg::Sorter //
//-----------------//


BasePkg::Sorter::Sorter(sort_t type /* = SORT_BY_NAME */)
:
	m_sort_func(type == SORT_BY_NAME ? &Sorter::sort_by_name : &Sorter::sort_by_size)
{ }


inline bool BasePkg::Sorter::operator()(File* left, File* right) const
{
	return (this->*m_sort_func)(left, right);
}


inline bool BasePkg::Sorter::sort_by_name(File* left, File* right) const
{
	return left->name() < right->name();
}


inline bool BasePkg::Sorter::sort_by_size(File* left, File* right) const
{
	return left->size() > right->size();
}

