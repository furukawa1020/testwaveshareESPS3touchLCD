/**
 * README: ESP32-S3-Touch-LCD-1.85 カービィ風キャラクター実装
 * 
 * このプロジェクトは、Waveshare ESP32-S3-Touch-LCD-1.85 円形ディスプレイで
 * カービィのようなかわいいキャラクターを表示し、音声認識と音声出力を統合した
 * インタラクティブなシステムです。
 * 
 * ===== ハードウェア仕様 =====
 * - ディスプレイ: 1.85インチ円形 IPS LCD (360×360)
 * - MCU: ESP32-S3 デュアルコア (最大240MHz)
 * - メモリ: 16MB Flash + 8MB PSRAM
 * - オーディオ: PCM5101A DAC、ICS-43434マイク、2030スピーカー (8Ω 2W)
 * - センサー: QMI8658C 6軸IMU
 * - その他: タッチパネル(I2C)、TFカードスロット、RTC
 * 
 * ===== 機能 =====
 * 1. カービィ風キャラクター描画
 *    - まん丸なピンク色の顔
 *    - 楕円形の目(ハイライト付き)
 *    - かわいい小さな口
 *    - ふんわりした頬の赤み
 * 
 * 2. スムーズなアニメーション
 *    - 自動まばたき(ランダム間隔)
 *    - 話すときの口パク
 *    - 驚きの表情
 *    - 滑らかなトランジション
 * 
 * 3. 音声認識(ESP-SR使用)
 *    - ウェイクワード: "Hi ESP" または "こんにちは"
 *    - 音声コマンドでインタラクション
 *    - リアルタイム処理
 * 
 * 4. かわいい音声出力
 *    - 合成音によるキュートな効果音
 *    - I2S経由でスピーカー出力
 *    - 音声に合わせた口パク
 * 
 * 5. ローカルLLM統合(オプション)
 *    - ESP32-S3のメモリ制約を考慮
 *    - WiFi経由でクラウドLLM連携可能
 *    - 自然な会話応答
 * 
 * ===== セットアップ手順 =====
 * 
 * 1. PlatformIOのインストール
 *    - VS Codeに PlatformIO IDE 拡張機能をインストール
 * 
 * 2. 必要なハードウェア
 *    - ESP32-S3-Touch-LCD-1.85 本体
 *    - 2030 スピーカー (8Ω 2W)
 *    - USB Type-Cケーブル
 *    - TFカード(MP3ファイル保存用、オプション)
 * 
 * 3. ハードウェア接続
 *    - スピーカーをボード上のスピーカー端子に接続
 *    - TFカードを挿入(オプション)
 *    - USBケーブルでPCに接続
 * 
 * 4. ビルドとアップロード
 *    - PlatformIOで "Build" をクリック
 *    - "Upload" をクリックしてESP32-S3に書き込み
 *    - シリアルモニタを開いてログを確認
 * 
 * 5. 動作確認
 *    - 画面にカービィ風キャラクターが表示されます
 *    - 自動的にまばたきします
 *    - シリアルコマンドでテスト:
 *      'b' - まばたき
 *      't' - 話す
 *      's' - 驚き
 *      'r' - リセット
 * 
 * ===== ピン配置 (重要!) =====
 * 
 * LCD (QSPI):
 *   CS   = GPIO 10
 *   DC   = GPIO 11
 *   RST  = GPIO 12
 *   BL   = GPIO 45
 *   MOSI = GPIO 13
 *   SCLK = GPIO 14
 * 
 * Touch (I2C):
 *   SDA  = GPIO 4
 *   SCL  = GPIO 5
 *   INT  = GPIO 7
 *   RST  = GPIO 8
 * 
 * Audio (I2S):
 *   BCLK = GPIO 15
 *   LRCK = GPIO 16
 *   DOUT = GPIO 17
 * 
 * Microphone (I2S):
 *   SCK  = GPIO 41
 *   WS   = GPIO 42
 *   DATA = GPIO 2
 * 
 * ===== 必要なライブラリ =====
 * - LVGL 8.3.x (GUI)
 * - TFT_eSPI (ディスプレイドライバ)
 * - ESP32-audioI2S (オーディオ再生)
 * - ESP-SR (音声認識、ESP-IDFコンポーネント)
 * 
 * ===== カスタマイズ =====
 * 
 * 1. キャラクターの色を変更:
 *    main.cpp内の lv_color_hex() の値を変更
 *    例: 0xFFB4D5 (ピンク) → 0xB4D5FF (青)
 * 
 * 2. アニメーション速度を調整:
 *    - まばたき間隔: random(2000, 5000) の値を変更
 *    - 口パク速度: talk_animation() の 150ms を変更
 * 
 * 3. 音声コマンドを追加:
 *    - voice_handler.h/cpp にコマンド処理を追加
 *    - ESP-SRのコマンド登録手順に従う
 * 
 * 4. LLM統合:
 *    - WiFi接続を追加
 *    - HTTPクライアントでAPI呼び出し
 *    - OpenAI互換APIまたはローカルLLM
 * 
 * ===== トラブルシューティング =====
 * 
 * 問題: 画面が表示されない
 * 解決: 
 *   - LCD_BL (GPIO 45) がHIGHになっているか確認
 *   - SPI配線を確認
 *   - ボードのRESETボタンを押す
 * 
 * 問題: 音が出ない
 * 解決:
 *   - スピーカーが正しく接続されているか確認
 *   - ボリュームノブを調整
 *   - I2Sピン配置を確認
 * 
 * 問題: コンパイルエラー
 * 解決:
 *   - platformio.ini の設定を確認
 *   - ライブラリの依存関係を確認
 *   - PlatformIOをクリーンビルド
 * 
 * 問題: 音声認識が動作しない
 * 解決:
 *   - ESP-SRコンポーネントが正しくインストールされているか確認
 *   - マイクが機能しているかテスト
 *   - パーティション設定を確認 (SR用に十分な領域が必要)
 * 
 * ===== 参考リンク =====
 * - Waveshare Wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85
 * - ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
 * - LVGL Documentation: https://docs.lvgl.io/
 * - ESP-SR: https://github.com/espressif/esp-sr
 * - PlatformIO ESP32: https://docs.platformio.org/en/latest/platforms/espressif32.html
 * 
 * ===== ライセンス =====
 * このプロジェクトはMITライセンスの下で公開されています。
 * 
 * ===== 連絡先 =====
 * 問題や質問がある場合は、GitHubのIssueを開いてください。
 * 
 * ===== 更新履歴 =====
 * v1.0.0 (2025-11-10)
 *   - 初回リリース
 *   - カービィ風キャラクター実装
 *   - 基本アニメーション機能
 *   - オーディオ統合の基礎
 *   - 音声認識フレームワーク
 */
