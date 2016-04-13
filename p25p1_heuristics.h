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

/**
 * This module is dedicated to improve the accuracy of the digitizer. The digitizer is the piece of code that
 * translates an analog value to an actual symbol, in the case of P25, a dibit.
 * It implements a simple Gaussian classifier. It's based in the assumption that the analog values from the
 * signal follow normal distributions, one single distribution for each symbol.
 * Analog values for the dibit "0" will fit into a Gaussian bell curve with a characteristic mean and std
 * distribution and the same goes for dibits "1", "2" and "3."
 * Hopefully those bell curves are well separated from each other so we can accurately discriminate dibits.
 * If we could model the Gaussian of each dibit, then given an analog value, the dibit whose Gaussian fits
 * better is the most likely interpretation for that value. By better fit we can calculate the PDF
 * (probability density function) for the Gaussian, the one with the highest value is the best fit.
 *
 * The approach followed here to model the Gaussian for each dibit is to use the error corrected information
 * as precise oracles. P25 uses strong error correction, some dibits are doubly protected by Hamming/Golay
 * and powerful Reed-Solomon codes. If a sequence of dibits clears the last Reed-Solomon check, we can be
 * quite confident that those values are correct. We can use the analog values for those cleared dibits to
 * calculate mean and std deviation of our Gaussians. With this we are ready to calculate the PDF of a new
 * unknown analog value when required.
 * Values that don't clear the Reed-Solomon check are discarded.
 * This implementation uses a circular buffer to keep track of the N latest cleared analog dibits so we can
 * adapt to changes in the signal.
 * A modification was made to improve results for C4FM signals. See next block comment.
 */

#ifndef P25P1_HEURISTICS_H_030dd3530b7546abbb56f8dd1e66a2f6
#define P25P1_HEURISTICS_H_030dd3530b7546abbb56f8dd1e66a2f6

/**
 * In the C4FM P25 recorded files from the "samples" repository, it can be observed that there is a
 * correlation between the correct dibit associated for a given analog value and the value of the previous
 * dibit. For instance, in one P25 recording, the dibits "0" come with an average analog signal of
 * 3829 when the previous dibit was also "0," but if the previous dibit was a "3" then the average
 * analog signal is 6875. These are the mean and std deviations for the full 4x4 combinations of previous and
 * current dibits:
 *
 * 00: count: 200 mean:    3829.12 sd:     540.43    <-
 * 01: count: 200 mean:   13352.45 sd:     659.74
 * 02: count: 200 mean:   -5238.56 sd:    1254.70
 * 03: count: 200 mean:  -13776.50 sd:     307.41
 * 10: count: 200 mean:    3077.74 sd:    1059.00
 * 11: count: 200 mean:   11935.11 sd:     776.20
 * 12: count: 200 mean:   -6079.46 sd:    1003.94
 * 13: count: 200 mean:  -13845.43 sd:     264.42
 * 20: count: 200 mean:    5574.33 sd:    1414.71
 * 21: count: 200 mean:   13687.75 sd:     727.68
 * 22: count: 200 mean:   -4753.38 sd:     765.95
 * 23: count: 200 mean:  -12342.17 sd:    1372.77
 * 30: count: 200 mean:    6875.23 sd:    1837.38    <-
 * 31: count: 200 mean:   14527.99 sd:     406.85
 * 32: count: 200 mean:   -3317.61 sd:    1089.02
 * 33: count: 200 mean:  -12576.08 sd:    1161.77
 * ||          |             |              |
 * ||          |             |              \_std deviation
 * ||          |             |
 * ||          |             \_mean of the current dibit
 * ||          |
 * ||          \_number of dibits used to calculate mean and std deviation
 * ||
 * |\_current dibit
 * |
 * \_previous dibit
 *
 * This effect is not observed on QPSK or GFSK signals, there the mean values are quite consistent regardless
 * of the previous dibit.
 *
 * The following define enables taking the previous dibit into account for C4FM signals. Comment out
 * to disable.
 */
#define USE_PREVIOUS_DIBIT

/**
 * There is a minimum of cleared analog values we need to produce a meaningful mean and std deviation.
 */
#define MIN_ELEMENTS_FOR_HEURISTICS 10

//Uncomment to disable the behaviour of this module.
//#define DISABLE_HEURISTICS

#define HEURISTICS_SIZE 200

namespace DSDcc
{

class DSDP25Heuristics
{
public:
    typedef struct
    {
        int values[HEURISTICS_SIZE];
        float means[HEURISTICS_SIZE];
        int index;
        int count;
        float sum;
        float var_sum;
    } SymbolHeuristics;

    typedef struct
    {
        unsigned int bit_count;
        unsigned int bit_error_count;
        SymbolHeuristics symbols[4][4];
    } P25Heuristics;

    typedef struct
    {
        int value;
        int dibit;
        int corrected_dibit;
        int sequence_broken;
    } AnalogSignal;

    /**
     * Initializes the heuristics state.
     * \param heuristics The P25Heuristics structure to initialize.
     */
    static void initialize_p25_heuristics(P25Heuristics* heuristics);

    /**
     * Important method that estimates the most likely symbol for a given analog signal value and previous dibit.
     * This is called by the digitizer.
     * \param rf_mod Indicates the modulation used. The previous dibit is only used on C4FM.
     * \param heuristics Pointer to the P25Heuristics module with all the needed state information.
     * \param previous_dibit The previous dibit.
     * \param analog_value The signal's analog value we want to interpret as a dibit.
     * \param dibit Address were to store the estimated dibit.
     * \return A boolean set to true if we are able to estimate a dibit. The reason why we might not be able
     * to estimate it is because we don't have enough information to model the Gaussians (not enough data
     * has been passed to contribute_to_heuristics).
     */
    static int estimate_symbol(int rf_mod, P25Heuristics* heuristics, int previous_dibit,
            int analog_value, int* dibit);

    /**
     * Log some useful information on the heuristics state.
     */
    static void debug_print_heuristics(P25Heuristics* heuristics);

    /**
     * This method contributes valuable information from dibits whose value we are confident is correct. We take
     * the dibits and corresponding analog signal values to model the Gaussians for each dibit (and previous
     * dibit if enabled).
     * \param rf_mod Indicates the modulation used. The previous dibit is only used on C4FM.
     * \param heuristics Pointer to the P25Heuristics module with all the needed state information.
     * \param analog_signal_array Sequence of AnalogSignal which contain the cleared dibits and analog values.
     * \param count number of cleared dibits passed (= number of elements to use from analog_signal_array).
     */
    static void contribute_to_heuristics(int rf_mod, P25Heuristics* heuristics,
            AnalogSignal* analog_signal_array, int count);

    /**
     * Updates the estimate for the BER (bit error rate). Mind this is method is not called for every single
     * bit in the data stream but only for those bits over which we have an estimate of its error rate,
     * specifically the bits that are protected by Reed-Solomon codes.
     * \param heuristics The heuristics state.
     * \param bits The number of bits we have read.
     * \param errors The number of errors we estimate in those bits.
     */
    static void update_error_stats(P25Heuristics* heuristics, int bits, int errors);

    /**
     * Returns the estimate for the BER (bit error rate).
     * \return The estimated BER. This is just the percentage of errors over the processed bits.
     */
    float get_P25_BER_estimate(P25Heuristics* heuristics);

private:
    static int use_previous_dibit(int rf_mod);
    static void update_p25_heuristics(P25Heuristics* heuristics, int previous_dibit,
            int original_dibit, int dibit, int analog_value);
    static void initialize_symbol_heuristics(SymbolHeuristics* sh);
    static float evaluate_pdf(SymbolHeuristics* se, int value);
    static void debug_log_pdf(P25Heuristics* heuristics, int previous_dibit, int analog_value);
    static void debug_print_symbol_heuristics(int previous_dibit, int dibit,
            SymbolHeuristics* sh);
};

}

#endif // P25P1_HEURISTICS_H_030dd3530b7546abbb56f8dd1e66a2f6
