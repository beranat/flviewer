// Plugin.h: API for plugin and methods for modules manager
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
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define PLGAPI_VERSION 		1
#define PLGAPI_ENTRYPOINT 	"module"
#define PLGPREFIX 			"fvp_"

#define PLGAPI_PRIORITY_HIGH    (-5)
#define PLGAPI_PRIORITY_NORMAL  (0)
#define PLGAPI_PRIORITY_LOW 	(5)

#include "MultipageImage.h"

namespace plugin
{
typedef bool (*FnInit)();
typedef void (*FnDone)();

typedef const char* (*FnDesc)();
typedef MultipageImage* (*FnLoad)(const char *filename);
typedef const char* (*FnPattern)();

struct Info
{
	const char 	*name;
	int 		version;
	int 		priority;

	FnInit		init;
	FnDone		done;

	FnDesc		desc;
	FnPattern	pattern;
	FnLoad 		load;
};

#ifndef FLVIEWER_PLUGIN
void init(const char* dirname);
void release();
typedef bool (*FnEnumerate)(const Info* info, void *data);
void apply(FnEnumerate fn, void* data = NULL);
#endif
};

#ifdef FLVIEWER_PLUGIN
extern "C" plugin::Info module;
#endif

#endif //#ifndef __PLUGIN_H__
