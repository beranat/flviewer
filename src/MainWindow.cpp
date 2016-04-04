// MainWindow.cpp: main window class
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

#include "i18n.h"

//------------------------------------------------------------------------------
#define PITCH (5)
#define WARNING_SIZE_HIGH (20000)

#define SMART_ZOOM_MAX  (4.00f)
#define ZOOM_SCALE_EPS 	(1.2f)

#define TOOL_BUTTON_SIZE 32
#define TOOL_BUTTON_SPACING 4
#define TOOL_HEIGHT TOOL_BUTTON_SIZE

#include "Plugin.h"
//------------------------------------------------------------------------------
const char* ABOUT = _NOOP("%s version %s\nCopyright (c) 2016 Anatoly madRat L. Berenblit\n\n"
					"This program is free software: you can redistribute it and/or modify\n"
					"it under the terms of the GNU General Public License as published by\n"
					"the Free Software Foundation, version 3 of the License.\n\n"
					"This program is distributed in the hope that it will be useful,\n"
					"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
					"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
					"See the GNU General Public License for more details.\n\n"
					"You should have received a copy of the GNU General Public License\n"
					"along with this program.  If not, see <http://www.gnu.org/licenses/>.");
const char* PLUGS_ABOUT = _NOOP("Plugins:\n\n%s");
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>

#include <vector>
#include <string>
#include <algorithm>

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Bar.H>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "load.xpm"
#include "prev.xpm"
#include "next.xpm"
#include "zoom_fit.xpm"
#include "zoom_in.xpm"
#include "zoom_out.xpm"
#include "flviewer.xpm"
//#include "icon.xpm"
#pragma GCC diagnostic pop

#include "DirImage.h"

#include "MainWindow.h"

class LogoImage : public MultipageImage
{
public:
	typedef MultipageImage baseClass;

public:
	LogoImage() : source_(flviewer)
	{
	}

	virtual size_t  num_pages() const       { return 1;	}
	virtual size_t  page() const  			{ return 0;	}
	virtual bool    page(size_t) 		  	{ return false;	}

	virtual size_t  w() const           	{ return source_.w();	}
	virtual size_t  h() const           	{ return source_.h();	}

	virtual Fl_Image* copy(int w, int h)
	{
		return source_.copy(w, h);
	}

protected:
	Fl_Pixmap source_;
};

MainWindow* MainWindow::window = NULL;

//------------------------------------------------------------------------------
const float MainWindow::scales[] =
{
	0.0625f, 	0.0833f,	0.1250f,	0.2500f,	0.3333f,	0.5000f,
	0.6667f,	0.7500f,
	1.0000f,
	1.2500f,	1.5000f,	2.0000f,	3.0000f,	4.0000f,	6.0000f,
	8.0000f,	12.000f,	16.000f
};

float MainWindow::scaleGet(int index)
{
	if (index < 0)
		return scales[0] * pow(2.0f, index);

	const int max = sizeof(scales) / sizeof(*scales);
	if (index < max)
		return scales[index];
	return scales[max - 1] * pow(2.0f, index - max + 1);
}

int   MainWindow::scaleCeil(const float value)
{
	const int max = sizeof(scales) / sizeof(*scales) - 1;
	if (value < scales[0])
		return scaleCeil(value * 2.0) - 1;
	else if (value > scales[max])
		return scaleCeil(value / 2.0) + 1;
	for (int index = 0; index <= max; ++index)
	{
		if (scales[index] > value)
			return index;
	}
	assert(false);
	return 0;
}

//------------------------------------------------------------------------------
static const struct ToolbarItem_t
{
	char**	 image;
	void*	cmd;
	const char* tooltip;
} toolbar[] =
{
	{load,		(void*)(MainWindow::CMD_OPEN),		_NOOP("Open a file")},
	{zoom_fit,	(void*)(MainWindow::CMD_ZOOM_FIT),	_NOOP("Fit a page to window size")},
	{zoom_out,	(void*)(MainWindow::CMD_ZOOM_OUT),	_NOOP("Zoom out")},
	{zoom_in,	(void*)(MainWindow::CMD_ZOOM_IN), 	_NOOP("Zoom in")},
	{prev,		(void*)(MainWindow::CMD_PAGE_PREV),	_NOOP("Go back one page")},
	{next,		(void*)(MainWindow::CMD_PAGE_NEXT),	_NOOP("Go forward one page")}
};

static const struct MenuItem_t
{
	const char* text;
	int 		hotkey;
	int			id;
} menuMain[] =
{
	{_NOOP("&File")},
		{_NOOP("&Open..."), 	FL_CTRL + 'o',	MainWindow::CMD_OPEN},
		{_NOOP("&Quit"), 		FL_CTRL + 'x',  MainWindow::CMD_EXIT},
		{NULL},
	{_NOOP("&View")},
		{_NOOP("&Zoom")},
			{"&33%",	FL_CTRL + '3', 	MainWindow::CMD_ZOOM_Z33},
			{"&50%",	FL_CTRL + '5',	MainWindow::CMD_ZOOM_Z50},
			{"&75%", 	FL_CTRL + '7',	MainWindow::CMD_ZOOM_Z75},
			{"&100%",	FL_CTRL + '1',	MainWindow::CMD_ZOOM_100},
			{"&200%",	FL_CTRL + '2',	MainWindow::CMD_ZOOM_200},
			{"&400%",	FL_CTRL + '4',	MainWindow::CMD_ZOOM_400},
			{NULL},
		{_NOOP("Zoom &Out"),	'-',			MainWindow::CMD_ZOOM_OUT},
		{_NOOP("Zoom &In"),	'+',			MainWindow::CMD_ZOOM_IN},
		{_NOOP("&Fit Page"),	FL_CTRL + 'f',	MainWindow::CMD_ZOOM_FIT},
		{_NOOP("Fit &Width"),	FL_CTRL + 'w',	MainWindow::CMD_ZOOM_WIDTH},
		{_NOOP("&Zoom To..."),	FL_CTRL + 'z',	MainWindow::CMD_ZOOM_TO},
		{NULL},
	{_NOOP("&Navigation")},
		{_NOOP("First Page"),	FL_Home,  		MainWindow::CMD_PAGE_FIRST},
		{_NOOP("Previous Page"),FL_Page_Up,  	MainWindow::CMD_PAGE_PREV},
		{_NOOP("Next Page"),	FL_Page_Down,	MainWindow::CMD_PAGE_NEXT},
		{_NOOP("Last Page"),	FL_End,  		MainWindow::CMD_PAGE_LAST},
		{_NOOP("&Page..."),	FL_CTRL + 'g',	MainWindow::CMD_PAGE_TO},
		{NULL},
	{_NOOP("&Help")},
		{_NOOP("About Plug-&Ins..."), 0,  MainWindow::CMD_ABOUT_PLUGINS},
		{_NOOP("&About..."), 0,  MainWindow::CMD_ABOUT},
		{NULL},
	{NULL}
};

//------------------------------------------------------------------------------
float MainWindow::scale() const
{
	if (scaleIsCustom_)
		return scaleValue_;
	return scaleGet(scaleIndex_);
}

void MainWindow::addScale(int index)
{
	if (0 == index)
		return;

	if (scaleIsCustom_)
	{
		scaleIsCustom_ = false;
		int orient = (index > 0) ? 1 : -1;
		scaleIndex_ = scaleCeil(scaleValue_) + (orient - 1) / 2; //this 1st scale factor
		const float factor = scaleGet(scaleIndex_) / scaleValue_;

		if (std::max(factor, 1.0f / factor) > ZOOM_SCALE_EPS)
		{
			//switch to scaleIndex_ will eat one index
			index -= orient;
		}
	}
	setScale(scaleIndex_ + index);
}

void MainWindow::setScale(int index)
{
	scaleIsCustom_ = false;
	scaleIndex_ = index;
	scaleValue_ = scale();
	updateImage();
}

void MainWindow::setScale(float value)
{
	scaleIsCustom_ = true;
	scaleValue_ = value;
	scaleIndex_ = scaleCeil(value);
	updateImage();
}

bool MainWindow::allowLargeImageWarning(int w, int h)
{
	if (isAllowedSizeWarning_ && (w > WARNING_SIZE_HIGH || h > WARNING_SIZE_HIGH))
	{
		int res = fl_choice(_("Scaled image have an excessively large size (%dx%d).\nContinue?"),
							_("No"), _("Yes"), _("Always Yes"), w, h);
		switch (res)
		{
		case 2:
			isAllowedSizeWarning_ = false;
		case 1:
			return true;
		case 0:
			return false;
		}
	}
	return true;
}

void MainWindow::uncacheImage()
{
	if (NULL != uiImageBox_)
	{
		delete uiImageBox_->image();
		uiImageBox_->image(NULL);
	}

}

void MainWindow::updateImage()
{
	if (NULL == source_ || 0 == source_->w() || 0 == source_->h())
	{
		uncacheImage();
		uiImageBox_->size(uiScroll_->w(), uiScroll_->h());
		uiImageBox_->image(new Fl_Image(uiScroll_->w() * 2 / 3, uiScroll_->h() * 2 / 3, 0));
		updateCaption();
		redraw();
		return;
	}

	const int w = scale() * source_->w();
	const int h = scale() * source_->h();

	if (!allowLargeImageWarning(w, h))
	{
		setScale(MainWindow::CMD_ZOOM_100 - MainWindow::CMD_ZOOM_INDEX);
		return;
	}

	if (w < 1 || h < 1)
	{
		setScale(std::max(1.05f / source_->w(), 1.05f / source_->h()));
		return;
	}

	Fl_Image* const oldImage = uiImageBox_->image();

	uiImageBox_->size(std::max(w, uiScroll_->w()), std::max(h, uiScroll_->h()));

	if (w <= uiScroll_->w())
		uiScroll_->scroll_to(0, uiScroll_->yposition());
	if (h <= uiScroll_->h())
		uiScroll_->scroll_to(uiScroll_->xposition(), 0);

	if (NULL != oldImage && oldImage->w() == w && oldImage->h() == h)
		return;

	fl_cursor(FL_CURSOR_WAIT, FL_BLACK, FL_WHITE);
	Fl_Image* uiImage = source_->copy(w, h);
	uiImageBox_->image(uiImage);
	delete oldImage;
	fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);

	updateCaption();
	redraw();
}

MainWindow::MainWindow(int x, int y, int cx, int cy, int nfiles, const char* files[]) : baseClass(x, y, cx, cy, PACKAGE),
	uiHeight_(30),	//recommended by fltk, but will be recalculated
	uiMenuItems_(NULL), uiMenu_(NULL), uiToolbarBox_(NULL), uiToolbar_(NULL), uiScroll_(NULL), uiImageBox_(NULL),
	scaleIndex_(0),	scaleIsCustom_(false), scaleValue_(1.0f),
	dragIsEnabled_(false), dragX_(0), dragY_(0), isCtrlPressed_(false),
	isAllowedSizeWarning_(true),
	source_(NULL)
{
	assert(NULL == window);
	window = this;

	Fl_Pixmap ico(flviewer);
	Fl_RGB_Image app(&ico);
	icon(&app);

	default_icon(&app);

	fl_message_hotspot(0);
	fl_message_title_default(PACKAGE);

	box(FL_NO_BOX);

	//* uiMenuItems_
	{
		const size_t mc = sizeof(menuMain)/sizeof(*menuMain);
		uiMenuItems_ = new Fl_Menu_Item[mc];

		const struct MenuItem_t	*s = ::menuMain;
		struct Fl_Menu_Item		*d = uiMenuItems_;
		for (size_t i = 0; i < mc; ++i, ++s, ++d)
		{
			(*d) = Fl_Menu_Item();
			if (NULL != s->text)
			{
				d->shortcut(s->hotkey);
				d->label(_(s->text));

				if (CMD_NONE != s->id)
					d->callback(MainWindow::commandCallback, static_cast<long>(s->id));
				else
					d->flags |= FL_SUBMENU;
			}
		}
	}

	uiMenu_ = new Fl_Menu_Bar(0, 0, cx, uiHeight_);
	uiMenu_->menu(uiMenuItems_);

	uiHeight_ = uiMenu_->textsize() * 3 / 2; //1.5 = 15 / 10 = 3/2
	uiMenu_->size(cx, uiHeight_);

	uiToolbarBox_ = new Fl_Box(0, uiHeight_, cx, TOOL_HEIGHT);
	uiToolbarBox_->box(FL_FLAT_BOX);

	uiToolbar_ = new Fl_Pack(0, uiHeight_, TOOL_BUTTON_SIZE, TOOL_HEIGHT);
	uiToolbar_->box(FL_THIN_UP_FRAME);
	uiToolbar_->type(Fl_Pack::HORIZONTAL);
	uiToolbar_->spacing(TOOL_BUTTON_SPACING);

	for (size_t ind = 0; ind < sizeof(toolbar) / sizeof(*toolbar); ++ind)
	{
		Fl_Button* b = new Fl_Button(0, 0, TOOL_BUTTON_SIZE, TOOL_BUTTON_SIZE);
		Fl_Image* ico = new Fl_Pixmap(toolbar[ind].image);
		uiImages_.push_back(ico);
		b->box(FL_THIN_UP_BOX);
		b->clear_visible_focus();
		b->image(ico);
		b->tooltip(_(toolbar[ind].tooltip));
		b->callback(commandCallback, toolbar[ind].cmd);
	}

	uiToolbar_->end();

	uiHeight_ += uiToolbar_->h();

	uiScroll_ = new Fl_Scroll(0, uiHeight_, cx, cy - uiHeight_);
	uiScroll_->box(FL_NO_BOX);
	uiImageBox_ = new Fl_Box(0, uiHeight_, uiScroll_->w(), uiScroll_->h());
	uiScroll_->end();
	end();
	resizable(uiToolbarBox_);
	resizable(uiToolbar_);
	resizable(uiScroll_);

	load(NULL);
	load(nfiles, files);
	commandCallback(this, MainWindow::CMD_ZOOM_FIT);
}

bool MainWindow::load(size_t nf, const char* fs[])
{
	if (0 == nf || NULL == fs)
		return load(NULL);

	if (1 == nf)
		return load(fs[0]);

	MultipageImage* image = NULL;
	do
	{
		image = new DirImage(nf, fs);
		if (NULL != image && 0 != image->w() && 0 != image->h())
			break;
		delete image;
		image = NULL;
	}
	while (false);

	if (NULL == image)
		return load(fs[0]);

	reset(image);
	return true;
}

struct LoadInfo
{
	const char		*filename;
	MultipageImage	*image;
};

bool pluginLoad(const plugin::Info* l, void* ptr)
{
	if (NULL == ptr || NULL == l)
		return false;

	if (NULL == l->load)
		return true;

	LoadInfo* info = reinterpret_cast<LoadInfo*>(ptr);

	MultipageImage* image = l->load(info->filename);
	if (NULL != image && 0 != image->w() && 0 != image->h())
	{
		info->image = image;
		return false;
	}
	delete image;
	return true;
}

bool MainWindow::load(const char* filename)
{
	if (NULL == filename || 0 == *filename)
	{
		reset(new LogoImage());
		return true;
	}

	MultipageImage* image = NULL;
	do
	{
		{
			LoadInfo info = {filename, NULL};
			plugin::apply(pluginLoad,  &info);
			if (NULL != info.image)
			{
				image = info.image;
				break;
			}
		}

		image = new DirImage(filename);
		if (NULL != image && 0 != image->w() && 0 != image->h())
			break;
		delete image;
		image = NULL;
	}
	while (false);

	if (NULL == image)
	{
		fl_alert(_("Can not load `%s'."), filename);
		return false;
	}
	reset(image);
	return true;
}

MainWindow::~MainWindow()
{
	assert(NULL != window);
	window = NULL;
	reset(NULL);

	delete uiImageBox_;
	delete uiScroll_;

	delete uiToolbar_;
	delete[] uiMenuItems_;
	delete uiMenu_;

	delete uiToolbarBox_;

	for (std::vector<Fl_Image*>::iterator it = uiImages_.begin(); it != uiImages_.end(); ++it)
	{
		delete *it;
	}
	uiImages_.clear();
}

void MainWindow::resize(int x, int y, int cx, int cy)
{
	baseClass::resize(x, y, cx, cy);
	updateImage();
}

void MainWindow::reset(MultipageImage* ptr)
{
	uncacheImage();
	delete source_;
	source_ = ptr;
};

bool pluginDesc(const plugin::Info* pi, void* ptr)
{
	if (NULL == pi || NULL == ptr)
		return false;
	std::string* info = reinterpret_cast<std::string*>(ptr);

	char buf[128];
	sprintf(buf, " * %s(%d)\n", pi->name, pi->version);
	info->append(buf);
	if (NULL != pi->desc)
	{
		const char* desc = pi->desc();
		if (NULL != desc)
		{
			info->append(desc);
			info->append("\n");
		}
	}
	info->append("\n");

	return true;
}

bool pluginPattern(const plugin::Info* pi, void* ptr)
{
	if (NULL == pi || NULL == ptr)
		return false;
	std::string* info = reinterpret_cast<std::string*>(ptr);

	if (NULL != pi->pattern)
	{
		const char* f = pi->pattern();
		if (NULL != f)
		{
			info->append("\t");
			info->append(f);
		}
	}
	return true;
}

void MainWindow::onAbout(bool isPlugs)
{
	Fl_Widget* ptr = fl_message_icon();

	ptr->label("");

	Fl_Pixmap ico(flviewer);
	ptr->image(&ico);

	Fl_Align oldalign = ptr->align();
	ptr->align(Fl_Align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_IMAGE_BACKDROP));

	if (isPlugs)
	{
		std::string pluginfo;
		plugin::apply(pluginDesc, &pluginfo);
		if (pluginfo.empty())
			pluginfo = _("Plugins are not available.");
		fl_message(_(PLUGS_ABOUT), pluginfo.c_str());
	}
	else
	{
		fl_message(_(ABOUT), PACKAGE, VERSION);
	}

	ptr->image(NULL);
	ptr->label(NULL);
	ptr->align(oldalign);
}

void MainWindow::onOpen()
{
	std::string formats = _("All Files (*)"
							"\tJPEG Files (*.{jpg,jpeg})"
							"\tBitmap Files (*.{png,gif,bmp})"
							"\tX-Window Files (*.{xpm,xbm})");
	plugin::apply(pluginPattern, &formats);

	Fl_File_Chooser::add_favorites_label = _("Add to Favorites");
	Fl_File_Chooser::all_files_label = _("All Files (*)");
	Fl_File_Chooser::custom_filter_label = _("Custom Filter");
	Fl_File_Chooser::existing_file_label = _("Please choose an existing file!");
	Fl_File_Chooser::favorites_label = _("Favorites");
	Fl_File_Chooser::filename_label = _("Filename:");
	Fl_File_Chooser::filesystems_label = _("File Systems");
	Fl_File_Chooser::hidden_label = _("Show hidden files:");
	Fl_File_Chooser::manage_favorites_label = _("Manage Favorites");
	Fl_File_Chooser::preview_label = _("Preview");
	Fl_File_Chooser::show_label = _("Show:");

	Fl_File_Chooser fc(".", formats.c_str(), Fl_File_Chooser::SINGLE, _("Open file"));

	fc.show();
	while (fc.shown())
		Fl::wait();

	if (NULL != fc.value())
	{
		load(fc.value());
		uncacheImage();
		commandCallback(this, MainWindow::CMD_ZOOM_FIT);
	}
}

void MainWindow::onZoomFit(bool isPage)
{
	if (NULL != source_ && 0 != source_->w() && 0 != source_->h())
	{
		float scale = (uiScroll_->w()-Fl::scrollbar_size()) * 1.0f / source_->w();
		if (isPage)
		{
			const float sx = uiScroll_->w() * 1.0f / source_->w();
			const float sy = uiScroll_->h() * 1.0f / source_->h();
			scale = std::min(sx, sy);
			if (scale > SMART_ZOOM_MAX)
				scale = SMART_ZOOM_MAX;
		}
		setScale(scale);
	}
	else
	{
		setScale(1.0f);
	}
}

void MainWindow::onPageCustom()
{
	const size_t npages = source_->num_pages();

	if (1 == npages)
		fl_alert(_("Only one page is available."));
	else
	{
		char str[32]; // 10^32 should be enouth 10^19 is a qword
		snprintf(str, sizeof(str) / sizeof(*str) - 1, "%zu", source_->page() + 1);

		const char* value = fl_input(
								N_("Input page's number (total one page):",
								   "Input page's number (total %zu pages):",
								   npages),
								str, npages);
		if (NULL != value)
		{
			size_t p;
			if (1 == sscanf(value, "%zu", &p) && p > 0)
			{
				setPage(p - 1);
			}
			else
				fl_alert(_("Invalid value `%s' can not be a integer value."), value);
		}
	}
}

void MainWindow::onZoomCustom()
{
	const char* value = fl_input(_("Magnification (%%):"));
	if (NULL != value)
	{
		float f;
		if (1 == sscanf(value, "%f", &f) && f > 0)
			setScale(f / 100.0f);
		else
			fl_alert(_("Invalid value `%s' can not be magnification."), value);
	}
}


void MainWindow::onCommand(Fl_Widget*, long cmd)
{
	if (cmd >= CMD_ZOOM_INDEX && cmd < CMD_ZOOM_INDEX_MAX)
	{
		setScale(static_cast<int>(cmd - MainWindow::CMD_ZOOM_INDEX));
		return;
	}

	int add = 1;

	switch (cmd)
	{
	case MainWindow::CMD_EXIT:
		hide();
		return;
	case MainWindow::CMD_OPEN:
		onOpen();
		return;
	case CMD_ZOOM_WIDTH:
		add = 0;
	case MainWindow::CMD_ZOOM_FIT:
		onZoomFit(add);
		return;
	case MainWindow::CMD_ZOOM_OUT:
		add = -1;
	case MainWindow::CMD_ZOOM_IN:
		addScale(add);
		return;
	case MainWindow::CMD_PAGE_TO:
		onPageCustom();
		return;
	case MainWindow::CMD_ZOOM_TO:
		onZoomCustom();
		return;
	case MainWindow::CMD_PAGE_PREV:
		add = -1;
	case MainWindow::CMD_PAGE_NEXT:
		{
			const size_t npages = source_->num_pages();
			setPage((source_->page() + npages + add) % npages);
			return;
		}
	case CMD_PAGE_LAST:
		add = 0;
	case CMD_PAGE_FIRST:
	{
		setPage(add?0:(source_->num_pages()-1));
		return;
	}
	case MainWindow::CMD_ABOUT:
		add = 0;
	case CMD_ABOUT_PLUGINS:
		onAbout(add);
		return;

	}
}

bool MainWindow::setPage(size_t page)
{
	const size_t npages = source_->num_pages();
	if (npages <= 1)
		return false;

	if (page > npages)
	{
		/// TRANSLATORS: This message has not plural forms
		fl_alert(_("Incorrect value `%zu', must be not greater then %zu."), page + 1, npages);
		return false;
	}

	const size_t    w = source_->w(),
					h = source_->h();
	fl_cursor(FL_CURSOR_WAIT, FL_BLACK, FL_WHITE);
	const bool result = source_->page(page);
	fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);

	if (!result)
		fl_alert(_("The page number `%zu' can not be found in the document."), page+1);

	uncacheImage();
	if (source_->w() == w && source_->h() == h)
		updateImage();
	else
		commandCallback(this, MainWindow::CMD_ZOOM_FIT);
	return result;
}


void MainWindow::updateCaption(void)
{
	if (NULL == source_ || NULL == source_->name())
	{
		label(PACKAGE);
		return;
	}

	static const size_t ndot 	= 3;			// quantity of dots
	static const size_t max_len = 2*ndot + 64;	// must be more then 2*ndot+1
	char *cutted = NULL;

	const char * name = source_->name();
	size_t len = fl_utf8towc(name, strlen(name), NULL, 0);

	if (len >= max_len)
	{

		wchar_t *m = new wchar_t[len+1];
		fl_utf8towc(name, strlen(name), m, len+1);

		// head ... [CENTER OF STRING] tail
		const size_t half = max_len >> 1;
		wchar_t * const phalf = m + half;
		wmemset(phalf-ndot, L'.', ndot);
		wmemmove(phalf, m+len-half, half);
		m[max_len] = 0;

		size_t u8len = fl_utf8fromwc(NULL, 0, m, max_len)+1;
		name = cutted = new char[u8len];
		fl_utf8fromwc(cutted, u8len, m, max_len);
		delete[] m;
	}

	copy_label(name);

	if (NULL != cutted)
		delete[] cutted;
}

int MainWindow::handle(int event)
{
	switch (event)
	{
	case FL_KEYUP:
		{
			const int key = Fl::event_key();
			if (FL_Control_L == key || FL_Control_R == key)
				isCtrlPressed_ = false;
		}
		break;

	case FL_KEYDOWN:
		{
			const int key = Fl::event_key();
			switch (key)
			{
			case FL_Control_L:
			case FL_Control_R:
				isCtrlPressed_ = true;
				break;
			case '-':
			case '_':
			case '-'+FL_KP:
				commandCallback(this, MainWindow::CMD_ZOOM_OUT);
				return 1;
			case '=':
			case '+':
			case '+'+FL_KP:
				commandCallback(this, MainWindow::CMD_ZOOM_IN);
				return 1;
			}
		}
		break;

	case FL_PUSH:
		{
			if (Fl::event_button() == FL_LEFT_MOUSE)
			{
				if (baseClass::handle(event))
					return 1;

				if (uiImageBox_->w() > uiScroll_->w() || uiImageBox_->h() > uiScroll_->h())
				{
					dragIsEnabled_ = true;
					Fl::grab(this);
					dragX_ = uiScroll_->xposition() + Fl::event_x();
					dragY_ = uiScroll_->yposition() + Fl::event_y();
					fl_cursor(FL_CURSOR_MOVE, FL_BLACK, FL_WHITE);
					return 1;
				}
			}
			else if (Fl::event_button() == FL_RIGHT_MOUSE)
			{
				commandCallback(this, MainWindow::CMD_ZOOM_OUT);
				return 1;
			}
			break;
		}

	case FL_DRAG:
		if (Fl::event_button() == FL_LEFT_MOUSE)
		{
			if (baseClass::handle(event))
				return 1;

			if (dragIsEnabled_)
			{
				int x = dragX_ - Fl::event_x();
				int y = dragY_ - Fl::event_y();

				bool xmove = true, ymove = true;
				const int xmax =  uiImageBox_->w() - uiScroll_->w() + uiScroll_->scrollbar.w();
				const int ymax =  uiImageBox_->h() - uiScroll_->h() + uiScroll_->hscrollbar.h();

				if (x <= 0)
				{
					xmove = false;
					dragX_ -= x;
					x = 0;
				}
				else if (x >= xmax)
				{
					xmove = false;
					dragX_ -= (x - xmax);
					x = xmax;
				}

				if (y <= 0)
				{
					ymove = false;
					dragY_ -= y;
					y = 0;
				}
				else if (y >= ymax)
				{
					ymove = false;
					dragY_ -= (y - ymax);
					y = ymax;
				}

				if (xmove || ymove)
					uiScroll_->scroll_to(x, y);
			}
			return 1;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_LEFT_MOUSE)
		{
			if (dragIsEnabled_)
				Fl::grab(NULL);

			if (baseClass::handle(event))
				return 1;

			if (dragIsEnabled_)
			{
				if (abs(dragX_ - Fl::event_x()) < PITCH && abs(dragY_ - Fl::event_y()) < PITCH)
					commandCallback(this, MainWindow::CMD_ZOOM_IN);

				fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
				dragIsEnabled_ = false;
				return 1;
			}
			else
				commandCallback(this, MainWindow::CMD_ZOOM_IN);
		}
		break;
	case FL_MOUSEWHEEL:
		if (isCtrlPressed_)
		{
			commandCallback(this, (Fl::event_dy() > 0) ? MainWindow::CMD_ZOOM_IN : MainWindow::CMD_ZOOM_OUT);
			return 1;
		}
		break;

	case FL_SHOW:
		return 1;
	}
	return baseClass::handle(event);
}

void MainWindow::commandCallback(Fl_Widget* widget, long cmd)
{
	if (NULL != window)
		window->onCommand(widget, cmd);
	else
		assert(false);
}

void MainWindow::commandCallback(Fl_Widget* widget, void* ptr)
{
	const long cmd = static_cast<long>(reinterpret_cast<intptr_t>(ptr));
	commandCallback(widget, cmd);
}
