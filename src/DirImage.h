// DirImage.h : Collection of images from directory
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
#ifndef __DirImage_H__
#define __DirImage_H__

#include <vector>
#include <string>

#include "MultipageImage.h"

class DirImage : public MultipageImage
{
public:
	typedef MultipageImage baseClass;

public:
	DirImage(const char *filename);
	DirImage(size_t count, const char *filenames[]);
	virtual ~DirImage();

	virtual size_t num_pages() const;
	virtual size_t page() const;
	virtual const char*  name() const;
	virtual bool page(size_t page);

	virtual size_t  w() const;
	virtual size_t  h() const;

	virtual Fl_Image *copy(int w, int h);

protected:
	static bool isCompatible(const char *name);

	bool reset(Fl_Image *source);

	bool load(const char *filename);
	bool load(size_t index);

	void enum_files() const;

	std::string filename_;

	Fl_Image *source_;

	mutable std::vector<std::string> 	files_;
	mutable size_t 						page_;
};
#endif //#ifndef DirImage_H
