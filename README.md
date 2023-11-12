## Usage
* Build the project using `./scripts/build.sh`

* Run the load balancer/dispatcher using `sudo ./build/src/dispatcher`. You can run commands on startup or during runtime. Run `--set-response-schema [EQUIVALENT/REVERSE/CENSORED CHAR=c/PALINDROME]` to change the response schema of the servers

* Run the client using `sudo ./build/src/echo_client localhost 6000` or `sudo ./build/src/echo_client /tmp/unix_socket` and enter your message into the CLI

## Tests
* To run all tests, use `./scripts/run_tests.sh`. 