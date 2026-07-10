# NSP

A single header null-safe pointer library for C++20.

Features:
  - `nullptr` default initialization
  - null checked dereferencing
  - errors as exceptions
  - `std::pointer_traits`
  - equality comparison and ordering

Does not support:
  - pointer arithmetic

## Installation

Install into `/usr/local/include`:
```sh
sudo make install
```

Uninstall:
```sh
sudo make uninstall
```

