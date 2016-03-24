// mupdf.h: PDF document support (using MuPDF library)
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
#ifndef __MUPDF_H__
#define __MUPDF_H__

#include <vector>
#include <string>
#include "MultipageImage.h"

extern "C" {
#include <mupdf/fitz.h>
}

class MuPDF : public MultipageImage
{
public:
	typedef MultipageImage baseClass;

public:
	MuPDF(const char *filename);
	virtual ~MuPDF();

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

	fz_document		*document_;
	fz_page			*page_;

	size_t					index_;
	size_t					w_, h_;
	float					scalew_, scaleh_;
};

#endif //#ifndef __MUPDF_H__
