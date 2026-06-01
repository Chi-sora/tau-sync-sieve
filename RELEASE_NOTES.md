# Release Notes

## v0.1.0 - Initial repository release

Author: Chisora

### Summary

This initial release introduces the tau-sync sieve framework: a sieve-assisted arithmetic framework based on tau stopping times, lapse prefixes, and sync transfer rules for finite Goldbach endpoint validation.

### Included

- Strict definitions for verified tau values, lapse prefixes, g-sync, valid_t, the mod 6 layer, TWIN companion certificates, and the project-local prime / semiprime / almost-prime classification.
- Mathematical notes for delta_x = 0 transfer, source-to-target endpoint bounds, lapse decomposition, endpoint-late evaluation, latent / observed sync, and the dependency graph.
- Reproducible C sources and Windows cmd scripts for validation and value export.
- Validation summaries for full sync validation, actual value checks, Goldbach HIT values, and TWIN companion validation.
- Separate licenses:
  - MIT License for source code.
  - Creative Commons Attribution 4.0 International (CC BY 4.0) for documentation, definitions, theory text, and explanatory materials.

### Verified finite checks included in this release

- Full sync validation:
  - source_cases = 335544320
  - sync_slots = 200371531
  - sync_fail = 0
  - delta_fail = 0
  - invalid_j = 0
  - target_direct_fail = 0
- M=8388608 actual-value classification:
  - prime unique x = 110734
  - semiprime unique x = 29151
  - almost_prime unique x = 32479
  - mismatch = 0
- TWIN companion certificate validation:
  - twin_true among prime x = 7045
  - twin mismatch = 0

### Scope and limitations

This release contains finite computational validations. It does not claim a proof of the Goldbach conjecture or any infinite-range theorem. Residue candidates from the mod 6 layer are not prime tau. A tau is recorded only after the relevant prime or class fact is verified or supplied by a certificate.

### Recommended GitHub topics

number-theory, goldbach, sieve, prime-numbers, semiprimes, twin-primes, arithmetic, computational-number-theory, reproducible-research, c-language, tau-sync-sieve
