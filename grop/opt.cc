//=======================================================================
// opt.cc
//-----------------------------------------------------------------------
// This file is part of the package grop
// Copyright (C) 2004-2012 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "mainwindow.h" // NCOLS
#include <glibmm/miscutils.h>
#include <fstream>

using std::string;
using std::vector;
using namespace Grop;


bool Opt::s_hour 	= false;
int Opt::s_width 	= Opt::DEFAULT_WIDTH;
int Opt::s_height 	= Opt::DEFAULT_HEIGHT;
int Opt::s_xpos 	= Opt::DEFAULT_XPOS;
int Opt::s_ypos 	= Opt::DEFAULT_YPOS;
vector<bool> Opt::s_columns;
bool Opt::s_initialized = false;


Opt::Opt()
:
	Porg::Porgrc(),
	Glib::KeyFile(),
	m_groprc(Glib::get_home_dir() + "/.grop")
{
	g_assert(s_initialized == false);

	try 
	{
		load_from_file(m_groprc);

		s_hour 		= get_boolean("gui", "hour");
		s_width 	= get_integer("gui", "width");
		s_height 	= get_integer("gui", "height");
		s_xpos		= get_integer("gui", "xpos");
		s_ypos		= get_integer("gui", "ypos");
		s_columns	= get_boolean_list("gui", "columns");
	}
	catch (...) 
	{
		// On error, remove config file (will be rebuild in ~Opt())
		unlink(m_groprc.c_str());
	}

	if (s_columns.size() < MainTreeView::NCOLS)
		s_columns = vector<bool>(MainTreeView::NCOLS, true);

	s_initialized = true;
}


Opt::~Opt()
{
	set_boolean("gui", "hour", s_hour);
	set_integer("gui", "width", s_width); 
	set_integer("gui", "height", s_height);
	set_integer("gui", "xpos", s_xpos);
	set_integer("gui", "ypos", s_ypos);
	set_boolean_list("gui", "columns", s_columns);

	std::ofstream os(m_groprc.c_str());
	if (os)
		os << to_data();
	else
		g_warning("Cannot open file '%s' for writing", m_groprc.c_str());
}


void Opt::init()
{
	static Opt opt;
}

