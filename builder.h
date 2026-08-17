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
#ifdef RAPIDJSON_ASSERT
#undef RAPIDJSON_ASSERT
#endif
#define RAPIDJSON_ASSERT(x) \
  if (x)                    \
    ;                       \
  else                      \
    throw std::runtime_error("Failed: " #x);
// rapidjson errors handling

#include <rapidjson/document.h>

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

/**
 * @brief array source type
 */
enum class array_source { vector_t, list_t };

/**
 * @brief object source type
 */
enum class object_source { vector_t, list_t };

/**
 * \brief internal array structure to trick value_holder constructor
 */
struct array_holder final {
  array_holder() = default;
  array_holder(size_t reserve) { items.reserve(reserve); };
  array_holder(std::initializer_list<value_holder> values) noexcept
      : list_items(values), source(array_source::list_t) {};
  array_holder(const array_holder& src) = default;
  array_holder(array_holder&& src) = default;
  ~array_holder() = default;

  // items source for array
  array_source source{array_source::vector_t};
  // actual values for container source
  std::vector<value_holder> items;
  // actual values for the initializer_list source give us a 25% performance gain for arrays that passed as
  // initializer_list.
  std::initializer_list<value_holder> list_items;
};

/**
 * \brief holder for object field: name + value
 */
struct field_holder final {
  // invalid name
  constexpr field_holder(const nullptr_t name, const value_holder& value) noexcept : name(name, 0), value(value) {}
  // name as char*
  constexpr field_holder(const char* name, const value_holder& value) noexcept : name(name), value(value) {}
  // name as std::string_view
  constexpr field_holder(std::string_view name, const value_holder& value) noexcept
      : name(std::move(name)), value(value) {}
  // name as std::string
  field_holder(const std::string& name, const value_holder& value) noexcept : name(name), value(value) {}
  field_holder(const field_holder& src) = default;
  field_holder(field_holder&& src) = default;
  ~field_holder() = default;
  // actual values
  const std::string_view name;
  const value_holder& value;
};

/**
 * \brief internal object field structure with owning storage
 */
struct owned_field_holder final {
  std::string name;
  std::shared_ptr<value_holder> value;
};

/**
 * \brief internal object structure to support both initializer_list and map-like sources
 */
struct object_holder final {
  object_holder() = default;
  object_holder(std::initializer_list<field_holder> values) noexcept : source(object_source::list_t), list_items(values) {}
  object_holder(const object_holder& src) = default;
  object_holder(object_holder&& src) = default;
  ~object_holder() = default;

  // items source for object
  object_source source{object_source::vector_t};
  // actual values for container source
  std::vector<owned_field_holder> items;
  // actual values for initializer_list source
  std::initializer_list<field_holder> list_items;
};

/**
 * \brief generic json value holder
 */
struct value_holder final {
  // bool
  constexpr value_holder(const bool value) noexcept : holder(value) {}

  // const nullptr_t
  constexpr value_holder(const nullptr_t value) noexcept : holder(value) {}

  // const char*
  constexpr value_holder(const char* value) noexcept : holder(value) {}
  // const std::string& value
  value_holder(const std::string& value) noexcept : holder(value) {}
  // std::string_view value
  constexpr value_holder(std::string_view value) noexcept : holder(std::move(value)) {}
  // int64_t
  constexpr value_holder(const int64_t value) noexcept : holder(value) {}
#ifdef __linux__
  // long long
  constexpr value_holder(const long long value) noexcept : value_holder(static_cast<int64_t>(value)) {}
#else
  // long
  constexpr value_holder(const long value) noexcept : value_holder(static_cast<int64_t>(value)) {}
#endif
  // int32_t
  constexpr value_holder(const int32_t value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // int16_t
  constexpr value_holder(const int16_t value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // char as 8 bit signed value
  constexpr value_holder(const char value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // uint64_t
  constexpr value_holder(const uint64_t value) noexcept : holder(value) {}
#ifdef __linux__
  // unsigned long long
  constexpr value_holder(const unsigned long long value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
#else
  // unsigned long
  constexpr value_holder(const unsigned long value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
#endif
  // uint32_t
  constexpr value_holder(const uint32_t value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // uint16_t
  constexpr value_holder(const uint16_t value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // unsigned char as 8 bit unsigned value
  constexpr value_holder(const unsigned char value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // double
  constexpr value_holder(const double value) noexcept : holder(value) {}
  // float
  constexpr value_holder(const float value) noexcept : value_holder(static_cast<double>(value)) {}

  // json::value wrapper
  value_holder(const value& value) noexcept;
  value_holder(value&& value) noexcept;

  // object from initializer_list
  value_holder(std::initializer_list<field_holder> value) noexcept : holder(object_holder(value)) {}

  // object from object_holder
  value_holder(object_holder&& value) noexcept : holder(std::move(value)) {}

  // array from initializer_list or container, safe to move out from
  // array_holder, because it's our internal structure
  value_holder(array_holder&& value) noexcept : holder(std::move(value)) {}

  // copy constructor
  value_holder(const value_holder& src) = default;
  // move constructor
  value_holder(value_holder&& src) = default;
  ~value_holder() = default;

  const std::variant<std::nullptr_t,
                     std::string_view,
                     int64_t,
                     uint64_t,
                     double,
                     bool,
                     object_holder,
                     array_holder>
      holder;
};

}  // namespace builder

class value final {
 public:
  constexpr value(const bool v) noexcept : holder_(v) {}
  constexpr value(const nullptr_t v) noexcept : holder_(v) {}
  constexpr value(const char* v) noexcept : holder_(v) {}
  value(const std::string& v) noexcept : holder_(v) {}
  constexpr value(std::string_view v) noexcept : holder_(std::move(v)) {}
  constexpr value(const int64_t v) noexcept : holder_(v) {}
#ifdef __linux__
  constexpr value(const long long v) noexcept : value(static_cast<int64_t>(v)) {}
#else
  constexpr value(const long v) noexcept : value(static_cast<int64_t>(v)) {}
#endif
  constexpr value(const int32_t v) noexcept : value(static_cast<int64_t>(v)) {}
  constexpr value(const int16_t v) noexcept : value(static_cast<int64_t>(v)) {}
  constexpr value(const char v) noexcept : value(static_cast<int64_t>(v)) {}
  constexpr value(const uint64_t v) noexcept : holder_(v) {}
#ifdef __linux__
  constexpr value(const unsigned long long v) noexcept : value(static_cast<uint64_t>(v)) {}
#else
  constexpr value(const unsigned long v) noexcept : value(static_cast<uint64_t>(v)) {}
#endif
  constexpr value(const uint32_t v) noexcept : value(static_cast<uint64_t>(v)) {}
  constexpr value(const uint16_t v) noexcept : value(static_cast<uint64_t>(v)) {}
  constexpr value(const unsigned char v) noexcept : value(static_cast<uint64_t>(v)) {}
  constexpr value(const double v) noexcept : holder_(v) {}
  constexpr value(const float v) noexcept : value(static_cast<double>(v)) {}

  value(const value& src) = default;
  value(value&& src) = default;
  value& operator=(const value& src) = default;
  value& operator=(value&& src) = default;
  ~value() = default;

 private:
  friend struct builder::value_holder;
  template <typename MAP>
  friend value object(MAP&& map);
  template <typename CONTAINER>
  friend value array(CONTAINER&& container);
  friend value array(std::initializer_list<builder::value_holder> list);
  friend std::string build(const value& value);
  friend rapidjson::Value build_value(const value& value, rapidjson::Document::AllocatorType& allocator);
  friend rapidjson::Document build_document(const value& value);

  explicit value(builder::value_holder&& holder) noexcept : holder_(std::move(holder)) {}

  const builder::value_holder& internal() const noexcept { return holder_; }

  builder::value_holder holder_;
};

inline builder::value_holder::value_holder(const value& value) noexcept : holder(value.internal().holder) {}

inline builder::value_holder::value_holder(value&& value) noexcept : holder(std::move(value.holder_.holder)) {}

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
}  // namespace detail

template <typename MAP>
value object(MAP&& map) {
  builder::object_holder object_value;
  object_value.source = builder::object_source::vector_t;

  if constexpr (detail::has_size_v<std::decay_t<MAP>>) {
    object_value.items.reserve(static_cast<size_t>(map.size()));
  }

  if constexpr (std::is_rvalue_reference_v<MAP&&>) {
    for (auto&& [name, value] : map) {
      static_assert(std::is_convertible_v<decltype(name), std::string_view>,
                    "Map key type must be convertible to std::string_view");
      object_value.items.push_back(
          builder::owned_field_holder{std::string(std::string_view(name)),
                                      std::make_shared<builder::value_holder>(std::move(value))});
    }
  } else {
    for (const auto& [name, value] : map) {
      static_assert(std::is_convertible_v<decltype(name), std::string_view>,
                    "Map key type must be convertible to std::string_view");
      object_value.items.push_back(
          builder::owned_field_holder{std::string(std::string_view(name)),
                                      std::make_shared<builder::value_holder>(value)});
    }
  }

  return value(builder::value_holder(std::move(object_value)));
}

template <typename CONTAINER>
value array(CONTAINER&& container) {
  builder::array_holder array_value;

  if constexpr (detail::has_size_v<std::decay_t<CONTAINER>>) {
    array_value.items.reserve(static_cast<size_t>(container.size()));
  }

  if constexpr (std::is_rvalue_reference_v<CONTAINER&&>) {
    for (auto&& value : container) {
      array_value.items.emplace_back(std::move(value));
    }
  } else {
    for (const auto& value : container) {
      array_value.items.emplace_back(value);
    }
  }
  return value(builder::value_holder(std::move(array_value)));
}

/**
 * \brief helper function to convert initializer_list of values explicitly to
 * ArrayHolder
 */
value array(std::initializer_list<builder::value_holder> list);

/**
 * \brief build json string
 */
std::string build(std::initializer_list<builder::field_holder> value);
std::string build(std::initializer_list<builder::value_holder> value);
std::string build(const value& value);

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
 * \brief build json string from rapidjson document
 */
std::string stringify(const rapidjson::Document& document);

}  // namespace json
