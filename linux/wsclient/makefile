# If your compiler cannot find boost, please specify it explicitly like this:
boost_include_dir = -I/usr/local/boost/include/
boost_lib_dir = -L/usr/local/boost/lib/


BUILD_TIME="$(shell date +"%Y-%M-%d %H:%M:%S")"
cflag = -Wall -fexceptions -std=c++11 -DAPP_BUILD_TIME=\"$(BUILD_TIME)\"

MAKECMDGOALS=debug
ifeq (${MAKECMDGOALS}, debug)
	cflag += -g -DDEBUG
	dir = debug
else
	cflag += -O0 -DNDEBUG -DNO_SOCKET_UNIFIED_OUT
	lflag = -s
	dir = release
endif
${shell mkdir -p ${dir}}


LIB_DIR = -I${dir}/../../../dep/linuxlibs
HEADER_INC =  -I${dir}/../../../src/common \
              -I${dir}/../../../src/WebClient \
              -I${dir}/../../../dep \
              -I${dir}/../../../dep/include/asio \
              -I${dir}/../../../dep/include\
              

ext_libs = -lglog -lssl -lcrypto



kernel = ${shell uname -s}
ifeq (${kernel}, SunOS)
cflag += -pthreads ${ext_cflag} $(HEADER_INC)
lflag += -pthreads -lsocket -lnsl ${ext_libs}
else
ifeq (${kernel}, FreeBSD)
cflag += -pthread ${ext_cflag} $(HEADER_INC)
lflag += -pthread ${ext_libs}
else # here maybe still have other kernels need to be separated out
cflag += -pthread ${ext_cflag} $(HEADER_INC)
lflag += -pthread ${ext_libs}
endif
endif



target = ${dir}/${module}
sources = \
	${dir}/../../../src/WebClient/WebClient.cpp \



OS_NAME= $(shell uname -s)
ifeq ($(OS_NAME), Linux)
    export COMPILER=gnu
    export CC=/usr/bin/g++
	CFLAGS += -std=gnu++0x -DLinux 
	SYSTEM_LIBS = -lnsl -lpthread -lrt -ldl
	LNCMD = ln -is
endif


all:deletel wsclient clean


deletel: 
	-rm -rf wsclient

wsclient:
	$(CC) -g3 -o $@ $(sources) $(cflag) $(HEADER_INC) $(lflag) $(LIB_DIR)

clean:
	-rm -rf debug release
	

