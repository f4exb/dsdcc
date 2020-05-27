<h1>Message file details</h1>

<h2>Introduction</h2>

Since version 1.6 dsdccx has the capability of sending regularly the traffic status messages to a file using the `-M` option. The `-m` option specifies the rate at which the information is polled and written to file. It is expressed in seconds at a 48 kS/s rate and has a minimum value of 0.1s. At 0.1s it will poll everu 48000*0.1 = 4800 samples.

The polling period will match an actual time interval only during live operation when samples are fed from a real device although buffering might alter this. 

<h2>File format</h2>

There is one line per polling occurence with a fixed format depending on the protocol. For all protocols the line starts with a timestamp and a protocol identifier. Details by protocol are given next.

<h3>DMR</h3>

```
           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8
 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....
 1484364328.297:DMR>Sta: BS S1: /04 IDL                    S2: *04 VLC 02222223>G00019535
 -------------- ---      -- --  .-- --- -------- .-------- --  .-- --- -------- .--------   
 1              2        3  4   56  7   8        9A        4   56  7   8        9A 

```
  - **1**: Timestamp in seconds since epoch with millisecond precision
  - **2**: Protocol indicator: `DMR` 
  - **3**: Station type:
    - MS: Mobile station
    - BS: Base station
  - **4**: Slot #1 (S1) or slot #2 (S2) follows with the same information for each slot
  - **5**: Channel status derived from CACH information:
    - *: Busy. That is the AT bit on the opposite channel is on.
    - .: Clear. That is the AT bit on the opposite channel is off.
    - /: The CACH could not be decoded and information is missing
  - **6**: Color code from 0 to 15 or `--` if color code could no be decoded
  - **7**: Slot type:
    - VOX: Voice block
    - IDL: Data idle block
    - VLC: Voice link control block
    - TLC: Terminator with Link Control information data block
    - CSB: CSBK (Control Signalling BlocK) data block
    - MBH: Multi Block Control block header data block
    - MBC: Multi Block Control block continuation data block
    - DAH: Data header block
    - D12: 1/2 rate data block
    - D34: 3/4 rate data block
    - DB1: Full rate data block
    - USB: Unified Single Block Data
    - RES: Reserved data block
    - UNK: unknown data type or could not be decoded
  - **8**: Source address (24 bits) as defined in the DMR ETSI standard
  - **9**: Address type indicator:
    - G: Ggroup address
    - U: Unit (individual) address  
  - **A**: Target address (24 bits) as defined in the DMR ETSI standard  
  
<h3>dPMR</h3>

```
           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8
 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....
 1484364141.663:DPM>VO CC: 1757 OI: 00000302 CI: 00014653
 -------------- --- --     ----     --------     --------   
 1              2   3      4        5            6

```
  - **1**: Timestamp in seconds since epoch with millisecond precision
  - **2**: Protocol indicator: `DMR` 
  - **3**: dPMR frame type:
    - --: Undefined
    - HD: Header of FS1 type
    - PY: Payload frame of a sitll undetermined type
    - VO: Voice frame
    - VD: Voice and data frame
    - D1: Data without FEC frame
    - D2: Data with FEC frame
    - XS: Extended search: looking for a new payload frame when out of sequence
    - EN: End frame
  - **4**: Colour code in decimal (12 bits)
  - **5**: Own ID. Sender's identification code in decimal (24 bits).
  - **6**: Called ID. Called party's identification code in decimal (24 bits).

<h3>D-Star</h3>

```
           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8    9    9
 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....
 1484364098.148:DST>F1NSR   /ID51>CQCQCQ  |F1ZIL  B>F1ZIL  B|YANNICK ST RAPHAEL  |      :000/00000.0
 -------------- --- -------- ---- -------- -------- -------- -------------------- ------ --- -------   
 1              2   3        4    5        6        7        8                    9      A   B

```
  - **1**: Timestamp in seconds since epoch with millisecond precision
  - **2**: Protocol indicator: `DST` 
  - **3**: Origin callsign (MY)
  - **4**: Origin informative suffix
  - **5**: Destination callsign (YOUR or UR)
  - **6**: Origin repeater callsign (RPT1)
  - **7**: Destination repeater callsign (RPT2)
  - **8**: Informative text
  - **9**: 6 character locator a.k.a. Maidenhead locator
  - **A**: Bearing to the origin station when locator is defined and own position is specified (options -P and -Q)
  - **B**: Distance to the origin station when locator is defined and own position is specified (options -P and -Q)
  
<h3>YSF (Yaesu System Fusion)</h3>

```
           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8    
 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....
 1484365141.179:YSF>C V2 GC 0:7 WL000|F6FCE     >**********|F5ZOO-R1  >F5ZOO-R1  |E55vv
 -------------- --- . -- -- . . ..--- ---------- ---------- ---------- ---------- -----   
 1              2   3 4  5  6 7 89A   B          C          D          E          F 

```
  - **1**: Timestamp in seconds since epoch with millisecond precision
  - **2**: Protocol indicator: `YSF` 
  - **3**: Frame type:
    - H: header
    - C: channel
    - T: terminator
    - S: test
  - **4**: Channel type:
    - V1: voice/data mode 1
    - V2: voice/data mode 2 (as in the example)
    - VF: voice full rate
    - DF: data full rate
  - **5**: Call mode:
    - GC: group call
    - RI: radio ID
    - RE: reserved
    - IN: individual call
  - **6**: Total number of blocks
  - **7**: Total number of frames
  - **8**: Bandwidth mode:
    - N: narrow band mode
    - W: wide band mode
  - **9**: Path type:
    - I: Internet path
    - L: local path
  - **A**: Squelch code (0..127) or dashes `---` if the YSF squelch is not active 
  - **B**: Origin callsign
  - **C**: Destination callsign. It is filled with stars `*` when call is made to all stations (similar to the CQCQCQ in D-Star)
  - **D**: Origin repeater callsign
  - **E**: Destination repeater callsign
  - **F**: Originator radio ID. This is the unique character string assigned to the device by the manufacturer.
  
<h3>Undefined</h3>

When the system has not acquired synchronization a dummy line with `XXX` as protocol indicator is recorded:

```
           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8    
 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....
 1484365141.179:XXX>
 -------------- ---    
 1              2    

```
  - **1**: Timestamp in seconds since epoch with millisecond precision
  - **2**: Protocol indicator: `XXX` 
