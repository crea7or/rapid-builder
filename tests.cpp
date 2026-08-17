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

#include <iostream>
#include <list>
#include <map>
#include <string_view>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "builder.h"

namespace {

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

}  // namespace

int main(int argc, char** argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
