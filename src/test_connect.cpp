//
// Created by Prateek Agarwal on 13/6/20.
//
#include <cstdio>
#include <cstdlib>
#include <sqlite3.h>
#include "config.hpp"

int main(int argc, char *argv[]) {
  sqlite3 *db;

  if ((sqlite3_open(DB_PATH, &db))) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return EXIT_FAILURE;
  } else
    fprintf(stdout, "Opened database successfully\n");

  if ((sqlite3_close(db))) {
    fprintf(stderr, "Error on closing database: %s\n", sqlite3_errmsg(db));
    return EXIT_FAILURE;
  } else
    fprintf(stdout, "Closed database successfully\n");
}