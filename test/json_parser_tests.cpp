//
// Copyright (c) 2015-2020 Chad Voegele
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//  * The name of Chad Voegele may not be used to endorse or promote products
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <gtest/gtest.h>

#include "json_parser.h"

void run_trim_whitespace_test(std::string input, std::string expected) {
  std::string actual = ledger_rest::trim_whitespace(input);
  ASSERT_EQ(actual, expected);
}

TEST(json_parser, trim_whitespace_test1) {
  std::string input = "duck duck";
  std::string expected = "duckduck";
  run_trim_whitespace_test(input, expected);
}

TEST(json_parser, trim_whitespace_test2) {
  std::string input = "duck\t\nduck";
  std::string expected = "duckduck";
  run_trim_whitespace_test(input, expected);
}

TEST(json_parser, trim_whitespace_test3) {
  std::string input = "\"duck duck\"";
  std::string expected = "\"duck duck\"";
  run_trim_whitespace_test(input, expected);
}

void run_parse_register_request_json_test(
    std::string input,
    std::list<std::unordered_map<std::string, std::list<std::string>>> expected) {
  std::list<std::unordered_map<std::string, std::list<std::string>>> actual
    = ledger_rest::parse_register_request_json(input);
  ASSERT_EQ(actual, expected);
}

TEST(json_parser, parse_register_request1) {
  std::string input = "[{\"arg1\": [\"cat\",\"dog\"], \"arg2\": [\"kiwi\"]},{\"arg1\": [\"panda\",\"tiger\"], \"arg2\": [\"coconut\"]}]";
  std::list<std::unordered_map<std::string, std::list<std::string>>> expected = {
    { {"arg1", {"cat", "dog"}}, {"arg2", {"kiwi"}} },
    { {"arg1", {"panda", "tiger"}}, {"arg2", {"coconut"}} }
  };
  run_parse_register_request_json_test(input, expected);
}

TEST(json_parser, parse_register_request2) {
  std::string input = "[]";
  std::list<std::unordered_map<std::string, std::list<std::string>>> expected = {
  };
  run_parse_register_request_json_test(input, expected);
}

TEST(json_parser, parse_register_request3) {
  std::string input = "[{\"arg1\": [], \"arg2\": []},{\"arg1\": [\"panda\",\"tiger\"], \"arg2\": [\"coconut\"]}]";
  std::list<std::unordered_map<std::string, std::list<std::string>>> expected = {
    { {"arg1", {}}, {"arg2", {}} },
    { {"arg1", {"panda", "tiger"}}, {"arg2", {"coconut"}} }
  };
  run_parse_register_request_json_test(input, expected);
}

TEST(json_parser, parse_register_request4) {
  std::string input = "[{\"arg1\": []},{\"arg1\": [\"panda\",\"tiger\"], \"arg2\": [\"coconut\"]}]";
  std::list<std::unordered_map<std::string, std::list<std::string>>> expected = {
    { {"arg1", {}} },
    { {"arg1", {"panda", "tiger"}}, {"arg2", {"coconut"}} }
  };
  run_parse_register_request_json_test(input, expected);
}

TEST(json_parser, parse_register_request5) {
  std::string input = "[{\"arg1\": [\"panda\",\"tiger\"], \"arg2\": [\"coconut\"]}]";
  std::list<std::unordered_map<std::string, std::list<std::string>>> expected = {
    { {"arg1", {"panda", "tiger"}}, {"arg2", {"coconut"}} }
  };
  run_parse_register_request_json_test(input, expected);
}
