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

#ifndef DOUBLEBUFFER_H_
#define DOUBLEBUFFER_H_

#include <string.h>

namespace DSDcc
{

template<typename T>
class DoubleBuffer
{
public:
    DoubleBuffer(unsigned int size) :
        m_size(size),
        m_index(0)
    {
        m_buffer = new T[2*m_size];
    }

    ~DoubleBuffer()
    {
        delete[] m_buffer;
    }

    void resize(unsigned int size)
    {
        delete[] m_buffer;
        m_size = size;
        m_buffer = new T[2*m_size];
        reset();
    }

    void push(T item)
    {
        m_buffer[m_index] = item;
        m_buffer[m_index + m_size] = item;
        m_index = (m_index + 1) % m_size;
    }

    void reset()
    {
        m_index = 0;
        memset(m_buffer, 0, 2*m_size*sizeof(T));
    }

    void move(int distance)
    {
        m_index = (m_index + distance) % m_size;
    }

    T *getData(unsigned int shift = 0)
    {
        if (shift < m_size)
        {
            return &m_buffer[m_index+shift];
        }
        else
        {
            return &m_buffer[m_index];
        }
    }

private:
    unsigned int m_size;
    int m_index;
    T *m_buffer;
};


} // namespace DSDcc


#endif /* DOUBLEBUFFER_H_ */
