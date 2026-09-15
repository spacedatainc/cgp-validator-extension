# CGP Validator Extension

このプロジェクトは、USD プラグイン `CGPValidator` で登録された `UsdValidator` を、Omniverse の `AssetValidator` で利用できるようにするための extension です。

`CGPValidator` 自体が実際の検査ロジックを持っており、この extension はその validator を OpenUSD / Omniverse 側から認識・利用可能にするための配線役です。つまり、どんなチェックが行われるかは `CGPValidator` の実装次第であり、この extension はその登録と配布を担います。

現在の `CGPValidator` の実装では、次のような検査が含まれています。

- Windows のドライブレターを含む asset path の検出
- RTX の OCIO config パスが想定値と一致しているかの検出

> 注意: このリポジトリは NVIDIA の OpenUSD Plugin Samples を参考にして作成されています。ベース構成やビルドの考え方はそのサンプルを踏まえていますが、実装と用途はこのプロジェクト向けに調整しています。

## 対応環境

- Linux のみ対応
- Windows はまだ未対応

このリポジトリでは Linux 向けのビルドスクリプトと CMake 設定が中心になっており、Windows 向けの build.bat や setenvwindows などはまだ整備されていません。

## ビルド方法

このプロジェクトのビルドは、ルートにある `build.sh` を使います。

```bash
./build.sh
```

## インストール方法

ビルドが完了したら、生成された extension を Kit テンプレートの `source/extensions` 配下へ配置します。

```bash
cp -r install/spacedata.cgp_validator_extension /path/to/KitTemplate/source/extensions/
```

`/path/to/KitTemplate` はあなたの Kit テンプレートのルートパスに置き換えてください。

その後、Kit テンプレートのルートで次を実行して extension を組み込みます。

```bash
./repo.sh build
```

これにより、Omniverse の Kit ビルドにこの extension が取り込まれます。

## Omniverse向けのビルド方法

この extension は、Omniverse 109.03 を対象にしてビルドする前提です。

この Omniverse 109.03 では、NVIDIA が提供している USD のバージョン `0.25.02.kit.8-gl.16788+c1c423f2` を使っています。つまり、この extension をあなたの Omniverse 環境に合わせてビルドするには、対象の kit で使っている USD の package 名と version を確認し、それに合わせてリポジトリ内の依存定義を更新する必要があります。

特定の Omniverse の USD 名や version は、KitTemplate のビルド後に生成される `KitTemplate/_build/linux-x86_64/release/kit/dev/all-deps.packman.xml` に記載されています。このファイルを確認して、実際に使用している USD package を見つけます。

その後、[deps/usd-deps.packman.xml](deps/usd-deps.packman.xml) を書き換えて、同じ package 名と version を指定してビルドします。

例:

```xml
<project toolsVersion="5.6">
  <dependency name="usd-release" linkPath="../_build/usd-deps/usd">
      <package name="usd.py312.manylinux_2_35_x86_64.stock.release" version="0.25.02.kit.8-gl.16788+c1c423f2" />
  </dependency>
</project>
```

このように、Kit のビルド生成物に書かれた USD の依存情報をそのままこのリポジトリの [deps/usd-deps.packman.xml](deps/usd-deps.packman.xml) に反映したうえで、次のコマンドでビルドします。

```bash
./build.sh
```

この手順を踏むことで、あなたの Omniverse 109.03 環境と同じ USD バージョンを使った extension をビルドできます。

## ビルドの前提

- CMake 3.23.1 以上
- Linux 環境

## 参考

- NVIDIA OpenUSD Plugin Samples
- OpenUSD validation framework
- Kit extension packaging
