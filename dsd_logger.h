///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
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

#ifndef DSD_LOGGER_H_
#define DSD_LOGGER_H_

#include <stdio.h>
#include <cstdarg>

#include "export.h"

namespace DSDcc
{

class DSDCC_API DSDLogger
{
public:
    DSDLogger();
    explicit DSDLogger(const char *filename);
    ~DSDLogger();

    void setFile(const char *filename);
    void setVerbosity(int verbosity) { m_verbosity = verbosity; }

    void log(const char* fmt, ...) const
    {
        if (m_verbosity > 0)
        {
            va_list argptr;
            va_start(argptr, fmt);
            vfprintf(m_logfp, fmt, argptr);
            va_end(argptr);
        }
    }

private:
    FILE *m_logfp;
    int  m_verbosity;
};

} // namespace DSDcc

#endif /* DSD_LOGGER_H_ */
