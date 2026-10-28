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

#include <cmath>

// builder checks throw regardless of the RAPIDJSON_ASSERT configuration
#define JSON_BUILDER_CHECK(x) ((x) ? static_cast<void>(0) : throw std::runtime_error("Failed: " #x))

namespace json {
namespace {

using holder_t = builder::value_holder::holder_t;
using string_writer = rapidjson::Writer<rapidjson::StringBuffer>;

// index of the type T in the variant V, to switch over the variant: MSVC doesn't inline std::visit
template <typename T, typename V>
struct variant_index;

template <typename T, typename... Ts>
struct variant_index<T, std::variant<Ts...>> {
  static constexpr size_t value = [] {
    size_t index = 0;
    static_cast<void>(((std::is_same_v<T, Ts> ? true : (++index, false)) || ...));
    return index;
  }();
};

template <typename T>
inline constexpr size_t index_of = variant_index<T, holder_t>::value;

// the variant index is already checked by the switch
template <typename T>
const T& get(const holder_t& holder) noexcept {
  return *std::get_if<T>(&holder);
}

// initial capacity of the document stack used to collect values before they are moved into the document
constexpr size_t kDocumentStackCapacity = 1024;
// thread buffer is released after building a json longer than this, to not keep memory of rare huge jsons
constexpr size_t kMaxRetainedBufferSize = 1024 * 1024;

template <typename Func>
void ForEachObjectField(const builder::object_holder& holder, Func&& func) {
  if (holder.items.empty()) {
    for (const builder::field_holder& field : holder.list_items) {
      JSON_BUILDER_CHECK(nullptr != field.name.data());
      func(field.name, field.value);
    }
  } else {
    for (const auto& field : holder.items) {
      func(field.name(), field.value);
    }
  }
}

// sends SAX events of the value to the handler: rapidjson::Writer or rapidjson::Document
template <typename Handler>
void RecursiveBuilder(Handler& handler, const builder::value_holder& value) {
  const holder_t& holder = value.holder;
  switch (holder.index()) {
    case index_of<std::nullptr_t>:
      handler.Null();
      break;
    case index_of<bool>:
      handler.Bool(get<bool>(holder));
      break;
    case index_of<int64_t>:
      handler.Int64(get<int64_t>(holder));
      break;
    case index_of<uint64_t>:
      handler.Uint64(get<uint64_t>(holder));
      break;
    case index_of<double>: {
      // NaN and Inf are not valid json numbers, rapidjson::Writer would produce a broken json
      const double number = get<double>(holder);
      if (std::isfinite(number)) {
        handler.Double(number);
      } else {
        handler.Null();
      }
      break;
    }
    case index_of<std::string_view>: {
      const std::string_view text = get<std::string_view>(holder);
      handler.String(text.data(), static_cast<rapidjson::SizeType>(text.size()), false);
      break;
    }
    case index_of<std::string>: {
      const std::string& text = get<std::string>(holder);
      handler.String(text.data(), static_cast<rapidjson::SizeType>(text.size()), false);
      break;
    }
    case index_of<builder::object_holder>: {
      const auto& object = get<builder::object_holder>(holder);
      handler.StartObject();
      ForEachObjectField(object, [&](std::string_view name, const builder::value_holder& field_value) {
        handler.Key(name.data(), static_cast<rapidjson::SizeType>(name.size()), false);
        RecursiveBuilder(handler, field_value);
      });
      handler.EndObject(static_cast<rapidjson::SizeType>(object.size()));
      break;
    }
    case index_of<builder::array_holder>: {
      const auto& array = get<builder::array_holder>(holder);
      handler.StartArray();
      for (const builder::value_holder& array_value : array) {
        RecursiveBuilder(handler, array_value);
      }
      handler.EndArray(static_cast<rapidjson::SizeType>(array.size()));
      break;
    }
    case index_of<const builder::value_holder*>:
      RecursiveBuilder(handler, *get<const builder::value_holder*>(holder));
      break;
    default:
      JSON_BUILDER_CHECK(false);
  }
}

// copies initializer_list items, they die at the end of the full expression
builder::value_holder MakeOwned(const builder::value_holder& value) {
  if (const auto* array = std::get_if<builder::array_holder>(&value.holder)) {
    std::vector<builder::value_holder> items;
    items.reserve(array->size());
    for (const builder::value_holder& item : *array) {
      items.push_back(MakeOwned(item));
    }
    return builder::value_holder(builder::array_holder(std::move(items)));
  }
  if (const auto* object = std::get_if<builder::object_holder>(&value.holder)) {
    std::vector<builder::owned_field_holder> fields;
    fields.reserve(object->size());
    if (object->items.empty()) {
      for (const builder::field_holder& field : object->list_items) {
        JSON_BUILDER_CHECK(nullptr != field.name.data());
        fields.push_back(builder::owned_field_holder{field.name, {}, MakeOwned(field.value)});
      }
    } else {
      for (const auto& field : object->items) {
        fields.push_back(builder::owned_field_holder{field.borrowed_name, field.owned_name, MakeOwned(field.value)});
      }
    }
    return builder::value_holder(builder::object_holder(std::move(fields)));
  }
  return value;
}

// per thread writer and buffer keep their memory between the calls
struct thread_writer final {
  rapidjson::StringBuffer buffer;
  string_writer writer{buffer};
};

template <typename Generator>
void WriteJson(std::string& out, Generator&& generator) {
  thread_local thread_writer context;
  context.buffer.Clear();
  context.writer.Reset(context.buffer);
  generator(context.writer);
  out.assign(context.buffer.GetString(), context.buffer.GetSize());
  if (context.buffer.GetSize() > kMaxRetainedBufferSize) {
    context.buffer.Clear();
    context.buffer.ShrinkToFit();
  }
}

// stateless allocator for the document stacks, so documents don't allocate own one
rapidjson::CrtAllocator& DocumentStackAllocator() {
  static rapidjson::CrtAllocator allocator;
  return allocator;
}

// the document collects values on its stack and allocates arrays and objects of the exact size
void PopulateDocument(rapidjson::Document& document, const builder::value_holder& value) {
  auto generator = [&value](rapidjson::Document& handler) {
    RecursiveBuilder(handler, value);
    return true;
  };
  document.Populate(generator);
}

void BuildInternal(const builder::value_holder& value, std::string& out) {
  WriteJson(out, [&value](string_writer& writer) { RecursiveBuilder(writer, value); });
}

rapidjson::Value BuildValueInternal(const builder::value_holder& value, rapidjson::Document::AllocatorType& allocator) {
  rapidjson::Document document(&allocator, kDocumentStackCapacity, &DocumentStackAllocator());
  PopulateDocument(document, value);
  rapidjson::Value result;
  result.Swap(document);
  return result;
}

rapidjson::Document BuildDocumentInternal(const builder::value_holder& value) {
  rapidjson::Document result(nullptr, kDocumentStackCapacity, &DocumentStackAllocator());
  PopulateDocument(result, value);
  return result;
}

}  // namespace

std::string stringify(const rapidjson::Document& document) {
  std::string result;
  WriteJson(result, [&document](string_writer& writer) { document.Accept(writer); });
  return result;
}

value array(std::initializer_list<builder::value_holder> list) {
  return value(MakeOwned(builder::value_holder(builder::array_holder(list))));
}

/**
 * \brief build json string
 */
std::string build(std::initializer_list<builder::field_holder> value) {
  std::string result;
  build(value, result);
  return result;
}

std::string build(std::initializer_list<builder::value_holder> value) {
  std::string result;
  build(value, result);
  return result;
}

std::string build(const value& value) {
  std::string result;
  build(value, result);
  return result;
}

void build(std::initializer_list<builder::field_holder> value, std::string& out) {
  BuildInternal(builder::value_holder(value), out);
}

void build(std::initializer_list<builder::value_holder> value, std::string& out) {
  BuildInternal(builder::value_holder(builder::array_holder(value)), out);
}

void build(const value& value, std::string& out) {
  BuildInternal(value.internal(), out);
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
  rapidjson::Document result(nullptr, kDocumentStackCapacity, &DocumentStackAllocator());
  build_document(value, result);
  return result;
}

void build_document(std::initializer_list<builder::field_holder> value, rapidjson::Document& document) {
  PopulateDocument(document, builder::value_holder(value));
}

void build_document(std::initializer_list<builder::value_holder> value, rapidjson::Document& document) {
  PopulateDocument(document, builder::value_holder(builder::array_holder(value)));
}

void build_document(const value& value, rapidjson::Document& document) {
  PopulateDocument(document, value.internal());
}

}  // namespace json
