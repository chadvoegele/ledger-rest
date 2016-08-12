# ledger-rest
## Usage
ledger-rest [OPTION...]

### Options
Short|Long                                    |Description                                                  |
-----|----------------------------------------|-------------------------------------------------------------|
-c   |--cert=certificate file                 | Certificate used by HTTPS.                                  |
-e   |--ledger_rest_prefix=ledger rest prefix | Prefix for ledger REST http queries. Default is /ledger_rest|
-f   |--file=ledger file                      | Leger file                                                  |
-k   |--key=private key file                  | File containing private key for HTTPS.                      |
-l   |--level=log level                       | Log level [0-9]. Higher numbers mean more logging.          |
-p   |--port=port number                      | Port for server to run on.                                  |
-t   |--client_cert=client certificate file   | Certificate used to validate client certs.                  |
-u   |--pass=user/pass file                   | File containing user:password in consecutive lines.         |
-?   |--help                                  | Give this help list                                         |
     |--usage                                 | Give a short usage message                                  |
-V   |--version                               | Print program version                                       |

## Endpoints
* Accounts
  * __Request__: GET /ledger_rest/accounts
  * __Example Reponse__: [ "assets", "expenses:car", "expenses:food", "income" ]
  * __ledger-cli__: `ledger bal --flat --balance-format "%a\n"`

* Budget Accounts
  * __Request__: GET /ledger_rest/budget_accounts
  * __Example Reponse__: [ "assets", "expenses:food", "income" ]
  * __ledger-cli__: `ledger bal --flat --budget --balance-format "%a\n"`

* Register
  * __Request__: GET /ledger_rest/report/register?args=-E&args=--collapse&args=--period&args=monthly&query=expenses
  * __Example Reponse__:
    [
      {"amount" : 100, "date" : "2028-09-01", "account_name" : "expenses"},
      {"amount" : 200, "date" : "2028-10-01", "account_name" : "expenses"}
    ]
  * __ledger-cli__: `ledger -E --collapse --period monthly --register-format '{amount: %t, date: %d, account_name: %a}\n' reg expenses`

* Batch Register
  * __Request__: POST /ledger_rest/report/register
  * __Request Body__:
    [
      { "args": [ "-E", "--collapse" ], "query": [ "expenses" ] },
      { "args": [ "-E", "--collapse" ], "query": [ "income" ] }
    ]
  * __Example Reponse__:
    [
      [
        { "amount" : 100, "date" : "2028-09-01", "account_name" : "expenses" },
        { "amount" : 200, "date" : "2028-10-01", "account_name" : "expenses" }
      ], [
        { "amount" : -1000, "date" : "2028-09-01", "account_name" : "income" },
        { "amount" : -2000, "date" : "2028-10-01", "account_name" : "income" }
      ]
    ]
  * __ledger-cli__: `ledger -E --collapse --period monthly --register-format '{amount: %t, date: %d, account_name: %a}\n' reg expenses` then same for `income`
