// Plugin.cpp : Manages dynamic loaded modules
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
#include <string.h>

#include <ltdl.h>

#include <sys/stat.h>
#include <dirent.h>

#include <vector>
#include <algorithm>
#include <string>

#include "Plugin.h"

#include "config.h"

class PluginManager
{
private:
	std::vector<lt_dlhandle>	libs_;
	std::vector<plugin::Info*>	modules_;

private:
	PluginManager& operator=(const PluginManager&);
	PluginManager(const PluginManager&);

private:
	PluginManager()
	{
		lt_dlinit();
	}

	void load(const char* filename);

public:
	static PluginManager& instance();
	~PluginManager()
	{
		release();
		lt_dlexit();
	}

	void release();
	void init(const char* dirname);

	void operator()(plugin::FnEnumerate fn, void* data)
	{
		for (std::vector<plugin::Info*>::const_iterator	it = modules_.begin();
				it != modules_.end();
				++it)
		{
			if (!fn(*it, data))
				break;
		}
	}
private:
	static int _load(const char* filename, void* ptr)
	{
		PluginManager* self = reinterpret_cast<PluginManager*>(ptr);
		self->load(filename);
		return 0;
	}
};

PluginManager& PluginManager::instance()
{
	static PluginManager inst;
	return inst;
}

void PluginManager::release()
{
	for (std::vector<plugin::Info*>::iterator it = modules_.begin(); it != modules_.end(); ++it)
	{
		if (NULL != (*it)->done)
			(*it)->done();
	}

	modules_.clear();

	std::for_each(libs_.begin(), libs_.end(), lt_dlclose);
	libs_.clear();
}

bool sortPriority(const plugin::Info *v0, const plugin::Info *v1)
{
	return  v0->priority < v1->priority;
}

void PluginManager::init(const char* dirname)
{
	if (NULL == dirname)
		return;
	lt_dlforeachfile(dirname, _load, this);

	std::sort(modules_.begin(), modules_.end(), sortPriority);
}

void PluginManager::load(const char* filename)
{
	const char* name = strrchr(filename, '/');
#ifdef LT_DIRSEP_CHAR
	if (NULL == name)
		name = strrchr(filename, LT_DIRSEP_CHAR);
#endif

	if (NULL == name)
		name = filename;
	else
		++name;

	if (0 != strncmp(name, PLGPREFIX, strlen(PLGPREFIX)))
	{
		fprintf(stderr, "Library `%s' not starts with %s\n", filename, PLGPREFIX);
		return;
	}

	lt_dlhandle h = lt_dlopenext(filename);
	if (NULL == h)
	{
		fprintf(stderr, "Plugin `%s' load error %s\n", filename, lt_dlerror());
		return;
	}

	plugin::Info* m = reinterpret_cast<plugin::Info*>(lt_dlsym(h, PLGAPI_ENTRYPOINT));

	do
	{
		if (NULL == m)
		{
			fprintf(stderr, "Library %s have no %s\n", filename, PLGAPI_ENTRYPOINT);
			break;
		}

		if (m->version < PLGAPI_VERSION)
		{
			fprintf(stderr, "%s (%s) version %d is not compatible", m->name, filename, m->version);
			m = NULL;
			break;
		}

		if (NULL != m->init && !m->init())
		{
			fprintf(stderr, "Plugin `%s' init failure\n", filename);
			m = NULL;
			break;
		}
	} while (false);

	if (NULL == m)
	{
		lt_dlclose(h);
		return;
	}

	modules_.push_back(m);
	libs_.push_back(h);
}

void plugin::init(const char* dirname)
{
	PluginManager::instance().init((NULL!=dirname)?dirname:PLUGIN_DIR);
}

void plugin::release()
{
	PluginManager::instance().release();
}

void plugin::apply(plugin::FnEnumerate fn, void* data)
{
	if (NULL != fn)
		(PluginManager::instance())(fn, data);
}
