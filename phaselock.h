///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 F4EXB                                                      //
// written by Edouard Griffiths                                                  //
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

#include <stdint.h>
#include <vector>

#include "export.h"

namespace DSDcc
{

/** Phase-locked loop. */
class DSDCC_API PhaseLock
{
public:
    /**
     * Construct phase-locked loop.
     *
     * freq       :: center frequency relative to sample freq
     *               (0.5 is Nyquist)
     * bandwidth  :: bandwidth relative to sample frequency
     * minsignal  :: minimum pilot amplitude
     */
    PhaseLock(float freq, float bandwidth, float minsignal);

    virtual ~PhaseLock() {}

    /**
     * Change phase locked loop parameters
     *
     * freq       :: 19 kHz center frequency relative to sample freq
     *               (0.5 is Nyquist)
     * bandwidth  :: bandwidth relative to sample frequency
     * minsignal  :: minimum pilot amplitude
     */
    void configure(float freq, float bandwidth, float minsignal);

    /**
     * Process samples and track a pilot tone. Generate samples for single or multiple phase-locked
     * signals. Implement the processPhase virtual method to produce the output samples.
     * In flow version. Ex: Use 19 kHz stereo pilot tone to generate 38 kHz (stereo) and 57 kHz
     * pilots (see RDSPhaseLock class below).
     * This is the in flow version
     */
    void process(const float& sample_in, float *samples_out);

    /**
     * Process samples and extract 19 kHz pilot tone.
     * Generate phase-locked 38 kHz tone with unit amplitude.
     * Bufferized version with input and output vectors
     */
    void process(const std::vector<float>& samples_in, std::vector<float>& samples_out);

    /** Return true if the phase-locked loop is locked. */
    bool locked() const
    {
        return m_lock_cnt >= m_lock_delay;
    }

protected:
    float    m_phase;
    float    m_psin;
    float    m_pcos;
    /**
     * Callback method to produce multiple outputs from the current phase value in m_phase
     * and/or the sin and cos values in m_psin and m_pcos
     */
    virtual void processPhase(float *samples_out) const = 0;

private:
    float    m_minfreq, m_maxfreq;
    float    m_phasor_b0, m_phasor_a1, m_phasor_a2;
    float    m_phasor_i1, m_phasor_i2, m_phasor_q1, m_phasor_q2;
    float    m_loopfilter_b0, m_loopfilter_b1;
    float    m_loopfilter_x1;
    float    m_freq;
    float    m_minsignal;
    int      m_lock_delay;
    int      m_lock_cnt;
    uint64_t m_sample_cnt;
};

class DSDCC_API SimplePhaseLock : public PhaseLock
{
public:
    SimplePhaseLock(float freq, float bandwidth, float minsignal) :
        PhaseLock(freq, bandwidth, minsignal)
    {}

protected:
    virtual void processPhase(float *samples_out) const
    {
        samples_out[0] = m_psin; // f Pilot
        samples_out[1] = m_pcos; // f Pilot
    }
};

} // namespace DSDCc
