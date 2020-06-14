//
// Created by Prateek Agarwal on 13/6/20.
//
#pragma once
#ifndef _SQLITE_H_
#define _SQLITE_H_

#include "handle.h"
#include <sqlite3.h>
#include <string>

#ifdef _DEBUG
#define VERIFY ASSERT
#define VERIFY_(result, expression) ASSERT(result == expression)
#else
#define VERIFY(expression) (expression)
#define VERIFY_(result, expression) (expression)
#endif

struct SqliteException {
  int Result = 0;
  std::string Message;

  explicit SqliteException(sqlite3 *const connection)
    : Result(sqlite3_extended_errcode(connection)), Message(sqlite3_errmsg(connection)) {}
};

class Connection {
  struct ConnectionHandleTraits : HandleTraits<sqlite3 *> {
    static void Close(Type value) noexcept {
      VERIFY_(SQLITE_OK, sqlite3_close(value));
      fprintf(stdout, "Connection closed\n");
    }
  };

  using ConnectionHandle = Handle<ConnectionHandleTraits>;
  ConnectionHandle m_handle;

  template <typename F, typename C>
  void InternalOpen(F open, const C *const filename) {
    Connection temp;
    if (SQLITE_OK != open(filename, temp.m_handle.Set())) {
      temp.ThrowLastError();
    }
    swap(m_handle, temp.m_handle);
  }

public:
  Connection() noexcept = default;

  template <typename C>
  explicit Connection(const C *const filename) {
    Open(filename);
  }

  static Connection Memory() { return Connection(":memory:"); }

  static Connection WideMemory() { return Connection(L":memory:"); }

  explicit operator bool() const noexcept { return static_cast<bool>(m_handle); }

  sqlite3 *GetAbi() const noexcept { return m_handle.Get(); }

  void ThrowLastError() const { throw SqliteException(GetAbi()); }

  void Open(const char *const filename) {
    InternalOpen(sqlite3_open, filename);
    fprintf(stdout, "Connection open: %s\n", filename);
  }

  void Open(const wchar_t *const filename) {
    InternalOpen(sqlite3_open16, filename);
    fprintf(stdout, "Connection open: %ls\n", filename);
  }
};

template <typename T>
struct Reader {
  int GetInt(const int column = 0) const noexcept {
    return sqlite3_column_int(static_cast<const T *>(this)->GetAbi(), column);
  }

  char const *GetString(const int column = 0) const noexcept {
    return reinterpret_cast<const char *>(sqlite3_column_text(static_cast<const T *>(this)->GetAbi(), column));
  }

  wchar_t const *GetWideString(const int column = 0) const noexcept {
    return reinterpret_cast<const wchar_t *>(sqlite3_column_text16(static_cast<const T *>(this)->GetAbi(), column));
  }

  int GetStringLength(const int column = 0) const noexcept {
    return sqlite3_column_bytes(static_cast<T const *>(this)->GetAbi(), column);
  }

  int GetWideStringLength(const int column = 0) const noexcept {
    return sqlite3_column_bytes16(static_cast<T const *>(this)->GetAbi(), column) / sizeof(wchar_t);
  }
};

class Row : Reader<Row> {
  sqlite3_stmt *m_statement = nullptr;

public:
  sqlite3_stmt *GetAbi() const noexcept { return m_statement; }

  Row(sqlite3_stmt *const statement) noexcept : m_statement(statement) {}
};

class Statement : public Reader<Statement> {
  struct StatementHandleTraits : HandleTraits<sqlite3_stmt *> {
    static void Close(Type value) noexcept { VERIFY_(SQLITE_OK, sqlite3_finalize(value)); }
  };

  using StatementHandle = Handle<StatementHandleTraits>;
  StatementHandle m_handle;

  template <typename F, typename C>
  void InternalPrepare(const Connection &connection, F prepare, const C *const text) {
    assert(connection);
    auto result = prepare(connection.GetAbi(), text, -1, m_handle.Set(), nullptr);
    if (SQLITE_OK != result) {
      connection.ThrowLastError();
    }
  }

public:
  Statement() noexcept = default;
  explicit operator bool() const noexcept { return static_cast<bool>(m_handle); }

  sqlite3_stmt *GetAbi() const noexcept { return m_handle.Get(); }

  void ThrowLastError() const { throw SqliteException(sqlite3_db_handle(GetAbi())); }

  void Prepare(const Connection &connection, const char *const text) {
    InternalPrepare(connection, sqlite3_prepare_v2, text);
  }

  void Prepare(const Connection &connection, const wchar_t *const text) {
    InternalPrepare(connection, sqlite3_prepare16_v2, text);
  }

  bool Step() const {
    const auto result = sqlite3_step(GetAbi());

    if (result == SQLITE_ROW)
      return true;
    if (result == SQLITE_DONE)
      return false;
    ThrowLastError();
  }

  void Execute() const { VERIFY(!Step()); }
};

class RowIterator {
  const Statement *m_statement = nullptr;

public:
  RowIterator() noexcept = default;

  RowIterator(const Statement &statement) noexcept {
    if (statement.Step()) {
      m_statement = &statement;
    }
  }

  RowIterator &operator++() noexcept {
    if (!m_statement->Step())
      m_statement = nullptr;
    return *this;
  }

  bool operator!=(const RowIterator &other) const noexcept { return m_statement != other.m_statement; }

  bool operator==(const RowIterator &other) const noexcept { return m_statement == other.m_statement; }

  Row operator*() const noexcept { return Row(m_statement->GetAbi()); }
};

inline RowIterator begin(const Statement &statement) noexcept { return RowIterator(statement); }

inline RowIterator end(const Statement &) noexcept { return RowIterator(); }
#endif //_SQLITE_H_
