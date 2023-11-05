## Usage
* Build the project using `./scripts/build.sh`

* Run the server using `./build/src/echo_server`. You can run commands on startup or during runtime. Run `--set-response-schema [EQUIVALENT/REVERSE/CENSORED CHAR=c/PALINDROME]` to change the server response schema

* Run the client using `./build/src/echo_client localhost 6000` or `./build/src/echo_client /tmp/unix_socket` and enter your message into the CLI

## Tests
* To run all tests, use `./scripts/run_tests.sh`. 