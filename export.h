///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2018 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef __DSDCC_EXPORT_H
#define __DSDCC_EXPORT_H

#if defined (__GNUC__) && (__GNUC__ >= 4)
#  define __DSDCC_EXPORT   __attribute__((visibility("default")))
#  define __DSDCC_IMPORT   __attribute__((visibility("default")))

#elif defined (_MSC_VER)
#  define __DSDCC_EXPORT   __declspec(dllexport)
#  define __DSDCC_IMPORT   __declspec(dllimport)

#else
#  define __DSDCC_EXPORT
#  define __DSDCC_IMPORT
#endif

/* The 'DSDCC_API' controls the import/export of 'sdrbase' symbols and classes.
 */
#if !defined(dsdcc_STATIC)
#  if defined dsdcc_EXPORTS
#    define DSDCC_API __DSDCC_EXPORT
#  else
#    define DSDCC_API __DSDCC_IMPORT
#  endif
#else
#  define DSDCC_API
#endif

#endif // __DSDCC_EXPORT_H
