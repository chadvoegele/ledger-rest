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
#include <gtest/gtest.h>

#include "ledger_rest_args.h"
#include "ledger_rest.h"

#include "black_hole_logger.h"
#include "definitions.h"

typedef ledger_rest::ledger_rest::post_result post_result;

class simple_args : public ledger_rest::ledger_rest_args {
  public:
    simple_args(std::string path) : path(path) { }

    virtual std::string get_ledger_file_path() {
      return path;
    }

    virtual std::string get_ledger_rest_prefix() {
      return std::string("ledger");;
    }

  private:
    std::string path;
};

post_result build_result(std::string date_str, std::string payee, std::string account_name,
    double amount, double total) {
  post_result r;
  r.date = boost::gregorian::from_string(date_str);
  r.payee = payee;
  r.account_name = account_name;
  r.amount = amount;
  r.total = total;
  return r;
}

void compare_post_results(const std::list<post_result>& actual,
    const std::vector<post_result>& expected) {
  ASSERT_EQ(expected.size(), actual.size());

  std::list<post_result>::const_iterator result = actual.cbegin();
  for (int i = 0; i < expected.size(); i++) {
    ASSERT_EQ(expected[i].amount, result->amount);
    ASSERT_EQ(expected[i].date, result->date);
    ASSERT_EQ(expected[i].account_name, result->account_name);
    result++;
  }
}

void run_register_test(const std::string& ledger_file, const std::list<std::string>& args,
    const std::list<std::string>& query, const std::vector<post_result>& expected) {
  black_hole_logger logger;
  simple_args lr_args(RESOURCE_PATH + std::string("/") + ledger_file);
  ledger_rest::ledger_rest lr(lr_args, logger);

  // Need to force the journal to load.
  http::request req(std::string("HEAD"), std::string("/a"),
      std::map<std::string, std::string>(),
      std::multimap<std::string, std::string>());
  http::response res(lr.respond(req));

  std::list<post_result> actual = lr.run_register(args, query);
  compare_post_results(actual, expected);
}

TEST(ledger_rest, register1) {
  std::list<std::string> args = { "--add-budget", "--empty", "--collapse",
    "--period", "monthly from 2015/05/01 to 2015/09/01" };
  std::list<std::string> query = { "expenses" };

  std::vector<post_result> expected = {
    build_result("2015/5/01", " - 15-May-31", "<Total>", -110, -110),
    build_result("2015/6/01", " - 15-Jun-30", "expenses:fun", -110, -220),
    build_result("2015/7/01", " - 15-Jul-31", "<Total>", -110, -330),
    build_result("2015/8/01", " - 15-Aug-31", "expenses:fun", -200, -530)
  };

  run_register_test(std::string("ledger1.txt"), args, query, expected);
}

TEST(ledger_rest, register2) {
  std::list<std::string> args = { "--add-budget", "--empty", "--collapse",
    "--period", "monthly from 2015/05/01 to 2015/09/01" };
  std::list<std::string> query = { "expenses", "and", "payee", "movie" };

  std::vector<post_result> expected = {
    build_result("2015/5/01", " - 15-May-31", "expenses:fun", 10, 10),
    build_result("2015/6/01", " - 15-Jun-30", "<None>", 0, 10),
    build_result("2015/7/01", " - 15-Jul-31", "expenses:movie", 20, 30)
  };

  run_register_test(std::string("ledger1.txt"), args, query, expected);
}

TEST(ledger_rest, register3) {
  std::list<std::string> args = { "--market", "--empty", "--collapse",
    "--period", "monthly from 2015/05/01 to 2015/09/01" };
  std::list<std::string> query = { "assets" };

  std::vector<post_result> expected = {
    build_result("2015/5/01", " - 15-May-31", "assets", 30, 30),
    build_result("2015/6/01", " - 15-Jun-30", "assets:cash", -10, 20),
    build_result("2015/7/01", " - 15-Jul-31", "assets:cash", -15, 5)
  };

  run_register_test(std::string("ledger2.txt"), args, query, expected);
}

TEST(ledger_rest, register4) {
  std::list<std::string> args = {};
  std::list<std::string> query = { "expenses", "and", "payee", "movie" };

  std::vector<post_result> expected = {
    build_result("2015/5/16", "movie", "expenses:fun", 10, 10),
    build_result("2015/7/17", "movie", "expenses:movie", 20, 30)
  };

  run_register_test(std::string("ledger1.txt"), args, query, expected);
}

template<typename T>
void run_generic_account_test(const std::string& ledger_file,
    T account_function,
    const std::vector<std::string>& expected) {
  black_hole_logger logger;
  simple_args args(RESOURCE_PATH + std::string("/") + ledger_file);
  ledger_rest::ledger_rest lr(args, logger);

  // Need to force the journal to load.
  http::request req(std::string("HEAD"), std::string("/a"),
      std::map<std::string, std::string>(),
      std::multimap<std::string, std::string>());
  http::response res(lr.respond(req));

  auto fn = std::bind(account_function, &lr);
  std::list<std::string> actual = fn();

  ASSERT_EQ(expected.size(), actual.size());
  std::list<std::string>::const_iterator result = actual.cbegin();
  for (int i = 0; i < expected.size(); i++) {
    ASSERT_EQ(expected[i], *result);
    result++;
  }
}

void run_account_test(const std::string& ledger_file,
    const std::vector<std::string>& expected) {
  run_generic_account_test(ledger_file, &ledger_rest::ledger_rest::get_accounts,
      expected);
}

TEST(ledger_rest, account) {
  std::vector<std::string> expected = {
    std::string("assets:cash"),
    std::string("expenses:books"),
    std::string("expenses:fun"),
    std::string("expenses:movie"),
  };

  run_account_test(std::string("/ledger1.txt"), expected);
}

TEST(ledger_rest, respond_fail) {
  black_hole_logger logger;
  simple_args args(RESOURCE_PATH + std::string("/ledger1.txt"));
  ledger_rest::ledger_rest lr(args, logger);

  http::request req(std::string("HEAD"), std::string("/a"),
      std::map<std::string, std::string>(),
      std::multimap<std::string, std::string>());

  http::response res(lr.respond(req));
  ASSERT_EQ(http::status_code::METHOD_NOT_ALLOWED, res.status_code);

  http::request req2(std::string("GET"), std::string("/ledger/accts"),
      std::map<std::string, std::string>(),
      std::multimap<std::string, std::string>());
  http::response res2(lr.respond(req2));
  ASSERT_EQ(http::status_code::NOT_FOUND, res2.status_code);

  http::request req3(std::string("GET"), std::string("/ledger/report/register"),
      std::map<std::string, std::string>(),
      std::multimap<std::string, std::string>{{"badarg", "badval"}});
  http::response res3(lr.respond(req3));
  ASSERT_EQ(http::status_code::BAD_REQUEST, res3.status_code);
}

TEST(ledger_rest, post_to_json) {
  post_result pr(build_result("2010/07/01", "paycheck", "assets", 100.534, 200.534));
  std::string json(ledger_rest::ledger_rest::to_json(pr));
  std::string expected("{\"amount\" : 100.53, \"total\" : 200.53, \"date\" : \"2010-07-01\", \"payee\" : \"paycheck\", \"account_name\" : \"assets\"}");
  ASSERT_EQ(expected, json);
}

TEST(ledger_rest, posts_to_json) {
  post_result pr1(build_result("2010/07/01", "paycheck", "assets", 100.534, 100.534));
  post_result pr2(build_result("2009/07/01", "shop", "liabilities", -10.5, 90.534));
  std::list<post_result> prs = { pr1, pr2 };
  std::string json(ledger_rest::ledger_rest::to_json(prs));
  std::string expected("[{\"amount\" : 100.53, \"total\" : 100.53, \"date\" : \"2010-07-01\", \"payee\" : \"paycheck\", \"account_name\" : \"assets\"}, "
      "{\"amount\" : -10.50, \"total\" : 90.53, \"date\" : \"2009-07-01\", \"payee\" : \"shop\", \"account_name\" : \"liabilities\"}]");
  ASSERT_EQ(expected, json);
}

TEST(ledger_rest, accounts_to_json) {
  std::list<std::string> accounts = { "grass", "is", "always", "greener" };
  std::string json(ledger_rest::ledger_rest::to_json(accounts));
  std::string expected("[\"grass\", \"is\", \"always\", \"greener\"]");
  ASSERT_EQ(expected, json);
}

TEST(ledger_rest, get_journal_include_files) {
  black_hole_logger logger;
  std::string ledger_file("ledger_inc.txt");
  simple_args args(RESOURCE_PATH + std::string("/") + ledger_file);
  ledger_rest::ledger_rest lr(args, logger);
  auto actual = lr.get_journal_include_files();

  std::list<std::string> expected = {
    RESOURCE_PATH + std::string("/") + std::string("ledger1.txt")
  };

  ASSERT_EQ(expected, actual);
}
