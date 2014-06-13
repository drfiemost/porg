//=======================================================================
// porgball.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
// Copyright (C) 2014 David Ricart
// For more information visit http://porg.sourceforge.net
//=======================================================================

#include "config.h"
#include "porgball.h"
#include "util.h"
#include "mainwindow.h"
#include "pkg.h"
#include "porg/file.h"
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/stock.h>
#include <gtkmm/grid.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/separator.h>
#include <glibmm/iochannel.h>
#include <glibmm/miscutils.h>	// Glib::get_home_dir()
#include <glibmm/fileutils.h>	// Glib::file_open_tmp()
#include <glibmm/stringutils.h>	// Glib::strerror()
#include <fstream>
#include <sys/wait.h>


Grop::Porgball::Last Grop::Porgball::s_last = { Glib::get_home_dir(), PROG_GZIP, 6, false };


Grop::Porgball::Porgball(Grop::Pkg const& pkg, Gtk::Window& parent)
:
	Gtk::Dialog("grop :: porgball", parent, true),
	m_pkg(pkg),
	m_label_progress("", 0.02, 0.5),
	m_label_tarball("", 0, 0),
	m_combo_prog(),
	m_combo_level(),
	m_filechooser_button(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	m_button_test("_Integrity test", true),
	mp_button_close(add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE)),
	mp_button_cancel(add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL)),
	mp_button_ok(add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK)),
	m_progressbar(),
	m_tmpfile(),
	m_pid(0),
	m_close(false),
	m_children()
{
	set_border_width(8);

	// widgets to be unsensitivized when creating the porgball
	m_children.push_back(&m_combo_prog);
	m_children.push_back(&m_combo_level);
	m_children.push_back(&m_filechooser_button);
	m_children.push_back(&m_button_test);
	m_children.push_back(mp_button_close);

	m_label_progress.set_ellipsize(Pango::ELLIPSIZE_MIDDLE);
	m_label_tarball.set_ellipsize(Pango::ELLIPSIZE_MIDDLE);

	m_combo_prog.append("gzip");
	if (Glib::find_program_in_path("bzip2").size())
		m_combo_prog.append("bzip2");
	if (Glib::find_program_in_path("xz").size())
		m_combo_prog.append("xz");
	m_combo_prog.signal_changed().connect(sigc::mem_fun(this, &Grop::Porgball::on_change_prog));

	m_combo_level.append("1 (faster)");
	char num[2];
	for (int i = 2; i < 9; ++i) {
		g_snprintf(num, sizeof(num), "%d", i);
		m_combo_level.append(num);
	}
	m_combo_level.append("9 (better)");

	Gtk::Box* box_vexpand = Gtk::manage(new Gtk::Box());
	Gtk::Grid* grid = Gtk::manage(new Gtk::Grid());
	grid->set_column_spacing(10);
	grid->set_row_spacing(10);
	grid->attach(*(Gtk::manage(new Gtk::Label("Name:", 0., 0.5))), 0, 0, 1, 1);
	grid->attach(m_label_tarball, 1, 0, 3, 1);
	grid->attach(*(Gtk::manage(new Gtk::Label("Save in folder:", 0, 0.5))), 0, 1, 1, 1);
	grid->attach(m_filechooser_button, 1, 1, 3, 1);
	grid->attach(*(Gtk::manage(new Gtk::Label("Compression:", 0., 0.5))), 0, 2, 1, 1);
	grid->attach(m_combo_prog, 1, 2, 1, 1);
	grid->attach(*(Gtk::manage(new Gtk::Label("Level:", 0., 0.5))), 2, 2, 1, 1);
	grid->attach(m_combo_level, 3, 2, 1, 1);
	grid->attach(m_button_test, 0, 3, 4, 1);
	grid->attach(*box_vexpand, 0, 4, 4, 1);
	grid->attach(m_label_progress, 0, 5, 3, 1);
	grid->attach(m_progressbar, 3, 5, 1, 1);
	grid->attach(*(Gtk::manage(new Gtk::Separator())), 0, 6, 4, 1);

	m_label_tarball.set_hexpand();
	m_filechooser_button.set_hexpand();
	m_combo_prog.set_hexpand();
	m_combo_level.set_hexpand();
	m_progressbar.set_hexpand();
	box_vexpand->set_vexpand();

	m_progressbar.set_pulse_step(0.007);

	get_content_area()->pack_start(*grid, Gtk::PACK_EXPAND_WIDGET);

	mp_button_cancel->signal_clicked().connect(sigc::mem_fun(*this, &Grop::Porgball::on_cancel));
	get_action_area()->set_layout(Gtk::BUTTONBOX_EDGE);

	if (::close(Glib::file_open_tmp(m_tmpfile, "grop")) < 0)
		m_tmpfile = "/tmp/grop" + getpid();

	m_filechooser_button.set_current_folder(s_last.folder);
	m_combo_prog.set_active(s_last.prog);
	m_combo_level.set_active(s_last.level);
	m_button_test.set_active(s_last.test);

	show_all();
	m_progressbar.hide();
	mp_button_cancel->hide();
}


Grop::Porgball::~Porgball()
{
	if (m_pid)
		kill(m_pid, SIGKILL);

	unlink(m_tmpfile.c_str());

	s_last.folder = m_filechooser_button.get_filename();
	s_last.test = m_button_test.get_active();
	s_last.level = m_combo_level.get_active_row_number();
	s_last.prog = m_combo_prog.get_active_row_number();
}


void Grop::Porgball::instance(Pkg const& pkg, Gtk::Window& parent)
{
	Grop::Porgball obj(pkg, parent);
	
	while (!obj.m_close && obj.run() == Gtk::RESPONSE_OK)
		obj.end_create(obj.create_porgball());
}


void Grop::Porgball::on_cancel()
{
	g_assert(m_pid > 0);

	if (m_pid)
		kill(m_pid, SIGKILL);

	mp_button_cancel->hide();
	mp_button_ok->show();
}


bool Grop::Porgball::on_delete_event(GdkEventAny*)
{
	if (m_pid && kill(m_pid, SIGSTOP) == 0) {

		if (run_question_dialog("A process is running. "
			"Do you want to terminate it ?", this)) {
			kill(m_pid, SIGKILL);
			m_pid = 0;
			m_close = true;
		}
		else if (kill(m_pid, SIGCONT) == 0)
			return true;
	}

	return false;
}


void Grop::Porgball::set_children_sensitive(bool setting /* = true */)
{
	for (guint i = 0; i < m_children.size(); ++i)
		m_children[i]->set_sensitive(setting);
}


bool Grop::Porgball::create_porgball()
{
	// check whether we have write permissions on the dest. directory
	
	std::string dir = m_filechooser_button.get_filename();
	if (access(dir.c_str(), W_OK) < 0) {
		run_error_dialog(dir + ": " + Glib::strerror(errno), this);
		return false;
	}

	// build porgball name

    std::string zipfile = dir + "/" + m_label_tarball.get_text();
	
	if (!access(zipfile.c_str(), F_OK)) {
		if (!run_question_dialog("File '" + zipfile + "' already exists.\n"
			"Do you want to overwrite it ?", this))
			return false;
	}

	set_children_sensitive(false);
	mp_button_ok->hide();
	mp_button_cancel->show();

	// get list of logged files

	std::ofstream ftmp(m_tmpfile.c_str());
	
	if (!ftmp) {
		run_error_dialog("Error opening temporary file '" 
			+ m_tmpfile + "':" + Glib::strerror(errno), this);
		return false;
	}
		
	m_progressbar.show();
	m_label_progress.set_text("Reading logged files");
	main_iter();

	for (guint i = 0; i < m_pkg.files().size(); ++i) {
		if (m_pkg.files()[i]->is_installed())
			ftmp << m_pkg.files()[i]->name() << "\n";
	}

	if (!ftmp.tellp()) {
		run_error_dialog("Empty package", this);
		return false;
	}
	
	ftmp.close();
	
	// build tar command and run it

	std::string tarfile = dir + "/" + m_pkg.name() + ".porg.tar";
	
	std::vector<std::string> argv;
	argv.push_back("tar");
	argv.push_back("--create");
	argv.push_back("--file=" + tarfile);
	argv.push_back("--files-from=" + m_tmpfile);
	argv.push_back("--ignore-failed-read");

	m_label_progress.set_text("Creating " + Glib::path_get_basename(tarfile));
	main_iter();

	if (!spawn(argv)) {
		unlink(tarfile.c_str());
		return false;
	}

	// build compression command and run it

	std::string prog = m_combo_prog.get_active_text();

	std::ostringstream level;
	level << (m_combo_level.get_active_row_number() + 1);
	
	argv.clear();
	argv.push_back(prog);
	argv.push_back(std::string("-") + level.str());
	argv.push_back("--force");
	argv.push_back(tarfile);
	
	m_label_progress.set_text("Creating " + Glib::path_get_basename(zipfile));
	main_iter();

	if (!spawn(argv)) {
		unlink(tarfile.c_str());
		return false;
	}

	// if needed, build test command and run it

	if (!m_button_test.get_active())
		return true;
	
	argv.clear();
	argv.push_back(prog);
	argv.push_back("--test");
	argv.push_back(zipfile);

	m_label_progress.set_text("Testing " + Glib::path_get_basename(zipfile));
	main_iter();

	return spawn(argv);
}


void Grop::Porgball::end_create(bool done /* = true */)
{
	m_pid = 0;
	set_children_sensitive();
	m_progressbar.hide();
	mp_button_ok->show();
	mp_button_cancel->hide();
	m_label_progress.set_markup(
		done ? "<span fgcolor=\"darkgreen\"><b>Done</b></span>" : "");
	main_iter();
}


void Grop::Porgball::on_change_prog()
{
	std::string suffix;

	switch (m_combo_prog.get_active_row_number()) {
		case PROG_GZIP:		suffix = "gz";	break;
		case PROG_BZIP2:	suffix = "bz2"; break;
		case PROG_XZ:		suffix = "xz";	break;
		default: g_assert_not_reached();
	}

	m_label_tarball.set_text(m_pkg.name() + ".porg.tar." + suffix);
}


bool Grop::Porgball::spawn(std::vector<std::string>& argv)
{
	int std_err, status;

	Glib::spawn_async_with_pipes(Glib::get_current_dir(), argv,
		Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_SEARCH_PATH,
		sigc::slot<void>(), &m_pid, NULL, NULL, &std_err);

	Glib::RefPtr<Glib::IOChannel> io = Glib::IOChannel::create_from_fd(std_err);
	io->set_close_on_unref(true);

	while (!m_close && waitpid(m_pid, &status, WNOHANG) != m_pid) {
		m_progressbar.pulse();
		main_iter();
		g_usleep(2000);	
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
		Glib::ustring aux, err = "Error while running the " + argv[0] + " process";
		if (io->read_to_end(aux) == Glib::IO_STATUS_NORMAL)
			err += ":\n\n" + aux;
		run_error_dialog(err, this);
		return false;
	}

	return !WIFSIGNALED(status);
}

