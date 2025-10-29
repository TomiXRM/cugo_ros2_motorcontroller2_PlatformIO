#!/bin/bash

# picotool udevルールをインストールするスクリプト

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RULES_FILE="$SCRIPT_DIR/60-picotool.rules"
RULES_DEST="/etc/udev/rules.d/60-picotool.rules"

# root権限チェック
if [ "$EUID" -ne 0 ]; then
    echo "エラー: このスクリプトはroot権限で実行する必要があります。"
    echo "sudo $0 を実行してください。"
    exit 1
fi

# ルールファイルの存在確認
if [ ! -f "$RULES_FILE" ]; then
    echo "エラー: $RULES_FILE が見つかりません。"
    exit 1
fi

# ルールファイルをコピー
echo "udevルールをインストール中..."
cp -v "$RULES_FILE" "$RULES_DEST"

# udevルールをリロード
echo "udevルールをリロード中..."
udevadm control --reload-rules
udevadm trigger

echo "完了: picotoolのudevルールが正常にインストールされました。"
echo "デバイスを再接続すると、新しいルールが適用されます。"
