//
// Copyright (c) 2015 Chad Voegele
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

#include "json_parser.h"

#include <sstream>
#include <cstring>

namespace budget_charts {
  std::string trim_whitespace(std::string s) {
    std::stringstream ss;

    bool in_quote = false;
    for (auto iter = s.cbegin(); iter != s.cend(); iter++) {
      if (in_quote || (*iter != ' ' && *iter != '\n' && *iter != '\t')) {
        ss << *iter;
      }

      if (*iter == '"') {
        in_quote = !in_quote;
      }
    }

    std::string trimmed = ss.str();
    return trimmed;
  }

  std::optional<std::string> parse_json_string(std::string& working_json) {
    if (working_json.length() == 0) {
      return {};
    }

    if (working_json.at(0) != '"') {
      return {};
    }
    working_json.erase(0, 1);  // "

    std::string::size_type end_quote_position = working_json.find('"');
    if (end_quote_position == std::string::npos) {
      return {};
    }

    std::string arg;
    arg = working_json.substr(0, end_quote_position);
    working_json.erase(0, end_quote_position);  // the string
    working_json.erase(0, 1);  // "

    return arg;
  }

  std::optional<std::list<std::string>> parse_json_array_of_strings(
      std::string& working_json) {
    if (working_json.length() == 0) {
      return {};
    }

    // []...
    // ["test"]...
    // ["test","test2"]...
    // ["test","test2","test3"]...

    if (working_json.at(0) != '[') {
      return {};
    }

    working_json.erase(0, 1); // [

    std::list<std::string> list;
    auto s = parse_json_string(working_json);
    while (s) {
      list.push_back(s.value());

      if (working_json.at(0) == ',') {
        working_json.erase(0, 1);
      }
      s = parse_json_string(working_json);
    }

    if (working_json.length() == 0) {
      return {};
    }

    if (working_json.at(0) != ']') {
      return {};
    }

    working_json.erase(0, 1); // ]

    return list;
  }

  std::optional<std::pair<std::string,std::list<std::string>>> parse_json_key_object(
      std::string& working_json) {
    if (working_json.length() == 0) {
      return {};
    }

    auto key = parse_json_string(working_json);

    if (working_json.length() > 0 && working_json.at(0) != ':') {
      return {};
    }
    working_json.erase(0, 1);  // :

    auto args = parse_json_array_of_strings(working_json);

    if (key && args) {
      return std::make_pair(key.value(), args.value());
    } else {
      return {};
    }
  }

  std::optional<std::unordered_map<std::string, std::list<std::string>>>
    parse_json_object_of_arrays_of_strings(std::string& working_json) {
      if (working_json.length() == 0) {
        return {};
      }

      if (working_json.at(0) != '{') {
        return {};
      }
      working_json.erase(0, 1); // {

      std::unordered_map<std::string, std::list<std::string>> req;

      auto keyObject = parse_json_key_object(working_json);

      while (keyObject) {
        auto key = std::get<0>(keyObject.value());
        auto args = std::get<1>(keyObject.value());
        req[key] = args;

        if (working_json.at(0) == ',') {
          working_json.erase(0, 1);
        }

        keyObject = parse_json_key_object(working_json);
      }

      if (working_json.length() == 0) {
        return {};
      }

      if (working_json.at(0) != '}') {
        return {};
      }

      working_json.erase(0, 1); // }

      return req;
  }

  std::optional<std::list<std::unordered_map<std::string,std::list<std::string>>>>
    parse_json_array_of_object_of_arrays_of_strings(std::string& working_json) {
    if (working_json.length() == 0) {
      return {};
    }

    if (working_json.at(0) != '[') {
      return {};
    }

    working_json.erase(0, 1);  // [

    std::list<std::unordered_map<std::string,std::list<std::string>>> array;

    auto obj = parse_json_object_of_arrays_of_strings(working_json);

    while (obj) {
      array.push_back(obj.value());

      if (working_json.at(0) == ',') {
        working_json.erase(0, 1);
      }

      obj = parse_json_object_of_arrays_of_strings(working_json);
    }

    if (working_json.length() == 0) {
      return {};
    }

    if (working_json.at(0) != ']') {
      return {};
    }

    working_json.erase(0, 1); // ]

    return array;
  }

  std::list<std::unordered_map<std::string, std::list<std::string>>>
    parse_register_request_json(std::string json) {
    std::string working_json = trim_whitespace(json);

    auto data = parse_json_array_of_object_of_arrays_of_strings(working_json);

    if (data) {
      return data.value();
    } else {
      std::list<std::unordered_map<std::string, std::list<std::string>>> empty;
      return empty;
    }
  }
}
