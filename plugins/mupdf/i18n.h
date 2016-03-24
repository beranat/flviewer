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

#define _(X) dgettext(module.name, X)
#define _N(SINGULAR,PLURAL,N) dngettext(module.name,SINGULAR, PLURAL, N)
#define I18N_INIT() { bindtextdomain(module.name, LOCALEDIR); }
#else
#define _(X)  (X)
#define _N(SINGULAR,PLURAL,N) ((N==1)?SINGULAR:PLURAL)
#define I18N_INIT() { }
#endif

#endif //#ifndef __I18N_H__
