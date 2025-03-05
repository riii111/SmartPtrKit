# SmartPtrKit

A modern C++ implementation of memory management smart pointers. This library provides custom implementations of C++ smart pointers for educational purposes.

## Features

* `unique_ptr` - Exclusive ownership smart pointer
* `shared_ptr` - Shared ownership smart pointer
* `weak_ptr` - Non-owning observer of shared_ptr

## Building

```bash
mkdir -p build && cd build
cmake ..
make
```

## Running tests

```bash
cd build
ctest
```

To run a specific test:

```bash
cd build
ctest -R unique_ptr_test
```

## Usage Examples

### unique_ptr

```cpp
#include "unique_ptr.hpp"

// Create a unique_ptr
sptr::unique_ptr<int> p1(new int(42));

// Use make_unique helper
auto p2 = sptr::make_unique<int>(42);

// Move ownership
sptr::unique_ptr<int> p3 = std::move(p2);
```

### shared_ptr and weak_ptr

```cpp
#include "shared_ptr.hpp"
#include "weak_ptr.hpp"

// Create a shared_ptr
sptr::shared_ptr<int> s1(new int(42));

// Create another shared_ptr that shares ownership
sptr::shared_ptr<int> s2 = s1;  // Reference count = 2

// Create a weak_ptr that observes but doesn't own
sptr::weak_ptr<int> w1 = s1;

// Check if the weak_ptr's resource still exists
if (!w1.expired()) {
    // Get a shared_ptr from the weak_ptr
    sptr::shared_ptr<int> s3 = w1.lock();
    
    // Use the resource
    *s3 = 100;
}
```