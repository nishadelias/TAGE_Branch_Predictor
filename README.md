# TAGE Branch Predictor

This project simulates a branch predictor using the TAGE prediction scheme. See TAGE_Branch_Predictor.pdf for more details on how the predictor works.

## Building

Build the program by running the following command in the src directory
```shell
make
```

## Running

Use the following command in the root directory to run the program and output the number of misses per thousand instructions (MPKI)
```shell
csh run traces
```

Here are the results for the above command
```shell
traces/164.gzip/gzip.trace.bz2                  12.220
traces/175.vpr/vpr.trace.bz2                    10.594
traces/176.gcc/gcc.trace.bz2                    6.998
traces/181.mcf/mcf.trace.bz2                    13.024
traces/186.crafty/crafty.trace.bz2              3.763
traces/197.parser/parser.trace.bz2              8.119
traces/201.compress/compress.trace.bz2          7.307
traces/202.jess/jess.trace.bz2                  1.125
traces/205.raytrace/raytrace.trace.bz2          2.594
traces/209.db/db.trace.bz2                      4.084
traces/213.javac/javac.trace.bz2                1.938
traces/222.mpegaudio/mpegaudio.trace.bz2        1.925
traces/227.mtrt/mtrt.trace.bz2                  2.622
traces/228.jack/jack.trace.bz2                  2.079
traces/252.eon/eon.trace.bz2                    1.585
traces/253.perlbmk/perlbmk.trace.bz2            2.143
traces/254.gap/gap.trace.bz2                    2.885
traces/255.vortex/vortex.trace.bz2              0.563
traces/256.bzip2/bzip2.trace.bz2                0.071
traces/300.twolf/twolf.trace.bz2                14.624
average MPKI: 5.013
```

## Cleaning up

Clean up the binary files by running the following command in the src directory (same directory where you ran "make"). 
```shell
make clean
```
