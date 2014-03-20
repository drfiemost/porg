//=======================================================================
// porgball.h
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#ifndef GROP_PORGBALL_H
#define GROP_PORGBALL_H

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/filechooserbutton.h>

namespace Grop
{

class Pkg;

class Porgball : public Gtk::Dialog
{
	public:

	static void instance(Pkg const&, Gtk::Window&);

	protected:

	Porgball(Pkg const&, Gtk::Window&);
	~Porgball();

	typedef struct {
		std::string	folder;
		int			prog;
		int			level;
		bool		test;
	} Last;
		
	enum { USE_GZIP, USE_BZIP2, USE_XZ };

	static Last s_last;

	Pkg const&				m_pkg;
	Gtk::Label				m_label;
	Gtk::Label				m_label_tarball;
	Gtk::ComboBoxText		m_combo_prog;
	Gtk::ComboBoxText		m_combo_level;
	Gtk::FileChooserButton	m_filechooser_button;
	Gtk::CheckButton		m_button_test;
	Gtk::ProgressBar		m_progressbar;
	std::string				m_tmpfile;

	void on_change_prog();
	void create_porgball();
	bool spawn_async(std::vector<std::string>&);
	void end_create(bool done = true);
};

} // namespace Grop


#endif  // GROP_PORGBALL_H
