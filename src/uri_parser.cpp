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

#include "uri_parser.h"

#include <sstream>
#include <cstring>

namespace ledger_rest {
  std::list<std::string> split_string(std::string str, std::string delimiter) {
    std::list<std::string> split;

    size_t begin = 0;
    size_t find = str.find_first_of(delimiter, begin);

    while (find != std::string::npos) {
      split.push_back(str.substr(begin, find - begin));
      begin = find + 1;
      find = str.find_first_of(delimiter, begin);
    }
    split.push_back(str.substr(begin, str.length() - begin));

    return split;
  }

  std::string join_string(std::list<std::string> parts, std::string delimiter) {
    std::stringstream joined;

    std::list<std::string>::const_iterator iter;
    for (iter = parts.cbegin(); iter != parts.cend(); iter++) {
      if (iter == parts.cbegin())
        joined << *iter;
      else {
        joined << delimiter;
        joined << *iter;
      }
    }

    std::string joined_str = joined.str();
    return joined_str;
  }

  std::unordered_map<std::string, std::list<std::string>> mapify_uri_args(
      std::multimap<std::string, std::string> uri_args) {
    std::unordered_map<std::string, std::list<std::string>> args;
    std::multimap<std::string, std::string>::const_iterator iter;
    for (iter = uri_args.cbegin(); iter != uri_args.cend(); iter++) {
      args[iter->first].push_back(iter->second);
    }
    return args;
  }
}
