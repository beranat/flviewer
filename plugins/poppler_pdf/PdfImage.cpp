// PdfImage.cpp: PDF document support (using poppler-cpp)
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
#include <inttypes.h>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-rectangle.h>

#include <FL/fl_ask.H>

#include <Plugin.h>
extern "C" plugin::Info module;

#include "i18n.h"

#include "PdfImage.h"

#define OPTIMAL_WIDTH	800
#define MIN_HEIGHT		16

#define NAME_SEPARATOR "`'"

PdfImage::PdfImage(const char* filename) : filename_(filename),
	document_(NULL), render_(NULL),
	page_(NULL), index_(0),
	w_(0), h_(0),
	scalew_(1.0f), scaleh_(1.0f)
{
	document_ = poppler::document::load_from_file(filename);

	if (NULL != document_)
	{
		if (document_->is_locked())
		{
			const char* msg = _("Please enter password to read the document\n%s:");
			bool isEncrypted = true;
			do
			{
				fl_beep(FL_BEEP_PASSWORD);
				const char* pass = fl_password(msg, NULL, filename);
				if (NULL == pass)
					break;

				if (!document_->unlock(pass, pass))
				{
					isEncrypted = false;
					break;
				}
				msg = _("Incorrect password, try again enter password to read the document\n%s:");
			}
			while (true);

			if (isEncrypted)
			{
				delete document_;
				document_ = NULL;
				return;
			}
		}

		render_ = new poppler::page_renderer();
		if (NULL != render_)
		{
			render_->set_render_hints(poppler::page_renderer::antialiasing |
								  poppler::page_renderer::text_antialiasing |
								  poppler::page_renderer::text_hinting);
			page(0);
		}
		else
		{
			fprintf(stderr, "%s: Can not create page renderer for `%s'.\n",module.name, filename);
			delete document_;
			document_ = NULL;
		}
	}
	else
		fprintf(stderr, "%s: `%s' is not a PDF file.\n", module.name, filename);
}

PdfImage::~PdfImage()
{
	release();
	delete render_;
	delete document_;
}

const char*  PdfImage::name() const
{
	return name_.c_str();
}

size_t PdfImage::num_pages() const
{
	return (NULL != document_)?static_cast<size_t>(document_->pages()):0;
}


void PdfImage::release()
{
	delete page_;
	page_ = NULL;
	name_.clear();
	w_ = h_ = 0;
	scalew_ = scaleh_ = 1.0f;
}

size_t PdfImage::page() const
{
	return index_;
}

bool PdfImage::page(size_t page)
{
	if (NULL == document_)
		return false;

	const size_t num_pages = this->num_pages();

	if (page >= num_pages)
		return false;

	index_ = page;
	release();

	if (num_pages > 1)
	{
		char buf[128] = { 0 };
		snprintf(buf, sizeof(buf) / sizeof(*buf) - 1, _("[%zu/%zu] "), page + 1, num_pages);
		name_ = buf;
	}
	name_ += filename_;

	page_ = document_->create_page(page);
	if (NULL == page_)
	{
		fprintf(stderr, "%s: Can not load page %zu for `%s'.\n",module.name, page, filename_.c_str());
		return false;
	}

	// let's geometry be about screen
	const poppler::rectf r = page_->page_rect();
	const float rw = r.width();
	const float rh = r.height();
	const float ratio = rh / rw;

	w_ = OPTIMAL_WIDTH;
	h_ = static_cast<size_t>(ratio * OPTIMAL_WIDTH);

	if (h_ < MIN_HEIGHT)
	{
		w_ = static_cast<size_t>(MIN_HEIGHT / ratio);
		h_ = MIN_HEIGHT;
	}

	scalew_ = static_cast<float>(72.0f / rw);
	scaleh_ = static_cast<float>(72.0f / rh);

	if (num_pages > 1)
	{
		const poppler::ustring label = page_->label();
		if (!label.empty())
		{
			name_.insert(0, NAME_SEPARATOR " ");
			std::vector<char> n = label.to_utf8();
			name_.insert(1, &*n.begin(), n.size());
		}
	}
	return true;
}

Fl_Image* PdfImage::copy(int cx, int cy)
{
	if (NULL == page_ || NULL == render_)
		return NULL;

	poppler::image i = render_->render_page(page_, scalew_*cx, scaleh_*cy);
	const int 	iw = i.width(),
				ih = i.height(),
				ir = i.bytes_per_row();

	if (iw <= 0 || ih <= 0 || ir <= 0)
	{
		fprintf(stderr, "%s: Can not render page %zu for `%s'.\n", module.name, index_, filename_.c_str());
		return NULL;
	}

	int depth = 0;
	switch (i.format())
	{
	case poppler::image::format_mono:
		depth = 1;
		break;
	case poppler::image::format_rgb24:
		{
			depth = 3;
			// R G B 24 -> 'R' 'G' 'B'
			char* line = i.data();
			for (int j = 0; j < ih; ++j, line += ir)
			{
				uint8_t*  d = reinterpret_cast<uint8_t*>(line);
				for (int i = 0; i < iw; ++i, d += 4)
				{
					uint32_t p = *reinterpret_cast<uint32_t*>(d);
					d[0] = p >> 16; // R
					d[1] = p >> 8;  // G
					d[2] = p;       // B
				}
			}
			break;
		}
	case poppler::image::format_argb32:
		{
			depth = 4;
			// A R G B 32 stored as 'B' 'G' 'R' 'A' in x86
			// Fl_RGB_Image needs 'R' 'G' 'B' 'A'
			char* line = i.data();
			for (int j = 0; j < ih; ++j, line += ir)
			{
				uint8_t*  d = reinterpret_cast<uint8_t*>(line);
				for (int i = 0; i < iw; ++i, d += 4)
				{
					uint32_t p = *reinterpret_cast<uint32_t*>(d);
					d[0] = p >> 16;	// R
					d[1] = p >> 8;	// G
					d[2] = p;		// B
					d[3] = p >> 24;	// A
				}
			}
			break;
		}
	default:
		fprintf(stderr, "%s: Invalid image for page %zu for `%s'.\n",module.name, index_, filename_.c_str());
		return NULL;
	}

	Fl_RGB_Image fl((const uchar*)i.data(), iw, ih, depth, ir - iw * depth);
	return fl.copy(cx, cy);
}

// * API
MultipageImage* load(const char *filename)
{
    return new PdfImage(filename);
}

const char* pattern()
{
	return _("Portable Document Files (*.pdf)");
}

const char* desc()
{
	return _("This software uses Poppler and Poppler Cpp for PDF rendering.\n"
			 "See http://poppler.freedesktop.org/ for details.");
}

bool init()
{
	I18N_INIT();
	return true;
}

void done()
{
}

plugin::Info module =
{
	"fvp_poppler_pdf",
	PLGAPI_VERSION,
	PLGAPI_PRIORITY_NORMAL,
	init,
	done,
	desc,
	pattern,
	load
};
