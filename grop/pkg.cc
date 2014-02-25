//=======================================================================
// pkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "pkg.h"

using namespace Grop;


Pkg::Pkg(std::string const& name_)
:
	Porg::BasePkg(name_),
	m_icon()
{
	get_files();

	try 
	{ 
		m_icon = Gdk::Pixbuf::create_from_file(m_icon_path, 72, 72); 
	}
	catch (...) 
	{ 
		m_icon.clear(); 
	}
}


