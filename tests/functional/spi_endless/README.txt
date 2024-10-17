Test description:

Test consists of 2 boards connected with jumper wires according to schematic below.
One of the board should be flashed with host variant, and another with Device variant of the test.

The host performs SPI xfer operation (bidirectional write/read).
Device listens to the host transmisions.
Every message received by Device is transmitted in the next xfer operation.

host sends deterministic values. first payload is the slice of the random table (table containing random bytes).
and every subsequent transfers from host consists of the previopus payload increased by 1 ( byte by byte)

so the valid communication looks as follows ( 3 B transfer example )
      
MOSI |12 23 34 | 13 24 35 | 14 25 36
MISO |00 00 00 | 12 23 34 | 13 24 35

The host expects to reveice on N th mesasge the conetent send on N-1.
The Slave can also assert the data it received, because the mechanism of creating new message is dependent on previous message
So it can also detect, if the host failed to send data correctly, if on MOSI line the values of each byte are not bigger by 1 from the previous transmision.

Building instructions:

Please consult testcase.yaml for manual build
But for most cases it is easier to use twister for building:

`west twister -p nrf52840dk/nrf52840 -p nrf54l15dk/nrf54l15/cpuapp -T .`

connection schematic:

Host          Device
nrf54l15        nrf52840

GND     -       GND
vddio   -       VIO_REF
2.6     -       1.2     SPIM_SCK 
1.11    -       1.4     SPIM_MISO 
2.8     -       1.8     SPIM_MOSI 
2.10    -       1.10    CS


The twister builds 2 variants, but at this time only described above configuration was tested

Flash the Development kits:
Device variant for 52
and host variant for 54

Configuration options:
There are overlays to change speed of the host (default 8Mhz) *does not require rebuild for Device
There is Kconfig to change delay between messages ( default 1ms ) * does not require rebuild for Device
There is Kconfig to change the amount of data to send (default 64)  * Requires rebuild for both Device and host

Observe the output on the serial of 54


The issue:


```
[00:00:52.818,041] [1;31m<err> host: readed
                               32 cd 0a 93 5c 98 bb d7  27 ff 50 63 5e 44 43 fe |2...\... '.Pc^DC.
                               76 8d 4b ad 68 47 46 c6  9d 70 1d ed e0 34 66 36 |v.K.hGF. .p...4f6
                               08 de bb 15 ea 09 ee 2b  2e ad 74 1b 4a a2 5e ba |.......+ ..t.J.^.
                               22 e5 3c 51 5f 69 95 1f  c4 bc 5f 80 00 05 d7 4f |".<Q_i.. .._....O[0m
[00:00:52.818,053] [1;31m<err> host: expected
                               32 cd 0a 93 5c 98 bb d7  27 ff 50 63 5e 44 43 cf |2...\... '.Pc^DC.
                               fc ed 1a 97 5a d0 8e 8d  8d 3a e0 3b db c0 68 cc |....Z... .:.;..h.
                               6c 11 bd 76 2b d4 13 dc  56 5d 5a e8 36 95 44 bd |l..v+... V]Z.6.D.
                               74 45 ca 78 a2 be d3 2a  3f 89 78 99 c6 9f 8b ae |tE.x...* ?.x.....[0m
```

Snippet of logs from the host.
This log shows that the host read 64 Bytes , but only first 14B were maching the expected values.
The log suggests that every transfer fail the comparason of the expected read values.

The analysis of the trace gathered by external logic analyzer suggests, that Device sends correct data on MISO line,
so the logical conclusion is that the host faild to read it correctly.
