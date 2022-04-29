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

#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <fstream>

#include "ledger_rest.h"
#include "uri_parser.h"
#include "json_parser.h"

namespace ledger_rest {
  typedef ledger_rest::post_result post_result;

  ledger_rest::ledger_rest(ledger_rest_args& args, logger& logger)
    : ledger_file(args.get_ledger_file_path()), lr_logger(logger), is_file_loaded(false), http_prefix(args.get_ledger_rest_prefix()) {
  }

  template<typename T>
  std::string ledger_rest::to_string(const std::list<T>& v) {
    std::stringstream s;
    s << '[';
    bool first = true;
    for (const auto& e : v) {
      if (first) {
        s << e;
        first = false;

      } else {
        s << ',' << e;
      }
    }
    s << ']';
    return s.str();
  }

  std::list<post_result> ledger_rest::run_register(
      std::list<std::string> args, std::list<std::string> query) {
    try {
      return run_register_or_throw(args, query);

    } catch (const std::exception& e) {
      lr_logger.log(5, e.what());
      lr_logger.log(5, to_string(args));
      lr_logger.log(5, to_string(query));

    } catch (...) {
      lr_logger.log(5, "Unknown error while respond to request:");
      lr_logger.log(5, to_string(args));
      lr_logger.log(5, to_string(query));
    }

    std::list<post_result> empty;
    return empty;
  }

  std::list<post_result> ledger_rest::run_register_or_throw(
      std::list<std::string> args, std::list<std::string> query) {
    ledger::report_t report(*session_ptr);
    ledger::scope_t::default_scope = &report;

    args = ledger::process_arguments(args, report);
    report.normalize_options("register");

    ledger::call_scope_t query_args(report);
    for(std::string str : query) {
      query_args.push_back(ledger::string_value(str));
    }
    if (query_args.size() > 0)
      report.parse_query_args(query_args.value(), "#r");

    post_capturer* capturer = new post_capturer();
    boost::shared_ptr<ledger::item_handler<ledger::post_t> > post_capturer_ptr(capturer);
    report.posts_report(post_capturer_ptr);

    std::list<post_result> results(capturer->get_post_results());
    return results;
  }

  std::string ledger_rest::to_json(post_result posts) {
    std::stringstream ss;
    ss << setiosflags(std::ios_base::fixed);
    ss << std::setprecision(2);
    ss << "{";
    ss << "\"amount\" : ";
    ss << posts.amount;
    ss << ", ";
    ss << "\"total\" : ";
    ss << posts.total;
    ss << ", ";
    ss << "\"date\" : ";
    ss << "\"" << boost::gregorian::to_iso_extended_string(posts.date) << "\"";
    ss << ", ";
    ss << "\"payee\" : ";
    ss << "\"" << posts.payee << "\"";
    ss << ", ";
    ss << "\"account_name\" : ";
    ss << "\"" << posts.account_name << "\"";
    ss << "}";

    std::string json = ss.str();
    return json;
  }

  template<typename T>
  std::string ledger_rest::to_json(std::list<T> x, std::function<std::string(T)> to_json) {
    std::stringstream ss;

    ss << "[";
    typename std::list<T>::const_iterator iter;
    for (iter = x.cbegin(); iter != x.end(); iter++) {
      if (iter == x.cbegin()) {
        ss << to_json(*iter);
      } else {
        ss << ", ";
        ss << to_json(*iter);
      }
    }
    ss << "]";

    std::string json = ss.str();
    return json;
  }

  std::string ledger_rest::to_json(std::list<post_result> posts) {
    std::string (*to_json_ptr)(post_result) = to_json;
    std::function<std::string(post_result)> to_json_fn = to_json_ptr;

    std::string json(to_json(posts, to_json_fn));
    return json;
  }

  std::string ledger_rest::to_json(std::list<std::string> accounts) {
    std::function<std::string(std::string)> to_json_fn
      = [](std::string s) { return std::string("\"") + s + std::string("\""); };

    std::string json(to_json(accounts, to_json_fn));
    return json;
  }

  std::string ledger_rest::to_json(std::list<std::list<post_result>> results) {
    std::list<std::string> intermediate_json;
    for (auto iter = results.cbegin(); iter != results.cend(); iter++) {
      intermediate_json.push_back(to_json(*iter));
    }
    std::function<std::string(std::string)> to_json_fn
      = [](std::string s) { return s; };

    std::string json(to_json(intermediate_json, to_json_fn));
    return json;
  }

  std::list<std::string> ledger_rest::get_accounts() {
    std::list<std::string> args;
    return get_balance_accounts(args);
  }

  std::list<std::string> ledger_rest::get_balance_accounts(std::list<std::string> args) {
    ledger::report_t report(*session_ptr);
    ledger::scope_t::default_scope = &report;

    ledger::process_arguments(args, report);
    report.normalize_options("balance");

    ledger::call_scope_t query_args(report);
    report.parse_query_args(query_args.value(), "#b");

    account_capturer* capturer = new account_capturer();
    boost::shared_ptr<ledger::item_handler<ledger::account_t> > account_capturer_ptr(capturer);
    report.accounts_report(account_capturer_ptr);

    std::list<std::string> results(capturer->get_account_results());
    return results;
  }

  http::response ledger_rest::respond(http::request request) {
    http::response bad_response(http::status_code::BAD_REQUEST, std::string(""),
        std::map<std::string, std::string>());

    // Don't reload eagerly because:
    // 1) If journal edits are made rapidly, reloads will be excessive
    // 2) Dropbox sync inotify events are hard to follow
    // This does introduce some latency on first HTTP request.
    if (!is_file_loaded) {
      reset_journal();
    }

    if (is_file_loaded) {
      try {
        return respond_or_throw(request);

      } catch (const std::exception& e) {
        lr_logger.log(5, e.what());
        lr_logger.log(5, request.to_string());

      } catch (...) {
        lr_logger.log(5, "Unknown error while respond to request:");
        lr_logger.log(5, request.to_string());
      }
    }
    return bad_response;
  }

  http::response ledger_rest::respond_or_throw(http::request request) {
    std::function<http::response(http::status_code)> build_fail = [](http::status_code code) {
      http::response res(code, std::string(""),
          std::map<std::string, std::string>());
      return res;
    };

    std::function<http::response(std::string)> build_ok = [](std::string s) {
      http::response res(http::status_code::OK,
          s, std::map<std::string, std::string>());
      return res;
    };

    if (request.method != std::string("GET") &&
        request.method != std::string("POST")) {
      return build_fail(http::status_code::METHOD_NOT_ALLOWED);
    }

    std::list<std::string> uri_parts
      = ::ledger_rest::split_string(request.url, "/");
    std::unordered_map<std::string, std::list<std::string>> uri_args
      = ::ledger_rest::mapify_uri_args(request.uri_args);

    std::list<std::string> register_request;
    std::list<std::string> accounts_request;
    if (http_prefix.size() > 0) {
      register_request = {"", http_prefix, "report", "register"};
      accounts_request = {"", http_prefix, "accounts"};
    } else {
      register_request = {"", "report", "register"};
      accounts_request = {"", "accounts"};
    }

    if (uri_parts == register_request) {
      if (request.method == std::string("GET")) {
        if (uri_args.find("query") != uri_args.end()) {
          std::list<std::string> args;
          if (uri_args.find("args") != uri_args.end()) {
            args = uri_args[std::string("args")];
          } else {
            args = {};
          }
          std::list<std::string> query = uri_args[std::string("query")];
          std::list<post_result> reg(ledger_rest::run_register(args, query));

          http::response res = build_ok(to_json(reg));
          return res;

        } else {
          return build_fail(http::status_code::BAD_REQUEST);
        }

      } else if (request.method == std::string("POST")) {
        std::list<std::unordered_map<std::string, std::list<std::string>>> parsed_json =
          ::ledger_rest::parse_register_request_json(request.upload_data);

        std::list<std::list<post_result>> results;
        for (auto iter = parsed_json.cbegin(); iter != parsed_json.end(); iter++) {
          std::unordered_map<std::string, std::list<std::string>> req = *iter;
          std::list<std::string> args = req[std::string("args")];
          std::list<std::string> query = req[std::string("query")];
          std::list<post_result> reg(ledger_rest::run_register(args, query));
          results.push_back(reg);
        }
        std::string responses_json = to_json(results);

        http::response res = build_ok(responses_json);
        return res;

      } else {
        return build_fail(http::status_code::METHOD_NOT_ALLOWED);

      }

    } else if (request.method == std::string("GET") &&
        uri_parts == accounts_request) {
      std::list<std::string> accounts(ledger_rest::get_accounts());

      http::response res = build_ok(to_json(accounts));
      return res;

    } else
      lr_logger.log(5, "Url not found: " + request.url + " for method: " + request.method);
      return build_fail(http::status_code::NOT_FOUND);
  }

  void ledger_rest::reset_journal() {
    try {
      reset_journal_or_throw();

    } catch (...) {
      lr_logger.log(5, "Unable to load ledger file");
      is_file_loaded = false;
    }
  }

  void ledger_rest::reset_journal_or_throw() {
    session_ptr = std::make_shared<ledger::session_t>();
    ledger::set_session_context(session_ptr.get());
    ledger::scope_t::default_scope = &empty_scope;
    ledger::scope_t::empty_scope = &empty_scope;
    session_ptr->read_journal(ledger_file);
    is_file_loaded = true;
    lr_logger.log(7, "Reloaded ledger file.");
  }

  void ledger_rest::lazy_reload_journal() {
    is_file_loaded = false;
  }

  std::list<std::string> ledger_rest::get_journal_include_files() {
    std::string include_directive("!include ");
    std::ifstream ledger_stream(ledger_file, std::ios::in);

    std::list<std::string> include_files;
    std::string line;
    if (ledger_stream.is_open()) {
      while (getline(ledger_stream, line)) {
        if (line.find(include_directive) == 0) {
          std::string include_file = line.substr(include_directive.length());
          auto last_slash = ledger_file.find_last_of('/');

          // Assume POSIX for absolute path check
          if (include_file.at(0) != '/' && last_slash != std::string::npos) {
            include_files.push_back(ledger_file.substr(0, 1+last_slash) + include_file);
          } else {
            include_files.push_back(include_file);
          }
        }
      }
    }

    return include_files;
  }

  ledger::value_t ledger_rest::post_capturer::get_total(ledger::post_t& post)
  {
    if (post.has_xdata() && !post.xdata().total.is_null()) {
      return post.xdata().total;

    } else if (post.amount.is_null()) {
      return 0L;

    } else {
      return post.amount;
    }
  }

  ledger::value_t ledger_rest::post_capturer::get_amount(ledger::post_t& post)
  {
    if (post.has_xdata() && post.xdata().has_flags(POST_EXT_COMPOUND)) {
      auto value = post.xdata().compound_value;

      if (value.value()) {
        return value.value();

      } else {
        return value;
      }

    } else if (post.amount.is_null()) {
      return 0L;

    } else {
      return post.amount;
    }
  }

  void ledger_rest::post_capturer::operator()(ledger::post_t& post) {
    post_result r;
    r.amount = get_amount(post).to_amount().to_double();
    r.total = get_total(post).value().to_amount().to_double();
    r.date = post.xact->date();
    ledger::account_t& account(*post.reported_account());
    r.account_name = account.fullname();
    r.payee = post.payee();
    result_capture.push_back(r);
  }

  std::list<post_result> ledger_rest::post_capturer::get_post_results() {
    return result_capture;
  }

  void ledger_rest::post_capturer::clear() {
    result_capture.clear();
    ledger::item_handler<ledger::post_t>::clear();
  }

  void ledger_rest::account_capturer::operator()(ledger::account_t& account) {
    if (account.has_xdata()) {
      std::string account_name = account.partial_name(true);
      result_capture.push_back(account_name);
    }
  }

  void ledger_rest::account_capturer::clear() {
    result_capture.clear();
    item_handler<ledger::account_t>::clear();
  }

  std::list<std::string> ledger_rest::account_capturer::get_account_results() {
    return result_capture;
  }
}
