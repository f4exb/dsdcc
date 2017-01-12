<h1>Samples directory</h1>

<h2>Discriminator output samples</h2>

File extension: `.dis`

These are samples taken at the output of the discriminator of the radio. These are S16LE 48 kS/s single channel (mono) samples. 

They can be used as pipe input to `dsdccx` using `sox` utility as in this example: `sox -t s16 -r 48k -c 1 dmr_it_8.dis -t s16 -r 48k -c 1 - | /opt/install/dsdcc/bin/dsdccx -T3 -i - -fa -o - | play -q -t s16 -r 8k -c 1 -`

The files are:

  - `dmr_it_8.dis`: Example of a DMR signal. This is some technical conversation in Italian. As this uses DMR slot #2 you have to specify the `-T2` (slot #2) or `-T3` (slots #1 and #2 mixed) option to get an output.
  - `dstar_f1zil_1.dis`: Example of a D-Star signal with header. This is an amateur radio conversation in French over F1ZIL repeater. It has quite a few errors.
  - `dstar_f1zil_2.dis`: Example of a D-Star signal captured on the fly i.e. without header. This is also a conversation over F1ZIL. It is shorter than the previous sample but has less errors. 