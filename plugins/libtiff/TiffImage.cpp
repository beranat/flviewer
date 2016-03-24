// TiffImage.cpp: TIFF image support (using libtiff)
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

#include <stdio.h>
#include <Plugin.h>

#include "TiffImage.h"

#define NAME_SEPARATOR "`'"

// ** localization **
static const char* pageNumber 	= "[%zu/%zu] ";
static const char* license 		= 	"This plugins uses libtiff for reading tiff-files\n"
									"Copyright (c) 1988-1997 Sam Leffler\n"
									"Copyright (c) 1991-1997 Silicon Graphics, Inc.\n"
									"For full license see http://www.remotesensing.org/libtiff/.\n";
static const char* patternMask  = "Tagged Image Files (*.{tif,tiff})";


TiffImage::TiffImage(const char* filename) : filename_(filename), tiff_(NULL), tiff_raster_(NULL)
												, num_pages_(0), page_(0), image_(NULL)
{
	tiff_ = TIFFOpen(filename_.c_str(), "r");
	if (NULL != tiff_)
	{
		do
		{
			++num_pages_;
		}
		while (TIFFReadDirectory(tiff_));
		page(0);
	}
	else
		fprintf(stderr, "%s: `%s' is not a supported file.\n", module.name, filename);
}

TiffImage::~TiffImage()
{
	release();
	num_pages_ = 0;
	if (NULL != tiff_)
		TIFFClose(tiff_);
}

const char*  TiffImage::name() const
{
	return name_.c_str();
}

void TiffImage::release()
{
	name_.clear();

	delete image_;
	image_ = NULL;

	if (tiff_raster_ != NULL)
		_TIFFfree(tiff_raster_);
	tiff_raster_ = NULL;
}

bool TiffImage::page(size_t page)
{
	if (NULL == tiff_)
		return false;

	if (page >= num_pages_)
		return false;

	page_ = page;
	release();

	if (num_pages_ > 1)
	{
		char buf[128] = { 0 };
		snprintf(buf, sizeof(buf) / sizeof(*buf) - 1, pageNumber, page_ + 1, num_pages_);
		name_ = buf;
	}
	name_ += filename_;

	if (1 != TIFFSetDirectory(tiff_, page_))
	{
		fprintf(stderr, "%s: Cannot get read metainfo for %s:%zu.\n", module.name, filename_.c_str(), page_);
		return false;
	}

	if (num_pages_ > 1)
	{
		char* name = NULL;
		if (!TIFFGetField(tiff_, TIFFTAG_PAGENAME, &name))
			name = NULL;

		if (NULL != name)
		{
			name_.insert(0, NAME_SEPARATOR" ");
			name_.insert(1, name);
		}
	}

	uint32 w = 0, h = 0;
	if (!TIFFGetField(tiff_, TIFFTAG_IMAGEWIDTH, &w) || !TIFFGetField(tiff_, TIFFTAG_IMAGELENGTH, &h))
	{
		fprintf(stderr, "%s: Cannot get geometry for %s:%zu.\n", module.name, filename_.c_str(), page_);
		return false;
	}

	const size_t npixels = w * h;
	if (0 != npixels)
	{
		tiff_raster_ = _TIFFmalloc(npixels * sizeof(uint32));
		if (NULL != tiff_raster_)
		{
			if (TIFFReadRGBAImageOriented(tiff_, w, h, reinterpret_cast<uint32*>(tiff_raster_), ORIENTATION_TOPLEFT, 0))
			{
				image_ = new Fl_RGB_Image(reinterpret_cast<const uchar*>(tiff_raster_), w, h, 4, 0);
			}
			else
				fprintf(stderr, "%s: Cannot get bitmap for %s:%zu.\n", module.name, filename_.c_str(), page_);
		}
	}

	if (NULL != image_)
		return true;

	release();
	return false;
}

Fl_Image* TiffImage::copy(int w, int h)
{
	return (NULL != image_) ? image_->copy(w, h) : NULL;
}

size_t  TiffImage::w() const
{
	return (NULL != image_) ? image_->w():0;
}

size_t  TiffImage::h() const
{
	return (NULL != image_) ? image_->h():0;
}

MultipageImage* load(const char* filename)
{
	return new TiffImage(filename);
}

const char* pattern()
{
	return patternMask;
}

const char* desc()
{
	return license;
}

bool init()
{
	return true;
}

plugin::Info module =
{
	"fvp_libtiff",
	PLGAPI_VERSION,
	PLGAPI_PRIORITY_NORMAL,
	init,
	NULL,
	desc,
	pattern,
	load
};
