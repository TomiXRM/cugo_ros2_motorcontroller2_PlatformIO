![image](https://github.com/CuboRex-Development/cugo_ros_control/assets/97714660/a2525198-fa61-4c4d-9a0f-7dd6824fa625)

# cugo_ros2_motorcontroller2
ROS 2でクローラロボット開発プラットフォームを制御するArduinoスケッチです。

セットでROS 2パッケージの[cugo_ros2_control2](https://github.com/CuboRex-Development/cugo_ros2_control2)と使用します。

> [!WARNING]
> このArduinoスケッチはクローラロボット開発プラットフォーム CuGo V4 / V3i 専用です。
> 
> ROS開発キット CuGo V3 をご利用の方は[こちら](https://github.com/CuboRex-Development/cugo_ros_motorcontroller/tree/uno-udp)を参照してください。
    

# Features
cugo_ros2_motorcontroller2では、ROS 2パッケージの [cugo_ros2_control2](https://github.com/CuboRex-Development/cugo_ros2_control2)から受信した目標回転数になるようにモータ制御し、エンコーダから取得したカウント数をPCに返信します。

また、付属のプロポ（MR-8）を用いて、コントローラ操作と自律走行の切り替えができます。左スティックを左に倒すことでコントローラ操作を受け付けるRC-MODEに、右に倒すことでROSの速度ベクトル受け付けるROS-MODEに切り替えることができます。

自律走行中に誤った判断をして障害物に衝突しそうなシーンでは、プロポよりRC-MODEに変更することで、コントローラ操作に即時に切り替わりロボットを止めることができます。  


![image](https://user-images.githubusercontent.com/22425319/234765585-23458585-ea44-40d5-b71f-395c93509fc8.png)

#### 対応製品
* クローラロボット開発プラットフォーム CuGo V4
* クローラロボット開発プラットフォーム CuGo V3i

でお使いいただけます。


# Requirement
ハードウェア
* RaspberryPiPico

Arduino標準ライブラリ 
* Servo.h
* PacketSerial.h
* RPi_Pico_TimerInterrupt.h
* hardware/watchdog.h
 
# Installation
 Arduino標準IDEでcugo_ros2_motorcontroller2.inoを開き、クローラロボット開発プラットフォームのRaspberryPiPicoに書き込んでください。

 ArduinoIDEのインストール方法や書き込み方法などの基本的な操作方法はこちらの[2.準備](https://github.com/CuboRex-Development/cugo-beginner-programming/tree/pico)をご覧ください。


 
# Usage

クローラロボット開発プラットフォームのRaspberryPiPicoとPCをUSBケーブルで接続してください。

スケッチがRaspberryPiPicoに書き込まれていれば、ROS 2パッケージ[cugo_ros2_control2](https://github.com/CuboRex-Development/cugo_ros2_control2)を起動すると、自動的にPCと通信します。

もし、うまく `/cmd_vel` 通りに走行を開始しない場合は、一度USBケーブルを抜き、 ロボットの電源を入れなおしてから再度PCとRaspberryPiPicoをUSBケーブルで接続してください。

![system](https://github.com/CuboRex-Development/cugo_ros_motorcontroller/assets/22425319/2b20c7a0-7947-4b92-96dc-3e4d41865eea)

![system](https://github.com/CuboRex-Development/cugo_ros_motorcontroller/assets/22425319/8da5af96-69a2-4591-a654-4b4bc1e0abde)



# Note
ご不明点がございましたら、[お問い合わせフォーム](https://cuborex.com/contact/)にてお問い合わせください。回答いたします。


# License
このプロジェクトはApache License 2.0のもと、公開されています。詳細はLICENSEをご覧ください。
 
