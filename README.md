# 軽量ファイラー

Windows、Debian、Ubuntuを対象にした、Rust + Qt 6の2ペインファイラーです。

## 技術構成

- **Rust**: ファイルシステム処理とアプリケーションロジック
- **Qt 6 Widgets**: クロスプラットフォームのデスクトップUI
- **CXX**: RustとQt/C++間のFFI
- **CMake**: アプリとRustライブラリの統合ビルド

初回起動はHomeワークスペースのみ、1ペインで開きます。メインワークスペースタブはアイコンと名前を横に並べたスクロール可能なバーに表示します。必要に応じて左右/上下に分割でき、アクティブなペインを枠線で示します。左のワークスペース名は右クリックから変更できます。ペイン内の場所タブにはフォルダ名を表示し、タブ列の＋で現在のフォルダを開くタブを追加できます。ワークスペース名や場所タブ、分割方向・幅、選択中のタブは変更時に保存し、次回起動時に復元します。

ウィンドウタイトルは **Lightweight File Manager** です。ファイル一覧は単クリックで選択し、フォルダはダブルクリックまたはEnterで開きます。ファイルはダブルクリックまたはEnterで既定のアプリから開きます。右クリックメニューと編集メニューで、新しいフォルダー（Ctrl+Shift+N）、名前変更（F2）、削除（Delete）、コピー/切り取り/貼り付け（Ctrl+C/X/V）ができます。削除はごみ箱を経由せず完全削除です。

表示メニューまたはツールバーの更新アイコン、F5キーで一覧を更新します。ファイルメニューやフォルダー一覧の右クリックから、「ターミナルを開く」を選ぶと、その場所を作業ディレクトリとして開きます。「Explorerを開く」では現在のフォルダーをExplorerで表示できます。WindowsではWindowsの既定ターミナル設定に従って新しいコンソールを開きます。ヘルプメニューの「バージョン情報」でアプリのバージョンを確認できます。アプリ、フォルダー、ファイルに専用アイコンを使用します。一覧取得は別スレッドで行い、常時ファイル監視はしません。

## Windowsでの環境構築

以下はWindows x64で、QtのMSVC版とRust MSVC版を組み合わせる手順です。Visual Studio IDEは不要で、Build Toolsだけでビルドできます。

### 1. Visual Studio 2022 Build Tools

[Visual Studio 2022 Build Toolsをダウンロード](https://aka.ms/vs/17/release/vs_BuildTools.exe)し、インストーラーで次を選択します。

- ワークロード: **C++によるデスクトップ開発**
- MSVC x64/x86ビルドツール（v143）
- Windows 10またはWindows 11 SDK

インストール後、スタートメニューから**x64 Native Tools Command Prompt for VS 2022**または**Developer PowerShell for VS 2022**を開きます。通常のPowerShellではMSVC環境が設定されません。

既にPowerShellを開いている場合、次のスクリプトでもx64環境を有効にできます。

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" `
  -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
```

`cl`を実行し、出力に`for x64`と表示されることを確認します。

### 2. Qt 6 MSVC 2022 x64

[Qt Online Installer for Windows x64](https://www.qt.io/development/download-qt-installer-oss)を入手し、Qtアカウントでログインして、**Qt 6.12.x → MSVC 2022 64-bit**をインストールします。Qt Creatorは任意です。

**MinGW 64-bit版は選ばないでください。** MSVCコンパイラーおよびRustのMSVCツールチェーンとは別のコンパイラー用パッケージです。インストール先の例:

```text
C:\Qt\6.12.0\msvc2022_64
```

Qt Maintenance Toolから後で追加することもできます。Qtのオープンソースライセンスを利用する際は、LGPL/GPLの条件を確認してください。

### 3. Rust stable (MSVC)

[Rust公式インストールページ](https://rust-lang.org/tools/install/)からWindows x64用`rustup-init.exe`を入手し、stable toolchainをインストールします。

```powershell
rustc -vV
```

出力の`host`が`x86_64-pc-windows-msvc`であることを確認します。

### 4. CMake

[CMake公式ダウンロード](https://cmake.org/download/)からWindows x64 Installerを入手してインストールします。CMakeは3.24以上が必要です。

Strawberry Perl等に同梱された別の`cmake.exe`がPATHの先にある場合があるため、下記ビルド例では公式版をフルパスで呼び出します。標準のインストール先の例:

```text
C:\Program Files\CMake\bin\cmake.exe
```

Ninjaは不要です。Visual StudioのNMakeを使います。

## Windowsでビルド

Developer PowerShell for VS 2022（x64）を開いて実行します。Qtのバージョンやインストール先が異なる場合は、`CMAKE_PREFIX_PATH`を実際のQt MSVCキットのルートに合わせてください。

プロジェクトフォルダーはASCII名の`LightweightFileManager`を使い、ビルド成果物もその配下に置きます。既に日本語名のフォルダーを開いている場合は、Codexで開いている間はフォルダー自体を改名できないため、プロジェクトを次の場所へコピーしてください。

```powershell
cd "C:\Users\<ユーザー名>\Documents\ChatGPT\LightweightFileManager"

$buildDir = Join-Path $PWD "build-windows-x64"

& "C:\Program Files\CMake\bin\cmake.exe" `
  -S . -B $buildDir `
  -G "NMake Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/msvc2022_64"

& "C:\Program Files\CMake\bin\cmake.exe" `
  --build $buildDir
```

Rust依存クレートは初回ビルド時にCargoがcrates.ioから取得します。

### 起動

```powershell
$env:PATH = "C:\Qt\6.12.0\msvc2022_64\bin;$env:PATH"
& "$PWD\build-windows-x64\app\lightweight-file-manager.exe"
```

## Windowsでのトラブルシューティング

- **`Qt6Config.cmake`が見つからない**: Qtの`mingw_64`ではなく`msvc2022_64`を指定してください。設定ファイルは通常`<Qtキットのルート>\lib\cmake\Qt6\Qt6Config.cmake`にあります。
- **`nmake`が見つからない**: 通常のPowerShellではなく、x64向けDeveloper PowerShell/Command Promptを使ってください。
- **`cl`が`for x86`と表示される**: x64 Native Toolsまたは`Launch-VsDevShell.ps1 -Arch amd64 -HostArch amd64`で環境を初期化してください。
- **CMakeのコンパイラー確認でリンクエラーになる**: CMake 3.29.2がStrawberryのものになっていないか確認し、公式CMakeをフルパスで呼び出してください。ビルド先はASCIIの短いパスにします。
- **CXX生成ヘッダーが見つからない/名前空間エラー**: 現在のCMake設定とソースにはCargo生成先（`cxxbridge/fm-core/src/lib.rs.h`）およびCXXの公開関数名が反映されています。古いビルド先ではなく、上記のコマンドで再構成してください。

## Debian / Ubuntuでビルド

Rust stable、CMake 3.24以上、C++20コンパイラー、Qt 6 Widgets/Concurrentの開発パッケージを用意します。

```sh
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
```

Qtのインストール場所をCMakeが認識できる状態で、次のようにビルドします。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Windows配布版

`release/LightweightFileManager-windows-x64.zip` をダウンロードして展開してください。QtのDLLとプラグイン、およびMicrosoft Visual C++ランタイムの公式インストーラーを同梱しています。フォルダー内の `lightweight-file-manager.exe` を起動し、ランタイム不足が表示された場合は `vc_redist.x64.exe` を実行してください。DLLやサブフォルダーはexeと同じ配布フォルダーに置いたまま使用してください。
