
#ifndef MULTIARRAYLIST_HPP
#define MULTIARRAYLIST_HPP

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <tuple>
#include <utility>

namespace soa {

template <typename... Ts> class MultiArrayList final {
public:
  constexpr MultiArrayList(const MultiArrayList &) = delete;
  constexpr MultiArrayList(MultiArrayList &&) = delete;
  constexpr MultiArrayList &operator=(const MultiArrayList &) = delete;
  constexpr MultiArrayList &operator=(MultiArrayList &&) = delete;
  constexpr ~MultiArrayList() { deallocate(); }

  explicit constexpr MultiArrayList(size_t initial_capacity = 1)
      : m_capacity(initial_capacity) {
    allocate();
  }

  constexpr MultiArrayList(std::initializer_list<std::tuple<Ts...>> init)
      : m_capacity(init.size()), m_size(init.size()) {
    allocate();
    size_t i = 0;
    for (const auto &row : init) {
      assign_row(row, i);
      ++i;
    }
  }

  constexpr void push_back(const Ts &...values) {
    if (m_size == m_capacity) {
      grow();
    }
    push_impl(values...);
    ++m_size;
  }

  constexpr void emplace_back(Ts &&...args) {
    if (m_size == m_capacity) {
      grow();
    }
    push_impl(std::forward<Ts>(args)...);
    ++m_size;
  }

  [[nodiscard]] constexpr size_t size() const { return m_size; }
  [[nodiscard]] constexpr size_t capacity() const { return m_capacity; }

  constexpr auto operator[](size_t index) { return ref_tuple(index); }

private:
  static constexpr size_t m_num_fields = sizeof...(Ts);
  size_t m_capacity{};
  size_t m_size{};
  std::tuple<Ts *...> m_columns = {};

  constexpr void allocate() {
    ((std::get<Ts *>(m_columns) = new Ts[m_capacity]()), ...);
  }

  constexpr void deallocate() { ((delete[] std::get<Ts *>(m_columns)), ...); }

  constexpr void reallocate(size_t new_cap) {
    (([&]() {
       Ts *old = std::get<Ts *>(m_columns);
       Ts *new_array = new Ts[new_cap](); // default-initialized
       std::copy_n(old, m_size, new_array);
       std::get<Ts *>(m_columns) = new_array;
       delete[] old;
     }()),
     ...);
    m_capacity = new_cap;
  }

  constexpr void grow() { reallocate(m_capacity * 2); }

  constexpr void assign_row(const std::tuple<Ts...> &row, size_t i) {
    ((std::get<Ts *>(m_columns)[i] = std::get<Ts>(row)), ...);
  }

  template <typename... Args> constexpr void push_impl(Args &&...args) {
    ((std::get<Ts *>(m_columns)[m_size] = std::forward<Args>(args)), ...);
  }
  constexpr auto ref_tuple(size_t i) {
    return std::apply([=](auto *...ptrs) { return std::tie(*(ptrs + i)...); },
                      m_columns);
  }

public:
  template <typename... Selected> class View {
  public:
    class Iterator {
    public:
      using value_type = std::tuple<Selected &...>;
      using reference = value_type;
      using pointer = void; // not really usable
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::random_access_iterator_tag;

      // Now store a reference instead of a pointer.
      constexpr Iterator(MultiArrayList &mal, size_t index) noexcept
          : m_mal(mal), m_index(index) {}

      constexpr reference operator*() const {
        return std::tie(*(std::get<Selected *>(m_mal.m_columns) + m_index)...);
      }

      constexpr Iterator &operator++() noexcept {
        ++m_index;
        return *this;
      }
      constexpr Iterator operator++(int) noexcept {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      constexpr bool operator==(const Iterator &other) const noexcept {
        return m_index == other.m_index;
      }
      constexpr bool operator!=(const Iterator &other) const noexcept {
        return m_index != other.m_index;
      }

    private:
      MultiArrayList &m_mal;
      size_t m_index;
    };

    // The view simply holds a reference, no pointer needed.
    explicit constexpr View(MultiArrayList &mal) noexcept : m_mal(mal) {}
    constexpr Iterator begin() const noexcept { return Iterator(m_mal, 0); }
    constexpr Iterator end() const noexcept {
      return Iterator(m_mal, m_mal.size());
    }

  private:
    MultiArrayList &m_mal;
  };

  // Update the convenience method to create a view by type.
  template <typename... Selected> constexpr View<Selected...> view() {
    return View<Selected...>(*this);
  }

public:
  class Iterator {
  public:
    using value_type = std::tuple<Ts &...>;
    using reference = value_type;
    using pointer = void; // not really usable
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    constexpr Iterator(MultiArrayList *parent, size_t pos)
        : m_parent(parent), m_index(pos) {}

    constexpr reference operator*() const {
      return m_parent->ref_tuple(m_index);
    }

    constexpr Iterator &operator++() {
      ++m_index;
      return *this;
    }
    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    constexpr Iterator &operator--() {
      --m_index;
      return *this;
    }
    constexpr Iterator operator--(int) {
      Iterator tmp = *this;
      --(*this);
      return tmp;
    }

    constexpr Iterator &operator+=(difference_type diff) {
      m_index += diff;
      return *this;
    }

    constexpr Iterator &operator-=(difference_type diff) {
      m_index -= diff;
      return *this;
    }

    constexpr Iterator operator+(difference_type diff) const {
      return Iterator(m_parent, m_index + diff);
    }
    constexpr Iterator operator-(difference_type diff) const {
      return Iterator(m_parent, m_index - diff);
    }
    constexpr difference_type operator-(const Iterator &other) const {
      return m_index - other.m_index;
    }

    constexpr bool operator==(const Iterator &other) const {
      return m_index == other.m_index;
    }
    constexpr bool operator!=(const Iterator &other) const {
      return m_index != other.m_index;
    }
    constexpr bool operator<(const Iterator &other) const {
      return m_index < other.m_index;
    }
    constexpr bool operator>(const Iterator &other) const {
      return m_index > other.m_index;
    }
    constexpr bool operator<=(const Iterator &other) const {
      return m_index <= other.m_index;
    }
    constexpr bool operator>=(const Iterator &other) const {
      return m_index >= other.m_index;
    }

  private:
    MultiArrayList *m_parent;
    size_t m_index;
  };

  constexpr Iterator begin() { return Iterator(this, 0); }
  constexpr Iterator end() { return Iterator(this, m_size); }

  constexpr Iterator erase(Iterator pos) {
    size_t idx = pos.m_index;
    (([&]() {
       for (size_t i = idx; i < m_size - 1; ++i) {
         std::get<Ts *>(m_columns)[i] =
             std::move(std::get<Ts *>(m_columns)[i + 1]);
       }
     }()),
     ...);
    --m_size;
    return Iterator(this, idx);
  }
};

} // namespace soa

#endif
