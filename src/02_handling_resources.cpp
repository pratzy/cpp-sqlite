//
// Created by Prateek Agarwal on 13/6/20.
//

#include "handle.h"
#include <cstdio>
#include <sqlite3.h>

#ifdef _DEBUG
#define VERIFY ASSERT
#define VERIFY_(result, expression) ASSERT(result == expression)
#else
#define VERIFY(expression) (expression)
#define VERIFY_(result, expression) (expression)
#endif

struct ConnectionHandleTraits : HandleTraits<sqlite3 *> {
  static void Close(Type value) noexcept {
    VERIFY_(SQLITE_OK, sqlite3_close(value));
    fprintf(stdout, "Connection closed\n");
  }
};

using ConnectionHandle = Handle<ConnectionHandleTraits>;

int main() {
  // Create a connection pointer
  ConnectionHandle connection;
  int result;

  // Open the connection to memory or desired DB file
  // by passing pointer to connection pointer
  result = sqlite3_open(":memory:", connection.Set());

  // If the result is not OK, print error and close the connection
  if (SQLITE_OK != result) {
    fprintf(stderr, "%s\n", sqlite3_errmsg(connection.Get()));
    return result;
  }
  fprintf(stdout, "Connection opened\n");
}
