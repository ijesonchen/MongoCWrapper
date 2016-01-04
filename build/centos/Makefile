objects = winMain.o MongoAuto.o MongocHelp.o MongoClient.o TestFunc.o TestObs.o Teacher.o TeacherAccess.o

prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${exec_prefix}/include

include_path=./../../src -I./../../samples -I${includedir}/libbson-1.0 -I${includedir}/libmongoc-1.0
VPATH =./../../src:./../../samples

mcwrapper : $(objects)
	g++ -std=c++11 -o mcwrapper.bin $(objects) -L./lib -L${libdir} -lbson-1.0  \
	-L${libdir} -lmongoc-1.0 -I$(include_path) -Wl,-rpath=./../../lib64mongoc
$(objects) : %.o:%.cpp
	g++ -std=gnu++11 -Wall -I$(include_path) -g -c $< -o $@

clean :
	rm -f *.o *.c~ .*.cpp.sw* *~ mcwrapper.bin
	