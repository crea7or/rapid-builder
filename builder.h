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

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace json {
namespace builder {

struct value_holder;

/**
 * @brief array source type
 */
enum class array_source { vector_t, list_t };

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
  field_holder(const nullptr_t name, const value_holder& value) noexcept : name(name, 0), value(value) {}
  // name as char*
  field_holder(const char* name, const value_holder& value) noexcept : name(name), value(value) {}
  // name as std::string_view
  field_holder(std::string_view name, const value_holder& value) noexcept : name(std::move(name)), value(value) {}
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
 * \brief generic json value holder
 */
struct value_holder final {
  // bool
  value_holder(const bool value) noexcept : holder(value) {}

  // const nullptr_t
  value_holder(const nullptr_t value) noexcept : holder(value) {}

  // const char*
  value_holder(const char* value) noexcept : holder(value) {}
  // const std::string& value
  value_holder(const std::string& value) noexcept : holder(value) {}
  // std::string_view value
  value_holder(std::string_view value) noexcept : holder(std::move(value)) {}
  // int64_t
  value_holder(const int64_t value) noexcept : holder(value) {}
#ifdef __linux__
  // long long
  value_holder(const long long value) noexcept : value_holder(static_cast<int64_t>(value)) {}
#else
  // long
  value_holder(const long value) noexcept : value_holder(static_cast<int64_t>(value)) {}
#endif
  // int32_t
  value_holder(const int32_t value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // int16_t
  value_holder(const int16_t value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // char as 8 bit signed value
  value_holder(const char value) noexcept : value_holder(static_cast<int64_t>(value)) {}
  // uint64_t
  value_holder(const uint64_t value) noexcept : holder(value) {}
#ifdef __linux__
  // unsigned long long
  value_holder(const unsigned long long value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
#else
  // unsigned long
  value_holder(const unsigned long value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
#endif
  // uint32_t
  value_holder(const uint32_t value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // uint16_t
  value_holder(const uint16_t value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // unsigned char as 8 bit unsigned value
  value_holder(const unsigned char value) noexcept : value_holder(static_cast<uint64_t>(value)) {}
  // double
  value_holder(const double value) noexcept : holder(value) {}
  // float
  value_holder(const float value) noexcept : value_holder(static_cast<double>(value)) {}

  // object from initializer_list
  value_holder(std::initializer_list<field_holder> value) noexcept : holder(value) {}

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
                     std::initializer_list<field_holder>,
                     array_holder>
      holder;
};

}  // namespace builder

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

template <typename CONTAINER>
builder::array_holder array(CONTAINER&& container) {
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
  return array_value;
}

/**
 * \brief helper function to convert initializer_list of values explicitly to
 * ArrayHolder
 */
builder::array_holder array(std::initializer_list<builder::value_holder> list);

/**
 * \brief build json string
 */
std::string build(const builder::value_holder& value);

/**
 * \brief build rapidjson value (array or object)
 */
rapidjson::Value build_value(const builder::value_holder& value, rapidjson::Document::AllocatorType& allocator);

/**
 * \brief build rapidjson document with array or object
 */
rapidjson::Document build_document(const builder::value_holder& value);

/**
 * \brief build json string from rapidjson document
 */
std::string stringify(const rapidjson::Document& document);

}  // namespace json
