#ifndef TestMongoc_h__
#define TestMongoc_h__

#include <iostream>
#include <string>
#include <vector>
#include <bson.h>
#include <mongoc.h>
#include <tuple>


/*
// $cmp Returns: 0 if the two values are equivalent, 1 if the first value is greater than the second, and -1 if the
// 	first value is less than the second.
// $eq Returns true if the values are equivalent.
// $gt Returns true if the first value is greater than the second.
// $gte Returns true if the first value is greater than or equal to the second.
// $lt Returns true if the first value is less than the second.
// $lte Returns true if the first value is less than or equal to the second.
// $ne Returns true if the values are not equivalent.

*/
void Trap(void* p);
void TestMongoC(void);


#endif // TestMongoc_h__
