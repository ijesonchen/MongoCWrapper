===============
mongo-c-wrapper
===============


Q&A
===
0. how to use?
  compile mongo-c-lib first.(download src from mongodb.org)
  example project:
  for linux, use build/cenos/Makefile
  for windows & vs2013, use build/vsprojects/MongoCWrapper.sln
  check code TeacherAccess for example (SudentAccess is only compiled with MFC)

1. why use raw data type in Auto class
   for flexibility & reducing complexity
   generally, you can use Auto class to do most of the work.
2. why most function not inline
  because compiler will auto inline when use optimization, most functions is not inline.
  make code clear & easy to read.
  if you prefer inline function
  just add inline keyword to declaration & paste body in bottom of .h files
  this will make code still clear.
3. about empty container:
  will insert an empty array field, and parse nothing.
  but if array field not exist, parser will regard as a failure.


NOTE
====

MongoClient:
 ctor: mongoc_init()
 dtor: mongoc_cleanup()
 may cause strange problem when application starts & exits if init & cleanup got panic.
 not encountered yet

BulkOperation
 win7 sp1, vs2013 update4, release
 bulk operation will cost only about 6.9%~10.6% time of single operation.
 test result: (time in ms)
cnt	   single	bulk	ratio
100		141		15		0.106382979
1000	1357	94		0.06927045
2000	2714	203		0.074797347
3000	3979	296		0.07439055
4000	5398	405		0.075027788
5000	6801	499		0.073371563
6000	7909	640		0.08092047
7000	9501	717		0.07546574
8000	10623	812		0.076437918
9000	11981	952		0.079459144
10000	13385	1014	0.075756444
20000	27254	2059	0.075548543
50000	67595	5429	0.080316591

 

About
=====

mongo-c-wrapper is a c++ wrapper class of mongo-c-driver for convenient.
use RAII class for resource manager (see MongoAuto.h)

mongo-c-wrapper depends on mongo-c-wrapper

Support / Feedback / Bug-report
===============================
this src is priveded 'as-is'. use at your own risk.

Enviroment
==========

mongo-c-driver-1.2.0.tar

windows:
cmake-3.3.2-win32-x86
win7 x64 sp1
VS2013 w update 4

Linux: 
Centos 7 x64 with dev
gcc: 4.8.2

How-to
======
1. build mongo-c-driver 1.2.0. you can get src from mongodb.org.
   after build, you will get pkgconfig file for how to use the driver, including lib / include path, lib to link.
2. involve src files in your project, include MongoClient.h in your src.

Folder structure
================
build:
Makefile for CentOS 7 x64 and solution file for VS2013 in WIN7 for simple test program.

docs:
Readme file

lib64mongoc:
CentOS 7 x64 built mongo c driver lib, test program will use this to run. You can change this in build/centos/Makefile
copy libbson-1.0.so.0 & libmongoc-1.0.so.0 here after mongo-c-driver built.

samples:
sample for how to use mongo-c-wrapper. Student & StudentAccess are used only in MSVC because MFC class member CString.

src:
mongo-c-wrapper source code.

win32libmongc:
VS2013 x32 built mongo c driver lib. used for test program compiling & running.
copy bin, include, lib filder here after mongo-c-driver built. usually entire CMAKE_INSTALL_PREFIX assigned to cmake -DCMAKE_INSTALL_PREFIX parameter when building mongo c driver.



Task TODO
=========
x: rejected		v: finished
x1. invisilbe mongolibc function, so avoid client to use raw mogolic data.
    reject: too complex
v2. merge ConstBson to AutoBson
  const: const AutoBson
  if destroy: AutoBson::isDestroy. use flag to indicate if destroy bson_t.
  parse bson error indicator. use flag to indicate bson -> value failure.
  realized in template
v3. check function & parameter const property
x4. every function: working fine with nullptr parameter, or null Auto object, 
  return false if invalid parameter
  operator! check
  rejected: too complex
v5. AutoCmd ? how & when to use.
	support $set, $orderby, $query QueryBson
v6. error msg collector ( with tag: )
v7. more sample & test case
v8. more help function, template non-member (like QueryLoad, etc...)  AccessBase.h
v9. AutoBson:: AddDoc AddArray in parameter bson_t* -> AutoBson (AutoBson::IsFailed)
v10. remove inline keyword
v11. AutoPoolColl::Find / Count parameter seq, make easier to use. 
 reneme some class.
 modify in BS proj
v12. remove BsonParser::operator BsonT* () & check in derived.