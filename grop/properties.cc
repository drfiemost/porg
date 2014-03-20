//=======================================================================
// properties.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "pkg.h"
#include "properties.h"
#include "filestreeview.h"
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>

using Glib::ustring;
using namespace Grop;


Properties::Properties(Pkg const& pkg, Gtk::Window& parent)
:
	Gtk::Dialog("grop :: properties", parent),
	m_notebook()
{
	set_border_width(4);
	set_default_size(500, 500);

	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	Gtk::Box* label_box = Gtk::manage(new Gtk::Box());
	label->set_markup("Package <b>" + pkg.name() + "</b>");
	label_box->pack_start(*label, Gtk::PACK_SHRINK);

	Gtk::Box* box = get_content_area();
	box->set_spacing(4);
	box->pack_start(*label_box, Gtk::PACK_SHRINK);
	box->pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);

	Gtk::ScrolledWindow* scrolled_window_files = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window_files->add(*(Gtk::manage(new FilesTreeView(pkg))));
	m_notebook.append_page(*scrolled_window_files, "Files");

	Gtk::TextView* text_view(Gtk::manage(new Gtk::TextView()));
	text_view->set_editable(false);
	text_view->set_cursor_visible(false);
	text_view->set_left_margin(10);
	text_view->set_right_margin(10);

	Glib::RefPtr<Gtk::TextBuffer> text_buffer(text_view->get_buffer());

	Glib::RefPtr<Gtk::TextTag> tag_wrap(text_buffer->create_tag());
	tag_wrap->property_wrap_mode() = Gtk::WRAP_WORD;
	
	Glib::RefPtr<Gtk::TextTag> tag_title(text_buffer->create_tag());
	tag_title->property_weight() = Pango::WEIGHT_BOLD;
	tag_title->property_family() = "monospace";

	if (pkg.icon()) {
		text_buffer->insert(text_buffer->end(), "\n");
		text_buffer->insert_pixbuf(text_buffer->end(), pkg.icon());
		text_buffer->insert(text_buffer->end(), "\n");
	}

	text_buffer->insert_with_tag(text_buffer->end(), "\nName:    ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.base_name(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nVersion: ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.version(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nSummary: ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.summary(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nAuthor:  ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.author(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nLicense: ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.license(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nURL:     ", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.url(), tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\n\nDescription:\n", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.description() + '\n', tag_wrap);
	text_buffer->insert_with_tag(text_buffer->end(), "\nConfigure options:\n", tag_title);
	text_buffer->insert_with_tag(text_buffer->end(), pkg.conf_opts(), tag_wrap);

	Gtk::ScrolledWindow* scrolled_window_info = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window_info->add(*text_view);
	m_notebook.append_page(*scrolled_window_info, "Info");

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

	show_all();
	run();
}


void Properties::instance(Pkg const& pkg, Gtk::Window& parent)
{
	Properties properties(pkg, parent);
}

