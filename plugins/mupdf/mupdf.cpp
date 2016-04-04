// mupdf.cpp: PDF document support (using MuPDF library)
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <stdio.h>
#include <inttypes.h>

#include <FL/fl_ask.H>

#include <Plugin.h>
extern "C" plugin::Info module;
#include "i18n.h"

#include "mupdf.h"

#define OPTIMAL_WIDTH	800
#define MIN_HEIGHT		16

volatile int 	 semaphore_ = -1;
fz_context*		 context_ = NULL;
std::string 	 pattern_;

void unlock()
{
	static struct sembuf _unlock = { 0, 1, IPC_NOWAIT};
	if ((semop(semaphore_, &_unlock, 1)) != 0)
		fprintf(stderr, "%s: Unlock error %d.\n", module.name, errno);
}

bool lock()
{
	static struct sembuf _lock = {0, -1, 0};
	if ((semop(semaphore_, &_lock, 1)) == 0)
		return true;
	fprintf(stderr, "%s: Lock error %d.\n", module.name, errno);
	return false;
}

class Guard
{
	bool isLocked_;
public:
	Guard() : isLocked_(false)
	{
		lock();
	}

	~Guard()
	{
		if (isLocked_)
			::unlock();
	}

	void lock()
	{
		isLocked_ = ::lock();
	}

	void unlock()
	{
		::unlock();
		isLocked_ = false;
	}
};

MuPDF::MuPDF(const char* filename) : filename_(filename),
	document_(NULL),
	page_(NULL), index_(0),
	w_(0), h_(0),
	scalew_(1.0f), scaleh_(1.0f)
{
	assert(NULL != context_);
	{
		Guard guard;
		fz_try(context_)
		{
			document_ = fz_open_document(context_, filename);
			if (NULL == document_)
				fz_throw(context_, 1, "Not supported format");

			if (fz_needs_password(context_, document_))
			{
				bool isEncrypted = true;
				const char* msg = _("Please enter password to read the document\n%s:");
				do
				{
					guard.unlock();
					fl_beep(FL_BEEP_PASSWORD);
					const char* pass = fl_password(msg, NULL, filename);
					guard.lock();
					if (NULL == pass)
						break;

					if (fz_authenticate_password(context_, document_, pass))
					{
						isEncrypted = false;
						break;
					}
					msg = _("Incorrect password, try again enter password to read the document\n%s:");
				}
				while (true);

				if (isEncrypted)
					fz_throw(context_, 1, "Encrypted");

			}
		}
		fz_catch(context_)
		{
			fprintf(stderr, "%s: Error %s while oppening `%s'.\n", module.name, fz_caught_message(context_), filename);
			if (NULL != document_)
			{
				fz_drop_document(context_, document_);
				document_ = NULL;
			}
			return;
		}
	}
	page(0);
}

MuPDF::~MuPDF()
{
	release();
	if (NULL != document_)
	{
		Guard guard;
		fz_drop_document(context_, document_);
		document_ = NULL;
	}
}

const char*  MuPDF::name() const
{
	return name_.c_str();
}

size_t MuPDF::num_pages() const
{
	if (NULL != document_)
	{
		Guard guard;
		const int i = fz_count_pages(context_, document_);
		return (i > 1) ? i : static_cast<size_t>(1);
	}
	return 0;
}


void MuPDF::release()
{
	name_.clear();
	w_ = h_ = 0;
	scalew_ = scaleh_ = 1.0f;

	if (NULL != page_)
	{
		Guard guard;
		fz_drop_page(context_, page_);
		page_ = NULL;
	}

}

size_t MuPDF::page() const
{
	return index_;
}

bool MuPDF::page(size_t page)
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
		snprintf(buf, sizeof(buf) / sizeof(*buf) - 1, _("[%zd/%zd] "), page + 1, num_pages);
		name_ = buf;
	}
	name_ += filename_;

	fz_rect r;
	{
		Guard guard;

		page_ = fz_load_page(context_, document_, page);
		if (NULL == page_)
		{
			fprintf(stderr, "%s: Can not load page %zu in `%s'.\n", module.name, page, filename_.c_str());
			return false;
		}
		fz_bound_page(context_, page_, &r);
	}
	const float rw = r.x1 - r.x0;
	const float rh = r.y1 - r.y0;
	const float ratio = rh / rw;

	w_ = OPTIMAL_WIDTH;
	h_ = static_cast<size_t>(ratio * OPTIMAL_WIDTH);

	if (h_ < MIN_HEIGHT)
	{
		w_ = static_cast<size_t>(MIN_HEIGHT / ratio);
		h_ = MIN_HEIGHT;
	}

	scalew_ = static_cast<float>(1.0f / rw);
	scaleh_ = static_cast<float>(1.0f / rh);
	return true;
}

Fl_Image* MuPDF::copy(int cx, int cy)
{
	if (NULL == page_)
		return NULL;

	Guard guard;

	fz_matrix tm;
	fz_scale(&tm, scalew_ * cx, scaleh_ * cy);

	fz_pixmap* pix = NULL;

	fz_try(context_)
	pix = fz_new_pixmap_from_page(context_, page_, &tm, fz_device_rgb(context_));
	fz_catch(context_)
	{
		fprintf(stderr, "%s: Render error `%s' in %s:%zu.\n", module.name, fz_caught_message(context_), filename_.c_str(), index_);
		return NULL;
	}

	Fl_RGB_Image fl((const uchar*)pix->samples, pix->w, pix->h, 4);
	Fl_Image* ptr = fl.copy(cx, cy);
	fz_drop_pixmap(context_, pix);
	return ptr;
}

// * API
MultipageImage* load(const char* filename)
{
	if (NULL == context_)
	{
		fprintf(stderr, "%s: Module has not been inited.\n", module.name);
		return NULL;
	}
	return new MuPDF(filename);
}

const char* pattern()
{
	return pattern_.c_str();
}

const char* desc()
{
	return _("This software uses MuPDF parsing and rendering PDF, XPS, CBZ, and(or) EPUB documents.\n"
			 "See http://mupdf.com/ for details.");
}

void register_document(fz_document_handler& handler, const char* name, const char* pattern)
{
	fz_try(context_)
	fz_register_document_handler(context_, &handler);
	fz_catch(context_)
	{
		fprintf(stderr, "%s: Format `%s' registration error `%s'.\n", module.name, name, fz_caught_message(context_));
		return;
	}

	if (!pattern_.empty())
		pattern_.append("\t");
	pattern_.append("MuPDF ");
	pattern_.append(pattern);
}

void done()
{
	if (NULL != context_)
	{
		fz_drop_context(context_);
		context_ = NULL;
	}

	if (-1 != semaphore_)
		semctl(semaphore_, 0, IPC_RMID, 0);
	semaphore_ = -1;
}

bool init()
{
	I18N_INIT();
	semaphore_ = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | S_IRWXU);
	if (-1 == semaphore_)
	{
		fprintf(stderr, "%s: Can not create semaphore (%d).\n", module.name, errno);
		return false;
	}

	union semun
	{
		int val;
		struct semid_ds* buf;
		ushort* array;
	} arg;
	arg.val = 1;
	if (-1 == semctl(semaphore_, 0, SETVAL, arg))
	{
		fprintf(stderr, "%s: Can not init semaphore (%d).\n", module.name, errno);
		done();
		return false;
	}

	context_ = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (NULL == context_)
	{
		fprintf(stderr, "%s: Can not create context.\n", module.name);
		done();
		return false;
	}

	register_document(pdf_document_handler, "PDF",  _("Portable Document Files (*.pdf)"));
	register_document(xps_document_handler, "XPS",  _("XML Paper Specification Files (*.xps)"));
	register_document(cbz_document_handler, "cbz",  _("Comic Book Archive (*.{cbz,zip})"));
	register_document(epub_document_handler, "EPub", _("E-Book Files (*.epub)"));
	register_document(img_document_handler, "Img",  _("Images Files (*.{jfif,jpe,jfif-tbnl,jfif})"));
	register_document(tiff_document_handler, "Tiff", _("Tagged Image Files (*.{tif,tiff})"));
	register_document(html_document_handler, "HTML/XML", _("HTML/XML Files (*.xml,xhtml,html,htm)"));
//	register_document(gprf_document_handler, "Gproof",_("Ghost Proof Files (*.gproof)"));

	if (pattern_.empty())
	{
		fprintf(stderr, "%s: No documents have been registered.\n", module.name);
		done();
		return false;
	}

	return true;
}

plugin::Info module =
{
	"fvp_mupdf",
	PLGAPI_VERSION,
	PLGAPI_PRIORITY_NORMAL,
	init,
	done,
	desc,
	pattern,
	load
};
