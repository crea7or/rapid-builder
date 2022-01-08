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

#include <benchmark/benchmark.h>

#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "builder.h"

using namespace rapidjson;

static void BM_JsonBuilderCreate(benchmark::State& state) {
  // Perform setup here
  std::string string_field_name("field_name");
  std::string string_field_value("field_value");

  std::vector<int8_t> values{1, 2, 3, 4, 5};

  unsigned char uchar_value = 'F' + 128;
  uint16_t uint16_value = 0xFFFF;
  uint32_t uint32_value = 0xFFFFFFFF;
  uint64_t uint64_value = 0xFFFFFFFFFFFFFFFF;
  char char_value = 'F';
  int16_t int16_value = -32767;
  int32_t int32_value = 0x8FFFFFF0;
  int64_t int64_value = 0x8FFFFFFFFFFFFFF0;
  double double_value = 1.1;
  float float_value = 2.2f;

  for (auto _ : state) {
    // This code gets timed

    const auto json_text = json::build({{string_field_name, "value"},
                                        {"field_name", string_field_value},
                                        {string_field_name, string_field_value},
                                        {"obj", {{"some", "other"}, {"int", 0}}},
                                        {"from vector", json::array(values)},
                                        {"int64_t", int64_value},
                                        {"uint64_t", uint64_value},
                                        {"int32_t", int32_value},
                                        {"uint32_t", uint32_value},
                                        {"int16_t", int16_value},
                                        {"uint16_t", uint16_value},
                                        {"char", char_value},
                                        {"uchar", uchar_value},
                                        {"double", double_value},
                                        {"float", float_value},
                                        {"l", -123l},
                                        {"ul", 123ul},
                                        {"ll", -123ll},
                                        {"ull", 123ull},
                                        {"bool", true}});
    uint64_value += json_text.size();
    // std::cout << json_text << std::endl;
  }
  // std::cout << "test hash: " << uint64_value << std::endl;
  benchmark::DoNotOptimize(uint64_value);
}

static void BM_RapidJsonCreate(benchmark::State& state) {
  // Perform setup here
  std::string string_field_name("field_name");
  std::string string_field_value("field_value");

  std::vector<int8_t> values{1, 2, 3, 4, 5};

  unsigned char uchar_value = 'F' + 128;
  uint16_t uint16_value = 0xFFFF;
  uint32_t uint32_value = 0xFFFFFFFF;
  uint64_t uint64_value = 0xFFFFFFFFFFFFFFFF;
  char char_value = 'F';
  int16_t int16_value = -32767;
  int32_t int32_value = 0x8FFFFFF0;
  int64_t int64_value = 0x8FFFFFFFFFFFFFF0;
  double double_value = 1.1;
  float float_value = 2.2f;

  for (auto _ : state) {
    // This code gets timed
    rapidjson::Document document(rapidjson::kObjectType);
    auto& allocator = document.GetAllocator();

    // {string_field_name, "value"}
    document.AddMember(rapidjson::Value(string_field_name, allocator), rapidjson::Value("value"), allocator);

    // {"field_name", string_field_value}
    document.AddMember("field_name", rapidjson::Value(string_field_value, allocator), allocator);

    // {string_field_name, string_field_value}
    document.AddMember(
        rapidjson::Value(string_field_name, allocator), rapidjson::Value(string_field_value, allocator), allocator);

    // {"obj", {{"some", "other"}, {"int", 0}}}
    rapidjson::Value obj(rapidjson::kObjectType);
    obj.AddMember("some", "other", allocator);
    obj.AddMember("int", 0, allocator);
    document.AddMember("obj", obj, allocator);

    // {"from vector", json::Array(values)}
    rapidjson::Value arr(rapidjson::kArrayType);
    for (const auto& value : values) {
      arr.PushBack(value, allocator);
    }
    document.AddMember("from vector", arr, allocator);

    // {"int64_t", int64_value}
    document.AddMember("int64_t", int64_value, allocator);
    // {"uint64_t", uint64_value}
    document.AddMember("uint64_t", uint64_value, allocator);
    // {"int32_t", int32_value}
    document.AddMember("int32_t", int32_value, allocator);
    // {"uint32_t", uint32_value}
    document.AddMember("uint32_t", uint32_value, allocator);
    // {"int16_t", int16_value}
    document.AddMember("int16_t", int16_value, allocator);
    // {"uint16_t", uint16_value}
    document.AddMember("uint16_t", uint16_value, allocator);
    // {"char", char_value}
    document.AddMember("char", char_value, allocator);
    // {"uchar", uchar_value}
    document.AddMember("uchar", uchar_value, allocator);
    // {"double", double_value}
    document.AddMember("double", double_value, allocator);
    // {"float", float_value}
    document.AddMember("float", float_value, allocator);
    // {"l", -123l}
    document.AddMember("l", static_cast<int32_t>(-123l), allocator);
    // {"ul", 123ul}
    document.AddMember("ul", static_cast<uint32_t>(123ul), allocator);
    // {"ll", -123ll}
    document.AddMember("ll", static_cast<int64_t>(-123ll), allocator);
    // {"ull", 123ull}
    document.AddMember("ull", static_cast<uint64_t>(123ull), allocator);
    // {"bool", true}
    document.AddMember("bool", true, allocator);

    const auto json_text = json::stringify(document);
    uint64_value += json_text.size();
    // std::cout << json_text << std::endl;
  }
  benchmark::DoNotOptimize(uint64_value);
  // std::cout << "test hash: " << uint64_value << std::endl;
}

// Register the function as a benchmark
BENCHMARK(BM_JsonBuilderCreate);

BENCHMARK(BM_RapidJsonCreate);

// Run the benchmark
BENCHMARK_MAIN();