// TiffImage.h: TIFF image support (using libtiff)
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
#ifndef __TiffImage_H__
#define __TiffImage_H__

#include <vector>
#include <string>
#include <tiffio.h>
#include "MultipageImage.h"

class TiffImage : public MultipageImage
{
public:
	typedef MultipageImage baseClass;

public:
	TiffImage(const char *filename);
	virtual ~TiffImage();

	virtual const char*  name() const;

	virtual size_t num_pages() const	{ return num_pages_; 	}
	virtual size_t page() const			{ return page_;			}
	virtual bool page(size_t page);

	virtual Fl_Image* copy(int w, int h);

	virtual size_t  w() const;
	virtual size_t  h() const;

protected:
	void release();

	std::string 	filename_;
    TIFF* 			tiff_;
	void*			tiff_raster_;
	size_t			num_pages_;
	size_t			page_;
	std::string		name_;
	Fl_RGB_Image*	image_;
};

#endif //#ifndef TiffImage_H
