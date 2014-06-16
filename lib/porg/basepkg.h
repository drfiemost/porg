//=======================================================================
// basepkg.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef LIBPORG_BASEPKG_H
#define LIBPORG_BASEPKG_H

#include "config.h"
#include "common.h"
#include <iosfwd>
#include <vector>


namespace Porg {

class File;

class BasePkg
{
	public:

	typedef std::vector<File*>::iterator 		iter;
	typedef std::vector<File*>::const_iterator 	const_iter;

	// codes used to identify fields in the header of log files
	
	static char const CODE_DATE			= 't';
	static char const CODE_SIZE			= 's';
	static char const CODE_NFILES		= 'f';
	static char const CODE_CONF_OPTS	= 'c';
	static char const CODE_ICON_PATH	= 'i';
	static char const CODE_SUMMARY		= 'S';
	static char const CODE_URL			= 'u';
	static char const CODE_LICENSE		= 'l';
	static char const CODE_AUTHOR		= 'a';
	static char const CODE_DESCRIPTION	= 'd';

	BasePkg(std::string const& name_, bool logged = true);
	
	virtual ~BasePkg();

	std::vector<File*> const& files() const	{ return m_files; }
	int date() const						{ return m_date; }
	ulong size() const						{ return m_size; }
	ulong nfiles() const					{ return m_nfiles; }
	std::string const& name() const			{ return m_name; }
	std::string const& log() const			{ return m_log; }
	std::string const& base_name() const	{ return m_base_name; }
	std::string const& version() const		{ return m_version; }
	std::string const& icon_path() const	{ return m_icon_path; }
	std::string const& url() const			{ return m_url; }
	std::string const& license() const		{ return m_license; }
	std::string const& summary() const		{ return m_summary; }
	std::string const& description() const	{ return m_description; }
	std::string const& conf_opts() const	{ return m_conf_opts; }
	std::string const& author() const		{ return m_author; }

	bool has_file(File*) const;
	bool has_file(std::string const& path) const;
	void get_files();
	virtual void unlog() const;
	
	static std::string get_base(std::string const& name);
	static std::string get_version(std::string const& name);

	template <typename T>	// T = {Porg,Grop}::Pkg
	bool is_shared(File* file, std::vector<T*> const& pkgs) const
	{
		for (typename std::vector<T*>::const_iterator p(pkgs.begin()); p != pkgs.end(); ++p) {
			if ((*p)->name() != m_name && (*p)->has_file(file))
				return true;
		}
		return false;
	}


	protected:

	void sort_files(sort_t type = SORT_BY_NAME, bool reverse = false);

	std::vector<File*> m_files;
	std::string const m_name;
	std::string const m_log;
	std::string const m_base_name;
	std::string const m_version;
	int m_date;
	ulong m_size;
	ulong m_nfiles;
	std::string m_icon_path;
	std::string m_url;
	std::string m_license;
	std::string m_summary;
	std::string m_description;
	std::string m_conf_opts;
	std::string m_author;


	class Sorter
	{
		public:

		Sorter(sort_t type = SORT_BY_NAME);
		bool operator()(File* left, File* right) const;

		private:

		bool (Sorter::*m_sort_func)(File* , File*) const;
		bool sort_by_name(File* left, File* right) const;
		bool sort_by_size(File* left, File* right) const;

	};	// class BasePkg::Sorter

};	// class BasePkg

}	// namespace Porg


#endif  // LIBPORG_BASEPKG_H
