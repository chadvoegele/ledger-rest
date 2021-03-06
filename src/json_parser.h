/* * Copyright (c) 2015-2020 Chad Voegele
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *  * The name of Chad Voegele may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <experimental/optional>

namespace std {
  template<class T>
  using optional = experimental::optional<T>;
}

namespace ledger_rest {
  std::string trim_whitespace(std::string);
  std::optional<std::string> parse_json_string(std::string&);
  std::optional<std::list<std::string>> parse_json_array_of_strings(std::string&);
  std::optional<std::pair<std::string,std::list<std::string>>> parse_json_key_object(
      std::string&);
  std::optional<std::unordered_map<std::string, std::list<std::string>>>
    parse_json_object_of_arrays_of_strings(std::string&);
  std::optional<std::list<std::unordered_map<std::string,std::list<std::string>>>>
    parse_json_array_of_object_of_arrays_of_strings(std::string&);
  std::list<std::unordered_map<std::string, std::list<std::string>>>
    parse_register_request_json(std::string json);
}
