//
// Created by Prateek Agarwal on 13/6/20.
//
#pragma once
#include <cassert>

/**
 * A generic trait of a handle, must be initialized with nullptr
 * @tparam T
 */
template <typename T>
struct HandleTraits {
  using Type = T;

  static Type invalid() noexcept { return nullptr; }

  // Derived classes to provide implementation of close()
  // static void close(Type value) noexcept
};

template <typename Traits>
class Handle {
  using Type = decltype(Traits::invalid());
  Type m_value;

  void close() noexcept {
    if (*this) {
      Traits::close(m_value);
    }
  }

public:
  // Delete copy and assignment constructors
  Handle(Handle const &) = delete;
  Handle &operator=(Handle const &) = delete;

  explicit Handle(Type value = Traits::invalid()) noexcept : m_value{value} {}

  // Move constructor
  Handle(Handle &&other) noexcept : m_value{other.detach()} {}

  // Move assignment
  Handle &operator=(Handle &&other) noexcept {
    if (this != &other) {
      reset(other.detach());
    }

    return *this;
  }

  // Destructor
  ~Handle() noexcept { close(); }

  explicit operator bool() const noexcept { return m_value != Traits::invalid(); }

  Type get() const noexcept { return m_value; }

  Type *set() noexcept {
    assert(!*this);
    return &m_value;
  }

  Type detach() noexcept {
    Type value = m_value;
    m_value = Traits::invalid();
    return value;
  }

  bool reset(Type value = Traits::invalid()) noexcept {
    if (m_value != value) {
      close();
      m_value = value;
    }
    return static_cast<bool>(*this);
  }

  void swap(Handle<Traits> &other) noexcept {
    Type temp = m_value;
    m_value = other.m_value;
    other.m_value = temp;
  }
};

template <typename Traits>
void swap(Handle<Traits> &left, Handle<Traits> &right) noexcept {
  left.swap(right);
}

template <typename Traits>
bool operator==(Handle<Traits> const &left, Handle<Traits> const &right) noexcept {
  return left.get() == right.get();
}

template <typename Traits>
bool operator!=(Handle<Traits> const &left, Handle<Traits> const &right) noexcept {
  return !(left == right);
}