# Reproducibility guide

## Environment

Recommended environment:

```text
Windows 11
MinGW-w64 gcc
cmd.exe
```

No Python is required for the planned C index-builder path.

## Packed-index path

The implementation is included in `tau_c0c2cm_index/`.  The command path is:

```bat
build.bat
tau_index.exe build <end> <index.bin> [threads] [chunk_mb]
tau_index.exe query <index.bin> <n>
tau_index.exe twins <index.bin> <start> <end> [count]
```

The builder always creates an index over:

```text
1 <= n <= end
```

The lower bound `1` is not passed on the command line.

## Build

```bat
build.bat
```

Expected build result:

```text
Build OK: tau_index.exe
```

## Build examples

Small check:

```bat
tau_index.exe build 60000000 index_60m.bin 8 1024
```

Large check:

```bat
tau_index.exe build 100000000000 index.bin 16 1024
```

## Query example

```bat
tau_index.exe query index.bin 10000
```

Expected arithmetic interpretation:

```text
10000 = 2^4 * 5^4
Omega(10000) = 8
resolved label = A
```

## Twin-prime search example

```bat
tau_index.exe twins index.bin 1 60000000 10
```

## Disk-size formula

For packed format version 3:

```text
index_size_bytes = 64 + end
```

For:

```text
end = 100000000000
```

the expected file size is:

```text
100000000064 bytes
approximately 93.13 GiB
```

## Progress

The intended build progress output is:

```text
[progress] xx.xx% (done/total)
```

The intended interval is approximately:

```text
5 seconds
```

## Reproducibility cautions

```text
1. Extract the zip before running build.bat.
2. Rebuild after replacing main.c.
3. Do not mix old executables with new source files.
4. Do not mix old index files with a new magic/version.
5. Do not open index.bin in spreadsheet or editor software while building.
6. Keep enough disk space for the full index.
```

## Legacy validation scripts

Earlier tau-sync validation notes used scripts such as full-sync validation,
M=8388608 comparison, and actual-value export scripts.

Those script names should not be shown as the current quick-start path unless
the scripts are present in the repository.  If they are restored later, this
file can add exact script names again.

## Current included implementation

The current implementation folder is included at repository root.

```text
tau_c0c2cm_index/
  README.txt
  build.bat
  main.c
```

Build from that folder:

```bat
cd tau_c0c2cm_index
build.bat
```

Create a small test index:

```bat
tau_index.exe build 60000000 index.bin 8 1024
```

Query one value:

```bat
tau_index.exe query index.bin 10000
```

Twin-prime search:

```bat
tau_index.exe twins index.bin 1 60000000 10
```

The implementation folder is the active path for the current package.  Historical
validation script names should not be treated as active commands unless those
files are restored.
