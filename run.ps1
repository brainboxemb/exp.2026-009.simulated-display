$ErrorActionPreference = "Stop"

cmake -S native -B native/build
cmake --build native/build --config Release
ctest --test-dir native/build -C Release --output-on-failure
mvn javafx:run
