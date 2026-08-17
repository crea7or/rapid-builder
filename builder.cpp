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

#include "builder.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace json {
namespace {

template <typename Func>
void ForEachArrayValue(const builder::array_holder& holder, Func&& func) {
  if (builder::array_source::vector_t == holder.source) {
    for (const auto& array_value : holder.items) {
      func(array_value);
    }
  } else {
    for (const auto& array_value : holder.list_items) {
      func(array_value);
    }
  }
}

template <typename Func>
void ForEachObjectField(const builder::object_holder& holder, Func&& func) {
  if (builder::object_source::vector_t == holder.source) {
    for (const auto& field : holder.items) {
      RAPIDJSON_ASSERT(nullptr != field.value);
      func(std::string_view(field.name), *field.value);
    }
  } else {
    for (const builder::field_holder& field : holder.list_items) {
      RAPIDJSON_ASSERT(nullptr != field.name.data());
      func(field.name, field.value);
    }
  }
}

void RecursiveJsonBuilder(rapidjson::Writer<rapidjson::StringBuffer>& writer, const builder::value_holder& value) {
  std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
          writer.Null();
        } else if constexpr (std::is_same_v<T, bool>) {
          writer.Bool(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
          writer.Int64(arg);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
          writer.Uint64(arg);
        } else if constexpr (std::is_same_v<T, double>) {
          writer.Double(arg);
        } else if constexpr (std::is_same_v<T, std::string_view>) {
          writer.String(arg.data(), static_cast<rapidjson::SizeType>(arg.size()));
        } else if constexpr (std::is_same_v<T, builder::object_holder>) {
          // start writing object recursively
          writer.StartObject();
          ForEachObjectField(arg, [&](std::string_view name, const builder::value_holder& field_value) {
            writer.Key(name.data(), static_cast<rapidjson::SizeType>(name.size()), false);
            RecursiveJsonBuilder(writer, field_value);
          });
          writer.EndObject();
          // end writing object recursively
        } else if constexpr (std::is_same_v<T, builder::array_holder>) {
          // start writing array recursively
          writer.StartArray();
          ForEachArrayValue(arg, [&](const builder::value_holder& array_value) {
            RecursiveJsonBuilder(writer, array_value);
          });
          writer.EndArray();
          // end writing array recursively
        } else {
          RAPIDJSON_ASSERT(false);
        }
      },
      value.holder);
}

// recursive function
void RecursiveValueBuilder(rapidjson::Value& result,
                           rapidjson::Document::AllocatorType& allocator,
                           const builder::value_holder& value) {
  std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
          result.SetNull();
        } else if constexpr (std::is_same_v<T, bool>) {
          result.SetBool(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
          result.SetInt64(arg);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
          result.SetUint64(arg);
        } else if constexpr (std::is_same_v<T, double>) {
          result.SetDouble(arg);
        } else if constexpr (std::is_same_v<T, std::string_view>) {
          result.SetString(rapidjson::StringRef(arg.data(), static_cast<rapidjson::SizeType>(arg.size())));
        } else if constexpr (std::is_same_v<T, builder::object_holder>) {
          // start writing object recursively
          result.SetObject();
          ForEachObjectField(arg, [&](std::string_view name, const builder::value_holder& field_value) {
            // create rapid json value from details::value
            rapidjson::Value member_value;
            RecursiveValueBuilder(member_value, allocator, field_value);
            result.AddMember(rapidjson::StringRef(name.data(), name.size()), std::move(member_value), allocator);
          });
          // end writing object recursively
        } else if constexpr (std::is_same_v<T, builder::array_holder>) {
          // start writing array recursively
          result.SetArray();
          ForEachArrayValue(arg, [&](const builder::value_holder& array_value) {
            rapidjson::Value member_value;
            RecursiveValueBuilder(member_value, allocator, array_value);
            result.PushBack(std::move(member_value), allocator);
          });
          // end writing array recursively
        } else {
          RAPIDJSON_ASSERT(false);
        }
      },
      value.holder);
}

}  // namespace

std::string BuildInternal(const builder::value_holder& value);
rapidjson::Value BuildValueInternal(const builder::value_holder& value, rapidjson::Document::AllocatorType& allocator);
rapidjson::Document BuildDocumentInternal(const builder::value_holder& value);

std::string stringify(const rapidjson::Document& document) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);
  return std::string(buffer.GetString(), buffer.GetSize());
}

value array(std::initializer_list<builder::value_holder> list) {
  return value(builder::value_holder(builder::array_holder(list)));
}

/**
 * \brief build json string
 */
std::string build(std::initializer_list<builder::field_holder> value) {
  return BuildInternal(builder::value_holder(value));
}

std::string build(std::initializer_list<builder::value_holder> value) {
  return BuildInternal(builder::value_holder(builder::array_holder(value)));
}

std::string build(const value& value) {
  return BuildInternal(value.internal());
}

std::string BuildInternal(const builder::value_holder& value) {
  rapidjson::StringBuffer string_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
  // recursive builder
  RecursiveJsonBuilder(writer, value);
  // get the json string and return it
  return std::string(string_buffer.GetString(), string_buffer.GetSize());
}

/**
 * \brief build rapidjson value (array or object)
 */
rapidjson::Value build_value(std::initializer_list<builder::field_holder> value,
                             rapidjson::Document::AllocatorType& allocator) {
  return BuildValueInternal(builder::value_holder(value), allocator);
}

rapidjson::Value build_value(std::initializer_list<builder::value_holder> value,
                             rapidjson::Document::AllocatorType& allocator) {
  return BuildValueInternal(builder::value_holder(builder::array_holder(value)), allocator);
}

rapidjson::Value build_value(const value& value, rapidjson::Document::AllocatorType& allocator) {
  return BuildValueInternal(value.internal(), allocator);
}

rapidjson::Value BuildValueInternal(const builder::value_holder& value, rapidjson::Document::AllocatorType& allocator) {
  rapidjson::Value result;
  // recursive builder
  RecursiveValueBuilder(result, allocator, value);
  return result;
}

/**
 * \brief build rapidjson document with array or object
 */
rapidjson::Document build_document(std::initializer_list<builder::field_holder> value) {
  return BuildDocumentInternal(builder::value_holder(value));
}

rapidjson::Document build_document(std::initializer_list<builder::value_holder> value) {
  return BuildDocumentInternal(builder::value_holder(builder::array_holder(value)));
}

rapidjson::Document build_document(const value& value) {
  return BuildDocumentInternal(value.internal());
}

rapidjson::Document BuildDocumentInternal(const builder::value_holder& value) {
  rapidjson::Document result;
  RecursiveValueBuilder(result, result.GetAllocator(), value);
  return result;
}

}  // namespace json
