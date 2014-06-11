//=======================================================================
// opt.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
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

static void help();
static void version();
static string get_dir_name();
static void die_help(string const& msg = "");
static string to_lower(string const& str);


namespace Porg
{
	bool Opt::s_all_pkgs = false;
	bool Opt::s_exact_version = false;
	bool Opt::s_print_sizes = false;
	bool Opt::s_print_nfiles = false;
	bool Opt::s_print_totals = false;
	bool Opt::s_print_symlinks = false;
	bool Opt::s_print_no_pkg_name = false;
	bool Opt::s_remove_batch = false;
	bool Opt::s_remove_unlog = false;
	bool Opt::s_log_append = false;
	bool Opt::s_log_missing = false;
	bool Opt::s_reverse_sort = false;
	bool Opt::s_print_date = false;
	bool Opt::s_print_hour = false;
	sort_t Opt::s_sort_type = SORT_BY_NAME;
	string Opt::s_log_pkg_name = string();
	int Opt::s_mode = MODE_NONE;
	vector<string> Opt::s_args = vector<string>();
	char Opt::s_mode_char = 0;
}


void Opt::init(int argc, char* argv[])
{
	if (argc == 1)
		die_help("No arguments provided");

	static Opt opt(argc, argv);
}


Opt::Opt(int argc, char* argv[])
:
	BaseOpt()
{
	enum {
		OPT_ALL				= 'a',
		OPT_BATCH			= 'b',
		OPT_DIRNAME			= 'D',
		OPT_DATE			= 'd',
		OPT_EXCLUDE			= 'E',
		OPT_SKIP			= 'e',
		OPT_NFILES			= 'F',
		OPT_FILES			= 'f',
		OPT_HELP 			= 'h',
		OPT_INCLUDE			= 'I',
		OPT_INFO			= 'i',
		OPT_LOGDIR			= 'L',
		OPT_LOG				= 'l',
		OPT_LOG_MISSING		= 1,	// no short opt.
		OPT_CONF_OPTS		= 'o',
		OPT_PACKAGE			= 'p',
		OPT_QUERY			= 'q',
		OPT_REVERSE			= 'R',
		OPT_REMOVE			= 'r',
		OPT_SORT			= 'S',
		OPT_SIZE			= 's',
		OPT_TOTAL			= 't',
		OPT_UNLOG			= 'U',
		OPT_VERSION			= 'V',
		OPT_VERBOSE			= 'v',
		OPT_EXACT_VERSION	= 'x',
		OPT_EXACT_VERSION_2	= 'X',
		OPT_SYMLINKS		= 'y',
		OPT_NO_PACKAGE_NAME	= 'z',
		OPT_APPEND			= '+'
	};

	struct option opt[] = {
		// General options
		{ "help", 				0, 0, OPT_HELP },
		{ "version", 			0, 0, OPT_VERSION },
		{ "logdir",				1, 0, OPT_LOGDIR },
		{ "verbose", 			0, 0, OPT_VERBOSE },
		{ "exact-version", 		0, 0, OPT_EXACT_VERSION },
		{ "all", 				0, 0, OPT_ALL },
	 	// List options
		{ "date", 				0, 0, OPT_DATE },
		{ "sort", 				1, 0, OPT_SORT },
		{ "size", 				0, 0, OPT_SIZE },
		{ "nfiles", 			0, 0, OPT_NFILES },
		{ "files", 				0, 0, OPT_FILES },
		{ "reverse", 			0, 0, OPT_REVERSE },
		{ "total", 				0, 0, OPT_TOTAL },
		{ "symlinks", 			0, 0, OPT_SYMLINKS },
		{ "no-package-name", 	0, 0, OPT_NO_PACKAGE_NAME },
		{ "info", 				0, 0, OPT_INFO },
		{ "query", 				0, 0, OPT_QUERY },
		{ "configure-options", 	0, 0, OPT_CONF_OPTS },
		// Remove options
		{ "remove", 			0, 0, OPT_REMOVE },
		{ "batch", 				0, 0, OPT_BATCH },
		{ "skip", 				1, 0, OPT_SKIP },
		{ "unlog", 				0, 0, OPT_UNLOG },
		// Log options
		{ "log", 				0, 0, OPT_LOG },
		{ "package", 			1, 0, OPT_PACKAGE },
		{ "include", 			1, 0, OPT_INCLUDE },
		{ "exclude", 			1, 0, OPT_EXCLUDE },
		{ "append", 			0, 0, OPT_APPEND },
		{ "dirname", 			0, 0, OPT_DIRNAME },
		{ "log-missing", 		0, 0, OPT_LOG_MISSING },
		{ 0 ,0, 0, 0 },
	};
	
	// Deal with the weird non-getopt-friendly '-p+' option
	// (convert it to '-+p')
	
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1] != '-') {
			for (char* p = argv[i]; (p = strstr(p, "p+")); )
				memcpy(p, "+p", 2);
		}
    }

	// build optstring for getopt_long()
	
	string optstring;
	
	for (uint i(0); opt[i].name; ++i) {
		optstring += (char)opt[i].val;
		if (opt[i].has_arg)
			optstring += ':';
	}
	
	int c;

	// First pass of getopt, to get general options and mode of operation

	while ((c = getopt_long(argc, argv, optstring.c_str(), opt, 0)) >= 0) {
		switch (c) {
			// general opts
			case OPT_VERSION: 	version(); break;
			case OPT_HELP: 		help(); break;
			case OPT_LOGDIR: 	s_logdir = optarg; break;
			case OPT_VERBOSE: 	Out::inc_verbosity(); break;
			// mode switches
			case OPT_INFO: 		set_mode(MODE_INFO, c); break;
			case OPT_CONF_OPTS:	set_mode(MODE_CONF_OPTS, c); break;
			case OPT_QUERY: 	set_mode(MODE_QUERY, c); break;
			case OPT_FILES: 	set_mode(MODE_LIST_FILES, c); break;
			case OPT_REMOVE: 	set_mode(MODE_REMOVE, c); break;
			case OPT_LOG: 		set_mode(MODE_LOG, c); break;
			// unrecognized option
			case '?':	die_help();
		}
	}

	// Second pass of getopt

	optind = 1;

    while ((c = getopt_long(argc, argv, optstring.c_str(), opt, 0)) >= 0) {
        
		switch (c) {
			
			case OPT_ALL: 
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS | MODE_INFO
					| MODE_CONF_OPTS, c);
				s_all_pkgs = true; 
				break;

			case OPT_EXACT_VERSION: case OPT_EXACT_VERSION_2:
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS | MODE_INFO
					| MODE_CONF_OPTS | MODE_REMOVE, c);
				s_exact_version = true; 
				break;

			case OPT_SORT:
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS, c);
				set_sort_type(optarg); 
				break;

			case OPT_REVERSE:
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS, c);
				s_reverse_sort = true; 
				break;

			case OPT_TOTAL:
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS, c);
				s_print_totals = true; 
				break;

			case OPT_SIZE:
				check_mode(MODE_LIST_FILES | MODE_LIST_PKGS, c);
				s_print_sizes = true;
				break;

			case OPT_DATE:
				check_mode(MODE_LIST_PKGS, c);
				s_print_hour = s_print_date; 
				s_print_date = true;
				break;

			case OPT_NFILES:
				check_mode(MODE_LIST_PKGS, c);
				s_print_nfiles = true; 
				break;

			case OPT_SYMLINKS:
				check_mode(MODE_LIST_FILES, c);
				s_print_symlinks = true;
				break;

			case OPT_NO_PACKAGE_NAME:
				check_mode(MODE_LIST_FILES, c);
				s_print_no_pkg_name = true;
				break;
			
			case OPT_SKIP:
				check_mode(MODE_REMOVE, c);
				s_remove_skip = optarg; 
				break;

			case OPT_BATCH:
				check_mode(MODE_REMOVE, c);
				s_remove_batch = true;
				break;

			case OPT_UNLOG:
				check_mode(MODE_REMOVE, c);
				s_remove_unlog = true;
				break;

			case OPT_PACKAGE:
				check_mode(MODE_LOG, c);
				s_log_pkg_name = to_lower(optarg); 
				break;

			case OPT_DIRNAME:
				check_mode(MODE_LOG, c);
				s_log_pkg_name = to_lower(get_dir_name());
				break;

			case OPT_INCLUDE:
				check_mode(MODE_LOG, c);
				s_include = optarg;
				break;

			case OPT_EXCLUDE:
				check_mode(MODE_LOG, c);
				s_exclude = optarg;
				break;

			case OPT_APPEND:
				check_mode(MODE_LOG, c);
				s_log_append = true;
				break;

			case OPT_LOG_MISSING:
				//check_mode(MODE_LOG, c);
				s_log_missing = true;
				break;
		}
	}

	// save non-option command line arguments into s_args
	s_args.assign(argv + optind, argv + argc);

	// Checkings

	check_logdir();

	switch (s_mode) {

		case MODE_QUERY:
			if (s_args.empty())
				die_help("No input files");
			break;

		case MODE_LOG:
			break;

		case MODE_NONE:
			s_mode = MODE_LIST_PKGS;
			// no break here

		default:
			if (!(s_print_sizes || s_print_nfiles))
				s_print_totals = false;

			if (s_args.empty() && !s_all_pkgs)
				die_help("No input packages");

			// convert package names to lower case
			for (uint i(0); i < s_args.size(); ++i)
				s_args[i] = to_lower(s_args[i]);
	}
}


void Opt::check_mode(int modes, char optchar)
{
	if (s_mode != MODE_NONE && !(s_mode & modes))
		die_help(string("-") + s_mode_char + optchar + ": Incompatible options");
}


void Opt::set_mode(int mode, char optchar)
{
	if (s_mode_char)
		die_help(string("-") + s_mode_char + optchar + ": Incompatible options");
	
	s_mode_char = optchar;
	s_mode = mode;
}


void Opt::set_sort_type(string const& s)
{
	if (!s.compare(0, s.size(), "size", s.size()))
		s_sort_type = SORT_BY_SIZE;
	else if (!s.compare(0, s.size(), "date", s.size()))
		s_sort_type = SORT_BY_DATE;
	else if (!s.compare(0, s.size(), "files", s.size()))
		s_sort_type = SORT_BY_NFILES;
	else if (!s.compare(0, s.size(), "name", s.size()))
		s_sort_type = SORT_BY_NAME;
	else
		die_help("'" + s + "': Invalid argument for option '-S|--sort'");
}


static void help()
{
cout <<
"porg - a source code package organizer\n\n"
"Usage:\n"
"  porg [OPTIONS] <packages|files|command>\n\n"
"General options:\n"
"  -L, --logdir=DIR         Use DIR as the log directory.\n"
"  -v, --verbose            Verbose output (-vv produces debugging messages).\n"
"  -x, --exact-version      Do not expand version of packages given as arguments.\n"
"  -h, --help               Display this help message.\n"
"  -V, --version            Display version information.\n\n"
"Package information options:\n"
"  -a, --all                Apply to all logged packages (not with -r or -U).\n"
"  -s, --size               Print the installed size of each package or file.\n"
"  -d, --date               Print the installation day (-dd prints the hour too).\n"
"  -F, --nfiles             Print the number of installed files.\n"
"  -S, --sort=WORD          Sort by WORD: 'name', 'date', 'size' or 'files'.\n"
"  -R, --reverse            Reverse order while sorting.\n"
"  -t, --total              Print totals.\n"
"  -f, --files              List installed files.\n"
"  -z, --no-package-name    Don't print the name of the package (with -f).\n"
"  -y, --symlinks           Print the contents of symbolic links (with -f).\n"
"  -i, --info               Print package information.\n"
"  -o, --configure-options  Print the arguments passed to configure when the\n"
"                           package was installed.\n"
"  -q, --query              Query for the packages that own one or more files.\n\n"
"Remove options:\n"
"  -r, --remove             Remove the (non shared) files of the package.\n"
"  -b, --batch              Do not ask for confirmation when removing.\n"
"  -e, --skip=PATH:...      Do not remove files in PATHs (see the man page).\n"
"  -U, --unlog              Unlog the package, without removing any file.\n\n"
"Log options:\n"
"  -l, --log                Enable log mode. See the man page.\n"
"  -p, --package=PKG        Name of the package to be logged.\n" 
"  -D, --dirname            Use the name of the current directory as the name\n"
"                           of the package.\n"
"  -+, --append             With -p or -D: If the package is already logged,\n"
"                           append the list of files to its log.\n"
"      --log-missing        Do not skip missing files.\n"
"  -I, --include=PATH:...   List of paths to scan.\n"
"  -E, --exclude=PATH:...   List of paths to skip.\n\n"
"Note: The package list mode is enabled by default.\n\n"
"Written by David Ricart <" PACKAGE_BUGREPORT ">\n";

	exit(EXIT_SUCCESS);
}


static void version()
{
	cout << "porg-" PACKAGE_VERSION " (" RELEASEDATE ")\n"
		"Written by David Ricart <" PACKAGE_BUGREPORT ">\n";

	exit(EXIT_SUCCESS);
}


static string get_dir_name()
{
	char dirname[4096];

	if (!getcwd(dirname, sizeof(dirname)))
		throw Error("getcwd()", errno);

	return strrchr(dirname, '/') + 1;
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


//
// convert a string to lowercase
//
static string to_lower(string const& str)
{
	string low(str);

	for (uint i(0); i < low.size(); ++i)
		low[i] = tolower(low[i]);

	return low;
}

