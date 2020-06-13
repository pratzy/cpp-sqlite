//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"
#include <cstdio>

int main() {

  try {
    // Create a connection pointer
    Connection connection = Connection::Memory();
    Connection w_connection = Connection::WideMemory();
    Connection f_connection{"../db/temp.db"};
  }

  catch (SqliteException const &e) {
    fprintf(stderr, "%s (%d)\n", e.Message.c_str(), e.Result);
  }
}
