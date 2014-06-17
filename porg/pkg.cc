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
#include "db.h"
#include "main.h"			// g_exit_status
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
}


void Pkg::print_info_dbg() const
{
	Out::dbg_title();
	Out::dbg("Name:       "	+ m_base_name);
	Out::dbg("Version:    "	+ m_version);
	Out::dbg("Summary:    "	+ m_summary);
	Out::dbg("Author:     "	+ m_author);
	Out::dbg("Bug report: "	+ m_bug_report);
	Out::dbg("URL:        "	+ m_url);
	Out::dbg("License:    "	+ m_license);
	Out::dbg("Conf. opt.: "	+ m_conf_opts);
	Out::dbg("Icon:       "	+ m_icon_path);
	Out::dbg(description_str(true));
}


void Pkg::print_info() const
{
	cout
		<< string(m_name.size() + 2, '-') << '\n'
		<< " " << m_name << " \n"
		<< string(m_name.size() + 2, '-') << '\n'
		<< "Name:       " << m_base_name << '\n'
		<< "Version:    " << m_version << '\n'
		<< "Summary:    " << m_summary << '\n'
		<< "Author:     " << m_author << '\n'
		<< "Bug report: " << m_bug_report << '\n'
		<< "License:    " << m_license << '\n'
		<< "URL:        " << m_url << "\n\n"
		<< description_str() << "\n\n";
}


string Pkg::description_str(bool debug /* = false */) const
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
		<< '#' << CODE_SIZE			<< ':' << std::setprecision(0) << std::fixed << m_size << '\n'
		<< '#' << CODE_NFILES		<< ':' << m_nfiles << '\n'
		<< '#' << CODE_AUTHOR		<< ':' << m_author << '\n'
		<< '#' << CODE_BUG_REPORT	<< ':' << m_bug_report << '\n'
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


void Pkg::add_file(string const& path)
{
	File* file = new File(path);
	m_files.push_back(file);
	m_size += file->size();
	m_nfiles++;
}


void Pkg::append(set<string> const& files_)
{
	get_files();

	bool appended(false);

	for (set<string>::const_iterator f(files_.begin()); f != files_.end(); ++f) {
		if (!find_file(*f)) {
			add_file(*f);
			appended = true;
		}
	}

	if (appended)
		write_log();
}


void Pkg::unlog() const
{
	try 
	{
		BasePkg::unlog(); 
		Out::vrb("Package '" + m_name + "' removed from database");
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

	if (!Opt::print_no_pkg_name())
		cout << m_name;
	
	cout << '\n';
}


void Pkg::list_files(int size_w)
{
	sort_files(Opt::sort_type(), Opt::reverse_sort());

	if (!Opt::print_no_pkg_name())
		cout << m_name << ":\n";

	for (const_iter f(m_files.begin()); f != m_files.end(); ++f) {
		
		if (Opt::print_sizes())
			cout << setw(size_w) << fmt_size((*f)->size()) << "  ";

		cout << (*f)->name();

		if (Opt::print_symlinks() && (*f)->is_symlink())
			cout << " -> " << (*f)->ln_name();

		cout << '\n';
	}
}


void Pkg::remove(DB const& db)
{
	for (iter f(m_files.begin()); f != m_files.end(); ++f) {

		// skip excluded
		if (in_paths((*f)->name(), Opt::remove_skip()))
			Out::vrb((*f)->name() + ": excluded");

		// skip shared files
		else if (is_shared(*f, db))
			Out::vrb((*f)->name() + ": shared");

		// remove file
		else if (!unlink((*f)->name().c_str())) {
			Out::vrb("Removed '" + (*f)->name());
			remove_parent_dir((*f)->name());
		}

		// an error occurred
		else if (errno != ENOENT) {
			Out::vrb("Failed to remove '" + (*f)->name() + "'", errno);
			g_exit_status = EXIT_FAILURE;
		}
	}

	if (g_exit_status == EXIT_SUCCESS)
		unlog();
}


static void remove_parent_dir(string const& path)
{
	string dir(strip_trailing(path, '/'));
	string::size_type i;

	if ((i = dir.rfind('/')) != string::npos) {
		dir.erase(i);
		if (rmdir(dir.c_str()) == 0) {
			Out::vrb("Removed directory '" + dir + "'");
			remove_parent_dir(dir);
		}
	}
}

