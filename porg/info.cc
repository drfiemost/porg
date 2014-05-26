//=======================================================================
// info.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porg/file.h"
#include "porg/regexp.h"
#include "out.h"
#include "info.h"
#include "pkg.h"
#include <fstream>
#include <string>
#include <glob.h>

using std::string;
using namespace Porg;


Info::Info(Pkg* pkg)
:
	m_pkg(pkg)
{
	assert(pkg != 0);

	Out::dbg_title("package information");

	get_info_config_log();
	get_info_pc();
	get_info_spec();
	get_info_desktop();

	get_icon_path();
}


void Info::get_info_spec()
{
	string spec(search_file(m_pkg->m_base_name + ".spec"));
	if (spec.empty())
		return;

	get_var(spec, "Icon", m_pkg->m_icon_path);
	get_var(spec, "Summary", m_pkg->m_summary);
	get_var(spec, "URL", m_pkg->m_url);
	get_var(spec, "Vendor", m_pkg->m_author);
	get_var(spec, "Packager", m_pkg->m_author);
	get_var(spec, "Copyright", m_pkg->m_license);
	get_var(spec, "License", m_pkg->m_license);

	// get description field

	std::ifstream f(spec.c_str());
	if (!f)
		return;
	
	string buf, desc;

	while (getline(f, buf) && buf.find("%description") != 0) ;
	if (f.eof())
		return;
	
	while (getline(f, buf) && buf[0] != '%' && buf[0] != '#')
		desc += buf + '\n';

	if (desc.size() > m_pkg->m_description.size())
		m_pkg->m_description = desc;
}


void Info::get_info_pc()
{
	string pc(search_file(m_pkg->m_base_name + ".pc"));
	if (pc.empty())
		return;

	get_var(pc, "Description", m_pkg->m_summary);
	get_var(pc, "URL", m_pkg->m_url);
}


void Info::get_info_desktop()
{
	string desktop(search_file(m_pkg->m_base_name + ".desktop"));
	if (desktop.empty())
		return;

	get_var(desktop, "Icon", m_pkg->m_icon_path);
	get_var(desktop, "GenericName", m_pkg->m_summary);
	get_var(desktop, "Comment", m_pkg->m_summary);
}


void Info::get_info_config_log()
{
	string config("config.log");

	get_var(config, "PACKAGE_URL", m_pkg->m_url, false);
	get_var(config, "PACKAGE_BUGREPORT", m_pkg->m_author, false);
	get_var(config, "PACKAGE_STRING", m_pkg->m_summary, false);
	
	// get configure options

	std::ifstream f(config.c_str());
	if (!f)
		return;

	Regexp re("\\$[ \\t].*/configure[ \\t]+(.*)$");

	for (string buf; getline(f, buf); ) {
		if (re.exec(buf)) {
			m_pkg->m_conf_opts = re.match(1);
			break;
		}
	}
}


string Info::search_file(string const& name) const
{
	Out::dbg("searching for " + name);
	
	glob_t g;
	memset(&g, 0, sizeof(g));
	
	string file;
	string patt[3] = { name, "*/" + name, "*/*/" + name };

	for (int i = 0; i < 3 && file.empty(); ++i) {
		if (0 == glob(patt[i].c_str(), 0, 0, &g) && g.gl_pathc)
			file = g.gl_pathv[0];
	}

	globfree(&g);

	if (file.empty())
		Out::dbg("\t(not found)\n", false);
	else
		Out::dbg("\t" + file + "\n", false);
		
	return file;
}


bool Info::get_var(string const& file, string const& tag,
                   string& val, bool icase /* = false */) const
{
	std::ifstream f(file.c_str());
	if (!f)
		return false;
		
	Regexp re(tag + "[^[:alnum:]]+([[:alnum:]].*[^\"'])", REG_ICASE & icase);

	for (string buf; getline(f, buf); ) {
		if (re.exec(buf)) {
			val = re.match(1);
			return true;
		}
	}

	return false;
}


void Info::get_icon_path()
{
	string& path(m_pkg->m_icon_path);

	if (path.empty())
		path = m_pkg->m_base_name;
	
	// If it's an absolute path, we're done	
	else if (path[0] == '/')
		return;

	// otherwise search for the icon file in the list of files installed by
	// the package
	
	// if path does not have any image format suffix, add 'suf' to the expression
	
	string exp("/" + path);
	string suf("\\.(png|xpm|jpg|ico|gif|svg)$");
	Regexp re1(suf, REG_ICASE);
	
	if (!re1.exec(path))
		exp += suf;

	// Search the logged files for the path of the icon
	
	Regexp re2(exp, REG_ICASE);

	for (uint i(0); i < m_pkg->m_files.size(); ++i) {
		if (re2.exec(m_pkg->m_files[i]->name())) {
			path = m_pkg->m_files[i]->name();
			if (0 == access(path.c_str(), F_OK))
				break;
		}
	}
}

