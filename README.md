# CSCE3530_ProxyServer
Proxy Server built for UNT's CSCE 3530's Computer Network class

Compile Database:
g++ createProxyDB.cpp -l sqlite3 -o db

Run Database:
./db

Compile:
g++ main.cpp -l sqlite3 -pthread -o proxy

Run (shell):
./proxy

Run (web browser):
CSEIP:65001/www.website.com

"129.120.151.98" // cse05
"129.120.151.97" // cse04
"129.120.151.96" // cse03
"129.120.151.95" // cse02
"129.120.151.94" // cse01