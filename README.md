# Install

1. Untar the archive

```
tar xf prpa-src.tar.gz

```

2. Create your build directory

```
mkdir build && cd build
```

3. Install project dependancies with the [conan](https://docs.conan.io/en/latest/introduction.html) package manager.

```
conan profile update settings.compiler.libcxx=libstdc++11 default
conan install .. --build missing
```

(using a C++ package manager avoid version incompatibities)

4. Build the project (in Debug or Release) with cmake

```
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

or

```
cmake .. -DCMAKE_BUILD_TYPE=Release
```


# Files

 - hashmap_implementation contains the hashmap implementation
 - trie_implementation contains the trie_implementation
 - async_implementation contains the async implementation
 - fusion_implementation contains the implementation our trie structure using our hashmap structure