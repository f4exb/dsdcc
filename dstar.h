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

namespace DSDcc
{

class DSDDecoder;

class DSDDstar
{
public:
    DSDDstar(DSDDecoder *dsdDecoder);
    ~DSDDstar();

    void init();
    void process();
    void processHD();

    const std::string& getRpt1() const { return m_rpt1; }
    const std::string& getRpt2() const { return m_rpt2; }
    const std::string& getYourSign() const { return m_yourSign; }
    const std::string& getMySign() const { return m_mySign; }

private:
    void initVoiceFrame();
    void initDataFrame();
    void processVoice();
    void processData();
    void dstar_header_decode();
    void reset_header_strings();

    DSDDecoder *m_dsdDecoder;
    int m_symbolIndex;    //!< Current symbol index in non HD sequence
    int m_symbolIndexHD;  //!< Current symbol index in HD sequence
    int m_dibitCache[97]; // has to handle a voice + data frame (97 dibits)
    int m_dibitIndex;     // index in dibit cache

    // DSTAR
    char ambe_fr[4][24];
    unsigned char data[9];
    unsigned int bits[4];
    int framecount;
    int sync_missed;
    unsigned char slowdata[4];
    unsigned int bitbuffer;
    const int *w, *x;

    // DSTAR-HD
    int radioheaderbuffer[660];
    std::string m_rpt1;
    std::string m_rpt2;
    std::string m_yourSign;
    std::string m_mySign;

    // constants
    static const int dW[72];
    static const int dX[72];
};

} // namespace DSDcc

#endif /* DSDCC_DSTAR_H_ */
