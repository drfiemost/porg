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
#include <iomanip>

using std::string;
using namespace Porg;

template<typename T> static T str2num(string const&);


BasePkg::BasePkg(string const& name_)
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
	m_author()
{ }


void BasePkg::read_log_header()
{
	// open log file
	
	FileStream<std::ifstream> f(m_log);

	// read '#!porg' header or die
	
	string buf;
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
			case CODE_SIZE:			m_size = str2num<float>(val);	break;
			case CODE_NFILES:		m_nfiles = str2num<ulong>(val);	break;
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

	FileStream<std::ifstream> f(m_log);
	Rexp re("^(/.+)\\|([0-9]+)\\|(.*)$");

	for (string buf; getline(f, buf); ) {
		assert(buf[0] == '/' || buf[0] == '#');
		if (re.exec(buf)) {
			m_files.push_back(new File(re.match(1), 
				str2num<ulong>(re.match(2)), re.match(3)));
		}
	}

	sort_files();
}


string BasePkg::description_str(bool debug /* = false */) const
{
	string const head(debug ? "porg :: " : "");
	string desc("Description: ");
	
	if (m_description.find('\n') == string::npos)
		desc += m_description;
	else {
		std::istringstream is(m_description);
		for (string buf; getline(is, buf); )
			desc += '\n' + head + "   " + buf;
	}

	return desc;
}


string BasePkg::format_description() const
{
	string code(string("#") + CODE_DESCRIPTION + ':');

	if (m_description.empty())
		return code + '\n';

	string ret;
	std::istringstream is(m_description);

	for (string buf; getline(is, buf); )
		ret += code + buf + '\n';

	return ret;
}


void BasePkg::write_log() const
{
	// Create log file

	FileStream<std::ofstream> of(m_log);

	// write info header

	of	<< "#!porg-" PACKAGE_VERSION "\n"
		<< '#' << CODE_DATE 		<< ':' << m_date << '\n'
		<< '#' << CODE_SIZE			<< ':' << std::setprecision(0) << std::fixed << m_size << '\n'
		<< '#' << CODE_NFILES		<< ':' << m_nfiles << '\n'
		<< '#' << CODE_AUTHOR		<< ':' << m_author << '\n'
		<< '#' << CODE_SUMMARY		<< ':' << Porg::strip_trailing(m_summary, '.') << '\n'
		<< '#' << CODE_URL			<< ':' << m_url << '\n'
		<< '#' << CODE_LICENSE		<< ':' << m_license << '\n'
		<< '#' << CODE_CONF_OPTS	<< ':' << m_conf_opts << '\n'
		<< '#' << CODE_ICON_PATH	<< ':' << m_icon_path << '\n'
		<< format_description();

	// write installed files
	
	for (const_iter f(m_files.begin()); f != m_files.end(); ++f)
		of << (*f)->name() << '|' << (*f)->size() << '|' << (*f)->ln_name() << '\n';
}


void BasePkg::add_file(string const& path)
{
	File* file = new File(path);
	m_files.push_back(file);
	m_size += file->size();
	m_nfiles++;
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

