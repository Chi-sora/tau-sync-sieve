# Documentation notes

## Preservation policy

This revision preserves the original detailed docs as much as possible.

```text
kept:
  01_definitions
  02_theory
  03_validation
  04_reproducibility
  05_tau_taxonomy
  06_outputs_and_labels
  07_classifier_and_parameters
  08_certificate_mode_and_no_is_prime
  09_parallel_tau_lanes

added:
  10_packed_index_format
  11_documentation_notes
```

## Reference policy

Active references should point only to files that are present in the repository.

```text
allowed:
  definitions
  equations
  finite validation summary values
  planned command interface
  historical validation descriptions

avoid:
  active links to missing scripts
  active links to missing CSV files
  active links to missing source files
```

If historical results are important, keep their numeric summaries but do not
claim that missing files are included.

## Code policy

Code is not included in this documentation package.

When code is added later, update:

```text
README.md
README.ja.md
docs/04_reproducibility.md
docs/10_packed_index_format.md
```

with the exact command output and index version.

## Wording policy

Use explicit status language.

```text
definition:
  notation or convention

finite validation:
  checked dataset or checked range

not claimed:
  infinite theorem

unknown:
  not established
```

Do not write that a computational check proves an infinite theorem.

## Implementation caveat for future code

When code is added, do not describe `RES_PR` as equivalent to the historical
`P_ROUGH(x)` predicate unless the implementation explicitly uses the
`P_def=193` boundary.

Correct wording:

```text
RES_PR is a packed-index route marker.
P_ROUGH(x) is defined by P_def=193.
```

## Included code-folder reference policy

This package includes one active implementation folder.

```text
tau_c0c2cm_index/
```

Active references to this folder are allowed because it is included in this ZIP.

The package still avoids active references to missing folders such as:

```text
src/
scripts/
results/
data/
```

unless those folders are later restored.
