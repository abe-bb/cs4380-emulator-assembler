# Testing
This project is tested using GoogleTest for unit testing. Unit tests can be
be found in the `test/` directory.

In addition, a set of integration tests using handcrafted binaries and bash
are used to validate functionality. The binaries can be built using `xxd`
and the hex to build them is stored in the `system_test/hex/` directory. The bash script
`system_test/runSystemTests.sh` assumes the working directory is `system_test/`
and then builds the project, converts the hex files to binary, and runs
through the integration tests. 
