//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"
#include <cstdio>

int main() {

  try {
    // Create a connection pointer
    auto connection = sqlite::Connection::Memory();
    auto w_connection = sqlite::Connection::WideMemory(); // utf-16 encoding
    sqlite::Connection f_connection{"../db/temp.db"};
  }

  catch (sqlite::Exception const &e) {
    fprintf(stderr, "%s (%d)\n", e.Message.c_str(), e.Result);
  }
}
