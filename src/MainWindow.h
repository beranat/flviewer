// MainWindow.h : Main window class
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
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <vector>
#include <string>
#include <assert.h>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tooltip.H>

#include "MultipageImage.h"

class MainWindow : public Fl_Double_Window
{
protected:
	static const float scales[];
	static float scaleGet(int index);
	static int 	 scaleCeil(const float value);

public:
	typedef Fl_Double_Window baseClass;

	enum
	{
		CMD_NONE = 0,
		CMD_OPEN,
		CMD_EXIT,
		CMD_ABOUT,
		CMD_ABOUT_PLUGINS,
		CMD_ZOOM_FIT,
		CMD_ZOOM_WIDTH,
		CMD_ZOOM_TO,
		CMD_ZOOM_IN,
		CMD_ZOOM_OUT,
		CMD_PAGE_FIRST,
		CMD_PAGE_PREV,
		CMD_PAGE_NEXT,
		CMD_PAGE_LAST,
		CMD_PAGE_TO,

		CMD_ZOOM_INDEX = 0x1000,	// must be a setted a bit 100, 200, 400, 800 - OK 300 - not
		CMD_ZOOM_Z06   = 0x1000,
		CMD_ZOOM_Z08,
		CMD_ZOOM_Z12,
		CMD_ZOOM_Z25,
		CMD_ZOOM_Z33,
		CMD_ZOOM_Z50,
		CMD_ZOOM_Z67,
		CMD_ZOOM_Z75,
		CMD_ZOOM_100,
		CMD_ZOOM_125,
		CMD_ZOOM_150,
		CMD_ZOOM_200,
		CMD_ZOOM_300,
		CMD_ZOOM_400,
		CMD_ZOOM_600,
		CMD_ZOOM_800,
		CMD_ZOOM_1200,
		CMD_ZOOM_1600,
		CMD_ZOOM_INDEX_MAX
	};

private:
	static MainWindow* window;

protected:
	int 		 			uiHeight_;
	std::vector<Fl_Image*> 	uiImages_;

	Fl_Menu_Item*	uiMenuItems_;
	Fl_Menu_Bar* 	uiMenu_;
	Fl_Box*			uiToolbarBox_;
	Fl_Pack*		uiToolbar_;
	Fl_Scroll*	 	uiScroll_;
	Fl_Box*  		uiImageBox_;

	int 	scaleIndex_;
	bool 	scaleIsCustom_;
	float 	scaleValue_;

	bool 	dragIsEnabled_;
	int 	dragX_, dragY_;

	bool	isCtrlPressed_;

	bool 	isAllowedSizeWarning_;

	MultipageImage*				 source_;

protected:
	float scale() const;
	int findNearScale(const float value, float& scale) const;
	void addScale(int index);
	void setScale(int index);
	void setScale(float value);

	bool setPage(size_t page);

	void uncacheImage();
	void updateImage();

	bool allowLargeImageWarning(int w, int h);

	void reset(MultipageImage* ptr);
	void updateCaption(void);
	int onHotKey(int key);

	virtual void resize(int x, int y, int cx, int cy);
	int handle(int event);

	virtual void onCommand(Fl_Widget*, long cmd);

	void onZoomCustom();
	void onZoomFit(bool isPage);
	void onPageCustom();
	void onOpen();
	void onAbout(bool isPlugs);
public:
	MainWindow(int x, int y, int cx, int cy, int nfiles, const char* files[]);
	~MainWindow();

	static MainWindow* getWindow()
	{
		return window;
	}

	bool load(const char* filename);
	bool load(size_t nf, const char* fs[]);

public:
	static void commandCallback(Fl_Widget* widget, long cmd);
	static void commandCallback(Fl_Widget* widget, void* ptr);
};

#endif //#ifndef __MAINWINDOW_H__
