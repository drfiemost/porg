//=======================================================================
// basepkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "common.h"
#include "basepkg.h"
#include "porgrc.h"
#include "file.h"
#include "rexp.h"
#include <fstream>
#include <algorithm>

using std::string;
using namespace Porg;


BasePkg::BasePkg(string const& name_, bool logged /* = true */)
:
	m_files(),
	m_name(name_),
	m_log(Porgrc::logdir() + "/" + name_),
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
	m_author()
{
	if (!logged)
		return;
	
	// open log file
	
	FileStream<std::ifstream> f(m_log);
	string buf;

	// read '#!porg' header or die
	
	if (!(getline(f, buf) && buf.find("#!porg") == 0))
		throw Error(m_log + ": Not a porg log file");

	//
	// Read info header:
	// Each line in header has the form '#<char>:<value>', where <char> is a single
	// character defining the info field, and <value> is its value.
	//	
	
	while (getline(f, buf) && buf[0] == '#') {

		if (buf.size() < 3) {
			assert(buf.size() > 2);
			continue;
		}

		string val(buf.substr(3));

		switch (buf[1]) {

			case CODE_DATE: 		m_date = str2num<int>(val);		break;
			case CODE_SIZE: 		m_size = str2num<ulong>(val); 	break;
			case CODE_NFILES: 		m_nfiles = str2num<long>(val);	break;
			case CODE_CONF_OPTS:	m_conf_opts = val; 				break;
			case CODE_ICON_PATH:	m_icon_path = val;				break;
			case CODE_SUMMARY: 		m_summary = val; 				break;
			case CODE_URL: 			m_url = val; 					break;
			case CODE_LICENSE: 		m_license = val; 				break;
			case CODE_AUTHOR: 		m_author = val;					break;
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
	for (file_it f(m_files.begin()); f != m_files.end(); delete *f++) ;
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

	m_files.reserve(m_nfiles);

	ulong cnt_size = 0, cnt_nfiles = 0, fsize;
	FileStream<std::ifstream> f(m_log);
	Rexp re("^(/.+)\\|([0-9]+)\\|(.*)$");

	for (string buf; getline(f, buf); ) {

		assert(buf[0] == '/' || buf[0] == '#');
		
		if (re.exec(buf)) {
			fsize = str2num<ulong>(re.match(2));
			m_files.push_back(new File(re.match(1), fsize, re.match(3)));
			cnt_size += fsize;
			cnt_nfiles++;
		}
	}

	sort_files();
	assert(cnt_size == m_size && cnt_nfiles == m_nfiles);
}


bool BasePkg::has_file(File* file) const
{
	assert(file != 0);
	return std::binary_search(m_files.begin(), m_files.end(), file, Sorter());
}


bool BasePkg::has_file(string const& path) const
{
	File file(path, 0);
	return has_file(&file);
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

