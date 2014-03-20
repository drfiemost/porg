//=======================================================================
// global.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef PORG_GLOBAL_H
#define PORG_GLOBAL_H

#include "config.h"
#include <iosfwd>


namespace Porg
{
	extern int g_exit_status;

	std::string realdir(std::string const&);
	std::string to_lower(std::string const&);

}	// namespace Porg


#endif  // PORG_GLOBAL_H

