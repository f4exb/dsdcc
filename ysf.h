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

#ifndef YSF_H_
#define YSF_H_

#include <string>

namespace DSDcc
{

class DSDDecoder;

class DSDYSF
{
public:
    DSDYSF(DSDDecoder *dsdDecoder);
    ~DSDYSF();

    void init();
    void process();

private:
    DSDDecoder *m_dsdDecoder;
    int m_symbolIndex;    //!< Current symbol index in non HD sequence
};

} // namespace DSDcc



#endif /* YSF_H_ */
