// flviewer.cpp: Entry point, args parsing and initialization
//------------------------------------------------------------------------------
// Copyright (c) 2016 Anatoly madRat L. Berenblit
//------------------------------------------------------------------------------
//  This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
//  This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------
#include "config.h"
#include "MainWindow.h"
#include "Plugin.h"
#include "i18n.h"
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>

#define HELP_OPT   		"--help"
#define HELP_ALLIGN 	"\t\t"
#define HELP_MSG   		"Show this help"

#define PLUGINS_OPT 	"--plugins"
#define PLUGINS_ALLIGN	" dir\t"
#define PLUGINS_MSG 	"Load additional plugins from directory"

#define SMOOTH_OPT 		"--smooth"
#define SMOOTH_ALLIGN	"\t"
#define SMOOTH_MSG 		"Enable SCALING_BILINEAR for images (looks nice, but very slow)"

#define ARGFIN_OPT 		"--"
#define ARGFIN_ALLIGN  	"\t\t"
#define ARGFIN_MSG 		"The follows items are file(s)"

void help(int errCode)
{
	Fl::error("%s [OPTION]... [FILE]...", PACKAGE);
	const char* as[]={"a", "b"};
	Fl::args(2, (char**)as);

	const char* arg[][3] = {	{SMOOTH_OPT, 	SMOOTH_ALLIGN,	_(SMOOTH_MSG)},
								{PLUGINS_OPT, 	PLUGINS_ALLIGN,	_(PLUGINS_MSG)},
								{HELP_OPT, 		HELP_ALLIGN,	_(HELP_MSG)},
								{ARGFIN_OPT, 	ARGFIN_ALLIGN,	_(ARGFIN_MSG)}
							};
	for (size_t i = 0; i < sizeof(arg)/sizeof(*arg); ++i)
		Fl::error(	" %s%s - %s.", arg[i][0], arg[i][1], arg[i][2]);
	exit(errCode);
}

int handleArgs(int argc, char **argv, int &i)
{
	const char *arg = argv[i];
	if (*arg != '-')
		return 0;

	if (0 == strcmp(HELP_OPT, arg))
		help(EXIT_SUCCESS);

	if (0 == strcmp(PLUGINS_OPT, arg))
	{
		++i;
		if (argc <= i)
		{
			Fl::error(_("Option `%s' need an argument."), arg);
			help(EXIT_FAILURE);
			return 0;
		}

		plugin::init(argv[i]);
		++i;
		return 1;
	}

	if (0 == strcmp(SMOOTH_OPT, arg))
	{
		Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR);
		++i;
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	fl_register_images();
	I18N_INIT();

	fl_no  = _("No");
	fl_yes = _("Yes");
	fl_ok  = _("OK");
	fl_cancel= _("Cancel");
	fl_close = _("Close");

	plugin::init(NULL);

	int opts = 0;
	Fl::args(argc, argv, opts, handleArgs);

	if (opts < argc && '-' == *argv[opts])
	{
		if (0 != strcmp(argv[opts], ARGFIN_OPT))
		{
			Fl::error(_("Unknown option `%s'."), argv[opts]);
			help(EXIT_FAILURE);
		}
		++opts;
	}

	MainWindow window(Fl::w()/6, Fl::h()/6, Fl::w()*2/3, Fl::h()*2/3, argc-opts, const_cast<const char**>(argv+opts));
	window.show(argc, argv);
	return Fl::run();
}
