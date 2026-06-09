# Documentation notes

## Current documentation set

```text
docs/01_definitions.md
docs/02_theory.md
docs/03_validation.md
docs/04_reproducibility.md
docs/05_tau_taxonomy.md
docs/06_outputs_and_labels.md
docs/07_classifier_and_parameters.md
docs/08_packed_index_format.md
docs/09_documentation_notes.md
docs/10_tau_c0c2cm_index.md
docs/11_formal_definitions_c0c2cm.md
```

## Active implementation folder

```text
tau_c0c2cm_index/
```

Active implementation files:

```text
tau_c0c2cm_index/README.txt
tau_c0c2cm_index/build.bat
tau_c0c2cm_index/main.c
```

## Current formal structure

```text
theorem:
  S(x) + Exc(x)

implementation:
  S_C0(x), S_C2(x), S_CM(x)

diagnostics:
  C0/C2/CM/PR
  endpoint states
  lapse and overlap features

packed index:
  P/S/A/O/U stored label
  C0/C2/CM/PR mask
```

## Reference policy

Active references should point only to files that are present in the package.

```text
allowed:
  definitions
  equations
  finite validation summary values
  current implementation folder
  current packed-index format

avoid:
  active links to missing files
  active links to missing folders
```

## Implementation caveats

```text
RES_PR:
  packed-index route marker

P_ROUGH_Pdef:
  predicate using P_def=193
```

Do not state:

```text
RES_PR = P_ROUGH_Pdef
```

unless the implementation explicitly computes and stores the `P_def=193`
boundary.

The current packed record does not fully store:

```text
S_total
sp(x)
Exc(x)
S_K/S_PR/A_K/A_PR
```

Use sidecar data, extra bits/classes, recomputation, or a trusted certificate
table when those values are needed.

## Wording policy

Use explicit status language.

```text
definition:
  notation or convention

established:
  proven under stated preconditions

finite validation:
  checked dataset or checked range

strong evidence:
  sampled or large finite evidence without global proof

not claimed:
  infinite theorem

unknown:
  not established
```

Do not write that a computational check proves an infinite theorem.

## Endpoint derivation documentation

The derivation path is documented in:

```text
docs/11_formal_definitions_c0c2cm.md
```

The intended path is:

```text
tau endpoint scan
  -> endpoint composite marks
  -> C0/C2/CM residue-separated implementation
  -> S_total + Exc classifier
  -> compact packed-index label and mask
```

This path should be preserved when future documentation is edited.

## Lapse and nearend safety documentation

The current package documents lapse and nearend features as diagnostic and audit
features.

```text
lapse:
  downstream of tau_g

nearend tau:
  requires predecessor endpoint/certificate data

window priority:
  may reorder candidates
  must not delete candidates without certificate
```

The detailed safety rules are in:

```text
docs/11_formal_definitions_c0c2cm.md
```
