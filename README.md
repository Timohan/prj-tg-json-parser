# prj-tg-json-parser

A small, strict JSON (RFC 8259) parser library for C++17 on Linux.

## Requirements

On Ubuntu

```sh
sudo apt install build-essential pkg-config
```

## Building and installing

```sh
cd lib
make
sudo make install
```

Build as a normal user first, `make install` doesn't build.
The library is installed under `/usr` by default, use `PREFIX` to change it:

```sh
sudo make install PREFIX=/usr/local
```

## Usage

```cpp
#include <prj_tg_json_parser_lib.h>
#include <iostream>

int main()
{
    PrjTgJsonParserLib parser;
    if (!parser.parseData(R"({"name":"Tom","age":13,"tags":["a","b"]})")) {
        std::cout << parser.getErrorMessage() << "\n";
        return 1;
    }
    PrjTgJsonParserObject *root = parser.getObject();
    std::cout << root->getObject("name")->getText() << "\n";            // Tom
    std::cout << root->getObject("age")->getValue() << "\n";            // 13
    std::cout << root->getObject("tags")->getObject(1)->getText() << "\n"; // b
    return 0;
}
```

```sh
g++ -std=c++17 example.cpp $(pkg-config --cflags --libs prj-tg-json-parser-lib)
```

Use `parse(filename)` to parse a file. For untrusted input, limit the
nesting depth and input size with `setMaxDepth()` and `setMaxInputSize()`.

## Running tests

```sh
cd test
make run
```

The tests build and use the library from `../lib`, it doesn't need to be installed.
