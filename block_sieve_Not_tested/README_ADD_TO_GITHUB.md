# block_sieve_addition

[established]
Flat package for adding the block-sieve candidate generator to GitHub.

Files:

```text
tau_gen_candidates_block.c
build_block_sieve.bat
run_block_sieve.bat
run_block_sieve_noemit.bat
block_sieve_src_note.md
README_ADD_TO_GITHUB.md
```

Suggested placement:

```text
tau_gen_candidates_block.c -> src/
build_block_sieve.bat -> scripts/ or project root
run_block_sieve.bat -> scripts/ or project root
run_block_sieve_noemit.bat -> scripts/ or project root
block_sieve_src_note.md -> docs/
```

Build:

```bat
build_block_sieve.bat
```

Run:

```bat
run_block_sieve.bat
```

No-output speed test:

```bat
run_block_sieve_noemit.bat
```
