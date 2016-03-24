// DirImage.cpp : Collection of images from directory
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
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>

#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_XBM_Image.H>
#include <FL/Fl_XPM_Image.H>

#include "DirImage.h"

DirImage::DirImage(const char* filename) : source_(NULL), page_(-1)
{
	load(filename);
}

DirImage::DirImage(size_t count, const char* filenames[]) : source_(NULL), page_(-1)
{
	if (0 != count && NULL != filenames)
	{
		files_.reserve(count);

		bool isLoaded = false;

		for (size_t i = 0; i < count; ++i)
		{
			files_.push_back(filenames[i]);
			if (!isLoaded)
				isLoaded = load(i);
		}
	}
}

DirImage::~DirImage()
{
	reset(NULL);
}

size_t DirImage::num_pages() const
{
	if (files_.empty())
		enum_files();
	return files_.size();
}

size_t DirImage::page() const
{
	if (files_.empty())
		enum_files();
	return page_;
}

bool DirImage::page(size_t page)
{
	if (page >= num_pages())
		return false;
	return load(page);
}

bool DirImage::isCompatible(const char* name)
{
	static const char* next[] = { "jpg", "jpeg", "png", "gif", "bmp", "xbm", "xpm"};
	const size_t len = strlen(name);
	for (size_t i = 0; i < sizeof(next) / sizeof(*next); ++i)
	{
		const size_t nlen = strlen(next[i]);
		if (len >= nlen && 0 == strncmp(next[i], name + len - nlen, nlen))
			return true;
	}
	return false;
}

#define TRY_LOAD(CLASS) { \
	image = new CLASS(filename); \
	if (NULL != image && 0 < image->w() && 0 < image->h()) \
		break; \
	delete image; image= NULL; \
	}

bool DirImage::load(const char* filename)
{
	Fl_Image* image = NULL;
	do
	{
		TRY_LOAD(Fl_JPEG_Image);
		TRY_LOAD(Fl_PNG_Image);
		TRY_LOAD(Fl_BMP_Image);
		TRY_LOAD(Fl_GIF_Image);
		TRY_LOAD(Fl_XBM_Image);
		TRY_LOAD(Fl_XPM_Image);
	}
	while (false);

	filename_ = filename;
	return reset(image);
}

const char*  DirImage::name() const
{
	return filename_.c_str();
}

bool DirImage::load(size_t index)
{
	if (index >= files_.size())
		return false;

	page_ = index;
	return load(files_[page_].c_str());
}

void DirImage::enum_files() const
{
	if (filename_.empty() || !files_.empty())
		return;

	std::string dirname("./");
	std::string::size_type pos = filename_.rfind('/');
	if (pos != std::string::npos)
		dirname.assign(filename_.c_str(), pos + 1);

	DIR*    dir = opendir(dirname.c_str());
	if (dir)
	{
		struct dirent* item;
		while ((item = readdir(dir)) != NULL)
		{
			std::string file(dirname);
			file.append(item->d_name);

			struct stat buf;
			if (0 == stat(file.c_str(), &buf))
			{
				if (S_ISREG(buf.st_mode) && isCompatible(file.c_str()))
					files_.push_back(file);
			}
		}
		closedir(dir);
	}

	std::sort(files_.begin(), files_.end());

	std::vector<std::string>::iterator it = std::find(files_.begin(), files_.end(), filename_);
	if (it != files_.end())
		page_ = std::distance(files_.begin(), it);
	else
	{
		page_ = files_.size();
		files_.push_back(filename_);
	}
}

bool DirImage::reset(Fl_Image* source)
{
	delete source_;
	source_ = source;

	if (NULL == source_)
		return false;

	return true;
}

size_t  DirImage::w() const
{
	return (NULL != source_)? source_->w() : 0;
}

size_t  DirImage::h() const
{
	return (NULL != source_)? source_->h() : 0;
}

Fl_Image *DirImage::copy(int w, int h)
{
	if (NULL == source_)
		return NULL;
	return source_->copy(w, h);
}
