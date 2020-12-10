# robosys2020_devicedriver

ロボットシステム学演習課題  
デバイスドライバ  

---

# 内容

講義内で作成した[デバイスドライバ](https://github.com/ryuichiueda/robosys_device_drivers/blob/master/myled.c)を編集し作成。  
・スイッチをONにしてAを入力した場合、LEDが徐々に細かく点滅し、最後に大きく点灯し終了する。  
・スイッチをOFFにしてAを入力した場合、5回点滅し終了する。  
・Bを入力した場合、スイッチの入力にかかわらず10回点滅し終了する。  
・スイッチの入力の状態を記録して表示する。  

---

# 環境・道具

## 環境
・Raspberry Pi 4 Model B  
・Ubuntu 20.04.1 LTS  
## 道具
・ブレッドボード  
・LED(緑)  
・抵抗 1KΩ  
・ジャンパー線 オス-メス 4  
・ジャンパー線 オス-オス 2  
・マイクロスイッチ  

---

# ビルド

## 実行方法
```sh
$ git clone https://github.com/zjzj-zz/robosys2020_devicedriver.git
$ cd robosys2020_devicedriver/myDeviceDriver
$ make
$ sudo insmod mydriver.ko
$ sudo chmod 666 /dev/mydriver0
```
## 回路
gpio23と3.3Vの間にマイクロスイッチ  
gpio24とGNDの間に抵抗とLED  

---

# 実行

## スイッチによるモード変更

```sh
$ echo A > /dev/mydriver0
```

## 通常の点滅

```sh
$ echo B > /dev/mydriver0
```

## スイッチの状態確認

```sh
$ cat /dev/mydriver0
```

---

# 終了

```sh
$ sudo rmmod mydriver
$ make clean
```
## ログの確認
```sh
$ dmesg
```

---

# デモ動画

youtubeにあげたデモ動画は[こちら](https://youtu.be/zEzk37fNaZQ)。

---

# 著者

[Hikaru Jitsukawa](https://github.com/zjzj-zz)  
ベースのプログラム開発者：[Ryuichi Ueda](https://github.com/ryuichiueda)  

---
# ライセンス
[GNU General Public License v3.0](https://github.com/zjzj-zz/robosys2020_devicedriver/blob/main/COPYING)  
