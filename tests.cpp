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

#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

#include <array>
#include <atomic>
#include <cmath>
#include <forward_list>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <string_view>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "builder.h"

namespace {

void ExpectRuntimeErrorMessage(const std::function<void()>& action, std::string_view expected_message) {
  try {
    action();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& ex) {
    EXPECT_EQ(std::string(ex.what()), expected_message);
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(BasicTests, CreateJSONviadifferentAPIcalls) {
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

  const auto rapid_json_object = json::build_document({{string_field_name, "value"},
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

  const auto json_from_document = json::stringify(rapid_json_object);

  const auto etalon =
      R"%({"field_name":"value","field_name":"field_value","field_name":"field_value","obj":{"some":"other","int":0},"from vector":[1,2,3,4,5],"int64_t":-8070450532247928848,"uint64_t":18446744073709551615,"int32_t":-1879048208,"uint32_t":4294967295,"int16_t":-32767,"uint16_t":65535,"char":70,"uchar":198,"double":1.1,"float":2.200000047683716,"l":-123,"ul":123,"ll":-123,"ull":123,"bool":true})%";

  EXPECT_EQ(etalon, json_text);
  EXPECT_EQ(etalon, json_from_document);
}


TEST(BasicTests, CreateObjects) {
  std::string string_field_name("string");
  std::string string_field_value("value");
  const auto json = json::build({{string_field_name, "value"},
                                 {"string", string_field_value},
                                 {string_field_name, string_field_value},
                                 {"obj", {{"string", "name"}, {"int", 0}}},
                                 {"uint64_t", (uint64_t)(123)},
                                 {"int64_t", (int64_t)(-123)},
                                 {"uint32_t", (uint32_t)(123)},
                                 {"int32_t", (int32_t)(-123)},
                                 {"uint16_t", (uint16_t)(123)},
                                 {"int16_t", (int16_t)(-123)},
                                 {"uchar", (unsigned char)('{')},
                                 {"char", '{'},
                                 {"bool", true},
                                 {"float", 1.0f},
                                 {"double", 0.123}});

  const auto rapid_json = json::build_document({{string_field_name, "value"},
                                                {"string", string_field_value},
                                                {string_field_name, string_field_value},
                                                {"obj", {{"string", "name"}, {"int", 0}}},
                                                {"uint64_t", (uint64_t)(123)},
                                                {"int64_t", (int64_t)(-123)},
                                                {"uint32_t", (uint32_t)(123)},
                                                {"int32_t", (int32_t)(-123)},
                                                {"uint16_t", (uint16_t)(123)},
                                                {"int16_t", (int16_t)(-123)},
                                                {"uchar", (unsigned char)('{')},
                                                {"char", '{'},
                                                {"bool", true},
                                                {"float", 1.0f},
                                                {"double", 0.123}});

  // expected value
  const std::string test(
      R"%({"string":"value","string":"value","string":"value","obj":{"string":"name","int":0},"uint64_t":123,"int64_t":-123,"uint32_t":123,"int32_t":-123,"uint16_t":123,"int16_t":-123,"uchar":123,"char":123,"bool":true,"float":1.0,"double":0.123})%");
  EXPECT_EQ(json, test);
  EXPECT_EQ(json::stringify(rapid_json), test);
}

TEST(BasicTests, CreateArraysWithNull) {
  // object with array inside
  {
    const auto json = json::build({{"name", "value"}, {"array", {{0, 1, 2, 3, 4, nullptr}}}});
    const auto rapid_json = json::build_document({{"name", "value"}, {"array", {{0, 1, 2, 3, 4, nullptr}}}});
    const std::string test(R"%({"name":"value","array":[0,1,2,3,4,null]})%");
    EXPECT_EQ(json, test);
    EXPECT_EQ(json::stringify(rapid_json), test);
  }

  // object with array inside array
  {
    const auto json = json::build({{"name", "value"}, {"array", {{0, 1, json::array({2, 3, 4}), 5, 6, nullptr}}}});
    const auto rapid_json =
        json::build_document({{"name", "value"}, {"array", {{0, 1, json::array({2, 3, 4}), 5, 6, nullptr}}}});
    const std::string test(R"%({"name":"value","array":[0,1,[2,3,4],5,6,null]})%");
    EXPECT_EQ(json, test);
    EXPECT_EQ(json::stringify(rapid_json), test);
  }
}

TEST(BasicTests, CreateArraysFromContainers) {
  // object with array from list
  {
    std::list<std::string> list_value{"a", "b", "c"};
    const auto json_array_list = json::build({{"name", "value"}, {"list", json::array(list_value)}});
    const auto rapid_json_array_list = json::build_document({{"name", "value"}, {"list", json::array(list_value)}});
    // expected value
    const std::string test(R"%({"name":"value","list":["a","b","c"]})%");
    EXPECT_EQ(json_array_list, test);
    EXPECT_EQ(json::stringify(rapid_json_array_list), test);
  }

  // object with array from vector
  {
    std::vector<std::string> vector_value{"1", "2", "3"};
    const auto json_array_vector = json::build({{"name", "value"}, {"vector", json::array(vector_value)}});
    const auto rapid_json_array_vector =
        json::build_document({{"name", "value"}, {"vector", json::array(vector_value)}});
    // expected value
    const std::string test(R"%({"name":"value","vector":["1","2","3"]})%");
    EXPECT_EQ(json_array_vector, test);
    EXPECT_EQ(json::stringify(rapid_json_array_vector), test);
  }

  // object with array from set
  {
    std::set<uint32_t> set_value{1, 2, 3, 4, 56};
    const auto json_array_set = json::build({{"name", "value"}, {"set", json::array(set_value)}});
    const auto rapid_json_array_set = json::build_document({{"name", "value"}, {"set", json::array(set_value)}});
    // expected value
    const std::string test(R"%({"name":"value","set":[1,2,3,4,56]})%");
    EXPECT_EQ(json_array_set, test);
    EXPECT_EQ(json::stringify(rapid_json_array_set), test);
  }
}

TEST(BasicTests, CreateObjectFromMap) {
  std::map<std::string, int32_t> map_value{{"b", 2}, {"a", 1}, {"c", 3}};
  const json::value map_source(json::object(map_value));
  const json::value source(json::object(std::vector<std::pair<std::string, json::value>>{{"name", "value"}, {"map", map_source}}));

  const auto json_text = json::build({{"name", "value"}, {"map", json::object(map_value)}});
  const auto rapid_json_object = json::build_document(source);

  const std::string test(R"%({"name":"value","map":{"a":1,"b":2,"c":3}})%");
  EXPECT_EQ(json_text, test);
  EXPECT_EQ(json::stringify(rapid_json_object), test);
}

TEST(BasicTests, CreateObjectFromUnorderedMap) {
  std::unordered_map<std::string, int32_t> map_value{{"b", 2}, {"a", 1}, {"c", 3}};
  const json::value map_source(json::object(map_value));
  const json::value source(
      json::object(std::vector<std::pair<std::string, json::value>>{{"name", "value"}, {"unordered_map", map_source}}));

  const auto json_text = json::build(source);
  const auto rapid_json_object = json::build_document(source);

  rapidjson::Document parsed_json_text;
  parsed_json_text.Parse(json_text.c_str());
  ASSERT_FALSE(parsed_json_text.HasParseError());
  ASSERT_TRUE(parsed_json_text.IsObject());
  ASSERT_TRUE(parsed_json_text.HasMember("name"));
  EXPECT_STREQ(parsed_json_text["name"].GetString(), "value");
  ASSERT_TRUE(parsed_json_text.HasMember("unordered_map"));
  ASSERT_TRUE(parsed_json_text["unordered_map"].IsObject());
  EXPECT_EQ(parsed_json_text["unordered_map"].MemberCount(), 3u);
  EXPECT_EQ(parsed_json_text["unordered_map"]["a"].GetInt(), 1);
  EXPECT_EQ(parsed_json_text["unordered_map"]["b"].GetInt(), 2);
  EXPECT_EQ(parsed_json_text["unordered_map"]["c"].GetInt(), 3);

  ASSERT_TRUE(rapid_json_object.IsObject());
  ASSERT_TRUE(rapid_json_object.HasMember("name"));
  EXPECT_STREQ(rapid_json_object["name"].GetString(), "value");
  ASSERT_TRUE(rapid_json_object.HasMember("unordered_map"));
  ASSERT_TRUE(rapid_json_object["unordered_map"].IsObject());
  EXPECT_EQ(rapid_json_object["unordered_map"].MemberCount(), 3u);
  EXPECT_EQ(rapid_json_object["unordered_map"]["a"].GetInt(), 1);
  EXPECT_EQ(rapid_json_object["unordered_map"]["b"].GetInt(), 2);
  EXPECT_EQ(rapid_json_object["unordered_map"]["c"].GetInt(), 3);
}

TEST(BasicTests, CreateObjectFromRvalueVectorPreservesOrderAndDuplicates) {
  std::vector<std::pair<std::string, int32_t>> fields;
  fields.emplace_back("first", 1);
  fields.emplace_back("duplicate", 2);
  fields.emplace_back("duplicate", 3);
  fields.emplace_back("last", 4);
  const json::value source{
      json::object(std::vector<std::pair<std::string, int32_t>>{{"first", 1}, {"duplicate", 2}, {"duplicate", 3}, {"last", 4}})};

  const auto json_text = json::build(json::object(std::move(fields)));
  const auto rapid_json_object = json::build_document(source);

  const std::string test(R"%({"first":1,"duplicate":2,"duplicate":3,"last":4})%");
  EXPECT_EQ(json_text, test);
  EXPECT_EQ(json::stringify(rapid_json_object), test);
}

TEST(BasicTests, CreateObjectAndArrayFromStringViews) {
  constexpr std::string_view object_name = "sv-object";
  constexpr std::string_view object_value = "sv-value";
  constexpr std::string_view array_value = "sv-array";
  const json::value object_source(
      json::object(std::vector<std::pair<std::string_view, std::string_view>>{{object_name, object_value}}));
  const json::value array_source(json::array(std::vector<std::string_view>{array_value}));
  const json::value source(json::object(std::vector<std::pair<std::string, json::value>>{
      {"name", object_name}, {"object", object_source}, {"array", array_source}}));

  const auto json_text = json::build(
      {{"name", object_name}, {"object", json::object(std::vector<std::pair<std::string_view, std::string_view>>{{object_name, object_value}})},
       {"array", json::array(std::vector<std::string_view>{array_value})}});

  const auto rapid_json_object = json::build_document(source);

  const std::string test(R"%({"name":"sv-object","object":{"sv-object":"sv-value"},"array":["sv-array"]})%");
  EXPECT_EQ(json_text, test);
  EXPECT_EQ(json::stringify(rapid_json_object), test);
}

TEST(BasicTests, CreateArrays) {
  {
    const auto json = json::build(json::array({"name", "value", "int64_t", -123000000000, false, -0.123123123, nullptr, 0}));
    const auto rapid_json =
        json::build_document(json::array({"name", "value", "int64_t", -123000000000, false, -0.123123123, nullptr, 0}));
    // expected value
    const std::string test(R"%(["name","value","int64_t",-123000000000,false,-0.123123123,null,0])%");
    EXPECT_EQ(json, test);
    EXPECT_EQ(json::stringify(rapid_json), test);
  }

  // array with array inside array
  {
    const auto json = json::build(json::array({"name", {{0, 1, json::array({2, 3, 4}), 5, 6, nullptr}}}));
    const auto rapid_json =
        json::build_document(json::array({"name", {{0, 1, json::array({2, 3, 4}), 5, 6, nullptr}}}));
    // expected value
    const std::string test(R"%(["name",[0,1,[2,3,4],5,6,null]])%");
    EXPECT_EQ(json, test);
    EXPECT_EQ(json::stringify(rapid_json), test);
  }

  // empty array
  {
    const auto jsonEmptyArray = json::build(json::array({}));
    const auto rapidJsonEmptyArray = json::build_document(json::array({}));
    // expected value
    EXPECT_EQ(jsonEmptyArray, "[]");
    EXPECT_EQ(json::stringify(rapidJsonEmptyArray), "[]");
    const auto jsonEmptyObject = json::build(json::object(std::vector<std::pair<std::string, json::value>>{}));
    const auto rapidJsonEmptyObject =
        json::build_document(json::object(std::vector<std::pair<std::string, json::value>>{}));
    // expected value
    EXPECT_EQ(jsonEmptyObject, "{}");
    EXPECT_EQ(json::stringify(rapidJsonEmptyObject), "{}");
  }

  // please, don't do this
  {
    const auto jsonEmptyArrayWithObject = json::build(json::array({{{}}}));
    const auto rapidJsonEmptyArrayWithObject = json::build_document(json::array({{{}}}));
    // expected value
    EXPECT_EQ(jsonEmptyArrayWithObject, "[{}]");
    EXPECT_EQ(json::stringify(rapidJsonEmptyArrayWithObject), "[{}]");
    // treated as array
    const auto jsonEmptyObjectWithArray = json::build({{json::array({})}});
    const auto rapidJsonEmptyObjectWithArray = json::build_document({{json::array({})}});
    EXPECT_EQ(jsonEmptyObjectWithArray, "[[]]");
    EXPECT_EQ(json::stringify(rapidJsonEmptyObjectWithArray), "[[]]");
  }

  // object with array and object inside array
  {
    const auto json = json::build({{"name", "value"},
                                   {"int64_t", -123000000000},
                                   {"array", {{false, -0.123123123, nullptr, {{"name", "value"}, {"int", 0}}}}}});
    const auto rapid_json =
        json::build_document({{"name", "value"},
                              {"int64_t", -123000000000},
                              {"array", {{false, -0.123123123, nullptr, {{"name", "value"}, {"int", 0}}}}}});
    // expected value
    const std::string test(
        R"%({"name":"value","int64_t":-123000000000,"array":[false,-0.123123123,null,{"name":"value","int":0}]})%");
    EXPECT_EQ(json, test);
    EXPECT_EQ(json::stringify(rapid_json), test);
  }
}

TEST(BasicTests, CreateNotValidObjectWithNull) {
  EXPECT_THROW(json::build({{nullptr, -123000000000}, {"nullptr", nullptr}}), std::runtime_error);
  EXPECT_THROW(json::build_document({{nullptr, -123000000000}, {"nullptr", nullptr}}), std::runtime_error);
  const std::string test("null");
  const std::string null_json = json::build(json::value(nullptr));
  EXPECT_EQ(null_json, test);
  const auto null_document = json::build_document(json::value(nullptr));
  const std::string stringified = json::stringify(null_document);
  EXPECT_EQ(stringified, test);
}

TEST(BasicTests, NullptrParamsAreSerializedAsJsonNull) {
  rapidjson::Document document;
  const json::value source(json::object(std::vector<std::pair<std::string, json::value>>{
      {"top_level_param", nullptr},
      {"nested", json::object(std::vector<std::pair<std::string, json::value>>{{"inner_param", nullptr}})},
      {"array_params", json::array(std::vector<json::value>{1, nullptr, "value"})}}));

  const auto json_text = json::build(source);
  const auto rapid_json_object = json::build_document(source);
  const auto rapid_json_value = json::build_value(source, document.GetAllocator());

  const std::string expected(
      R"%({"top_level_param":null,"nested":{"inner_param":null},"array_params":[1,null,"value"]})%");

  EXPECT_EQ(json_text, expected);
  EXPECT_EQ(json::stringify(rapid_json_object), expected);

  ASSERT_TRUE(rapid_json_value.IsObject());
  ASSERT_TRUE(rapid_json_value.HasMember("top_level_param"));
  EXPECT_TRUE(rapid_json_value["top_level_param"].IsNull());
  ASSERT_TRUE(rapid_json_value.HasMember("nested"));
  ASSERT_TRUE(rapid_json_value["nested"].IsObject());
  ASSERT_TRUE(rapid_json_value["nested"].HasMember("inner_param"));
  EXPECT_TRUE(rapid_json_value["nested"]["inner_param"].IsNull());
  ASSERT_TRUE(rapid_json_value.HasMember("array_params"));
  ASSERT_TRUE(rapid_json_value["array_params"].IsArray());
  ASSERT_EQ(rapid_json_value["array_params"].Size(), 3u);
  EXPECT_EQ(rapid_json_value["array_params"][0].GetInt(), 1);
  EXPECT_TRUE(rapid_json_value["array_params"][1].IsNull());
  EXPECT_STREQ(rapid_json_value["array_params"][2].GetString(), "value");
}

TEST(BasicTests, InvalidObjectFieldNameReportsDetailedError) {
  rapidjson::Document document;
  constexpr std::string_view expected_message = "Failed: nullptr != field.name.data()";

  ExpectRuntimeErrorMessage(
      []() { static_cast<void>(json::build({{nullptr, -123000000000}, {"nullptr", nullptr}})); }, expected_message);
  ExpectRuntimeErrorMessage(
      []() { static_cast<void>(json::build_document({{nullptr, -123000000000}, {"nullptr", nullptr}})); },
      expected_message);
  ExpectRuntimeErrorMessage(
      [&document]() {
        static_cast<void>(json::build_value({{nullptr, -123000000000}, {"nullptr", nullptr}}, document.GetAllocator()));
      },
      expected_message);
}

TEST(BasicTests, InvalidNestedObjectFieldNameReportsDetailedError) {
  rapidjson::Document document;
  constexpr std::string_view expected_message = "Failed: nullptr != field.name.data()";

  ExpectRuntimeErrorMessage(
      []() { static_cast<void>(json::build({{"wrapper", {{nullptr, 1}}}})); }, expected_message);
  ExpectRuntimeErrorMessage(
      []() { static_cast<void>(json::build_document({{"wrapper", {{nullptr, 1}}}})); }, expected_message);
  ExpectRuntimeErrorMessage(
      [&document]() { static_cast<void>(json::build_value({{"wrapper", {{nullptr, 1}}}}, document.GetAllocator())); },
      expected_message);
}

TEST(BasicTests, BuildValueCreatesNestedRapidJsonValue) {
  rapidjson::Document document;
  const json::value items_source(json::array(std::vector<json::value>{1, std::string_view("two"), nullptr}));
  const json::value nested_source(
      json::object(std::vector<std::pair<std::string, json::value>>{{"flag", true}, {"items", items_source}}));
  const json::value source(
      json::object(std::vector<std::pair<std::string, json::value>>{{"nested", nested_source}, {"number", 7}}));
  auto value = json::build_value(source, document.GetAllocator());

  ASSERT_TRUE(value.IsObject());
  ASSERT_TRUE(value.HasMember("nested"));
  ASSERT_TRUE(value["nested"].IsObject());
  ASSERT_TRUE(value["nested"]["flag"].IsBool());
  EXPECT_TRUE(value["nested"]["flag"].GetBool());
  ASSERT_TRUE(value["nested"]["items"].IsArray());
  ASSERT_EQ(value["nested"]["items"].Size(), 3u);
  EXPECT_EQ(value["nested"]["items"][0].GetInt(), 1);
  EXPECT_STREQ(value["nested"]["items"][1].GetString(), "two");
  EXPECT_TRUE(value["nested"]["items"][2].IsNull());
  ASSERT_TRUE(value.HasMember("number"));
  EXPECT_EQ(value["number"].GetInt(), 7);
}

TEST(BasicTests, BuildDocumentRequiresSourceLifetimeForContainerBackedObjects) {
  std::map<std::string, int32_t> map_value{{"b", 2}, {"a", 1}, {"c", 3}};
  const json::value map_source(json::object(map_value));
  const json::value source(json::object(std::vector<std::pair<std::string, json::value>>{{"name", "value"}, {"map", map_source}}));

  const auto rapid_json_object = json::build_document(source);

  EXPECT_EQ(json::stringify(rapid_json_object), R"%({"name":"value","map":{"a":1,"b":2,"c":3}})%");
}

// overwrites the stack area used by the previous full expression temporaries
void ClobberStack() {
  volatile char buffer[4096];
  for (auto& item : buffer) {
    item = static_cast<char>(0xCD);
  }
}

TEST(BasicTests, PersistedArrayFromInitializerListOwnsItems) {
  const json::value source = json::array({1, "two", {{"name", "value"}}, {{3, 4}}, nullptr});
  const json::value nested = json::array(
      {json::object(std::map<std::string, int32_t>{{"name longer than the small string buffer", 1}}), false});
  ClobberStack();

  const std::string test(R"%([1,"two",{"name":"value"},[3,4],null])%");
  EXPECT_EQ(json::build(source), test);
  EXPECT_EQ(json::stringify(json::build_document(source)), test);
  EXPECT_EQ(json::build(nested), R"%([{"name longer than the small string buffer":1},false])%");
}

TEST(BasicTests, NanAndInfAreSerializedAsJsonNull) {
  const auto json_text = json::build({{"nan", std::nan("")},
                                      {"inf", std::numeric_limits<double>::infinity()},
                                      {"float", -std::numeric_limits<float>::infinity()},
                                      {"number", 1.5}});
  const auto rapid_json_object = json::build_document({{"nan", std::nan("")}, {"number", 1.5}});

  EXPECT_EQ(json_text, R"%({"nan":null,"inf":null,"float":null,"number":1.5})%");
  EXPECT_EQ(json::stringify(rapid_json_object), R"%({"nan":null,"number":1.5})%");
}

TEST(BasicTests, ValueIsAssignableAndMovable) {
  static_assert(std::is_copy_assignable_v<json::value>);
  static_assert(std::is_nothrow_move_constructible_v<json::value>);
  static_assert(std::is_nothrow_move_assignable_v<json::value>);
  // json::value keeps a view of the string, a temporary string would dangle
  static_assert(!std::is_constructible_v<json::value, std::string&&>);

  const std::vector<int32_t> values{1, 2, 3};
  json::value source(nullptr);
  source = json::array(values);
  json::value copy(source);
  json::value moved(std::move(source));
  source = copy;

  EXPECT_EQ(json::build(copy), "[1,2,3]");
  EXPECT_EQ(json::build(moved), "[1,2,3]");
  EXPECT_EQ(json::build(source), "[1,2,3]");
}

TEST(BasicTests, NullCharPointer) {
  const char* null_value = nullptr;
  EXPECT_EQ(json::build({{"value", null_value}}), R"%({"value":null})%");
  EXPECT_EQ(json::build(json::value(null_value)), "null");

  ExpectRuntimeErrorMessage([null_value]() { static_cast<void>(json::build({{null_value, 1}})); },
                            "Failed: nullptr != field.name.data()");
}

TEST(BasicTests, IntegerTypes) {
  enum unscoped_enum { unscoped_value = 7 };
  const json::value source = json::array(std::vector<json::value>{static_cast<signed char>(-1),
                                                                  static_cast<short>(-2),
                                                                  -3l,
                                                                  -4ll,
                                                                  5ul,
                                                                  6ull,
                                                                  unscoped_value,
                                                                  std::numeric_limits<int64_t>::min(),
                                                                  std::numeric_limits<uint64_t>::max()});

  EXPECT_EQ(json::build(source), "[-1,-2,-3,-4,5,6,7,-9223372036854775808,18446744073709551615]");
}

TEST(BasicTests, ValuesFromLvalueContainerAreBorrowed) {
  std::vector<json::value> values{1, "two"};
  const json::value source = json::array(values);
  values[0] = 3;

  EXPECT_EQ(json::build(source), R"%([3,"two"])%");
  EXPECT_EQ(json::build({{"source", source}, {"values", values[1]}}), R"%({"source":[3,"two"],"values":"two"})%");
}

TEST(BasicTests, BuildIntoExistingStringAndDocument) {
  std::string out;
  out.reserve(256);
  const char* const data = out.data();
  json::build({{"name", "value"}}, out);
  EXPECT_EQ(out, R"%({"name":"value"})%");
  json::build(json::array({1, 2}), out);
  EXPECT_EQ(out, "[1,2]");
  EXPECT_EQ(out.data(), data);

  char buffer[1024];
  rapidjson::MemoryPoolAllocator<> allocator(buffer, sizeof(buffer));
  rapidjson::Document document(&allocator);
  const size_t capacity = allocator.Capacity();
  json::build_document({{"name", "value"}, {"array", {{1, 2, 3}}}}, document);
  EXPECT_EQ(json::stringify(document), R"%({"name":"value","array":[1,2,3]})%");
  // no chunk allocated beyond the user buffer
  EXPECT_EQ(allocator.Capacity(), capacity);
}

TEST(BasicTests, DocumentContainersHaveExactCapacity) {
  const std::vector<int32_t> values{1, 2, 3};
  const auto document = json::build_document({{"list", {{1, 2}}}, {"vector", json::array(values)}});

  EXPECT_EQ(document["list"].Capacity(), 2u);
  EXPECT_EQ(document["vector"].Capacity(), 3u);
}

TEST(BasicTests, ValueAssignmentOfSameKind) {
  const std::map<std::string, int32_t> first_map{{"a", 1}};
  const std::map<std::string, int32_t> second_map{{"b", 2}};
  const std::vector<int32_t> first_vector{1};
  const std::vector<int32_t> second_vector{2, 3};

  json::value object_value = json::object(first_map);
  const json::value other_object = json::object(second_map);
  object_value = other_object;
  EXPECT_EQ(json::build(object_value), R"%({"b":2})%");
  object_value = json::object(first_map);
  EXPECT_EQ(json::build(object_value), R"%({"a":1})%");

  json::value array_value = json::array(first_vector);
  const json::value other_array = json::array(second_vector);
  array_value = other_array;
  EXPECT_EQ(json::build(array_value), "[2,3]");
  array_value = json::array(first_vector);
  EXPECT_EQ(json::build(array_value), "[1]");

  // self assignment keeps the value
  json::value& same_array = array_value;
  array_value = same_array;
  array_value = std::move(same_array);
  EXPECT_EQ(json::build(array_value), "[1]");
}

TEST(BasicTests, ArrayHolderOverInitializerList) {
  // the backing array of a named initializer_list lives until the end of the scope
  const std::initializer_list<json::builder::value_holder> items{1, 2};
  const json::builder::array_holder list(items);

  const json::builder::array_holder copy(list);
  EXPECT_EQ(copy.begin(), items.begin());
  EXPECT_EQ(copy.size(), 2u);

  json::builder::array_holder assigned(std::vector<json::builder::value_holder>{3});
  assigned = list;
  EXPECT_EQ(assigned.begin(), items.begin());
  EXPECT_EQ(assigned.size(), 2u);

  json::builder::array_holder owned(std::vector<json::builder::value_holder>{4, 5, 6});
  const json::builder::value_holder* owned_data = owned.begin();
  json::builder::array_holder moved(std::move(owned));
  EXPECT_EQ(moved.begin(), owned_data);
  EXPECT_EQ(owned.size(), 0u);
  assigned = std::move(moved);
  EXPECT_EQ(assigned.begin(), owned_data);
  EXPECT_EQ(assigned.size(), 3u);
  EXPECT_EQ(moved.size(), 0u);

  const json::builder::array_holder owned_copy(assigned);
  EXPECT_NE(owned_copy.begin(), assigned.begin());
  EXPECT_EQ(json::build(json::array(std::vector<json::builder::value_holder>(owned_copy.begin(), owned_copy.end()))),
            "[4,5,6]");
}

TEST(BasicTests, FloatingPointValues) {
  EXPECT_EQ(json::build(json::value(1.5)), "1.5");
  EXPECT_EQ(json::build(json::value(2.5f)), "2.5");
  EXPECT_EQ(json::build(json::value(std::numeric_limits<double>::quiet_NaN())), "null");

  const auto document = json::build_document(json::value(-0.25));
  ASSERT_TRUE(document.IsDouble());
  EXPECT_EQ(document.GetDouble(), -0.25);
}

TEST(BasicTests, BorrowedValueInDocumentAndValue) {
  rapidjson::Document document;
  const std::vector<int32_t> values{1, 2};
  const json::value inner = json::array(values);

  const auto built_document = json::build_document({{"inner", inner}, {"list", {{inner, inner}}}});
  const auto built_value = json::build_value({inner, inner}, document.GetAllocator());

  EXPECT_EQ(json::stringify(built_document), R"%({"inner":[1,2],"list":[[1,2],[1,2]]})%");
  ASSERT_TRUE(built_value.IsArray());
  ASSERT_EQ(built_value.Size(), 2u);
  EXPECT_EQ(built_value[1][1].GetInt(), 2);
}

TEST(BasicTests, InitializerListArrayOverloads) {
  rapidjson::Document allocator_owner;
  const auto value = json::build_value({1, "two", nullptr}, allocator_owner.GetAllocator());
  ASSERT_TRUE(value.IsArray());
  ASSERT_EQ(value.Size(), 3u);
  EXPECT_STREQ(value[1].GetString(), "two");

  rapidjson::Document document;
  json::build_document({1, 2}, document);
  EXPECT_EQ(json::stringify(document), "[1,2]");
  // building into the same document replaces its content
  json::build_document(json::array({3}), document);
  EXPECT_EQ(json::stringify(document), "[3]");

  std::string out;
  json::build({true, false}, out);
  EXPECT_EQ(out, "[true,false]");
  EXPECT_EQ(json::build({1, {{"name", "value"}}}), R"%([1,{"name":"value"}])%");
  EXPECT_EQ(json::stringify(json::build_document({1, 2})), "[1,2]");
}

TEST(BasicTests, LargeJsonReleasesThreadBuffer) {
  const std::string large(2 * 1024 * 1024, 'x');
  const auto json_text = json::build({{"large", large}});
  EXPECT_EQ(json_text.size(), large.size() + 12);
  EXPECT_EQ(json::stringify(json::build_document({{"large", large}})), json_text);
  EXPECT_EQ(json::build({{"small", 1}}), R"%({"small":1})%");
}

TEST(BasicTests, OtherContainers) {
  const std::forward_list<int32_t> forward_list{1, 2, 3};
  const std::list<std::string> list{"a", "b"};
  const std::array<double, 2> doubles{0.5, 1.5};
  const std::vector<bool> bools{true, false};
  const std::vector<int32_t> empty_vector;
  const std::map<std::string, int32_t> empty_map;
  const std::map<const char*, int32_t> char_map{{"key", 1}};

  EXPECT_EQ(json::build({{"forward_list", json::array(forward_list)},
                         {"list", json::array(list)},
                         {"doubles", json::array(doubles)},
                         {"bools", json::array(bools)},
                         {"empty_vector", json::array(empty_vector)},
                         {"empty_map", json::object(empty_map)},
                         {"char_map", json::object(char_map)}}),
            R"%({"forward_list":[1,2,3],"list":["a","b"],"doubles":[0.5,1.5],"bools":[true,false],)%"
            R"%("empty_vector":[],"empty_map":{},"char_map":{"key":1}})%");
}

TEST(BasicTests, RvalueContainers) {
  // one-shot: rvalue containers live until the end of the full expression
  EXPECT_EQ(json::build(json::array(std::vector<std::string>{"a", "b"})), R"%(["a","b"])%");
  EXPECT_EQ(json::build(json::array(std::vector<bool>{true})), "[true]");

  // persisted: json::value items are moved, owning names are copied
  const std::vector<int32_t> values{1, 2};
  const json::value array_source = json::array(std::vector<json::value>{json::array(values), "text", 3});
  const json::value object_source = json::object(std::map<std::string, json::value>{
      {"", 1}, {"name longer than the small string buffer", json::array(values)}});
  const json::value borrowed_names =
      json::object(std::vector<std::pair<std::string_view, int32_t>>{{"view", 1}});
  ClobberStack();

  EXPECT_EQ(json::build(array_source), R"%([[1,2],"text",3])%");
  EXPECT_EQ(json::build(object_source), R"%({"":1,"name longer than the small string buffer":[1,2]})%");
  EXPECT_EQ(json::stringify(json::build_document(object_source)),
            R"%({"":1,"name longer than the small string buffer":[1,2]})%");
  EXPECT_EQ(json::build(borrowed_names), R"%({"view":1})%");
}

TEST(BasicTests, ArrayFromInitializerListKeepsBorrowedValues) {
  json::value inner = 5;
  const json::value source = json::array({inner, {{"name", inner}}});
  inner = 6;

  EXPECT_EQ(json::build(source), R"%([6,{"name":6}])%");

  ExpectRuntimeErrorMessage([]() { static_cast<void>(json::array({{{nullptr, 1}}})); },
                            "Failed: nullptr != field.name.data()");
}

TEST(BasicTests, BuildAfterException) {
  EXPECT_THROW(json::build({{"array", {{1, {{"name", 1}, {nullptr, 2}}}}}}), std::runtime_error);
  EXPECT_THROW(json::build_document({{"array", {{1, {{nullptr, 2}}}}}}), std::runtime_error);

  EXPECT_EQ(json::build({{"name", "value"}}), R"%({"name":"value"})%");
  EXPECT_EQ(json::stringify(json::build_document({{"name", "value"}})), R"%({"name":"value"})%");
}

TEST(BasicTests, StringEscaping) {
  const std::string_view empty_view;
  const std::string with_zero("a\0b", 3);
  const std::string text =
      "long text to use the SIMD scan \"quoted\" back\\slash\ttab\nnew line \x01 control, \xD0\xBF\xD1\x80\xD0\xB8";

  const std::string expected =
      "{\"empty\":\"\",\"zero\":\"a\\u0000b\","
      "\"text\":\"long text to use the SIMD scan \\\"quoted\\\" back\\\\slash\\ttab\\nnew line \\u0001 control, "
      "\xD0\xBF\xD1\x80\xD0\xB8\",\"key \\\"quoted\\\"\":1}";
  EXPECT_EQ(json::build({{"empty", empty_view}, {"zero", with_zero}, {"text", text}, {"key \"quoted\"", 1}}),
            expected);

  const auto document = json::build_document({{"zero", with_zero}});
  ASSERT_EQ(document["zero"].GetStringLength(), 3u);
  EXPECT_EQ(std::string(document["zero"].GetString(), 3), with_zero);
}

TEST(BasicTests, MoreIntegerTypes) {
  const std::vector<json::value> values{static_cast<unsigned char>(255),
                                        static_cast<int8_t>(-128),
                                        static_cast<uint16_t>(65535),
                                        u'A',
                                        std::numeric_limits<int32_t>::min(),
                                        std::numeric_limits<uint32_t>::max()};
  EXPECT_EQ(json::build(json::array(values)), "[255,-128,65535,65,-2147483648,4294967295]");

  const auto document = json::build_document({{"int64", std::numeric_limits<int64_t>::min()},
                                              {"uint64", std::numeric_limits<uint64_t>::max()},
                                              {"bool", true}});
  ASSERT_TRUE(document["int64"].IsInt64());
  EXPECT_EQ(document["int64"].GetInt64(), std::numeric_limits<int64_t>::min());
  ASSERT_TRUE(document["uint64"].IsUint64());
  EXPECT_EQ(document["uint64"].GetUint64(), std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(document["bool"].GetBool());
}

TEST(BasicTests, DeepNesting) {
  constexpr int depth = 100;
  json::value nested = 0;
  for (int i = 0; i < depth; ++i) {
    nested = json::array(std::vector<json::value>{nested});
  }

  const std::string expected = std::string(depth, '[') + "0" + std::string(depth, ']');
  EXPECT_EQ(json::build(nested), expected);
  EXPECT_EQ(json::stringify(json::build_document(nested)), expected);
}

TEST(BasicTests, BuildFromManyThreads) {
  static constexpr int thread_count = 4;
  static constexpr int iterations = 1000;
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  for (int thread_index = 0; thread_index < thread_count; ++thread_index) {
    threads.emplace_back([thread_index, &failures]() {
      const std::string expected = R"%({"thread":)%" + std::to_string(thread_index) + R"%(,"list":[1,2,3]})%";
      for (int i = 0; i < iterations; ++i) {
        if (json::build({{"thread", thread_index}, {"list", {{1, 2, 3}}}}) != expected ||
            json::stringify(json::build_document({{"thread", thread_index}, {"list", {{1, 2, 3}}}})) != expected) {
          ++failures;
        }
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  EXPECT_EQ(failures.load(), 0);
}

}  // namespace

int main(int argc, char** argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
