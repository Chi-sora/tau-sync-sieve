# tau-sync index sieve

著者: Chisora

## 状態

```text
確立:
  有限計算の定義
  算術恒等式
  検証範囲の記述
  packed full-index format specification

主張しない:
  Goldbach予想の証明
  無限範囲の定理
  検証なしに素数事実を作ること
```

## 目的

`tau-sync index sieve` は、次の2層を文書化します。

```text
tau-sync arithmetic layer:
  q_s(j)
  x_s(N,j)
  tau
  lapse
  sync
  valid_t
  certificates
  no-left-behind rule

packed index layer:
  1 から end までの full-index file
  1整数あたり1 byte
  P/S/A/O/U labels
  C0/C2/CM/PR mask
  stored label と query resolved label
```

mathematical layer は監査と推論のための層です。packed index layer は有限範囲の
lookup と再現可能な計算のための層です。

## 中心的注意

```text
mod 6:
  structural residue layer
  tau ではない

tau:
  verified first-hit fact
```

residue candidate は prime fact ではなく、tau value でもありません。

## 予定command interface

コードはあとで追加します。予定している Windows command interface は次です。

```bat
build.bat
tau_index.exe build <end> <index.bin> [threads] [chunk_mb]
tau_index.exe query <index.bin> <n>
tau_index.exe twins <index.bin> <start> <end> [count]
```

packed index の範囲は次です。

```text
1 <= n <= end
```

これは full index です。segmented sieve ではありません。

## packed labels

```text
P:
  prime

S:
  semiprime
  Omega(n) = 2

A:
  almost-prime class
  Omega(n) >= 3

O:
  outside arithmetic domain

U:
  unresolved stored label
```

`AlmostPrimeClass` は project-local terminology です。

```text
AlmostPrimeClass(n):
  n は composite
  かつ n は semiprime ではない
```

## packed record

```text
record_size:
  1 byte per integer

bits 0..2:
  stored label

bits 3..6:
  C0/C2/CM/PR mask

bit 7:
  reserved
```

stored label と query で表示される label は意図的に分けます。

```text
stored_label:
  build時に書かれるlabel

resolved_label:
  query時に factor 2 / factor 3 を剥がして表示されるlabel
```

## 文書

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
```

## ライセンス

```text
文書・定義・理論説明:
  CC BY 4.0

source code:
  MIT
```

参照:

```text
LICENSE
CODE-LICENSE-MIT.txt
```

## 同梱実装フォルダ

このパッケージには、現在の Windows/MinGW 用 index builder フォルダを同梱しています。

```text
tau_c0c2cm_index/
  README.txt
  build.bat
  main.c
```

現在の packed full-index 実装はこのフォルダを使います。

```bat
cd tau_c0c2cm_index
build.bat
tau_index.exe build 60000000 index.bin 8 1024
tau_index.exe query index.bin 10000
tau_index.exe twins index.bin 1 60000000 10
```

このコードフォルダは repository root 直下に置きます。このパッケージでは
`src/` や `scripts/` の下には置きません。

## 著者の一言

God Bless You!
