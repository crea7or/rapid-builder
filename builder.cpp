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

// recursive function
void RecursiveJsonBuilder(rapidjson::Writer<rapidjson::StringBuffer>& writer, const builder::value_holder& value) {
  switch (value.type) {
    case builder::value_type::string_t:
      writer.String(
          value.primitive.string.data(), static_cast<rapidjson::SizeType>(value.primitive.string.size()), false);
      break;

    case builder::value_type::null_t:
      writer.Null();
      break;

    case builder::value_type::bool_t:
      writer.Bool(value.primitive.bool_flag);
      break;

    case builder::value_type::int_t:
      writer.Int64(value.primitive.int_num);
      break;

    case builder::value_type::uint_t:
      writer.Uint64(value.primitive.uint_num);
      break;

    case builder::value_type::double_t:
      writer.Double(value.primitive.double_num);
      break;

    case builder::value_type::object_t:
      // start writing object recursively
      writer.StartObject();
      for (const auto& field : value.object_holder) {
        RAPIDJSON_ASSERT(nullptr != field.name.data());
        writer.Key(field.name.data(), static_cast<rapidjson::SizeType>(field.name.size()), false);
        RecursiveJsonBuilder(writer, field.value);
      }
      writer.EndObject();
      // end writing object recursively
      break;

    case builder::value_type::array_t:
      // start writing array recursively
      writer.StartArray();
      // detect source of items
      if (builder::array_source::vector_t == value.array_value.source) {
        for (const auto& array_holder : value.array_value.items) {
          RecursiveJsonBuilder(writer, array_holder);
        }
      } else {
        for (const auto& array_holder : value.array_value.list_items) {
          RecursiveJsonBuilder(writer, array_holder);
        }
      }
      writer.EndArray();
      // end writing array recursively
      break;

    default:
      RAPIDJSON_ASSERT(false);
  }
}

// recursive function
void RecursiveValueBuilder(rapidjson::Value& result,
                           rapidjson::Document::AllocatorType& allocator,
                           const builder::value_holder& value) {
  switch (value.type) {
    case builder::value_type::string_t:
      result.SetString(value.primitive.string.data(), static_cast<rapidjson::SizeType>(value.primitive.string.size()));
      break;

    case builder::value_type::null_t:
      result.SetNull();
      break;

    case builder::value_type::bool_t:
      result.SetBool(value.primitive.bool_flag);
      break;

    case builder::value_type::int_t:
      result.SetInt64(value.primitive.int_num);
      break;

    case builder::value_type::uint_t:
      result.SetUint64(value.primitive.uint_num);
      break;

    case builder::value_type::double_t:
      result.SetDouble(value.primitive.double_num);
      break;

    case builder::value_type::object_t:
      // start writing object recursively
      result.SetObject();
      for (const auto& field : value.object_holder) {
        RAPIDJSON_ASSERT(nullptr != field.name.data());
        // create rapid json value from details::value
        rapidjson::Value member_value;
        RecursiveValueBuilder(member_value, allocator, field.value);
        result.AddMember(
            rapidjson::StringRef(field.name.data(), field.name.size()), std::move(member_value), allocator);
      }
      // end writing object recursively
      break;

    case builder::value_type::array_t:
      // start writing array recursively
      result.SetArray();
      // detect source of items
      if (builder::array_source::vector_t == value.array_value.source) {
        for (const auto& array_holder : value.array_value.items) {
          rapidjson::Value member_value;
          RecursiveValueBuilder(member_value, allocator, array_holder);
          result.PushBack(std::move(member_value), allocator);
        }
      } else {
        for (const auto& array_holder : value.array_value.list_items) {
          rapidjson::Value member_value;
          RecursiveValueBuilder(member_value, allocator, array_holder);
          result.PushBack(std::move(member_value), allocator);
        }
      }
      // end writing array recursively
      break;

    default:
      RAPIDJSON_ASSERT(false);
  }
}
}  // namespace

std::string stringify(const rapidjson::Document& document) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);
  return buffer.GetString();
}

builder::array_holder array(std::initializer_list<builder::value_holder> list) {
  return builder::array_holder(list);
}

/**
 * \brief build json string
 */
std::string build(const builder::value_holder& value) {
  RAPIDJSON_ASSERT(builder::value_type::array_t == value.type || builder::value_type::object_t == value.type);
  rapidjson::StringBuffer string_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);
  // recursive builder
  RecursiveJsonBuilder(writer, value);
  // get the json string and return it
  return string_buffer.GetString();
}

/**
 * \brief build rapidjson value (array or object)
 */
rapidjson::Value build_value(const builder::value_holder& value, rapidjson::Document::AllocatorType& allocator) {
  rapidjson::Value result;
  RAPIDJSON_ASSERT(builder::value_type::array_t == value.type || builder::value_type::object_t == value.type);
  // recursive builder
  RecursiveValueBuilder(result, allocator, value);
  return result;
}

/**
 * \brief build rapidjson document with array or object
 */
rapidjson::Document build_document(const builder::value_holder& value) {
  rapidjson::Document result;
  rapidjson::Value rapid_value(build_value(value, result.GetAllocator()));
  rapid_value.Swap(result);
  return result;
}

}  // namespace json
