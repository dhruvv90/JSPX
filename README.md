# JSPX
Header-only JSON Parsing Library based in C++


## Introduction

JSPX is fast and light weight JSON Parser created in C++17.

* Uses STL containers. Does not rely on Boost.
* Supports DOM-style JSON parsing. Transforms input stream into DOM tree which can be traversed and converted into desired form. Current implementation is <b>read-only</b>.
* JSPX is fast and Memory-friendly. Every tree node is of deterministic size (160 bytes on 32 bit machine).
* Optimized with proper copy and move semantics to optimize unnecessary/expensive operations of nodes/tree.

---

## Compatibility

JSPX has been created and tested in Windows (32/64 bit) using MSVC++ 14.27 compiler. Language standard : c++17.


## Installation:
JSPX is a header only library. It can be directly used by cloning/downloading and including "document.h".


## License

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
