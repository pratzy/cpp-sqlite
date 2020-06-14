//
// Created by Prateek Agarwal on 13/6/20.
//
#pragma once

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

namespace sqlite {

struct Exception {
  int Result = 0;
  std::string Message;

  explicit Exception(sqlite3 *const connection)
    : Result(std::move(sqlite3_extended_errcode(connection))), Message(std::move(sqlite3_errmsg(connection))) {}
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

  void ThrowLastError() const { throw Exception(GetAbi()); }

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

class Row : public Reader<Row> {
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

  template <typename F, typename C, typename... Values>
  void InternalPrepare(const Connection &connection, F prepare, const C *const text, Values &&... values) {
    assert(connection);
    auto result = prepare(connection.GetAbi(), text, -1, m_handle.Set(), nullptr);
    if (SQLITE_OK != result) {
      connection.ThrowLastError();
    }
    BindAll(std::forward<Values>(values)...);
  }

  void InternalBind(const int index) const noexcept {}

  template <typename First, typename... Rest>
  void InternalBind(const int index, First &&first, Rest &&... rest) const {
    Bind(index, std::forward<First>(first));
    InternalBind(index + 1, std::forward<Rest>(rest)...);
  }

public:
  Statement() noexcept = default;

  template <typename C, typename... Values>
  Statement(const Connection &conn, const C *const text, Values &&... values) {
    Prepare(conn, text, std::forward<Values>(values)...);
  }

  explicit operator bool() const noexcept { return static_cast<bool>(m_handle); }

  sqlite3_stmt *GetAbi() const noexcept { return m_handle.Get(); }

  void ThrowLastError() const { throw Exception(sqlite3_db_handle(GetAbi())); }

  template <typename... Values>
  void Prepare(const Connection &connection, const char *const text, Values &&... values) {
    InternalPrepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values)...);
  }

  template <typename... Values>
  void Prepare(const Connection &connection, const wchar_t *const text, Values &&... values) {
    InternalPrepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values)...);
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

  bool Step() const {
    const auto result = sqlite3_step(GetAbi());

    if (result == SQLITE_ROW)
      return true;
    if (result == SQLITE_DONE)
      return false;
    ThrowLastError();
  }

#pragma GCC diagnostic pop

  void Execute() const { VERIFY(!Step()); }

  void Bind(const int index, const int value) {
    if (SQLITE_OK != sqlite3_bind_int(GetAbi(), index, value)) {
      ThrowLastError();
    }
  }

  void Bind(const int index, const char *const value, const int size = -1) const {
    if (SQLITE_OK != sqlite3_bind_text(GetAbi(), index, value, size, SQLITE_STATIC)) {
      ThrowLastError();
    }
  }

  void Bind(const int index, const wchar_t *const value, const int size = -1) const {
    if (SQLITE_OK != sqlite3_bind_text16(GetAbi(), index, value, size, SQLITE_STATIC)) {
      ThrowLastError();
    }
  }

  void Bind(const int index, const std::string &value) const { Bind(index, value.c_str(), value.size()); }

  void Bind(const int index, const std::wstring &value) const {
    Bind(index, value.c_str(), value.size() * sizeof(wchar_t));
  }

  void Bind(const int index, const std::string &&value) const {
    if (SQLITE_OK != sqlite3_bind_text(GetAbi(), index, value.c_str(), value.size(), SQLITE_TRANSIENT)) {
      ThrowLastError();
    }
  }

  void Bind(const int index, const std::wstring &&value) const {
    if (SQLITE_OK !=
        sqlite3_bind_text16(GetAbi(), index, value.c_str(), value.size() * sizeof(wchar_t), SQLITE_TRANSIENT)) {
      ThrowLastError();
    }
  }

  template <typename... Values>
  void BindAll(Values &&... values) const {
    InternalBind(1, std::forward<Values>(values)...);
  }
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

template <typename C, typename... Values>
void Execute(const Connection &connection, const C *const text, Values &&... values) {
  Statement(connection, text, std::forward<Values>(values)...);
}

} // namespace sqlite
