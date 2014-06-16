//=======================================================================
// info.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_INFO_H
#define PORG_INFO_H

#include "config.h"


namespace Porg {

class Pkg;

class Info
{
	public:

	Info(Pkg*);

	private:

	Pkg* m_pkg;

	void get_icon_path();
	void get_info_spec();
	void get_info_pc();
	void get_info_desktop();
	void get_info_config_log();

};	// class Info

}	// namespace Porg


#endif	// PORG_INFO_H
