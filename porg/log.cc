//=======================================================================
// log.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "out.h"
#include "opt.h"
#include "porg/common.h"	// in_paths()
#include "util.h"
#include "info.h"
#include "pkg.h"
#include "log.h"
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <glob.h>
#include <sys/wait.h>

using namespace Porg;
using namespace std;

static string search_libporg();
static void set_env(char const* var, string const& val);


Log::Log()
:
	m_package(Opt::log_pkg_name()),
	m_tmpfile(),
	m_files()
{
	if (Opt::args().empty())
		read_files_from_stream(cin);
	else
		read_files_from_command();

	filter_files();

	if (m_package.empty())
		write_files_to_stream(cout);
	else
		write_files_to_pkg();
}


void Log::run()
{
	static Log log;
}


void Log::write_files_to_pkg() const
{
	bool done(false);

	if (Opt::log_append()) {
		try 
		{
			Pkg already_logged_pkg(m_package);
			already_logged_pkg.append(m_files);
			done = true;
		}
		catch (...) { }
	}

	if (!done) {
		Pkg pkg(m_package, m_files);
		Info info(pkg);
		if (Out::debug())
			pkg.print_info_dbg();
		pkg.write_log();
	}

	if (Out::debug()) {
		Out::dbg_title("logged files");
		write_files_to_stream(cerr);
		Out::dbg_title();
	}
}


void Log::write_files_to_stream(ostream& s) const
{
	copy(m_files.begin(), m_files.end(), ostream_iterator<string>(s, "\n"));
}


void Log::read_files_from_stream(istream& f)
{
	for (string buf; getline(f, buf); m_files.insert(buf)) ;
}


void Log::read_files_from_command()
{
	try
	{
		do_read_files_from_command();
	}
	catch (...)
	{
		unlink(m_tmpfile.c_str());
		throw;
	}
}


void Log::do_read_files_from_command()
{
	get_tmpfile();
	
	pid_t pid = fork();

	if (pid == 0) { // child

		string command, libporg = search_libporg();
		
		for (uint i(0); i < Opt::args().size(); ++i)
			command += Opt::args()[i] + " ";
		
		set_env("LD_PRELOAD", libporg);
		set_env("PORG_TMPFILE", m_tmpfile);
		if (Out::debug())
			set_env("PORG_DEBUG", "yes");

		Out::dbg_title("settings");
		Out::dbg("LD_PRELOAD: " + libporg); 
		Out::dbg("include: " + Opt::include()); 
		Out::dbg("exclude: " + Opt::exclude()); 
		Out::dbg("command: " + command);
		Out::dbg_title("libporg-log");

		char* cmd[] = { (char*)"sh", (char*)"-c", (char*)(command.c_str()), 0 };
		execv("/bin/sh", cmd);

		throw Error("execv()", errno);
	}

	else if (pid == -1)
		throw Error("fork()", errno);

	wait(0);

	FileStream<ifstream> f(m_tmpfile);
	read_files_from_stream(f);
	
	unlink(m_tmpfile.c_str());
}


void Log::get_tmpfile()
{
	char* tmpdir = getenv("TMPDIR");
	char name[4096];

	snprintf(name, sizeof(name), "%s/porgXXXXXX", tmpdir ? tmpdir : "/tmp");
	
	if (close(mkstemp(name)) < 0)
		snprintf(name, sizeof(name), "/tmp/porg%d", getpid());

	m_tmpfile = name;
}


//
// Convert input files to absolute paths, skip excluded or not included
// files, and skip non-regular or missing files.
//
void Log::filter_files()
{
	vector<string> filtered;
	struct stat s;
	
	for (set<string>::iterator p = m_files.begin(); p != m_files.end(); ++p) {

		if ((*p).empty())
			continue;

		// get absolute path
		string path(clear_path((*p)));

		// skip excluded or not included files
		if (in_paths(path, Opt::exclude()) || !in_paths(path, Opt::include()))
			continue;
	
		// skip missing files, if needed
		else if (lstat(path.c_str(), &s) && !Opt::log_missing())
			continue;

		// log only regular files or symlinks
		else if (s.st_mode & (S_IFREG | S_IFLNK))
			filtered.push_back(path);
	}

	m_files.clear();
	copy(filtered.begin(), filtered.end(), inserter(m_files, m_files.begin()));
}


//
// Search for libporg-log.so in the filesystem.
//
static string search_libporg()
{
	string libpath(LIBDIR "/libporg-log.so");
	struct stat s;
	
	if (!stat(libpath.c_str(), &s))
		return libpath;
	
	glob_t g;
	memset(&g, 0, sizeof(g));
	
	if (!glob(LIBDIR "/libporg-log.so.[0-9]*", GLOB_NOSORT, 0, &g) && g.gl_pathc)
		libpath = g.gl_pathv[0];
	
	globfree(&g);
	
	return libpath;
}


static void set_env(char const* var, string const& val)
{
	if (setenv(var, val.c_str(), 1) < 0)
		throw Error(string("setenv('") + var + "', '" + val + "', 1)", errno);
}

