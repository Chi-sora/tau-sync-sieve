# Reproducibility guide

## Environment

Recommended environment:

```text
Windows 11
MinGW64 gcc
cmd.exe
```

No Python is required for the included C checker path.

## Build all tools

```bat
scripts\build_windows.bat
```

This creates executables under:

```text
build\
```

## Full sync validation

```bat
scripts\run_full_validation_windows.bat
```

Output:

```text
results\tau_sync_sieve_full_results.csv
```

## M=8388608 all-row comparison

```bat
scripts\run_m8388608_values_windows.bat
scripts\check_all_rows_windows.bat
```

Expected checker values:

```text
rows_ok = 1
class_mismatch = 0
twin_mismatch = 0
ok = 1
```

## Export actual value CSV files

```bat
scripts\export_actual_values_windows.bat
```

Outputs under `results\`:

```text
m8388608_actual_values_unique_all.csv
m8388608_actual_values_prime_values.csv
m8388608_actual_values_semiprime_values.csv
m8388608_actual_values_almost_prime_values.csv
m8388608_actual_values_summary.csv
```

## Data files

Certificate data:

```text
data\certificates\tau_only_classification_rows.csv
data\certificates\twin_companion_certificate.csv
```

Input data:

```text
data\inputs\test_input.csv
data\inputs\base_external_validation_inputs.csv
```

## Reproducibility cautions

1. Do not mix old executables with new source files. Rebuild before running.
2. Do not treat mod 6 residue candidates as verified prime tau.
3. Certificate mode is fast because verified facts are already available.
4. Raw build mode and certificate mode answer different performance questions.
5. If a CSV is opened in spreadsheet software while a program writes to it, Windows may lock the file.
