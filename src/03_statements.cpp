//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"
#include <cstdio>

int main() {

  try {
    // Create a connection
    auto connection = Connection::Memory();

    Statement statement;
    statement.Prepare(connection, "select 'Hello' union all select 'World'");

    for (const auto &row : statement) {
      fprintf(stdout, "%s\n", statement.GetString(0));
    }
  }

  catch (const SqliteException &e) {
    fprintf(stderr, "%s (%d)\n", e.Message.c_str(), e.Result);
  }
}
