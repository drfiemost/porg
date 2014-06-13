//=======================================================================
// pkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "pkg.h"
#include "out.h"
#include "opt.h"
#include "pkgset.h"
#include "info.h"
#include "porg/common.h"	// in_paths(), strip_trailing()
#include "porg/file.h"
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>

using std::string;
using std::cout;
using std::setw;
using std::set;
using namespace Porg;

static void remove_parent_dir(string const& path);


//
// ctor for a Pkg already logged in database
//
Pkg::Pkg(string const& name_)
:
	BasePkg(name_)
{ }


//
// ctor for new package
//
Pkg::Pkg(string const& name_, set<string> const& files_)
:
	BasePkg(name_, false)
{
	for (set<string>::const_iterator f(files_.begin()); f != files_.end(); ++f)
		add_file(*f);
	
	if (m_files.empty())
		throw Error(m_name + ": No files to log");;

	Info info(this);
	
	if (Out::debug())
		print_info_dbg();

	write_log();
}


void Pkg::print_info_dbg() const
{
	Out::dbg_title();
	Out::dbg("Name:       "	+ m_base_name + '\n');
	Out::dbg("Version:    "	+ m_version + '\n');
	Out::dbg("Summary:    "	+ m_summary + '\n');
	Out::dbg("Author:     "	+ m_author + '\n');
	Out::dbg("URL:        "	+ m_url + '\n');
	Out::dbg("License:    "	+ m_license + '\n');
	Out::dbg("Conf. opt.: "	+ m_conf_opts + '\n');
	Out::dbg("Icon:       "	+ m_icon_path + '\n');
	Out::dbg(str_description(true));
}


void Pkg::print_info() const
{
	cout
		<< string(m_name.size() + 2, '-') << '\n'
		<< " " << m_name << " \n"
		<< string(m_name.size() + 2, '-') << '\n'
		<< "Name:     " << m_base_name << '\n'
		<< "Version:  " << m_version << '\n'
		<< "Summary:  " << m_summary << '\n'
		<< "Author:   " << m_author << '\n'
		<< "License:  " << m_license << '\n'
		<< "URL:      " << m_url << "\n\n"
		<< str_description() << "\n";
}


string Pkg::str_description(bool debug /* = false */) const
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

	return desc + '\n';
}


string Pkg::format_description() const
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


void Pkg::write_log() const
{
	// Create log file

	FileStream<std::ofstream> of(m_log);

	// write info header

	of	<< "#!porg-" PACKAGE_VERSION "\n"
		<< '#' << CODE_DATE 		<< ':' << m_date << '\n'
		<< '#' << CODE_AUTHOR		<< ':' << m_author << '\n'
		<< '#' << CODE_SUMMARY		<< ':' << Porg::strip_trailing(m_summary, '.') << '\n'
		<< '#' << CODE_URL			<< ':' << m_url << '\n'
		<< '#' << CODE_LICENSE		<< ':' << m_license << '\n'
		<< '#' << CODE_CONF_OPTS	<< ':' << m_conf_opts << '\n'
		<< '#' << CODE_ICON_PATH	<< ':' << m_icon_path << '\n'
		<< format_description();

	// write installed files
	
	for (file_cit f(m_files.begin()); f != m_files.end(); ++f)
		of << (*f)->name() << '|' << (*f)->size() << '|' << (*f)->ln_name() << '\n';
}


bool Pkg::add_file(string const& path)
{
	try 
	{
		File* file = new File(path);
		m_files.push_back(file);
		m_size += file->size();
		m_nfiles++;
		return true;
	}
	catch (...) 
	{ 
		return false; 
	}
}


void Pkg::append(set<string> const& files_)
{
	get_files();

	bool appended(false);

	for (set<string>::const_iterator f(files_.begin()); f != files_.end(); ++f) {
		if (!has_file(*f) && add_file(*f))
			appended = true;
	}

	if (appended)
		write_log();
}


void Pkg::unlog() const
{
	try 
	{ 
		BasePkg::unlog(); 
		Out::vrb("Package '" + m_name + "' removed from database\n");
	}
	catch (Error const& x) 
	{
		Out::vrb(x.what());
	}
}


void Pkg::list(int size_w, int nfiles_w) const
{
	if (Opt::print_sizes())
		cout << setw(size_w) << fmt_size(m_size) << "  ";

	if (Opt::print_nfiles())
		cout << setw(nfiles_w) << m_nfiles << "  ";

	if (Opt::print_date())
		cout << fmt_date(m_date, Opt::print_hour()) << "  ";

	cout << m_name << '\n';
}


void Pkg::list_files(int size_w)
{
	sort_files(Opt::sort_type(), Opt::reverse_sort());

	if (!Opt::print_no_pkg_name())
		cout << m_name << ":\n";

	for (file_it f(m_files.begin()); f != m_files.end(); ++f) {
		
		if (Opt::print_sizes())
			cout << setw(size_w) << fmt_size((*f)->size()) << "  ";

		cout << (*f)->name();

		if (Opt::print_symlinks() && (*f)->is_symlink())
			cout << " -> " << (*f)->ln_name();

		cout << '\n';
	}
}


bool Pkg::remove(PkgSet const& pset)
{
	if (!Opt::remove_batch()) {
		cout << "Remove package '" << m_name << "' (y/N) ? ";
		string buf;
		if (!(getline(std::cin, buf) && (buf == "y" || buf == "yes")))
			return false;
	}

	for (file_it f(m_files.begin()); f != m_files.end(); ++f) {

		// skip excluded
		if (in_paths((*f)->name(), Opt::remove_skip()))
			Out::vrb((*f)->name() + ": excluded (skipped)\n");

		// skip shared files
		else if (is_shared(*f, pset))
			Out::vrb((*f)->name() + ": shared (skipped)\n");

		// remove file
		else if (unlink((*f)->name().c_str()) == 0) {
			Out::vrb("Removed '" + (*f)->name() + "'\n");
			remove_parent_dir((*f)->name());
		}

		// an error occurred
		else if (errno != ENOENT)
			throw Error("unlink(" + (*f)->name() + ")", errno);
	}

	unlog();

	return true;
}


static void remove_parent_dir(string const& path)
{
	string dir(path);
	string::size_type i;

	// remove trailing slashes
	for (i = dir.size() - 1; i > 0 && dir.at(i) == '/'; dir.erase(i--)) ;

	// get parent dir and remove it
	if ((i = dir.rfind('/')) != string::npos) {
		dir.erase(i);
		if (rmdir(dir.c_str()) == 0) {
			Out::vrb("Removed directory '" + dir + "'\n");
			remove_parent_dir(dir);
		}
	}
}

