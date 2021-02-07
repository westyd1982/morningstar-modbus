all: powersystemstatus.c dailygraphs.c dailylog.c sunsaverRAM.c sunsaverEEPROM.c sunsaverlog.c sunsaverlog2file.c powersystem.h
	cc `pkg-config --cflags --libs libmodbus` powersystemstatus.c -o ../bin/powersystemstatus -lgd -lpng -lz
	cc dailygraphs.c -o ../bin/dailygraphs
	cc `pkg-config --cflags --libs libmodbus` dailylog.c -o ../bin/dailylog
	cc `pkg-config --cflags --libs libmodbus` sunsaverRAM.c -o ../tools/sunsaverRAM
	cc `pkg-config --cflags --libs libmodbus` sunsaverEEPROM.c -o ../tools/sunsaverEEPROM
	cc `pkg-config --cflags --libs libmodbus` sunsaverlog.c -o ../tools/sunsaverlog
	cc `pkg-config --cflags --libs libmodbus` sunsaverlog2file.c -o ../tools/sunsaverlog2file
	