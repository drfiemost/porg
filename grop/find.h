//=======================================================================
// find.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef GROP_FIND_H
#define GROP_FIND_H

#include "config.h"
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>


namespace Grop
{

class Pkg;

class Find : public Gtk::Dialog
{
	class PkgsTreeView : public Gtk::TreeView
	{
		friend class Find;

		class ModelColumns : public Gtk::TreeModel::ColumnRecord
		{
			friend class Find;
	
			ModelColumns()
			{
				add(m_pkg);
				add(m_name);
			}

			Gtk::TreeModelColumn<Pkg*>			m_pkg;
			Gtk::TreeModelColumn<Glib::ustring>	m_name;

		};	// class Find::ModelColumns

		PkgsTreeView();

		ModelColumns                    m_columns;
		Glib::RefPtr<Gtk::ListStore>	m_model;
	};

	public:

	static void instance(Gtk::Window&);

	private:

	Find(Gtk::Window&);

	Gtk::Entry		m_entry;
	PkgsTreeView	m_treeview;

	static Find* s_find;

	Gtk::TreeModel::iterator reset_treeview();
	void browse();
	void find();
	virtual void on_response(int id);
};

} // namespace Grop


#endif  // GROP_FIND_H

