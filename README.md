# tau-sync sieve

Author: Chisora

License:
- Documentation and theory text: CC BY 4.0
- Source code: MIT

## What this repository contains

`tau-sync sieve` is a sieve-assisted arithmetic framework for studying finite Goldbach endpoint data through three linked objects:

```text
tau   = verified first-hit time
lapse = no-hit prefix before a tau
sync  = arithmetic transfer across streams by delta_x = 0
```

The main idea is that a verified source endpoint can force a target endpoint when the same `x` value is reused across streams:

```text
source G at t
AND valid_t(t,B)
AND delta_x(j_forced,t) = 0
  -> target G at j_forced
  -> tau_g_target <= j_forced
```

This repository is organized to avoid ambiguity between residue candidates and verified prime facts:

```text
mod 6 layer:
  structural residue prefilter, not a tau

tau layer:
  verified first-hit facts only
```

## Status

The project contains finite computational validations. It does not claim a proof of the Goldbach conjecture or of any infinite-range theorem. The verified claims are scoped to the included datasets and scripts.

Main checked results included here:

```text
Full sync validation:
  source_cases = 335544320
  sync_slots   = 200371531
  sync_fail    = 0
  delta_fail   = 0
  invalid_j    = 0

M=8388608 value comparison:
  class_mismatch = 0
  twin_mismatch  = 0

Actual value classes:
  prime        = 110734 unique x
  semiprime    = 29151 unique x
  almost_prime = 32479 unique x
```

See `docs/03_validation.md` for validation details and `docs/05_tau_taxonomy.md` for the full tau taxonomy.

## Quick start on Windows

Requirements:

```text
Windows 11
MinGW64 gcc in PATH
cmd.exe
```

Build:

```bat
scripts\build_windows.bat
```

Run the full sync validation:

```bat
scripts\run_full_validation_windows.bat
```

Run the M=8388608 all-row comparison:

```bat
scripts\run_m8388608_values_windows.bat
scripts\check_all_rows_windows.bat
```

Export actual values for prime, semiprime, and almost-prime classes:

```bat
scripts\export_actual_values_windows.bat
```

## Repository layout

```text
README.md
README.ja.md
LICENSE.md
LICENSE-CODE-MIT.txt
LICENSE-DOCS-CC-BY-4.0.txt
CITATION.cff
ACKNOWLEDGEMENTS.md
src/
  tau_sync_sieve_full_validation.c
  tau_m8388608_benchmark.c
  actual_values_exporter.c
  check_all_rows_results.c
scripts/
  build_windows.bat
  run_full_validation_windows.bat
  run_m8388608_values_windows.bat
  check_all_rows_windows.bat
  export_actual_values_windows.bat
data/
  inputs/
  certificates/
docs/
  01_definitions.md
  02_theory.md
  03_validation.md
  04_reproducibility.md
  05_tau_taxonomy.md
  06_outputs_and_labels.md
  07_classifier_and_parameters.md
results/
  summary CSV files
```

## License details

This repository uses a dual-license layout:

```text
source code: MIT
documentation and theory text: CC BY 4.0
```

See `LICENSE.md`, `LICENSE-CODE-MIT.txt`, and `LICENSE-DOCS-CC-BY-4.0.txt`.


## Documentation

- `docs/01_definitions.md`: strict definitions and notation
- `docs/02_theory.md`: tau-sync sieve equations and interpretation
- `docs/03_validation.md`: validation results and what they mean
- `docs/04_reproducibility.md`: reproducible build and run instructions
- `docs/05_tau_taxonomy.md`: complete tau taxonomy and label definitions
- `docs/06_outputs_and_labels.md`: CSV output and label interpretation guide
- `docs/07_classifier_and_parameters.md`: classifier flow, parameters, and residual cases

## Important terminology

```text
prime number:
  n > 1 and divisors(n) = {1,n}

semiprime:
  x = p*q where p and q are prime; p may equal q

almost_prime in this repository:
  composite and not semiprime

Goldbach endpoint:
  N = q + x, q prime, x prime

TWIN companion certificate:
  x prime and x+2 prime
```

## What is not claimed

This project does not claim:

```text
1. a proof for all even integers;
2. that tau removes the need for primality tests during first raw construction;
3. that candidate residues are prime tau;
4. that certificate mode is the same as raw build mode.
```

Correct distinction:

```text
raw build mode:
  creates verified tau/class facts from direct tests and classification

certificate mode:
  reuses verified tau/lapse/class/certificate facts for fast validation
```

## Author note

God Bless You!
