# 使い方

このドキュメントでは、プロジェクトのセットアップ、ビルド、デプロイ、設定変更の方法を説明します。

## 目次

1. [必要な環境](#必要な環境)
2. [セットアップ](#セットアップ)
3. [ビルドと書き込み](#ビルドと書き込み)
4. [モーターシステムの切り替え](#モーターシステムの切り替え)
5. [ピン設定のカスタマイズ](#ピン設定のカスタマイズ)
6. [デバッグ出力の有効化/無効化](#デバッグ出力の有効化無効化)
7. [ROS2との接続](#ros2との接続)

---

## 必要な環境

### ソフトウェア

- **PlatformIO**: VSCode拡張機能またはCLI
- **Python 3.x**: PlatformIOのため
- **Git**: ライブラリの依存関係管理のため

### ハードウェア

- **Raspberry Pi Pico** または **Pico W**
- **USB Type-Cケーブル**: プログラム書き込みとROS2通信用
- **USB-シリアル変換器** (3.3V): デバッグ出力用（オプション）

#### モーターシステムA: CugoSDK使用時
- **LD2 モータードライバー**
- **BLDC モーター × 2** (エンコーダー付き)

#### モーターシステムB: 汎用DCモータ使用時
- **DCモータードライバー** (L298N、DRV8833など)
- **DCモーター × 2**
- **インクリメンタルエンコーダー × 2** (A/B相)

---

## セットアップ

### 1. リポジトリのクローン

```bash
git clone <repository-url>
cd cugo_ros2_motorcontroller2
```

### 2. PlatformIOのインストール

#### VSCode拡張機能を使用する場合

1. VSCodeを開く
2. 拡張機能タブで "PlatformIO IDE" を検索してインストール
3. VSCodeを再起動

#### CLIを使用する場合

```bash
pip install platformio
```

### 3. 依存ライブラリの自動インストール

PlatformIOは`platformio.ini`に記載された依存ライブラリを自動的にインストールします：

- **PacketSerial**: シリアル通信のパケット化
- **RPI_PICO_TimerInterrupt**: タイマー割り込み

---

## ビルドと書き込み

### 方法1: VSCodeのGUIを使用

1. VSCodeでプロジェクトフォルダを開く
2. 下部のPlatformIOツールバーで以下を実行：
   - **ビルド**: チェックマークアイコン
   - **書き込み**: 矢印アイコン
   - **シリアルモニター**: プラグアイコン

### 方法2: コマンドラインを使用

```bash
# ビルド
pio run

# 書き込み
pio run --target upload

# シリアルモニター（ROS2通信ポート）
pio device monitor
```

### 書き込み時の注意

Raspberry Pi Picoを**BOOTSELボタンを押しながら接続**すると、RP2040がUSBマスストレージデバイスとして認識されます。PlatformIOはこれを自動検出して書き込みます。

---

## モーターシステムの切り替え

### CugoSDKを使用する場合（デフォルト）

`src/main.cpp`の以下の行を**そのまま**にします：

```cpp
#define USE_CUGO_SDK
```

プロダクトIDを設定（0: V4, 1: V3i）：

```cpp
motorController = new CugoMotorController(0);  // V4の場合
// motorController = new CugoMotorController(1);  // V3iの場合
```

### 汎用DCモータを使用する場合

#### 1. `src/main.cpp`の修正

`#define USE_CUGO_SDK`をコメントアウト：

```cpp
// #define USE_CUGO_SDK
```

ヘッダーのインクルードを変更：

```cpp
#include "GenericMotorController.h"  // コメント解除
```

#### 2. ピン番号の設定

`setup()`関数内で、実際のハードウェアに合わせてピン番号を設定：

```cpp
#else
  Log.infoln("Using Generic Motor Controller");
  auto* genericMotor = new GenericMotorController(
    2,  // left_pwm_pin - 左モーターPWMピン
    3,  // left_dir_pin - 左モーター方向ピン
    4,  // left_enc_a_pin - 左エンコーダーA相
    5,  // left_enc_b_pin - 左エンコーダーB相
    6,  // right_pwm_pin - 右モーターPWMピン
    7,  // right_dir_pin - 右モーター方向ピン
    8,  // right_enc_a_pin - 右エンコーダーA相
    9,  // right_enc_b_pin - 右エンコーダーB相
    300.0f  // max_rpm - 想定最大RPM
  );
  GenericMotorController::DriverConfig config{};
  config.mechanical_ppr = 13.0f;
  config.gear_ratio = 45.0f;
  config.control_interval_us = 1000;  // 制御周期(us)
  config.invert_direction = false;
  config.pid_kp = 6.0f;
  config.pid_ki = 4.0f;
  config.pid_kd = 0.0f;
  config.pwm_frequency_hz = 100000;   // 100kHz
  config.max_duty = 1.0f;
  config.deadband = 0.05f;
  config.max_rpm = 300.0f;
  config.feedforward_gain = 1.0f;
  config.velocity_alpha = 0.3f;

  genericMotor->applyDriverConfigSymmetric(config);
  motorController = genericMotor;
#endif
```

#### 3. 制御パラメータの調整

`GenericMotorController::DriverConfig`を左右それぞれに適用することで、PPR・ギヤ比・制御周期・正逆転に加え、PIDゲインやPWM設定、デッドバンド、ローパス係数なども一括で渡せます。左右同一構成であれば`applyDriverConfigSymmetric()`を使うと記述が簡潔になります。ログを確認しながら`config.pid_k*`や`config.deadband`、`config.velocity_alpha`などの値を調整してください。

---

## ピン設定のカスタマイズ

### Raspberry Pi Picoのピン配置

```
        ┌─────────────┐
    GP0 │1    USB   40│ VBUS
    GP1 │2          39│ VSYS
    GND │3          38│ GND
    GP2 │4          37│ 3V3_EN
    GP3 │5          36│ 3V3(OUT)
    GP4 │6          35│ ADC_VREF
    GP5 │7          34│ GP28
    GND │8          33│ GND
    GP6 │9          32│ GP27
    GP7 │10         31│ GP26
    GP8 │11         30│ RUN
    GP9 │12         29│ GP22
    GND │13         28│ GND
   GP10 │14         27│ GP21
   GP11 │15         26│ GP20
   GP12 │16         25│ GP19
   GP13 │17         24│ GP18
    GND │18         23│ GND
   GP14 │19         22│ GP17
   GP15 │20         21│ GP16
        └─────────────┘
```

### デフォルトのピンアサイン

#### CugoSDK使用時
- **GP0 (UART0 TX)**: LD2通信 TX
- **GP1 (UART0 RX)**: LD2通信 RX
- **GP20 (UART1 TX)**: デバッグ出力
- **USB CDC**: ROS2通信

#### 汎用DCモータ使用時（例）
- **GP20 (UART1 TX)**: デバッグ出力
- **GP2**: 左モーター PWM
- **GP3**: 左モーター DIR
- **GP4**: 左エンコーダー A相
- **GP5**: 左エンコーダー B相
- **GP6**: 右モーター PWM
- **GP7**: 右モーター DIR
- **GP8**: 右エンコーダー A相
- **GP9**: 右エンコーダー B相
- **USB CDC**: ROS2通信

※ Serial1(UART0, GP0/GP1)はLD2通信に使用しないため、他の用途に利用可能

---

## デバッグログの設定

デバッグログはArduinoLogライブラリ経由で出力します。初期化ロジックは`lib/DebugPrint/DebugPrint.h`にまとめられており、`debugInit()`を呼び出すことでSerial2(UART1)へログを流す設定が行われます。

### 有効化（デフォルト）

```cpp
#define DEBUG_ENABLED 1
```

`DEBUG_ENABLED`が`1`のとき、`debugInit()`は`Log.begin(LOG_LEVEL_VERBOSE, &Serial2)`を実行し、ログレベルプレフィックスを設定します。`Log.trace()`から`Log.fatalln()`までの呼び出しが利用可能です。

### 無効化

```cpp
#define DEBUG_ENABLED 0
```

無効化すると`debugInit()`は空実装となり、`Log.*`呼び出しも出力されません（シリアルに何も送信されません）。ファームウェアサイズ削減や通信負荷を抑えたいときに切り替えてください。

### ログ出力の例

```cpp
#include "DebugPrint.h"  // debugInit() と ArduinoLog の初期化を提供

void myFunction() {
  Log.noticeln("Function started");

  int value = 42;
  Log.notice("Value: %d", value);

  float rpm = 100.5f;
  Log.traceln("RPM: %.2f", rpm);
}
```

レベルに応じて`Log.errorln()`, `Log.warningln()`, `Log.infoln()`などを使い分けると、シリアルモニタでのフィルタリングが容易になります。改行を付けたい場合は`xxxln()`系、連結して表示したい場合は`xxx()`系のAPIを使用してください。

---

## ROS2との接続

### ハードウェア接続

1. Raspberry Pi PicoをUSBケーブルでPCに接続
2. デバイスが`/dev/ttyACM0`などとして認識されることを確認

```bash
ls /dev/ttyACM*
```

### ROS2パッケージの起動

このファームウェアは、ROS2の`cugo_ros2_control2`などのパッケージと連携します。

```bash
# ROS2ワークスペースに移動
cd ~/ros2_ws
source install/setup.bash

# コントローラーノードを起動
ros2 launch cugo_control cugo_control.launch.py
```

### 通信プロトコル

#### 受信パケット（ROS2 → マイコン）

```
┌─────────────────────────────────────────┐
│ Header (8 bytes)                        │
├─────────────────────────────────────────┤
│ - Product ID (2 bytes)                  │
│ - Reserved (4 bytes)                    │
│ - Checksum (2 bytes)                    │
├─────────────────────────────────────────┤
│ Body (64 bytes)                         │
├─────────────────────────────────────────┤
│ - Left Motor RPM (4 bytes, float)       │
│ - Right Motor RPM (4 bytes, float)      │
│ - Reserved (56 bytes)                   │
└─────────────────────────────────────────┘
```

#### 送信パケット（マイコン → ROS2）

```
┌─────────────────────────────────────────┐
│ Header (8 bytes)                        │
├─────────────────────────────────────────┤
│ - Port (2 bytes): 8888                  │
│ - Port (2 bytes): 8888                  │
│ - Length (2 bytes)                      │
│ - Checksum (2 bytes)                    │
├─────────────────────────────────────────┤
│ Body (64 bytes)                         │
├─────────────────────────────────────────┤
│ - Left Encoder Count (4 bytes, int)     │
│ - Right Encoder Count (4 bytes, int)    │
│ - Reserved (56 bytes)                   │
└─────────────────────────────────────────┘
```

### 動作確認

#### 1. デバッグ出力を確認

Serial2に接続したシリアルモニターで、以下のようなログが表示されることを確認：

```
=== Cugo Motor Controller Started ===
Using CugoSDK
Motor controller initialized
PacketSerial initialized
Setup completed
```

#### 2. ROS2からTwistコマンドを送信

```bash
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist \
  "{linear: {x: 0.2, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
```

デバッグ出力に以下が表示されるはず：

```
RPM set: L=50.00 R=50.00
```

#### 3. エンコーダーデータを確認

```bash
ros2 topic echo /odom
```

エンコーダーカウントが含まれたOdometryメッセージが表示されるはず。

---

## トラブルシューティング

### 問題: モーターが動かない

**確認事項:**
1. フェイルセーフが作動していないか？
   - デバッグ出力で`Motor stopped`が表示されていないか確認
   - ROS2からのコマンドが0.5秒以内に届いているか確認

2. RPM値が正しいか？
   - デバッグ出力で`RPM set`の値を確認

3. ハードウェア接続は正しいか？
   - LD2とSerial1の接続を確認
   - モータードライバーの電源供給を確認

### 問題: エンコーダーカウントが増えない

**CugoSDK使用時:**
- LD2からのフィードバックが正しく受信されているか確認
- `ld2_set_feedback()`の設定を確認

**汎用DCモータ使用時:**
- エンコーダーの結線を確認（A相、B相、GND、VCC）
- 割り込みハンドラが正しく呼ばれているか確認
- `GenericMotorController.cpp`の`encoderISR_*()`をデバッグ

### 問題: デバッグ出力が表示されない

1. Serial2の接続を確認
   - GP20 (TX) が USB-シリアル変換器のRXに接続されているか
   - GNDが共通になっているか

2. ボーレートを確認
   - 115200 baudに設定されているか

3. `DEBUG_ENABLED`が有効か確認
   - `lib/DebugPrint/DebugPrint.h`で`#define DEBUG_ENABLED 1`になっているか

### 問題: ビルドエラーが発生する

**`IMotorController.h: No such file or directory`**
- インクルードパスが正しいか確認
- `platformio.ini`の`lib_deps`を確認

**PlatformIOが依存ライブラリをダウンロードできない**
```bash
# キャッシュをクリアして再ビルド
pio run --target clean
rm -rf .pio
pio run
```

---

## 次のステップ

- 汎用DCモータ使用時は、PID制御を実装して速度フィードバックを追加
- エンコーダーから実RPMを計算し、目標RPMに追従させる
- 複数のロボットで同じファームウェアを使用する場合、設定ファイルで切り替えられるように拡張

詳細は[デバッグ方法](debug.md)を参照してください。

---

[← アーキテクチャ](architecture.md) | [デバッグ方法 →](debug.md)
