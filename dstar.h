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

#ifndef DSDCC_DSTAR_H_
#define DSDCC_DSTAR_H_

#include <string>
#include "dstarold.h"

namespace DSDcc
{

class DSDDstar : public DSDDstarOld
{
public:
	DSDDstar(DSDDecoder *dsdDecoder);
    ~DSDDstar();
};

} // namespace DSDcc

#endif /* DSTAR_H_ */
