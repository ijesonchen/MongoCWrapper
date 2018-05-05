#include "mfcafx.h"


#include "TestMongoc.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>

#include <mongoc.h>



#include "mfcnew.h"

using namespace std;





void TestMongoC(void)
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	bson_error_t error = {};
	const bson_t *doc;
	const char *uristr = "mongodb://192.168.13.75/?appname=client-example";
	const char *collection_name = "test";
	bson_t query;
	char *str;

	mongoc_init();

	client = mongoc_client_new(uristr);

	if (!client) {
		Trap(nullptr);
	}
	
	bson_init(&query);

//	bson_append_utf8(&query, "hello", -1, "world", -1);
	
	collection = mongoc_client_get_collection(client, "test", collection_name);

	/*
	mongoc_cursor_t *
		mongoc_collection_find (mongoc_collection_t *collection,
		mongoc_query_flags_t flags,
		uint32_t skip,
		uint32_t limit,
		uint32_t batch_size,
		const bson_t *query,
		const bson_t *fields,
		const mongoc_read_prefs_t *read_prefs)
	*/
	cursor = mongoc_collection_find(
		collection, MONGOC_QUERY_NONE, 0, 0, 0, &query, NULL, NULL);

	while (mongoc_cursor_next(cursor, &doc)) {
		str = bson_as_json(doc, NULL);
		fprintf(stdout, "%s\n", str);
		bson_free(str);
	}

	// bool mongoc_cursor_error(mongoc_cursor_t *cursor, bson_error_t *error);

	// if cursor failed (e.g. connection error), return true and set error.
	// if cursor finished normally, return false and leave error alone.
	auto ret = mongoc_cursor_error(cursor, &error);

	if (ret)
	{
		cout << "cursor error: " << error.message << endl;
	}
	
	bson_destroy(&query);
	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);

	mongoc_cleanup();

}