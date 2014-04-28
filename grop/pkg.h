//=======================================================================
// pkg.h
//-----------------------------------------------------------------------
// This file is part of the package grop
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef GROP_PKG_H
#define GROP_PKG_H

#include "config.h"
#include "porg/basepkg.h"
#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>


namespace Grop {

typedef Porg::File File;


class Pkg : public Porg::BasePkg
{
	public:

	Pkg(std::string const&);

	Glib::RefPtr<Gdk::Pixbuf> const& icon() const	{ return m_icon; }

	private:

	Glib::RefPtr<Gdk::Pixbuf>	m_icon;
};

}	// namespace Grop


#endif  // GROP_PKG_H
