//=======================================================================
// info.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porg/file.h"
#include "out.h"
#include "info.h"
#include "pkg.h"
#if HAVE_REGEX_H
#	include "regexp.h"
#endif
#include <fstream>
#include <string>
#include <glob.h>

using std::string;
using std::vector;
using std::set;
using namespace Porg;


Info::Info(Pkg* pkg)
:
	m_defs(),
	m_dirs(),
	m_pkg(pkg)
{
	Out::dbg_title("information");

	get_dirs();

	get_info_spec();
	get_info_pc();
	get_info_desktop();

	get_icon_path();
	get_config_opts();
}


Info::Define::Define(char const fmt, string const& var, string const& val)
:
	m_fmt(fmt),
	m_var(var),
	m_val(val)
{ }


void Info::Define::resolve(string& str) const
{
	if (str.find(m_fmt) == string::npos)
		return;
			
	string::size_type p;
	string var0(1, m_fmt), var1(1, m_fmt);
	
	var0 += m_var;
	var1 += "{" + m_var + "}";

	for (p = 0; (p = str.find(var0, p)) != string::npos; )
		str.replace(p, var0.size(), m_val);

	for (p = 0; (p = str.find(var1, p)) != string::npos; )
		str.replace(p, var1.size(), m_val);
}


string Info::resolve_defines(string const& str) const
{
	string ret(str);

	for (uint i = 0; i < m_defs.size(); ++i)
		m_defs[i].resolve(ret);
	
	return ret;
}


void Info::get_defs_spec(string const& spec)
{
	std::ifstream f(spec.c_str());
	if (!f)
		return;
		
	char* var;
	char* p;
	char* val;
	char buf[8192];

	m_defs.clear();

	while (!f.getline(buf, sizeof(buf)).eof()) {
		if ((p = strtok(buf, " \t")) && !strcmp(p, "%define")
		&& (var = strtok(NULL, " \t\n"))
		&& (val = strtok(NULL, " \t\n")) && val[0] != '%')
			m_defs.push_back(Define(Define::FMT_SPEC, var, val));
	}
}


void Info::get_defs_pc(string const& pc)
{
	std::ifstream f(pc.c_str());
	if (!f)
		return;
		
	string buf;
	string::size_type p;

	m_defs.clear();

	while (getline(f, buf)) {
		if ((p = buf.find("=")) != string::npos && buf.size() > p)
			m_defs.push_back
				(Define(Define::FMT_PC, buf.substr(0, p), buf.substr(p + 1)));
	}
}


void Info::get_spec_desc(string const& spec)
{
	std::ifstream f(spec.c_str());
	if (!f)
		return;
	
	string buf, desc;

	while (getline(f, buf) && buf.find("%description") != 0) ;
	if (f.eof())
		return;
	
	while (getline(f, buf) && buf[0] != '%' && buf[0] != '#')
		desc += resolve_defines(buf) + '\n';

	if (desc.size() > m_pkg->m_description.size())
		m_pkg->m_description = desc;
}


void Info::get_info_spec()
{
	string spec(search_file(m_pkg->m_base_name + ".spec"));
	if (spec.empty())
		return;

	get_defs_spec(spec);

	get_var(spec, "Name", m_pkg->m_base_name);
	get_var(spec, "Version", m_pkg->m_version);
	get_var(spec, "Summary", m_pkg->m_summary);
	get_var(spec, "URL", m_pkg->m_url);

	if (!get_var(spec, "Vendor", m_pkg->m_author))
		get_var(spec, "Packager", m_pkg->m_author);
	
	if (!get_var(spec, "License", m_pkg->m_license))
		get_var(spec, "Copyright", m_pkg->m_license);

	get_spec_desc(spec);
}


void Info::get_info_desktop()
{
	string desktop(search_file(m_pkg->m_base_name + ".desktop"));
	if (desktop.empty())
		return;

	get_var(desktop, "Icon", m_pkg->m_icon_path, false);
	get_var(desktop, "Name", m_pkg->m_base_name, false);
	get_var(desktop, "GenericName", m_pkg->m_summary, false);

	string desc;
	get_var(desktop, "Comment", desc, false);
	if (desc.size() > m_pkg->m_description.size())
		m_pkg->m_description = desc;
}


void Info::get_info_pc()
{
	string pc(search_file(m_pkg->m_base_name + ".pc"));
	if (pc.empty())
		return;

	get_defs_pc(pc);
	
	get_var(pc, "Description",	m_pkg->m_summary);
	get_var(pc, "Name", 		m_pkg->m_base_name);
	get_var(pc, "Version",		m_pkg->m_version);
}


string Info::search_file(string const& name) const
{
	Out::dbg("searching " + name);
	
	glob_t g;
	memset(&g, 0, sizeof(g));
	
	string file, pat = "*/*/" + name;

	for (set<string>::iterator d(m_dirs.begin()); file.empty() && d != m_dirs.end(); ++d) {
		for (int k = 2; file.empty() && k >= 0; --k) {
			string s = *d + "/" + pat.substr(k << 1);
			if (!glob(s.c_str(), GLOB_NOSORT, 0, &g) && g.gl_pathc)
				file = g.gl_pathv[0];
		}
	}
	
	globfree(&g);

	if (file.empty())
		Out::dbg("\t(not found)\n", false);
	else
		Out::dbg("\t" + file + "\n", false);
		
	return file;
}


void Info::get_config_opts()
{
	std::ifstream f("config.log");
	if (!f)
		return;

	string buf;
	string::size_type p;

	while (getline(f, buf) && m_pkg->m_conf_opts.empty()) {
		if ((p = buf.find("$")) != string::npos
		&&	(p = buf.find("/configure", p)) != string::npos
		&&	(p = buf.find("-", p)) != string::npos)
			m_pkg->m_conf_opts = buf.substr(p);
	}
}


void Info::get_icon_path()
{
	string& path(m_pkg->m_icon_path);

	if (path.empty() && (path = m_pkg->m_base_name).empty())
		return;

	// If it's an absolute path, we're done
	
	if (path.at(0) == '/')
		return;

	// otherwise search for the icon file in the list of files installed by
	// the package
	
#if HAVE_REGEX_H

	string exp("/" + path), suf(".(png|xpm|jpg|ico|gif|svg)$");

	// if path does not have any image format suffix, add 'suf' to the expression
	
	Regexp re1(suf, true);
	if (!re1.run(path))
		exp += suf;

	// Search the logged files for the path of the icon
	
	path.clear();

	Regexp re2(exp, true);

	for (uint i(0); path.empty() && i < m_pkg->m_files.size(); ++i) {
		if (re2.run(m_pkg->m_files[i]->name()))
			path = m_pkg->m_files[i]->name();
	}
#endif
}


void Info::get_dirs()
{
	m_dirs.insert(".");

	string dir;
	
	if (get_build_var("top_srcdir", dir))
		m_dirs.insert(dir);

	if (get_build_var("top_builddir", dir))
		m_dirs.insert(dir);
}


bool Info::get_var(
		string const& file, 
		string const& tag, 
		string& val, 
		bool resolve /* = true */) const
{
	std::ifstream f(file.c_str());
	if (!f)
		return false;
		
	string buf;
	string::size_type p;
	bool found = false;

	while (!found && getline(f, buf)) {
		if (buf.find(tag) == 0
		&& (p = buf.find_first_not_of(" \t:=", tag.size())) != string::npos) {
			val = buf.substr(p);
			if (resolve)
				val = resolve_defines(val);
			found = true;
		}
	}

	return found;
}


//
// Extract the value of 'var' from Makefile, makefile or config.log
//
bool Info::get_build_var(string const& tag, string& var) const
{
	return get_var("Makefile", tag, var, false)
		|| get_var("makefile", tag, var, false)
		|| get_var("config.log", tag, var, false);
}


