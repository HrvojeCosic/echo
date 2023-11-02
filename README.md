# Usage
* Create executables using `make all`

* Run the server using `./echo_server`. You can run commands on startup or during runtime. Run `--set-response-schema EQUIVALENT/REVERSE/CENSORED CHAR=c/PALINDROME ` to change a response schema

* Run the client using `./echo_client localhost 6000` or `./echo_client /tmp/unix_socket` and enter your text in the CLI

* Running all tests can be done using `make test`. GoogleTest library is a prerequisite for running tests and can be acquired automatically using `make install_libgtest`