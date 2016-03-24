// PdfImage.h: PDF document support (using poppler-cpp)
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
#ifndef __PdfImage_H__
#define __PdfImage_H__

#include <vector>
#include <string>
#include "MultipageImage.h"

namespace poppler
{
class page;
class document;
class page_renderer;
};


class PdfImage : public MultipageImage
{
public:
	typedef MultipageImage baseClass;

public:
	PdfImage(const char *filename);
	virtual ~PdfImage();

	virtual const char*  name() const;
	virtual size_t page() const;
	virtual size_t num_pages() const;
	virtual bool page(size_t index);

	virtual Fl_Image* copy(int w, int h);

	virtual size_t  w() const	{ return w_; }
	virtual size_t  h() const   { return h_; }

protected:
	void release();

	std::string 	filename_;
	std::string		name_;

	poppler::document*		document_;
	poppler::page_renderer*	render_;

	poppler::page*			page_;
	size_t					index_;
	size_t					w_, h_;
	float					scalew_, scaleh_;
};

#endif //#ifndef PdfImage_H
