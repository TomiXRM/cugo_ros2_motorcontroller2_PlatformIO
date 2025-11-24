# Cugo ROS2 Motor Controller 2 - Documents

このドキュメントは、Cugo ROS2 Motor Controller 2プロジェクトの設計と実装について説明します。

## ドキュメント構成

- **[README.md](README.md)** - このファイル。プロジェクト概要
- **[architecture.md](architecture.md)** - アーキテクチャ設計とMermaid図
- **[usage.md](usage.md)** - 使い方とセットアップ手順
- **[debug.md](debug.md)** - デバッグ方法とトラブルシューティング

## プロジェクト概要

このプロジェクトは、ROS2の`Twist`コマンドをUSBシリアル経由で受信し、2輪差動駆動ロボットのモーターを制御するマイコンファームウェアです。ホイールオドメトリーも計算してROS2に送信します。

### 主な特徴

1. **インターフェイス抽象化**
   - モーター制御を抽象インターフェイス(`IMotorController`)で統一
   - CugoSDKと汎用DCモータを簡単に切り替え可能

2. **デバッグ機能**
   - Serial2 (UART1, GP20)を使った専用のデバッグ出力
   - ROS2通信中でもprintfデバッグが可能

3. **安全機能**
   - フェイルセーフ機構（通信断で自動停止）
   - RPMクランプ（回転優先アルゴリズム）
   - チェックサムによる通信データ検証

4. **ROS2統合**
   - PacketSerialによる高信頼性通信
   - Twist/Odomメッセージ対応

## 🔧 対応ハードウェア

### モーターシステムA: CugoSDK (デフォルト)

- **モータードライバー**: LD2 (Serial1, UART0通信)
- **モーター**: BLDC × 2
- **エンコーダー**: LD2経由で取得

### モーターシステムB: 汎用DCモータ + エンコーダ

- **モータードライバー**: L298N、DRV8833など
- **モーター**: DCモーター × 2
- **エンコーダー**: インクリメンタルエンコーダー (A/B相)
- **制御方式**: PWM + PIDフィードバック制御

## 📁 プロジェクト構造

```shell
cugo_ros2_motorcontroller2/
├── src/
│   └── main.cpp                    # メインプログラム
├── lib/
│   ├── CugoSDK/                    # Cugo専用SDKライブラリ
│   │   ├── CugoSDK.h
│   │   └── CugoSDK.cpp
│   ├── DebugPrint/                 # デバッグ出力ライブラリ
│   │   └── DebugPrint.h
│   └── MotorController/            # モーター制御抽象化レイヤー
│       ├── IMotorController.h      # インターフェイス定義
│       ├── CugoMotorController.h   # Cugo実装
│       ├── CugoMotorController.cpp
│       ├── GenericMotorController.h # 汎用実装
│       └── GenericMotorController.cpp
├── docs/                           # ドキュメント
│   ├── README.md
│   ├── architecture.md
│   ├── usage.md
│   └── debug.md
└── platformio.ini                  # PlatformIO設定
```

## クイックスタート

### 1. ビルド

```bash
cd cugo_ros2_motorcontroller2
pio run
```

### 2. 書き込み

```bash
pio run --target upload
```

### 3. デバッグモニター接続

Serial2 (UART1, GPIO20) に115200baudでシリアルモニターを接続すると、デバッグログが表示されます。

詳細は [usage.md](usage.md) を参照してください。

## アーキテクチャ

このプロジェクトは、以下の3層アーキテクチャで設計されています：

1. **Application Layer** (`main.cpp`)
   - ROS2プロトコル処理
   - RPMクランプロジック
   - フェイルセーフ機構

2. **Abstraction Layer** (`IMotorController`)
   - モーター制御の抽象インターフェイス
   - 実装の切り替えが容易

3. **Implementation Layer**
   - `CugoMotorController`: CugoSDK実装
   - `GenericMotorController`: 汎用DCモータ実装

詳細は [architecture.md](architecture.md) を参照してください。

次のドキュメント: [アーキテクチャ設計 →](architecture.md)
