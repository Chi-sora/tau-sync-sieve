# tau-sync sieve 日本語README

著者: Chisora

ライセンス:
- repositoryの主ライセンス: 文書・理論説明・定義は CC BY 4.0
- ソースコード: `src/` と `scripts/` は MIT

## ライセンス詳細

このrepositoryは、ソースコードと文書でライセンスを分けています。

```text
文書・定義・理論説明: CC BY 4.0
`src/` と `scripts/` のソースコード: MIT
```

root の `LICENSE` には GitHub が主ライセンスとして検出しやすいように CC BY 4.0 legalcode 全文だけを入れています。MIT のソースコードライセンス全文は `CODE-LICENSE-MIT.txt` を見てください。
初回リリース内容と有限検証の要約は `RELEASE_NOTES.md` にあります。



## 概要

`tau-sync sieve` は、Goldbach endpoint と nearend 構造を調べるための算術的な篩フレームワークです。

中心は次の3つです。

```text
tau:
  検証済みの first-hit 位置

lapse:
  tau が出る前の no-hit prefix

sync:
  delta_x = 0 による stream 間の転送規則
```

特に、source 側で Goldbach endpoint が出て、target 側でも同じ `x` を再利用できる場合、target 側の endpoint 上限が得られます。

```text
source G at t
AND valid_t(t,B)
AND delta_x(j_forced,t) = 0
  -> target G at j_forced
  -> tau_g_target <= j_forced
```

## 重要な注意

`mod 6` は tau ではありません。候補を作る residue prefilter です。

```text
mod 6:
  構造層
  q = 6j +/- 1 を作る
  C3 scan exclusion や B, j_forced に使う

tau:
  実際に prime / semiprime / almost-prime などが検証された first-hit
```

この区別により、candidate tau という紛らわしい概念を使わずに済みます。

## 検証状態

このリポジトリは有限範囲の計算検証を含みます。Goldbach予想などの無限範囲命題を証明するものではありません。

含まれる主な検証結果:

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

実値分類:
  prime        = 110734 unique x
  semiprime    = 29151 unique x
  almost_prime = 32479 unique x
```

## Windowsでの実行

必要環境:

```text
Windows 11
MinGW64 gcc
cmd.exe
```

ビルド:

```bat
scripts\build_windows.bat
```

full sync validation:

```bat
scripts\run_full_validation_windows.bat
```

M=8388608 の全行比較:

```bat
scripts\run_m8388608_values_windows.bat
scripts\check_all_rows_windows.bat
```

prime / semiprime / almost_prime の実値CSV出力:

```bat
scripts\export_actual_values_windows.bat
```

## ファイル構成

```text
README.md
README.ja.md
src/
scripts/
data/
docs/
results/
```

## 追加の技術メモ

tau-sync sieve では、次の2つの実行方式を分けています。

* **certificate mode**: 検証済みの tau, class label, lapse, sync, companion certificate を再利用して高速に検証する方式です。対象行が certificate で覆われている場合、`is_prime(x)` を呼びません。
* **parallel tau lanes**: endpoint, lapse, sync, nearend, TWIN などを別々の lane として処理し、すべての tau の着火を互いに待たせない方式です。

詳しくは次を参照してください。

* [docs/08_certificate_mode_and_no_is_prime.md](docs/08_certificate_mode_and_no_is_prime.md)
* [docs/09_parallel_tau_lanes.md](docs/09_parallel_tau_lanes.md)

厳密定義や数式は `docs/` に分割しています。tau の細かい種類は `docs/05_tau_taxonomy.md` にまとめています。CSV列や出力の読み方は `docs/06_outputs_and_labels.md` にあります。分類器、パラメータ、残余ケースは `docs/07_classifier_and_parameters.md` にあります。

## 誤解しやすい点

```text
tau-only certificate mode:
  既に作られた tau / lapse / certificate を使う高速検証

raw build mode:
  tau を最初に作る処理
  ここでは素数判定や分類が必要
```

つまり、tau は初回から魔法のように存在するものではありません。最初に直接判定で作り、その後の検証では certificate として高速に再利用します。

## 著者の一言

God Bless You!
