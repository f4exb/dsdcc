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

#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dsd_decoder.h"

int exitflag;

static void usage ();
static void sigfun (int sig);

void usage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  dsd [options] Live scanner mode\n");
    fprintf(stderr, "  dsd -h        Show help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Display Options:\n");
    fprintf(stderr, "  -e            Show Frame Info and errorbars (default)\n");
    fprintf(stderr, "  -pe           Show P25 encryption sync bits\n");
    fprintf(stderr, "  -pl           Show P25 link control bits\n");
    fprintf(stderr, "  -ps           Show P25 status bits and low speed data\n");
    fprintf(stderr, "  -pt           Show P25 talkgroup info\n");
    fprintf(stderr, "  -q            Don't show Frame Info/errorbars\n");
    fprintf(stderr, "  -s            Datascope (disables other display options)\n");
    fprintf(stderr, "  -t            Show symbol timing during sync\n");
    fprintf(stderr, "  -v <num>      Frame information Verbosity\n");
    fprintf(stderr, "  -z <num>      Frame rate for datascope\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Input/Output options:\n");
    fprintf(stderr,
            "  -i <device>   Audio input device (default is /dev/audio, - for piped stdin)\n");
    fprintf(stderr, "  -o <device>   Audio output device (default is /dev/audio, - for stdout)\n");
    fprintf(stderr, "  -g <num>      Audio output gain (default = 0 = auto, disable = -1)\n");
    fprintf(stderr, "  -U            Force upsampling to 48kHz on audio output\n");
    fprintf(stderr, "  -n            Do not send synthesized speech to audio output device\n");
    fprintf(stderr, "  -L <filename> Log messages to file with file name <filename>. Default is stderr\n");
    fprintf(stderr, "                If file name is invalid messages will go to stderr\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Scanner control options:\n");
    fprintf(stderr,
            "  -R <num>      Resume scan after <num> TDULC frames or any PDU or TSDU\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Decoder options:\n");
    fprintf(stderr, "  -fa           Auto-detect frame type (default)\n");
    fprintf(stderr, "  -f1           Decode only P25 Phase 1\n");
    fprintf(stderr, "  -fd           Decode only D-STAR\n");
    fprintf(stderr, "  -fi           Decode only NXDN48* (6.25 kHz) / IDAS*\n");
    fprintf(stderr, "  -fn           Decode only NXDN96 (12.5 kHz)\n");
    fprintf(stderr, "  -fp           Decode only ProVoice*\n");
    fprintf(stderr, "  -fr           Decode only DMR/MOTOTRBO\n");
    fprintf(stderr, "  -fx           Decode only X2-TDMA\n");
    fprintf(stderr, "  -l            Disable DMR/MOTOTRBO and NXDN input filtering\n");
    fprintf(stderr, "  -ma           Auto-select modulation optimizations (default)\n");
    fprintf(stderr, "  -mc           Use only C4FM modulation optimizations\n");
    fprintf(stderr, "  -mg           Use only GFSK modulation optimizations\n");
    fprintf(stderr, "  -mq           Use only QPSK modulation optimizations\n");
    fprintf(stderr, "  -pu           Unmute Encrypted P25\n");
    fprintf(stderr, "  -u <num>      Unvoiced speech quality (default=3)\n");
    fprintf(stderr, "  -xx           Expect non-inverted X2-TDMA signal\n");
    fprintf(stderr, "  -xr           Expect inverted DMR/MOTOTRBO signal\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  * denotes frame types that cannot be auto-detected.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Advanced decoder options:\n");
    fprintf(stderr,
            "  -A <num>      QPSK modulation auto detection threshold (default=26)\n");
    fprintf(stderr,
            "  -S <num>      Symbol buffer size for QPSK decision point tracking\n");
    fprintf(stderr, "                 (default=36)\n");
    fprintf(stderr,
            "  -M <num>      Min/Max buffer size for QPSK decision point tracking\n");
    fprintf(stderr, "                 (default=15)\n");
    exit(0);
}

void sigfun(int sig)
{
    exitflag = 1;
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind, opterr, optopt;
    DSDcc::DSDDecoder dsdDecoder;
    DSDcc::DSDOpts *opts = dsdDecoder.getOpts();
    DSDcc::DSDState *state = dsdDecoder.getState();
    char in_file[1023];
    int  in_file_fd = -1;
    char out_file[1023];
    int  out_file_fd = -1;
    char log_file[1023];

    fprintf(stderr, "Digital Speech Decoder DSDcc\n");

    exitflag = 0;
    signal(SIGINT, sigfun);

    while ((c = getopt(argc, argv,
            "hep:qstv:z:i:o:g:nR:f:m:u:Ux:A:S:M:lL:")) != -1)
    {
        opterr = 0;
        switch (c)
        {
        case 'h':
            usage();
            exit(0);
        case 'e':
            opts->errorbars = 1;
            opts->datascope = 0;
            break;
        case 'p':
            if (optarg[0] == 'e')
            {
                opts->p25enc = 1;
            }
            else if (optarg[0] == 'l')
            {
                opts->p25lc = 1;
            }
            else if (optarg[0] == 's')
            {
                opts->p25status = 1;
            }
            else if (optarg[0] == 't')
            {
                opts->p25tg = 1;
            }
            else if (optarg[0] == 'u')
            {
                opts->unmute_encrypted_p25 = 1;
            }
            break;
        case 'q':
            opts->errorbars = 0;
            opts->verbose = 0;
            dsdDecoder.setLogVerbosity(0);
            break;
        case 's':
            opts->errorbars = 0;
            opts->p25enc = 0;
            opts->p25lc = 0;
            opts->p25status = 0;
            opts->p25tg = 0;
            opts->datascope = 1;
            opts->symboltiming = 0;
            break;
        case 't':
            opts->symboltiming = 1;
            opts->errorbars = 1;
            opts->datascope = 0;
            break;
        case 'v':
            sscanf(optarg, "%d", &opts->verbose);
            dsdDecoder.setLogVerbosity(opts->verbose);
            break;
        case 'z':
            sscanf(optarg, "%d", &opts->scoperate);
            opts->errorbars = 0;
            opts->p25enc = 0;
            opts->p25lc = 0;
            opts->p25status = 0;
            opts->p25tg = 0;
            opts->datascope = 1;
            opts->symboltiming = 0;
            fprintf(stderr, "Setting datascope frame rate to %i frame per second.\n",
                    opts->scoperate);
            break;
        case 'L':
            strncpy(log_file, (const char *) optarg, 1023);
            log_file[1023] = '\0';
            dsdDecoder.setLogFile(log_file);
            break;
        case 'i':
            strncpy(in_file, (const char *) optarg, 1023);
            in_file[1023] = '\0';
            break;
        case 'o':
            strncpy(out_file, (const char *) optarg, 1023);
            out_file[1023] = '\0';
            break;
        case 'g':
            sscanf(optarg, "%f", &opts->audio_gain);
            if (opts->audio_gain < (float) 0)
            {
                fprintf(stderr, "Disabling audio out gain setting\n");
            }
            else if (opts->audio_gain == (float) 0)
            {
                opts->audio_gain = (float) 0;
                fprintf(stderr, "Enabling audio out auto-gain\n");
            }
            else
            {
                fprintf(stderr, "Setting audio out gain to %f\n", opts->audio_gain);
                state->aout_gain = opts->audio_gain;
            }
            break;
        case 'n':
            opts->audio_out = 0;
            fprintf(stderr, "Disabling audio output to soundcard.\n");
            break;
        case 'R':
            sscanf(optarg, "%d", &opts->resume);
            fprintf(stderr, "Enabling scan resume after %i TDULC frames\n", opts->resume);
            break;
        case 'f':
            if (optarg[0] == 'a')
            {
                opts->frame_dstar = 1;
                opts->frame_x2tdma = 1;
                opts->frame_p25p1 = 1;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 1;
                opts->frame_dmr = 1;
                opts->frame_provoice = 0;
            }
            else if (optarg[0] == 'd')
            {
                opts->frame_dstar = 1;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 0;
                opts->frame_provoice = 0;
                fprintf(stderr, "Decoding only D-STAR frames.\n");
            }
            else if (optarg[0] == 'x')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 1;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 0;
                opts->frame_provoice = 0;
                fprintf(stderr, "Decoding only X2-TDMA frames.\n");
            }
            else if (optarg[0] == 'p')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 0;
                opts->frame_provoice = 1;
                state->samplesPerSymbol = 5;
                state->symbolCenter = 2;
                opts->mod_c4fm = 0;
                opts->mod_qpsk = 0;
                opts->mod_gfsk = 1;
                state->rf_mod = 2;
                fprintf(stderr, "Setting symbol rate to 9600 / second\n");
                fprintf(stderr, "Enabling only GFSK modulation optimizations.\n");
                fprintf(stderr, "Decoding only ProVoice frames.\n");
            }
            else if (optarg[0] == '1')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 1;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 0;
                opts->frame_provoice = 0;
                fprintf(stderr, "Decoding only P25 Phase 1 frames.\n");
            }
            else if (optarg[0] == 'i')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 1;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 0;
                opts->frame_provoice = 0;
                state->samplesPerSymbol = 20;
                state->symbolCenter = 10;
                opts->mod_c4fm = 0;
                opts->mod_qpsk = 0;
                opts->mod_gfsk = 1;
                state->rf_mod = 2;
                fprintf(stderr, "Setting symbol rate to 2400 / second\n");
                fprintf(stderr, "Enabling only GFSK modulation optimizations.\n");
                fprintf(stderr, "Decoding only NXDN 4800 baud frames.\n");
            }
            else if (optarg[0] == 'n')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 1;
                opts->frame_dmr = 0;
                opts->frame_provoice = 0;
                opts->mod_c4fm = 0;
                opts->mod_qpsk = 0;
                opts->mod_gfsk = 1;
                state->rf_mod = 2;
                fprintf(stderr, "Enabling only GFSK modulation optimizations.\n");
                fprintf(stderr, "Decoding only NXDN 9600 baud frames.\n");
            }
            else if (optarg[0] == 'r')
            {
                opts->frame_dstar = 0;
                opts->frame_x2tdma = 0;
                opts->frame_p25p1 = 0;
                opts->frame_nxdn48 = 0;
                opts->frame_nxdn96 = 0;
                opts->frame_dmr = 1;
                opts->frame_provoice = 0;
                fprintf(stderr, "Decoding only DMR/MOTOTRBO frames.\n");
            }
            break;
        case 'm':
            if (optarg[0] == 'a')
            {
                opts->mod_c4fm = 1;
                opts->mod_qpsk = 1;
                opts->mod_gfsk = 1;
                state->rf_mod = 0;
            }
            else if (optarg[0] == 'c')
            {
                opts->mod_c4fm = 1;
                opts->mod_qpsk = 0;
                opts->mod_gfsk = 0;
                state->rf_mod = 0;
                fprintf(stderr, "Enabling only C4FM modulation optimizations.\n");
            }
            else if (optarg[0] == 'g')
            {
                opts->mod_c4fm = 0;
                opts->mod_qpsk = 0;
                opts->mod_gfsk = 1;
                state->rf_mod = 2;
                fprintf(stderr, "Enabling only GFSK modulation optimizations.\n");
            }
            else if (optarg[0] == 'q')
            {
                opts->mod_c4fm = 0;
                opts->mod_qpsk = 1;
                opts->mod_gfsk = 0;
                state->rf_mod = 1;
                fprintf(stderr, "Enabling only QPSK modulation optimizations.\n");
            }
            break;
        case 'u':
            sscanf(optarg, "%i", &opts->uvquality);
            if (opts->uvquality < 1)
            {
                opts->uvquality = 1;
            }
            else if (opts->uvquality > 64)
            {
                opts->uvquality = 64;
            }
            fprintf(stderr, "Setting unvoice speech quality to %i waves per band.\n",
                    opts->uvquality);
            break;
        case 'U':
            opts->upsample = 1;
            break;
        case 'x':
            if (optarg[0] == 'x')
            {
                opts->inverted_x2tdma = 0;
                fprintf(stderr, "Expecting non-inverted X2-TDMA signals.\n");
            }
            else if (optarg[0] == 'r')
            {
                opts->inverted_dmr = 1;
                fprintf(stderr, "Expecting inverted DMR/MOTOTRBO signals.\n");
            }
            break;
        case 'A':
            sscanf(optarg, "%i", &opts->mod_threshold);
            fprintf(stderr, "Setting C4FM/QPSK auto detection threshold to %i\n",
                    opts->mod_threshold);
            break;
        case 'S':
            sscanf(optarg, "%i", &opts->ssize);
            if (opts->ssize > 128)
            {
                opts->ssize = 128;
            }
            else if (opts->ssize < 1)
            {
                opts->ssize = 1;
            }
            fprintf(stderr, "Setting QPSK symbol buffer to %i\n", opts->ssize);
            break;
        case 'M':
            sscanf(optarg, "%i", &opts->msize);
            if (opts->msize > 1024)
            {
                opts->msize = 1024;
            }
            else if (opts->msize < 1)
            {
                opts->msize = 1;
            }
            fprintf(stderr, "Setting QPSK Min/Max buffer to %i\n", opts->msize);
            break;
        case 'l':
            opts->use_cosine_filter = 0;
            break;
        default:
            usage();
            exit(0);
        }
    }

    if (strncmp(in_file, (const char *) "-", 1) == 0)
    {
        in_file_fd = STDIN_FILENO;
    }
    else
    {
        in_file_fd = open(in_file, O_RDONLY);
    }

    if (in_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for input.\n", in_file);
    }
    else
    {
        fprintf(stderr, "Cannot open %s for input. Aborting\n", in_file);
        return 0;
    }

    if (strncmp(out_file, (const char *) "-", 1) == 0)
    {
        out_file_fd = STDOUT_FILENO;
    }
    else
    {
        out_file_fd = open(out_file, O_WRONLY);
    }

    if (out_file_fd > -1)
    {
        fprintf(stderr, "Opened %s for output.\n", in_file);
    }
    else
    {
        if (in_file_fd != STDIN_FILENO) {
            close(in_file_fd);
        }

        fprintf(stderr, "Cannot open %s for output. Aborting\n", out_file);
        return 0;
    }

    while (exitflag == 0)
    {
        short sample;
        int nbAudioSamples;
        short *audioSamples;

        int result = read(in_file_fd, (void *) &sample, sizeof(short));

        if (result == 0)
        {
            fprintf(stderr, "No more input\n");
            break;
        }

        dsdDecoder.run(sample);
        audioSamples = dsdDecoder.getAudio(nbAudioSamples);

        if (nbAudioSamples > 0)
        {
            result = write(out_file_fd, (const void *) audioSamples, sizeof(short) * nbAudioSamples);

            if (result == -1) {
                fprintf(stderr, "Error writing to output\n");
            } else if (result != sizeof(short) * nbAudioSamples) {
                fprintf(stderr, "Written %d out of %d audio samples\n", result/2, nbAudioSamples);
            }

            dsdDecoder.resetAudio();
        }
    }

    fprintf(stderr, "End of process\n");

    if ((out_file_fd > -1) && (out_file_fd != STDOUT_FILENO)) {
        close(out_file_fd);
    }

    if ((in_file_fd > -1) && (in_file_fd != STDIN_FILENO)) {
        close(in_file_fd);
    }

    return (0);
}
