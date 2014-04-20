//=======================================================================
// porgball.cc
//-----------------------------------------------------------------------
// This file is part of the package porg
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
#include <glibmm/spawn.h>
#include <glibmm/miscutils.h>	// Glib::get_home_dir()
#include <glibmm/fileutils.h>	// Glib::file_open_tmp()
#include <glibmm/stringutils.h>	// Glib::strerror()
#include <fstream>
#include <sys/wait.h>

using sigc::mem_fun;
using std::vector;
using std::string;
using Glib::ustring;
using namespace Grop;

static void unlink_async(string const&);

Porgball::Last Porgball::s_last = { Glib::get_home_dir(), USE_GZIP, 6, false };


Porgball::Porgball(Pkg const& pkg, Gtk::Window& parent)
:
	Gtk::Dialog("grop :: porgball", parent, true),
	m_pkg(pkg),
	m_label("", 0.02, 0.5),
	m_label_tarball("", 0, 0),
	m_combo_prog(),
	m_combo_level(),
	m_filechooser_button(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	m_button_test("_Integrity test", true),
	m_progressbar(),
	m_tmpfile()
{
	set_border_width(8);

	m_label.set_ellipsize(Pango::ELLIPSIZE_MIDDLE);
	m_label_tarball.set_ellipsize(Pango::ELLIPSIZE_MIDDLE);

	m_filechooser_button.set_current_folder(s_last.folder);

	m_combo_prog.append("gzip");
	if (Glib::find_program_in_path("bzip2").size())
		m_combo_prog.append("bzip2");
	if (Glib::find_program_in_path("xz").size())
		m_combo_prog.append("xz");
	m_combo_prog.signal_changed().connect(mem_fun(this, &Porgball::on_change_prog));
	m_combo_prog.set_active(s_last.prog);

	m_combo_level.append("1 (faster)");
	char num[2];
	for (int i = 2; i < 9; ++i) {
		g_snprintf(num, sizeof(num), "%d", i);
		m_combo_level.append(num);
	}
	m_combo_level.append("9 (better)");
	m_combo_level.set_active(s_last.level);

	m_button_test.set_tooltip_text("Make an integrity test after creating the package");
	m_button_test.set_active(s_last.test);

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
	grid->attach(m_label, 0, 5, 3, 1);
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

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
	add_button(Gtk::Stock::EXECUTE, Gtk::RESPONSE_OK);
	get_action_area()->set_layout(Gtk::BUTTONBOX_EDGE);

	if (close(Glib::file_open_tmp(m_tmpfile, "grop")) < 0)
		m_tmpfile = "/tmp/grop" + getpid();

	show_all();
	m_progressbar.hide();
}


Porgball::~Porgball()
{
	unlink(m_tmpfile.c_str());

	s_last.folder = m_filechooser_button.get_filename();
	s_last.test = m_button_test.get_active();
	s_last.level = m_combo_level.get_active_row_number();
	s_last.prog = m_combo_prog.get_active_row_number();
}


void Porgball::instance(Pkg const& pkg, Gtk::Window& parent)
{
	Porgball porgball(pkg, parent);
	
	while (porgball.run() == Gtk::RESPONSE_OK)
		porgball.create_porgball();
}


void Porgball::create_porgball()
{
	// Check whether we have write permissions on the dest. directory
	ustring dir = m_filechooser_button.get_filename();
	if (access(dir.c_str(), W_OK) < 0) {
		run_error_dialog(dir + ": " + Glib::strerror(errno));
		return;
	}

    string tar = dir + "/" + m_pkg.name() + ".porg.tar";
    string zip = tar + ".";
	
	int prog = m_combo_prog.get_active_row_number();

	switch (prog) {
		case USE_GZIP: 	zip += "gz";	break;
		case USE_BZIP2:	zip += "bz2";	break;
		case USE_XZ:	zip += "xz";	break;
		default:	g_assert_not_reached();
	}
	g_assert(Glib::path_get_basename(zip) == m_label_tarball.get_text());

	if (!access(zip.c_str(), F_OK)) {
		if (!run_question_dialog("File " + zip + " already exists.\n"
		"Do you want to overwrite it ?"))
			return;
	}

	std::ofstream ftmp(m_tmpfile.c_str());
	if (!ftmp) {
		run_error_dialog("Error opening temporary file '" 
			+ m_tmpfile + "':" + Glib::strerror(errno));
		return;
	}
		
	m_progressbar.show();
	m_label.set_text("Reading logged files");
	main_iter();

	struct stat s;

	for (uint i = 0; i < m_pkg.files().size(); ++i) {
		if (lstat(m_pkg.files()[i]->name().c_str(), &s) == 0)
			ftmp << m_pkg.files()[i]->name() << "\n";
	}
	if (!ftmp.tellp()) {
		end_create(false);
		run_error_dialog("Empty package");
		return;
	}
	ftmp.close();
	
	vector<string> argv;
	argv.push_back("tar");
	argv.push_back("--create");
	argv.push_back("--file=" + tar);
	argv.push_back("--files-from=" + m_tmpfile);
	argv.push_back("--ignore-failed-read");

	m_label.set_text("Creating " + Glib::path_get_basename(tar));
	main_iter();

	if (!spawn_async(argv)) {
		unlink_async(tar);
		end_create(false);
		return;
	}

	argv.clear();
	
	string prog_str;
	switch (prog) {
		case USE_GZIP:	prog_str = "gzip";	break;
		case USE_BZIP2:	prog_str = "bzip2";	break;
		case USE_XZ:	prog_str = "xz";	break;
	}
	argv.push_back(prog_str);

	std::ostringstream level;
	level << (m_combo_level.get_active_row_number() + 1);
	argv.push_back("-" + level.str());

	argv.push_back("--force");
	argv.push_back(tar);
	
	m_label.set_text("Creating " + Glib::path_get_basename(zip));
	main_iter();

	if (!spawn_async(argv)) {
		unlink_async(tar);
		unlink_async(zip);
		end_create(false);
		return;
	}

	if (!m_button_test.get_active()) {
		end_create();
		return;
	}
	
	argv.clear();
	argv.push_back(prog_str);
	argv.push_back("--test");
	argv.push_back(zip);

	m_label.set_text("Testing " + Glib::path_get_basename(zip));
	main_iter();

	end_create(spawn_async(argv));
}


void Porgball::end_create(bool done /* = true */)
{
	m_progressbar.hide();
	m_label.set_text(done ? "Done" : "");
	main_iter();
}


void Porgball::on_change_prog()
{
	string txt = m_pkg.name() + ".porg.tar.";

	switch (m_combo_prog.get_active_row_number()) {
		case USE_GZIP:	m_label_tarball.set_text(txt + "gz");	break;
		case USE_BZIP2:	m_label_tarball.set_text(txt + "bz2");	break;
		case USE_XZ:	m_label_tarball.set_text(txt + "xz");	break;
		default:	g_assert_not_reached();
	}
}


bool Porgball::spawn_async(vector<string>& argv)
{
	Glib::Pid pid;
	int std_err, status;

	Glib::spawn_async_with_pipes(Glib::get_current_dir(), argv,
		Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_SEARCH_PATH,
		sigc::slot<void>(), &pid, NULL, NULL, &std_err);

	Glib::RefPtr<Glib::IOChannel> io = Glib::IOChannel::create_from_fd(std_err);
	io->set_close_on_unref(true);

	while (waitpid(pid, &status, WNOHANG) != pid) {
		m_progressbar.pulse();
		main_iter();
		g_usleep(2000);	
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
		ustring aux, err = "Error while running the " + argv[0] + " process";
		if (io->read_to_end(aux) == Glib::IO_STATUS_NORMAL)
			err += ":\n\n" + aux;
		run_error_dialog(err);
		return false;
	}

	return true;
}


static void unlink_async(string const& file)
{
	vector<string> argv;
	argv.push_back("unlink");
	argv.push_back(file);
	Glib::spawn_async(Glib::get_current_dir(), argv, Glib::SPAWN_SEARCH_PATH
		| Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL);
}

