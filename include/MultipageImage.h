// MultipageImage.h: Interface for image(document) handler
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
#ifndef __MultipageImage_H__
#define __MultipageImage_H__

#include <cstddef>
#include <FL/Fl_Image.H>

class MultipageImage
{
	MultipageImage(const MultipageImage&);
	MultipageImage& operator=(const MultipageImage&);

protected:
	MultipageImage() { };

public:
	virtual ~MultipageImage() { };

	virtual const char*  name() const 		{ return NULL; }

	virtual size_t  num_pages() const	= 0;
	virtual size_t  page() const 		= 0;
	virtual bool 	page(size_t page) 	= 0;
	virtual size_t 	w() const 			= 0;
	virtual size_t 	h() const			= 0;

	virtual Fl_Image *copy(int w, int h) = 0;
};

#endif //#ifndef __MultipageImage_H__
