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

#include "dsd_logger.h"

namespace DSDcc
{

DSDLogger::DSDLogger()
{
    m_verbosity = 1;
    m_logfp = stderr;
}

DSDLogger::DSDLogger(const char *filename)
{
    m_verbosity = 1;
    m_logfp = fopen(filename, "w");

    if (!m_logfp) {
        m_logfp = stderr;
    }
}

DSDLogger::~DSDLogger()
{
    if (m_logfp != stderr) {
        fclose(m_logfp);
    }
}

void DSDLogger::setFile(const char *filename)
{
    if (m_logfp != stderr) {
        fclose(m_logfp);
    }

    m_logfp = fopen(filename, "w");

    if (!m_logfp) {
        m_logfp = stderr;
    }
}

} // namespace DSDcc
