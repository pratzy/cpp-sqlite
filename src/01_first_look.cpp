//
// Created by Prateek Agarwal on 13/6/20.
//

/**
 * First look at the SQLite C API
 */

#include <cstdio>
#include <sqlite3.h>

int main() {
  // Create a connection pointer
  sqlite3 *connection = nullptr;
  int result;

  // open the connection to memory or desired DB file
  // by passing pointer to connection pointer
  result = sqlite3_open(":memory:", &connection);

  // If the result is not OK, print error and close the connection
  if (SQLITE_OK != result) {
    fprintf(stderr, "%s\n", sqlite3_errmsg(connection));
    sqlite3_close(connection);
    return result;
  }

  // Create a stmt pointer
  sqlite3_stmt *query = nullptr;

  // prepare the statement.
  result = sqlite3_prepare_v2(connection, "select 'Hello World!'", -1, &query, nullptr);

  // If the statement cannot be prepared, close the connection
  // The error in preparing statement will always return a nullptr,
  // so we don't need to close the statement itself.
  if (SQLITE_OK != result) {
    fprintf(stderr, "%s\n", sqlite3_errmsg(connection));
    sqlite3_close(connection);
    return result;
  }

  // step over the results and print the first column
  while (SQLITE_ROW == sqlite3_step(query)) {
    fprintf(stdout, "%s\n", sqlite3_column_text(query, 0));
  }

  // Destroy the statement
  sqlite3_finalize(query);

  // close the connection
  sqlite3_close(connection);
}
