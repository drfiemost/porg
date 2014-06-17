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

	Info(Pkg&);

	private:

	Pkg& m_pkg;

	void get_icon_path();
	void get_configure_options();
	void read_spec();
	void read_pc();
	void read_desktop();
	void read_config();

};	// class Info

}	// namespace Porg


#endif	// PORG_INFO_H
