/*

 MIT License

 Copyright (c) 2021 pavel.sokolov@gmail.com / CEZEO software Ltd. All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#ifndef RAPIDJSON_HAS_CXX11_RVALUE_REFS
#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1
#endif
#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif

// rapidjson errors handling
#include <stdexcept>

#ifndef RAPIDJSON_ASSERT_THROWS
#define RAPIDJSON_ASSERT_THROWS 1
#endif
// a user-provided RAPIDJSON_ASSERT is respected: redefining it would give different rapidjson code in different
// translation units (ODR violation). builder own checks always throw, see JSON_BUILDER_CHECK in builder.cpp.
#ifndef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) ((x) ? static_cast<void>(0) : throw std::runtime_error("Failed: " #x))
#endif
// rapidjson errors handling

#include <rapidjson/document.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace json {
class value;
namespace builder {

struct value_holder;
struct owned_field_holder;

/**
 * \brief integral types (except bool) and unscoped enums are stored as int64_t or uint64_t
 */
template <typename T, bool = std::is_enum_v<T>>
struct is_unscoped_enum : std::false_type {};

template <typename T>
struct is_unscoped_enum<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template <typename T>
inline constexpr bool is_integer_v =
    (std::is_integral_v<T> && !std::is_same_v<T, bool>) || is_unscoped_enum<T>::value;

// string types are accepted by template constructors only: templates don't take part in braced list
// initialization, so {{1, 2}} is an array and {{"name", 1}} is an object, not std::string{...}
template <typename S, typename T>
using if_same_t = std::enable_if_t<std::is_same_v<S, T>, int>;

template <typename T>
constexpr auto to_integer(const T value) noexcept {
  if constexpr (std::is_enum_v<T>) {
    return to_integer(static_cast<std::underlying_type_t<T>>(value));
  } else if constexpr (std::is_signed_v<T>) {
    return static_cast<int64_t>(value);
  } else {
    return static_cast<uint64_t>(value);
  }
}

/**
 * \brief internal array structure to trick value_holder constructor
 * \note iterates over initializer_list items (borrowed) or over own storage (container source)
 */
struct array_holder final {
  array_holder() noexcept = default;
  array_holder(std::initializer_list<value_holder> values) noexcept;
  explicit array_holder(std::vector<value_holder>&& values) noexcept;
  array_holder(const array_holder& src);
  array_holder(array_holder&& src) noexcept;
  array_holder& operator=(const array_holder& src);
  array_holder& operator=(array_holder&& src) noexcept;
  ~array_holder() = default;

  const value_holder* begin() const noexcept { return data_; }
  const value_holder* end() const noexcept;
  size_t size() const noexcept { return size_; }

 private:
  // actual values for container source, empty for initializer_list source
  std::vector<value_holder> storage_;
  // items to iterate: initializer_list items or storage_ data
  const value_holder* data_{nullptr};
  size_t size_{0};
};

/**
 * \brief holder for object field: name + value
 */
struct field_holder final {
  // invalid name
  constexpr field_holder(const std::nullptr_t, const value_holder& value) noexcept : name(nullptr, 0), value(value) {}
  // name as char*, nullptr is an invalid name
  constexpr field_holder(const char* name, const value_holder& value) noexcept
      : name(name ? std::string_view(name) : std::string_view(nullptr, 0)), value(value) {}
  // name as std::string_view
  template <typename S, if_same_t<S, std::string_view> = 0>
  constexpr field_holder(const S name, const value_holder& value) noexcept : name(name), value(value) {}
  // name as std::string
  template <typename S, if_same_t<S, std::string> = 0>
  field_holder(const S& name, const value_holder& value) noexcept : name(name), value(value) {}
  field_holder(const field_holder& src) = default;
  field_holder(field_holder&& src) = default;
  ~field_holder() = default;
  // actual values
  const std::string_view name;
  const value_holder& value;
};

/**
 * \brief internal object structure to support both initializer_list and map-like sources
 */
struct object_holder final {
  object_holder() noexcept = default;
  object_holder(std::initializer_list<field_holder> values) noexcept : list_items(values) {}
  explicit object_holder(std::vector<owned_field_holder>&& values) noexcept;
  object_holder(const object_holder& src) = default;
  object_holder(object_holder&& src) noexcept = default;
  object_holder& operator=(const object_holder& src) = default;
  object_holder& operator=(object_holder&& src) noexcept = default;
  ~object_holder() = default;

  size_t size() const noexcept;

  // actual values for container source, empty for initializer_list source
  std::vector<owned_field_holder> items;
  // actual values for initializer_list source
  std::initializer_list<field_holder> list_items;
};

/**
 * \brief generic json value holder
 */
struct value_holder final {
  using holder_t = std::variant<std::nullptr_t,
                                std::string_view,
                                int64_t,
                                uint64_t,
                                double,
                                bool,
                                object_holder,
                                array_holder,
                                // borrowed json::value
                                const value_holder*,
                                // string moved or copied out of an rvalue container
                                std::string>;

  // bool
  constexpr value_holder(const bool value) noexcept : holder(value) {}

  // const nullptr_t
  constexpr value_holder(const std::nullptr_t value) noexcept : holder(value) {}

  // const char*, nullptr is stored as json null
  value_holder(const char* value) noexcept : holder(value ? holder_t(std::string_view(value)) : holder_t(nullptr)) {}
  // const std::string& value
  template <typename S, if_same_t<S, std::string> = 0>
  value_holder(const S& value) noexcept : holder(std::string_view(value)) {}
  // std::string_view value
  template <typename S, if_same_t<S, std::string_view> = 0>
  constexpr value_holder(const S value) noexcept : holder(value) {}
  // integral types and unscoped enums as int64_t or uint64_t
  template <typename T, std::enable_if_t<is_integer_v<T>, int> = 0>
  constexpr value_holder(const T value) noexcept
      : holder(std::in_place_type<decltype(to_integer(value))>, to_integer(value)) {}
  // double
  constexpr value_holder(const double value) noexcept : holder(value) {}
  // float
  constexpr value_holder(const float value) noexcept : value_holder(static_cast<double>(value)) {}

  // json::value is borrowed: it shall outlive the build call, like any other source
  value_holder(const value& value) noexcept;
  // temporary json::value is moved in
  value_holder(value&& value) noexcept;
  // a const temporary cannot be moved, so copy its storage instead of borrowing it
  value_holder(const value&& value);

  // object from initializer_list
  value_holder(std::initializer_list<field_holder> value) noexcept
      : holder(std::in_place_type<object_holder>, value) {}

  // object from object_holder
  value_holder(object_holder&& value) noexcept : holder(std::move(value)) {}

  // owned string, for items of rvalue containers only: a borrowed one would dangle with the container
  value_holder(std::in_place_type_t<std::string>, std::string value) noexcept
      : holder(std::in_place_type<std::string>, std::move(value)) {}

  // array from initializer_list or container, safe to move out from
  // array_holder, because it's our internal structure
  value_holder(array_holder&& value) noexcept : holder(std::move(value)) {}

  value_holder(const value_holder& src) = default;
  value_holder(value_holder&& src) noexcept = default;
  value_holder& operator=(const value_holder& src) = default;
  value_holder& operator=(value_holder&& src) noexcept = default;
  ~value_holder() = default;

  holder_t holder;
};

/**
 * \brief internal object field structure for container source
 */
struct owned_field_holder final {
  std::string_view name() const noexcept {
    return nullptr != borrowed_name.data() ? borrowed_name : std::string_view(owned_name);
  }

  // name borrowed from the source, null if the name is owned
  std::string_view borrowed_name;
  // name copied from an rvalue source
  std::string owned_name;
  value_holder value;
};

inline array_holder::array_holder(std::initializer_list<value_holder> values) noexcept
    : data_(values.begin()), size_(values.size()) {}

inline array_holder::array_holder(std::vector<value_holder>&& values) noexcept
    : storage_(std::move(values)), data_(storage_.data()), size_(storage_.size()) {}

inline array_holder::array_holder(const array_holder& src)
    : storage_(src.storage_), data_(src.storage_.empty() ? src.data_ : storage_.data()), size_(src.size_) {}

// std::vector move keeps the buffer, so data_ stays valid
inline array_holder::array_holder(array_holder&& src) noexcept
    : storage_(std::move(src.storage_)), data_(src.data_), size_(src.size_) {
  src.data_ = nullptr;
  src.size_ = 0;
}

inline array_holder& array_holder::operator=(const array_holder& src) {
  if (this != &src) {
    storage_ = src.storage_;
    data_ = src.storage_.empty() ? src.data_ : storage_.data();
    size_ = src.size_;
  }
  return *this;
}

inline array_holder& array_holder::operator=(array_holder&& src) noexcept {
  if (this != &src) {
    storage_ = std::move(src.storage_);
    data_ = src.data_;
    size_ = src.size_;
    src.data_ = nullptr;
    src.size_ = 0;
  }
  return *this;
}

inline const value_holder* array_holder::end() const noexcept {
  return data_ + size_;
}

inline object_holder::object_holder(std::vector<owned_field_holder>&& values) noexcept : items(std::move(values)) {}

inline size_t object_holder::size() const noexcept {
  return items.empty() ? list_items.size() : items.size();
}

}  // namespace builder

class value final {
 public:
  constexpr value(const bool v) noexcept : holder_(v) {}
  constexpr value(const std::nullptr_t v) noexcept : holder_(v) {}
  value(const char* v) noexcept : holder_(v) {}
  template <typename S, builder::if_same_t<S, std::string> = 0>
  value(const S& v) noexcept : holder_(v) {}
  // json::value keeps std::string_view of the string, a temporary (const one too) would dangle
  template <typename S, builder::if_same_t<std::remove_const_t<S>, std::string> = 0>
  value(S&& v) = delete;
  template <typename S, builder::if_same_t<S, std::string_view> = 0>
  constexpr value(const S v) noexcept : holder_(v) {}
  template <typename T, std::enable_if_t<builder::is_integer_v<T>, int> = 0>
  constexpr value(const T v) noexcept : holder_(v) {}
  constexpr value(const double v) noexcept : holder_(v) {}
  constexpr value(const float v) noexcept : holder_(v) {}

  value(const value& src) = default;
  value(value&& src) noexcept = default;
  value& operator=(const value& src) = default;
  value& operator=(value&& src) noexcept = default;
  ~value() = default;

 private:
  friend struct builder::value_holder;
  template <typename MAP>
  friend value object(MAP&& map);
  template <typename CONTAINER>
  friend value array(CONTAINER&& container);
  friend value array(std::initializer_list<builder::value_holder> list);
  friend void build(const value& value, std::string& out);
  friend rapidjson::Value build_value(const value& value, rapidjson::Document::AllocatorType& allocator);
  friend void build_document(const value& value, rapidjson::Document& document);

  explicit value(builder::value_holder&& holder) noexcept : holder_(std::move(holder)) {}

  const builder::value_holder& internal() const noexcept { return holder_; }

  builder::value_holder holder_;
};

inline builder::value_holder::value_holder(const value& value) noexcept
    : holder(std::in_place_type<const value_holder*>, &value.internal()) {}

inline builder::value_holder::value_holder(value&& value) noexcept : holder(std::move(value.holder_.holder)) {}

inline builder::value_holder::value_holder(const value&& value) : holder(value.internal().holder) {}

/**
 * \brief helper function to convert container explicitly to Array
 */
namespace detail {
template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<const T&>().size())>> : std::true_type {};

template <typename T>
inline constexpr bool has_size_v = has_size<T>::value;

// item of an rvalue container: json::value and std::string are moved in, or copied when const
template <typename T>
builder::value_holder rvalue_item(T&& item) {
  if constexpr (std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::string>) {
    return builder::value_holder(std::in_place_type<std::string>, std::move(item));
  } else {
    return builder::value_holder(std::move(item));
  }
}

// object name of a container key, a null char pointer is rejected like a null name in the initializer_list
template <typename N>
std::string_view key_name(const N& name) {
  if constexpr (std::is_pointer_v<N>) {
    if (nullptr == name) {
      throw std::runtime_error("Failed: nullptr != name");
    }
  }
  return std::string_view(name);
}
}  // namespace detail

/**
 * \brief helper function to convert map-like container explicitly to Object
 * \note names and values are borrowed from the map, names of rvalue map are copied if the name type owns its data
 */
template <typename MAP>
value object(MAP&& map) {
  std::vector<builder::owned_field_holder> fields;

  if constexpr (detail::has_size_v<std::decay_t<MAP>>) {
    fields.reserve(static_cast<size_t>(map.size()));
  }

  for (auto&& [name, field_value] : map) {
    using name_t = std::remove_cv_t<std::remove_reference_t<decltype(name)>>;
    static_assert(std::is_convertible_v<decltype(name), std::string_view>,
                  "Map key type must be convertible to std::string_view");
    if constexpr (!std::is_rvalue_reference_v<MAP&&>) {
      fields.push_back(builder::owned_field_holder{detail::key_name(name), {}, builder::value_holder(field_value)});
    } else if constexpr (std::is_same_v<name_t, std::string_view> || std::is_same_v<name_t, const char*> ||
                         std::is_same_v<name_t, char*>) {
      fields.push_back(
          builder::owned_field_holder{detail::key_name(name), {}, detail::rvalue_item(std::move(field_value))});
    } else {
      // Even trivially copyable key types can own inline character storage.
      fields.push_back(builder::owned_field_holder{
          {}, std::string(detail::key_name(name)), detail::rvalue_item(std::move(field_value))});
    }
  }

  return value(builder::value_holder(builder::object_holder(std::move(fields))));
}

/**
 * \brief helper function to convert container explicitly to Array
 */
template <typename CONTAINER>
value array(CONTAINER&& container) {
  std::vector<builder::value_holder> items;

  if constexpr (detail::has_size_v<std::decay_t<CONTAINER>>) {
    items.reserve(static_cast<size_t>(container.size()));
  }

  if constexpr (std::is_rvalue_reference_v<CONTAINER&&>) {
    for (auto&& item : container) {
      items.push_back(detail::rvalue_item(std::move(item)));
    }
  } else {
    for (const auto& item : container) {
      items.emplace_back(item);
    }
  }
  return value(builder::value_holder(builder::array_holder(std::move(items))));
}

/**
 * \brief helper function to convert initializer_list of values explicitly to Array
 * \note initializer_list items die at the end of the full expression, so they are copied into the returned value,
 * use nested `{{...}}` syntax inside build calls to avoid the copy
 */
value array(std::initializer_list<builder::value_holder> list);

/**
 * \brief build json string
 */
std::string build(std::initializer_list<builder::field_holder> value);
std::string build(std::initializer_list<builder::value_holder> value);
std::string build(const value& value);

/**
 * \brief build json string into the existing string, its capacity is reused
 */
void build(std::initializer_list<builder::field_holder> value, std::string& out);
void build(std::initializer_list<builder::value_holder> value, std::string& out);
void build(const value& value, std::string& out);

/**
 * \brief build rapidjson value (array or object)
 * \note Returned value borrows strings and object member names from the source
 * builder tree. The source `value` and any nested values used to build it must
 * outlive the returned rapidjson::Value.
 */
rapidjson::Value build_value(std::initializer_list<builder::field_holder> value,
                             rapidjson::Document::AllocatorType& allocator);
rapidjson::Value build_value(std::initializer_list<builder::value_holder> value,
                             rapidjson::Document::AllocatorType& allocator);
rapidjson::Value build_value(const value& value, rapidjson::Document::AllocatorType& allocator);

/**
 * \brief build rapidjson document with array or object
 * \note Returned document borrows strings and object member names from the
 * source builder tree. The source `value` and any nested values used to build
 * it must outlive the returned rapidjson::Document.
 */
rapidjson::Document build_document(std::initializer_list<builder::field_holder> value);
rapidjson::Document build_document(std::initializer_list<builder::value_holder> value);
rapidjson::Document build_document(const value& value);

/**
 * \brief build into the existing rapidjson document, its allocator is used (e.g. a MemoryPoolAllocator over a user
 * buffer to avoid the default 64KB chunk allocation)
 */
void build_document(std::initializer_list<builder::field_holder> value, rapidjson::Document& document);
void build_document(std::initializer_list<builder::value_holder> value, rapidjson::Document& document);
void build_document(const value& value, rapidjson::Document& document);

/**
 * \brief build json string from rapidjson document
 */
std::string stringify(const rapidjson::Document& document);

}  // namespace json
