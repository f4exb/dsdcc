///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This is an adaptation of: https://github.com/lemire/runningmaxmin.git         //
//                                                                               //
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

#ifndef RUNNINGMAXMIN_H_
#define RUNNINGMAXMIN_H_

#include <stdlib.h>
#include <stdint.h>

namespace DSDcc
{

// nextPowerOfTwo returns a power of two that is larger or equal than x.
inline uint32_t nextPowerOfTwo(uint32_t x)
{
    uint32_t result = 1;
    while (result < x)
    {
        result <<= 1;
    }
    return result;
}


// actual streaming implementation

template<typename valuetype>
class lemiremaxmintruestreaming
{
public:
    explicit lemiremaxmintruestreaming(uint32_t width) :
            up(), lo(), n(0), ww(width)
    {
        init(&up, ww);
        init(&lo, ww);
    }

    ~lemiremaxmintruestreaming()
    {
        freenodes(&up);
        freenodes(&lo);
    }

    void resize(uint32_t width)
    {
        ww = width;
        freenodes(&up);
        freenodes(&lo);
        init(&up, ww);
        init(&lo, ww);
    }

    void update(valuetype value)
    {
        if (nonempty(&up) != 0)
        {
            if (value > tailvalue(&up))
            {
                prunetail(&up);
                while (((nonempty(&up)) != 0) && (value >= tailvalue(&up)))
                {
                    prunetail(&up);
                }
            }
            else
            {
                prunetail(&lo);
                while (((nonempty(&lo)) != 0) && (value <= tailvalue(&lo)))
                {
                    prunetail(&lo);
                }
            }
        }
        push(&up, n, value);
        if (n == ww + headindex(&up))
        {
            prunehead(&up);
        }

        push(&lo, n, value);
        if (n == ww + headindex(&lo))
        {
            prunehead(&lo);
        }
        n++;
    }

    valuetype max()
    {
        return headvalue(&up);
    }
    valuetype min()
    {
        return headvalue(&lo);
    }

private:
    struct valuenode
    {
        uint32_t index;
        valuetype value;
    };

    struct valuesqueue
    {
        valuenode * nodes;
        uint32_t head;
        uint32_t tail;
        uint32_t mask;
    };

    // Dequeue methods

    inline uint32_t count(valuesqueue * q)
    {
        return (q->tail - q->head) & q->mask;
    }

    inline void init(valuesqueue * q, uint32_t size)
    {
        size = nextPowerOfTwo(size + 1);
        q->nodes = reinterpret_cast<valuenode *>(malloc(sizeof(valuenode) * size));
        q->head = 0;
        q->tail = 0;
        q->mask = size - 1;
    }

    inline void freenodes(valuesqueue * q)
    {
        free(q->nodes);
    }

    inline uint32_t headindex(valuesqueue * q)
    {
        return q->nodes[q->head].index;
    }

    inline void push(valuesqueue * q, uint32_t index, valuetype value)
    {
        q->nodes[q->tail].index = index;
        q->nodes[q->tail].value = value;
        q->tail = (q->tail + 1) & q->mask;
    }

    inline valuetype tailvalue(valuesqueue * q)
    {
        return q->nodes[(q->tail - 1) & q->mask].value;
    }

    inline valuetype headvalue(valuesqueue * q)
    {
        return q->nodes[q->head].value;
    }
    inline void prunehead(valuesqueue * q)
    {
        q->head = (q->head + 1) & q->mask;
    }

    inline void prunetail(valuesqueue * q)
    {
        q->tail = (q->tail - 1) & q->mask;
    }

    inline int nonempty(valuesqueue * q)
    {
        return static_cast<int>(q->tail != q->head);
    }

    valuesqueue up;
    valuesqueue lo;
    uint32_t n;
    uint32_t ww;
};

} // namespce DSDcc

#endif /* RUNNINGMAXMIN_H_ */
