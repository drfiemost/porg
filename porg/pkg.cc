//=======================================================================
// pkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "pkg.h"
#include "out.h"
#include "opt.h"
#include "pkgset.h"
#include "info.h"
#include "porg/common.h"	// in_paths()
#include "porg/file.h"
#include <string>
#include <iomanip>
#include <fstream>

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
	assert(Out::debug());

	Out::dbg_title();
	Out::dbg("Name: "			+ m_base_name + '\n');
	Out::dbg("Version: "		+ m_version + '\n');
	Out::dbg("Summary: "		+ m_summary + '\n');
	Out::dbg("Author: "			+ m_author + '\n');
	Out::dbg("URL: "			+ m_url + '\n');
	Out::dbg("License: "		+ m_license + '\n');
	Out::dbg("Conf. opts.: "	+ m_conf_opts + '\n');
	Out::dbg("Icon: "			+ m_icon_path + '\n');
	Out::dbg(str_description(true));
}


void Pkg::print_info() const
{
	cout
		<< "Name: " << m_base_name << '\n'
		<< "Version: " << m_version << '\n'
		<< "Summary: " << m_summary << '\n'
		<< "Author: " << m_author << '\n'
		<< "License: " << m_license << '\n'
		<< "URL: " << m_url << '\n'
		<< str_description() << '\n';
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


void Pkg::write_log() const
{
	// format the description field
	
	string code(string("#") + CODE_DESCRIPTION + ':');
	string desc(code + m_description);
	
	for (string::size_type p(0); (p = desc.find('\n', p)) != string::npos; )
		desc.insert(p + 1, code);
	
	// write info header
	
	FileStream<std::ofstream> of(m_log);

	of	<< "#!porg-" PACKAGE_VERSION "\n"
		<< '#' << CODE_DATE 		<< ':' << m_date << '\n'
		<< '#' << CODE_NFILES 		<< ':' << m_nfiles << '\n'
		<< '#' << CODE_SIZE 		<< ':' << m_size << '\n'
		<< '#' << CODE_BASE_NAME	<< ':' << m_base_name << '\n'
		<< '#' << CODE_VERSION		<< ':' << m_version << '\n'
		<< '#' << CODE_AUTHOR		<< ':' << m_author << '\n'
		<< '#' << CODE_SUMMARY		<< ':' << m_summary << '\n'
		<< '#' << CODE_URL			<< ':' << m_url << '\n'
		<< '#' << CODE_LICENSE		<< ':' << m_license << '\n'
		<< '#' << CODE_CONF_OPTS	<< ':' << m_conf_opts << '\n'
		<< '#' << CODE_ICON_PATH	<< ':' << m_icon_path << '\n'
		<< desc << '\n';

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
		cout << setw(size_w) << fmt_size(m_size, Opt::size_unit()) << "  ";

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
			cout << setw(size_w) << fmt_size((*f)->size(), Opt::size_unit()) << "  ";

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
		if (in_paths((*f)->name(), Opt::remove_exclude()))
			Out::vrb((*f)->name() + ": excluded (skipped)\n");

		// skip shared files
		else if (is_shared(*f, pset))
			Out::vrb((*f)->name() + ": shared (skipped)\n");

		// remove file
		else if (!unlink((*f)->name().c_str())) {
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
	if ((i = dir.rfind('/'))) {

		if (i != string::npos)
			dir.erase(i);
		
		if (!rmdir(dir.c_str())) {
			Out::vrb("Removed directory '" + dir + "'\n");
			remove_parent_dir(dir);
		}
	}
}

