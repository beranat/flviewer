// i18n.h: Internationalization defines
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
#ifndef __I18N_H__
#define __I18N_H__

#include <config.h>

#define _NOOP(X)  (X)
#define N_NOOP(SINGULAR,PLURAL,N) ((N==1)?SINGULAR:PLURAL)

#ifdef SUPPORT_I18N
#include <libintl.h>
#define I18N_INIT() { setlocale (LC_ALL, ""); bindtextdomain(PACKAGE, LOCALEDIR); textdomain (PACKAGE); }
#define _(X) gettext(X)
#define N_(SINGULAR,PLURAL,N) ngettext(SINGULAR, PLURAL, N)
#else
#define I18N_INIT() { }
#define _(X) _NOOP(X)
#define N_(SINGULAR,PLURAL,N) N_NOOP(SINGULAR, PLURAL, N)
#endif

#endif //#ifndef __I18N_H__
