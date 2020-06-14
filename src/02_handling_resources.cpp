//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"

int main() {

  try {
    // Create a connection pointer
    auto connection = sqlite::Connection::memory();
    auto w_connection = sqlite::Connection::wide_memory(); // utf-16 encoding
    sqlite::Connection f_connection{"../db/temp.db"};
  }

  catch (sqlite::Exception const &e) {
    std::cerr << e.message.c_str() << "(" << e.result << ")" << std::endl;
  }
}
