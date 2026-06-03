# Advanced Encryption Standard

A multi-threaded C implementation of AES 

## Source

![wikipedia](https://fr.wikipedia.org/wiki/Advanced_Encryption_Standard)

## Build

```bash
cmake -B build <OPTIONS> && cmake --build build
```

Options:
    -DNO_OPENSSL=TRUE
        Disable the dependency to openssl

## Usage

aes [OPTION]

By default read the standard input and write in the standard output in cypher mode

--cypher
        Cypher the input
-d, --decypher
        Decypher the input
--file_in   FILE
        Input file
--file_out  FILE
        Output_file
--passwd    PASSWORD
        The password key to compute the output
--thread    NB_OF_THREAD
        Number of thread to compute the output

## Dependencies

- cmake
- openssl


