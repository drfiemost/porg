//=======================================================================
// opt.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "out.h"
#include <getopt.h>

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using namespace Porg;

static sort_t get_sort_type(string const& s);
static void help();
static void version();
static int to_size_unit(char* arg);
static string get_dir_name();
static void die_help(string const& msg = "");


namespace Porg
{
	bool Opt::s_all_pkgs = false;
	bool Opt::s_print_sizes = false;
	bool Opt::s_print_files = false;
	bool Opt::s_print_nfiles = false;
	bool Opt::s_print_totals = false;
	bool Opt::s_print_symlinks = false;
	bool Opt::s_print_no_pkg_name = false;
	bool Opt::s_remove_batch = false;
	bool Opt::s_log_append = false;
	bool Opt::s_log_ignore_errors = false;
	bool Opt::s_log_missing = false;
	bool Opt::s_reverse_sort = false;
	bool Opt::s_print_date = false;
	bool Opt::s_print_hour = false;
	sort_t Opt::s_sort_type = SORT_BY_NAME;
	int Opt::s_size_unit = HUMAN_READABLE;
	string Opt::s_log_pkg_name = string();
	Mode Opt::s_mode = MODE_LIST_PKGS;
	vector<string> Opt::s_args = vector<string>();
}


void Opt::init(int argc, char* argv[])
{
	if (argc == 1)
		die_help("No arguments provided");

	static Opt opt(argc, argv);
}


Opt::Opt(int argc, char* argv[])
:
	Porgrc()
{
	enum { OPT_LOG_MISSING = 1 };
	
	struct option opt[] = {
		// General options
		{ "help", 0, 0, 'h' },
		{ "version", 0, 0, 'V' },
		{ "logdir",	1, 0, 'L' },
		{ "verbose", 0, 0, 'v' },
		{ "all", 0, 0, 'a' },
	 	// List options
		{ "date", 0, 0, 'd' },
		{ "sort", 1, 0, 'S' },
		{ "block-size", 1, 0, 'b' },
		{ "kilobytes", 0, 0, 'k' },
		{ "size", 0, 0, 's' },
		{ "nfiles", 0, 0, 'F' },
		{ "files", 0, 0, 'f' },
		{ "reverse", 0, 0, 'R' },
		{ "total", 0, 0, 't' },
		{ "symlinks", 0, 0, 'y' },
		{ "no-package-name", 0, 0, 'z' },
		{ "info", 0, 0, 'i' },
		{ "query", 0, 0, 'q' },
		{ "configure-options", 0, 0, 'o' },
		// Remove options
		{ "remove", 0, 0, 'r' },
		{ "batch", 0, 0, 'B' },
		{ "skip", 1, 0, 'e' },
		{ "unlog", 0, 0, 'U' },
		// Log options
		{ "log", 0, 0, 'l' },
		{ "package", 1, 0, 'p' },
		{ "include", 1, 0, 'I' },
		{ "exclude", 1, 0, 'E' },
		{ "append", 0, 0, '+' },
		{ "dirname", 0, 0, 'D' },
		{ "ignore-errors", 0, 0, 'g' },
		{ "log-missing", 0, 0, OPT_LOG_MISSING },
		{ NULL ,0, 0, 0 },
	};
	
	// Deal with the weird non-getopt-friendly '-p+' option
	// (convert it to '-+p')
	
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1] != '-') {
			for (char* p = argv[i]; (p = strstr(p, "p+")); )
				memcpy(p, "+p", 2);
		}
    }

	// get opt chars
	
	string optchars;
	
	for (uint i(0); opt[i].name; ++i) {
		optchars += (char)opt[i].val;
		if (opt[i].has_arg)
			optchars += ':';
	}
	
	// parse options
	
	int op;
    while ((op = getopt_long(argc, argv, optchars.c_str(), opt, 0)) >= 0) {
        
		switch (op) {
			
			// General options
			case 'V': version(); break;
			case 'h': help(); break;
			case 'L': s_logdir = optarg; break;
			case 'v': Out::inc_verbosity(); break;
			case 'a': s_all_pkgs = true; break;

			// List options
			case 'i': set_mode(MODE_INFO, op); break;
			case 'o': set_mode(MODE_CONF_OPTS, op); break;
			case 'q': set_mode(MODE_QUERY, op); break;
			case 'S': s_sort_type = get_sort_type(optarg); break;
			case 'R': s_reverse_sort = true; break;
			case 't': s_print_totals = true; break;
			case 's': s_print_sizes = true; break;
			case 'b': s_size_unit = to_size_unit(optarg); break;
			case 'k': s_size_unit = KILOBYTE; break;
			
			// Package list options
			case 'd': case 'F':
				set_mode(MODE_LIST_PKGS, op);
				switch (op) {
					case 'd':
						s_print_hour = s_print_date; 
						s_print_date = true;
						break;
					case 'F': s_print_nfiles = true; break;
				}
				break;

			// File list options
			case 'f': case 'y': case 'z':
				set_mode(MODE_LIST_FILES, op);
				switch (op) {
					case 'f': s_print_files = true; break;
					case 'y': s_print_symlinks = true; break;
					case 'z': s_print_no_pkg_name = true; break;
				}
				break;
			
			// Remove / unlog options
			case 'U': set_mode(MODE_UNLOG, op); break;
			case 'r': case 'e': case 'B':
				set_mode(MODE_REMOVE, op);
				switch (op) {
					case 'e': s_remove_skip = optarg; break;
					case 'B': s_remove_batch = true; break;
				}
				break;
			
			// Log options
			case 'l': case 'p': case 'D': case 'I': case 'E': case '+': case 'g':
			case OPT_LOG_MISSING:
				set_mode(MODE_LOG, op);
				switch (op) {
					case 'p': s_log_pkg_name = optarg; break;
					case 'D': s_log_pkg_name = get_dir_name(); break;
					case 'I': s_include = optarg; break;
					case 'E': s_exclude = optarg; break;
					case '+': s_log_append = true; break;
					case 'g': s_log_ignore_errors = true; break;
					case OPT_LOG_MISSING: s_log_missing = true; break;
				}
				break;
			
			default: die_help();
		}
	}

	if (!(s_print_sizes || s_print_nfiles))
		s_print_totals = false;

	s_args.assign(argv + optind, argv + argc);

	if (s_args.empty()) {
		if (s_mode == MODE_QUERY)
			die_help("No input files");
		else if ((!s_all_pkgs && s_mode != MODE_LOG) || s_mode == MODE_REMOVE || s_mode == MODE_UNLOG)
			die_help("No input packages");
	}

	if (!logdir_writable()) {
		if (s_mode == MODE_REMOVE || s_mode == MODE_UNLOG)
			throw Error(s_logdir, errno);
		else if (s_mode == MODE_LOG && !s_log_pkg_name.empty()) {
			if (errno != ENOENT || mkdir(s_logdir.c_str(), 0755) < 0)
				throw Error(s_logdir, errno);
		}
	}
}


void Opt::set_mode(Mode m, char optchar)
{
	static char modes[NMODES] = { 0 };
	string optstr("-");

	modes[m] = optchar;

	for (int i = 0; i < NMODES; ++i) {
		if (modes[i]) {
			optstr += modes[i];
			if (optstr.size() > 2)
				die_help(optstr + ": Incompatible options");
		}
	}

	s_mode = m;
}


static void help()
{
cout <<
"porg - a source code package organizer\n\n"
"Usage:\n"
"  porg [OPTIONS] <packages|files|command>\n\n"
"General options:\n"
"  -L, --logdir=DIR         Use DIR as the log directory.\n"
"  -a, --all                Apply to all logged packages (not with -r or -U).\n"
"  -v, --verbose            Verbose output (-vv produces debugging messages).\n"
"  -h, --help               Display this help message.\n"
"  -V, --version            Display version information.\n\n"
"List options:\n"
"  -b, --block-size=SIZE    Use blocks of SIZE bytes for the sizes.\n"
"  -k, --kilobytes          Like '--block-size=1024'.\n"
"  -R, --reverse            Reverse order while sorting.\n"
"  -S, --sort=WORD          Sort by WORD: 'name', 'date', 'size' or 'files'.\n"
"  -F, --nfiles             Print the number of installed files.\n"
"  -d, --date               Print the installation day (-dd prints the hour too).\n"
"  -s, --size               Print the installed size of each packagei or file.\n"
"  -f, --files              List installed files.\n"
"  -z, --no-package-name    Don't print the name of the package (with -f).\n"
"  -t, --total              Print totals.\n"
"  -y, --symlinks           Print the contents of symbolic links (with -f).\n\n"
"Information options:\n"
"  Note: Information may be not available for all packages.\n"
"  -i, --info               Print package information.\n"
"  -o, --configure-options  Print the options passed to configure when the\n"
"                           package was installed.\n"
"  -q, --query              Query for the packages that own one or more files.\n\n"
"Remove options:\n"
"  -r, --remove             Remove the (non shared) files of the package.\n"
"  -B, --batch              Do not ask for confirmation when removing.\n"
"  -e, --skip=PATH:...      Do not remove files in PATHs (see the man page).\n"
"  -U, --unlog              Unlog the package, without removing any file.\n\n"
"Log options:\n"
"  -l, --log                Enable log mode. See the man page.\n"
"  -p, --package=PKG        Name of the package to log.\n" 
"  -D, --dirname            Use the name of the current directory as the name\n"
"                           of the package.\n"
"  -+, --append             With -p or -D: If the package is already logged,\n"
"                           append the list of files to its log.\n"
"  -g, --ignore-errors      Do not exit if the install command fails.\n"
"      --log_missing        Do not skip missing files.\n"
"  -I, --include=PATH:...   List of paths to scan.\n"
"  -E, --exclude=PATH:...   List of paths to skip.\n\n"
"Note: The package list mode is enabled by default.\n\n"
"Send bugs to: David Ricart <" PACKAGE_BUGREPORT ">\n";

	exit(EXIT_SUCCESS);
}


static void version()
{
	cout << "porg-" PACKAGE_VERSION "  (" RELEASEDATE ")\n"
		"Copyright (C) David Ricart <" PACKAGE_BUGREPORT ">\n";

	exit(EXIT_SUCCESS);
}


static string get_dir_name()
{
	char dirname[4096];

	if (!getcwd(dirname, sizeof(dirname)))
		throw Error("getcwd()", errno);

	return strrchr(dirname, '/') + 1;
}


// 
// Process the '--block-size=SIZE' option
//
static int to_size_unit(char* arg)
{
	int i = -1, b = 0, unit;
	
	while (isdigit(arg[++i])) ;

	switch (arg[i]) {
		case 'k': case 'K': b = KILOBYTE; break;
		case 'm': case 'M': b = MEGABYTE; break;
		case 'b': case 'B': case 0: b = 1; break;
		default: throw Error(string(arg) + ": Invalid block size");
	}
	
	if ((unit = i ? (Porg::str2num<int>(arg) * b) : b))
		return unit ? unit : HUMAN_READABLE;

	throw Error(string(arg) + ": Invalid block size");
}


//
// Process the '--sort=WORD' option
//
static sort_t get_sort_type(string const& s)
{
	if (!s.compare(0, s.size(), "size", s.size()))
		return SORT_BY_SIZE;
	
	else if (!s.compare(0, s.size(), "date", s.size()))
		return SORT_BY_DATE;
	
	else if (!s.compare(0, s.size(), "files", s.size()))
		return SORT_BY_NFILES;
	
	else if (!s.compare(0, s.size(), "name", s.size()))
		return SORT_BY_NAME;

	throw Error(string("Invalid argument '") + s + "' for option '-S|--sort'.\n"
		"Valid arguments are:\n"
		"  - 'name'\n"
		"  - 'size'\n"
		"  - 'date'\n"
		"  - 'files'\n");
}


static void die_help(string const& msg /* = "" */)
{
	string out(msg);
	if (!out.empty()) {
		out.insert(0, "porg: ");
		out += '\n';
	}
	cerr << out << "Try 'porg --help' for more information\n";
	exit(EXIT_FAILURE);
}


