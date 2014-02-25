//=======================================================================
// removepkg.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copy (C) 2004-2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "opt.h"
#include "db.h"
#include "util.h"
#include "pkg.h"
#include "porg/file.h"
#include "porg/common.h"
#include "removepkg.h"
#include "porg/common.h"
#include <glibmm/miscutils.h>	// path_get_dirname()
#include <glibmm/main.h>	// signal_timeout()
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <glibmm/stringutils.h>	// strerror()

using std::string;
using sigc::mem_fun;
using namespace Grop;


RemovePkg::RemovePkg(Pkg& pkg, Gtk::Window& parent)
:
	Gtk::Dialog("Grop :: Remove", parent, true),
	m_error(false),
	m_label(),
	m_progressbar(),
	m_expander("Details"),
	m_button_close(Gtk::Stock::CLOSE),
	m_text_buffer(Gtk::TextBuffer::create()),
	m_text_view(m_text_buffer),
	m_tag_ok(m_text_buffer->create_tag()),
	m_tag_skipped(m_text_buffer->create_tag()),
	m_tag_error(m_text_buffer->create_tag()),
	m_initial_width(),
	m_initial_height(),
	m_pkg(pkg)
{
	set_border_width(4);
	set_size_request(450, 0);

	Glib::signal_timeout().connect_once(mem_fun(this, &RemovePkg::remove), 100);

	m_expander.property_expanded().signal_changed().connect(
		mem_fun(this, &RemovePkg::on_expander_changed));

	m_label.set_markup("<i>Removing package '" + pkg.name() + "'...</i>"),
	m_label.set_ellipsize(Pango::ELLIPSIZE_MIDDLE);

	m_text_view.set_editable(false);
	m_text_view.set_cursor_visible(false);
	m_text_view.set_right_margin(4);
	m_text_view.set_left_margin(4);
	m_text_view.override_background_color(Gdk::RGBA("black"), Gtk::STATE_FLAG_NORMAL);

	m_tag_ok->property_foreground() = "white";
	m_tag_skipped->property_foreground() = "#ffff44";	// light yellow
	m_tag_error->property_foreground() = "#ff4444";		// light red

	Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window->add(m_text_view);

	m_expander.add(*scrolled_window);

	Gtk::Box* label_box = Gtk::manage(new Gtk::Box());
	label_box->pack_start(m_label, Gtk::PACK_SHRINK);

	Gtk::Box* box = get_content_area();
	box->set_spacing(4);
	box->pack_start(*label_box, Gtk::PACK_SHRINK);
	box->pack_start(m_progressbar, Gtk::PACK_SHRINK);
	box->pack_start(m_expander, Gtk::PACK_EXPAND_WIDGET);

	m_button_close.set_sensitive(false);
	add_action_widget(m_button_close, Gtk::RESPONSE_CLOSE);

	show_all();
	
	m_initial_width = get_width();
	m_initial_height = get_height();

	run();
}


void RemovePkg::on_expander_changed()
{
	if (!m_expander.get_expanded()) {
		resize(m_initial_width, m_initial_height);
		set_size_request(m_initial_width, m_initial_height);
	}
}


bool RemovePkg::instance(Pkg& pkg, Gtk::Window& parent)
{
	RemovePkg remove_pkg(pkg, parent);
	
	remove_pkg.hide();
	
	return !remove_pkg.m_error;
}


void RemovePkg::report(string const& msg, Glib::RefPtr<Gtk::TextTag> const& tag)
{
	m_text_buffer->insert_with_tag(m_text_buffer->end(), msg + '\n', tag);
	Gtk::TextIter end = m_text_buffer->end();
	m_text_view.scroll_to(end);
}


void RemovePkg::remove()
{
	int cnt = 1;

	for (Pkg::file_it f(m_pkg.files().begin()); f != m_pkg.files().end(); ++f) {
		
		string file = (*f)->name();

		m_progressbar.set_fraction(cnt++ / m_pkg.files().size());
		main_iter();

		// skip excluded
		if (Porg::in_paths(file, Opt::remove_skip()))
			report("'" + file + "': excluded (skipped)", m_tag_skipped);

		// skip shared files
		else if (m_pkg.is_shared(*f, DB::pkgs()))
			report("'" + file + "': shared (skipped)", m_tag_skipped);

		// remove file
		else if (unlink(file.c_str()) == 0 || errno == ENOENT) {
			report("Removed '" + file + "'", m_tag_ok);
			remove_parent_dir(file);
		}

		// an error occurred
		else {
			m_error = true;
			report("unlink(\"" + file + "\"): " + Glib::strerror(errno), m_tag_error);
		}
	}

	if (m_error)
		m_label.set_markup("<span fgcolor=\"darkred\"><b>Completed with errors (see Details)</b></span>");
	else
		m_label.set_markup("<span fgcolor=\"darkgreen\"><b>Done</b></span>");

	m_button_close.set_sensitive();
}


void RemovePkg::remove_parent_dir(string const& path)
{
	string parent = Glib::path_get_dirname(path);
	
	if (!rmdir(parent.c_str())) {
		report("Removed directory '" + parent + "'", m_tag_ok);
		remove_parent_dir(parent);
	}
}

