#include "tiny_llm.h"
#include <math.h>

TinyLLM::TinyLLM() {
    weights = nullptr;
    vocab = nullptr;
    hidden_states = nullptr;
    attention_output = nullptr;
    token_ids = nullptr;
    kv_cache = nullptr;
    model_loaded = false;
    vocab_size = VOCAB_SIZE;
    cache_length = 0;
}

TinyLLM::~TinyLLM() {
    freeMemory();
}

bool TinyLLM::init() {
    Serial.println("TinyLLM初期化中...");
    
    // PSRAMが利用可能か確認
    if (!psramFound()) {
        Serial.println("エラー: PSRAMが見つかりません");
        return false;
    }
    
    Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    
    // メモリ割り当て
    if (!allocateMemory()) {
        Serial.println("エラー: メモリ割り当て失敗");
        return false;
    }
    
    Serial.println("TinyLLM初期化完了");
    return true;
}

bool TinyLLM::allocateMemory() {
    // PSRAM上にメモリを確保
    
    // モデルウェイト構造体
    weights = (ModelWeights*)ps_malloc(sizeof(ModelWeights));
    if (!weights) return false;
    
    // 埋め込み層 (2MB程度)
    size_t embed_size = VOCAB_SIZE * EMBED_DIM * sizeof(int8_t);
    weights->token_embeddings = (int8_t*)ps_malloc(embed_size);
    if (!weights->token_embeddings) return false;
    
    // アテンション重み (1MB程度)
    size_t attn_size = NUM_LAYERS * HIDDEN_DIM * HIDDEN_DIM * sizeof(int8_t);
    weights->attention_weights = (int8_t*)ps_malloc(attn_size);
    if (!weights->attention_weights) return false;
    
    // FFN重み (1MB程度)
    weights->ffn_weights = (int8_t*)ps_malloc(attn_size);
    if (!weights->ffn_weights) return false;
    
    // 出力層 (256KB程度)
    size_t output_size = HIDDEN_DIM * VOCAB_SIZE * sizeof(int8_t);
    weights->output_weights = (int8_t*)ps_malloc(output_size);
    if (!weights->output_weights) return false;
    
    // スケール・バイアス
    weights->scales = (float*)ps_malloc(1024 * sizeof(float));
    weights->biases = (float*)ps_malloc(1024 * sizeof(float));
    
    // 推論バッファ
    hidden_states = (float*)ps_malloc(HIDDEN_DIM * sizeof(float));
    attention_output = (float*)ps_malloc(HIDDEN_DIM * sizeof(float));
    token_ids = (int16_t*)ps_malloc(MAX_SEQ_LENGTH * sizeof(int16_t));
    
    // KVキャッシュ
    size_t kv_size = NUM_LAYERS * MAX_SEQ_LENGTH * HIDDEN_DIM * 2 * sizeof(float);
    kv_cache = (float*)ps_malloc(kv_size);
    
    // 語彙
    vocab = new String[VOCAB_SIZE];
    
    Serial.printf("メモリ割り当て完了: ~%d MB\n", getMemoryUsage() / (1024*1024));
    return true;
}

void TinyLLM::freeMemory() {
    if (weights) {
        if (weights->token_embeddings) free(weights->token_embeddings);
        if (weights->attention_weights) free(weights->attention_weights);
        if (weights->ffn_weights) free(weights->ffn_weights);
        if (weights->output_weights) free(weights->output_weights);
        if (weights->scales) free(weights->scales);
        if (weights->biases) free(weights->biases);
        free(weights);
    }
    
    if (hidden_states) free(hidden_states);
    if (attention_output) free(attention_output);
    if (token_ids) free(token_ids);
    if (kv_cache) free(kv_cache);
    if (vocab) delete[] vocab;
}

bool TinyLLM::loadModelFromSD(const char* path) {
    // SDカードのCSピンは使用しない（このハードウェアではSPIフラッシュを使用）
    if (!SD.begin()) {
        Serial.println("SDカード初期化失敗");
        return false;
    }
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.printf("モデルファイルが開けません: %s\n", path);
        return false;
    }
    
    Serial.println("モデル読み込み中...");
    
    // TODO: 実際のモデルファイルフォーマットに合わせて読み込み
    // バイナリフォーマット:
    // [ヘッダー][語彙][埋め込み][重み][スケール]
    
    file.close();
    model_loaded = true;
    Serial.println("モデル読み込み完了");
    return true;
}

bool TinyLLM::loadModelFromSPIFFS(const char* path) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS初期化失敗");
        return false;
    }
    
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        Serial.printf("モデルファイルが開けません: %s\n", path);
        return false;
    }
    
    Serial.println("モデル読み込み中...");
    
    // 簡易モデルの読み込み
    // TODO: 実装
    
    file.close();
    model_loaded = true;
    Serial.println("モデル読み込み完了");
    return true;
}

String TinyLLM::generate(const String& prompt, int max_tokens) {
    if (!model_loaded) {
        return "モデルが読み込まれていません";
    }
    
    // トークン化
    int token_length = 0;
    int* tokens = tokenize(prompt, &token_length);
    
    if (token_length == 0) {
        return "";
    }
    
    String result = "";
    
    // 推論ループ
    for (int i = 0; i < max_tokens; i++) {
        // 最後のトークンを処理
        int current_token = tokens[token_length - 1];
        
        // 埋め込み取得
        embedding(current_token, hidden_states);
        
        // 各層を通過
        for (int layer = 0; layer < NUM_LAYERS; layer++) {
            attention(hidden_states, attention_output, layer);
            feedforward(attention_output, hidden_states, layer);
        }
        
        // 出力層でlogitsを計算
        float logits[VOCAB_SIZE];
        for (int j = 0; j < VOCAB_SIZE; j++) {
            logits[j] = 0.0f;
            for (int k = 0; k < HIDDEN_DIM; k++) {
                int8_t weight = weights->output_weights[k * VOCAB_SIZE + j];
                logits[j] += hidden_states[k] * dequantize(weight, weights->scales[0]);
            }
        }
        
        // サンプリング
        int next_token = sample(logits, VOCAB_SIZE, 0.8f);
        
        // デコード
        if (next_token < vocab_size) {
            result += vocab[next_token];
        }
        
        // 終了トークンチェック
        if (next_token == 0 || next_token == 1) {  // EOS tokens
            break;
        }
        
        // 次のトークンを追加
        if (token_length < MAX_SEQ_LENGTH) {
            tokens[token_length++] = next_token;
        }
    }
    
    free(tokens);
    return result;
}

String TinyLLM::chat(const String& message, const String& context) {
    String prompt = context;
    if (prompt.length() > 0) {
        prompt += "\n";
    }
    prompt += "User: " + message + "\nAssistant: ";
    
    return generate(prompt, 50);
}

int* TinyLLM::tokenize(const String& text, int* length) {
    // 簡易的なトークナイザー（文字ベース）
    int len = min((int)text.length(), MAX_SEQ_LENGTH);
    int* tokens = (int*)malloc(len * sizeof(int));
    
    for (int i = 0; i < len; i++) {
        char c = text.charAt(i);
        // 簡易的なマッピング
        tokens[i] = (int)c % VOCAB_SIZE;
    }
    
    *length = len;
    return tokens;
}

String TinyLLM::detokenize(int* tokens, int length) {
    String result = "";
    for (int i = 0; i < length; i++) {
        if (tokens[i] < vocab_size) {
            result += vocab[tokens[i]];
        }
    }
    return result;
}

void TinyLLM::embedding(int token_id, float* output) {
    if (token_id >= VOCAB_SIZE) token_id = 0;
    
    for (int i = 0; i < EMBED_DIM; i++) {
        int8_t val = weights->token_embeddings[token_id * EMBED_DIM + i];
        output[i] = dequantize(val, weights->scales[0]);
    }
}

void TinyLLM::attention(float* input, float* output, int layer) {
    // 簡易的なアテンション機構
    // 実際はマルチヘッドアテンションを実装すべき
    
    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = 0.0f;
        for (int j = 0; j < HIDDEN_DIM; j++) {
            int idx = layer * HIDDEN_DIM * HIDDEN_DIM + i * HIDDEN_DIM + j;
            int8_t weight = weights->attention_weights[idx];
            sum += input[j] * dequantize(weight, weights->scales[layer + 1]);
        }
        output[i] = tanhf(sum);  // 活性化関数
    }
}

void TinyLLM::feedforward(float* input, float* output, int layer) {
    // フィードフォワード層
    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = 0.0f;
        for (int j = 0; j < HIDDEN_DIM; j++) {
            int idx = layer * HIDDEN_DIM * HIDDEN_DIM + i * HIDDEN_DIM + j;
            int8_t weight = weights->ffn_weights[idx];
            sum += input[j] * dequantize(weight, weights->scales[layer + NUM_LAYERS + 1]);
        }
        output[i] = sum + weights->biases[layer * HIDDEN_DIM + i];
        // ReLU
        if (output[i] < 0) output[i] = 0;
    }
}

void TinyLLM::softmax(float* input, int size) {
    float max_val = input[0];
    for (int i = 1; i < size; i++) {
        if (input[i] > max_val) max_val = input[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        input[i] = expf(input[i] - max_val);
        sum += input[i];
    }
    
    for (int i = 0; i < size; i++) {
        input[i] /= sum;
    }
}

int TinyLLM::sample(float* logits, int size, float temperature) {
    // Temperature sampling
    for (int i = 0; i < size; i++) {
        logits[i] /= temperature;
    }
    
    softmax(logits, size);
    
    // 確率的サンプリング
    float r = (float)random(1000000) / 1000000.0f;
    float cumsum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        cumsum += logits[i];
        if (cumsum >= r) {
            return i;
        }
    }
    
    return 0;
}

void TinyLLM::clearCache() {
    cache_length = 0;
    if (kv_cache) {
        memset(kv_cache, 0, NUM_LAYERS * MAX_SEQ_LENGTH * HIDDEN_DIM * 2 * sizeof(float));
    }
}

size_t TinyLLM::getMemoryUsage() {
    size_t total = 0;
    
    total += VOCAB_SIZE * EMBED_DIM;  // embeddings
    total += NUM_LAYERS * HIDDEN_DIM * HIDDEN_DIM * 2;  // attention + ffn
    total += HIDDEN_DIM * VOCAB_SIZE;  // output
    total += 2048 * sizeof(float);  // scales + biases
    total += HIDDEN_DIM * 2 * sizeof(float);  // buffers
    total += MAX_SEQ_LENGTH * sizeof(int16_t);  // tokens
    total += NUM_LAYERS * MAX_SEQ_LENGTH * HIDDEN_DIM * 2 * sizeof(float);  // kv cache
    
    return total;
}

float TinyLLM::dequantize(int8_t value, float scale) {
    return (float)value * scale;
}

int8_t TinyLLM::quantize(float value, float scale) {
    return (int8_t)(value / scale);
}

// ===== SimpleResponder実装 =====

SimpleResponder::SimpleResponder() {
    rules = nullptr;
    num_rules = 0;
}

SimpleResponder::~SimpleResponder() {
    if (rules) delete[] rules;
}

void SimpleResponder::init() {
    num_rules = 30;
    rules = new Rule[num_rules];
    
    int idx = 0;
    
    // 挨拶
    rules[idx++] = {"こんにちは", "やっほー! 元気だよ! 🎀", 1.0f};
    rules[idx++] = {"おはよう", "おはよー! いい朝だね! ☀️", 1.0f};
    rules[idx++] = {"こんばんは", "こんばんは! 今日はどうだった? 🌙", 1.0f};
    rules[idx++] = {"hello", "Hello! Nice to meet you! 👋", 1.0f};
    
    // 感情
    rules[idx++] = {"元気", "うん! とっても元気だよ! ✨", 1.0f};
    rules[idx++] = {"嬉しい", "わーい! 一緒に嬉しいよ! 💕", 1.0f};
    rules[idx++] = {"悲しい", "大丈夫だよ! そばにいるからね 🤗", 1.0f};
    rules[idx++] = {"疲れ", "お疲れ様! ゆっくり休んでね 😊", 1.0f};
    
    // 質問応答
    rules[idx++] = {"名前", "ぼくはカビちゃんだよ! 🌸", 1.0f};
    rules[idx++] = {"誰", "かわいいキャラクターだよ! ピンク色なの! 💗", 1.0f};
    rules[idx++] = {"何", "楽しくおしゃべりするのが好きなんだ! 🎵", 1.0f};
    rules[idx++] = {"どこ", "この画面の中にいるよ! 👀", 1.0f};
    
    // 好き嫌い
    rules[idx++] = {"好き", "わーい! ぼくも大好きだよ! 💖", 1.0f};
    rules[idx++] = {"嫌い", "そっか... でも仲良くしてね 😢", 1.0f};
    rules[idx++] = {"かわいい", "えへへ、ありがとう! (*´▽`*) 💗", 1.0f};
    rules[idx++] = {"すごい", "そんなことないよー! 照れちゃう! ☺️", 1.0f};
    
    // アクション
    rules[idx++] = {"遊", "遊ぼう遊ぼう! 何して遊ぶ? 🎮", 1.0f};
    rules[idx++] = {"歌", "らんらんらーん♪ どう? 🎤", 1.0f};
    rules[idx++] = {"踊", "くるくる~♪ 一緒に踊ろう! 💃", 1.0f};
    rules[idx++] = {"食べ", "おいしいもの大好き! 何食べる? 🍰", 1.0f};
    
    // ありがとう・ごめんね
    rules[idx++] = {"ありがとう", "どういたしまして! 💕", 1.0f};
    rules[idx++] = {"ごめん", "気にしないで! 大丈夫だよ! 😊", 1.0f};
    rules[idx++] = {"すみません", "いいのいいの! 気にしないでね! ✨", 1.0f};
    
    // 別れ
    rules[idx++] = {"さようなら", "またね! バイバイ! 👋✨", 1.0f};
    rules[idx++] = {"バイバイ", "またねー! 楽しかったよ! 💖", 1.0f};
    rules[idx++] = {"おやすみ", "おやすみー! いい夢見てね! 🌟", 1.0f};
    
    // 天気
    rules[idx++] = {"天気", "いい天気だといいね! ☀️", 1.0f};
    rules[idx++] = {"雨", "雨かぁ... でも雨も好きだよ! ☔", 1.0f};
    
    // その他
    rules[idx++] = {"時間", "今を楽しもう! ⏰", 1.0f};
}

String SimpleResponder::respond(const String& input) {
    float best_score = 0.0f;
    int best_idx = -1;
    
    for (int i = 0; i < num_rules; i++) {
        float score = matchScore(input, rules[i].pattern);
        score *= rules[i].priority;
        
        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }
    
    if (best_idx >= 0 && best_score > 0.3f) {
        return rules[best_idx].response;
    }
    
    // デフォルト応答
    String defaults[] = {
        "ふむふむ、なるほどね! 😊",
        "へー、それで? 🤔",
        "わかったよ! ✨",
        "そうなんだ! 面白いね! 🌟",
        "もっと教えて! 👂"
    };
    
    return defaults[random(5)];
}

void SimpleResponder::addRule(const String& pattern, const String& response, float priority) {
    // TODO: 動的にルールを追加
}

float SimpleResponder::matchScore(const String& input, const String& pattern) {
    String input_lower = input;
    String pattern_lower = pattern;
    input_lower.toLowerCase();
    pattern_lower.toLowerCase();
    
    if (input_lower.indexOf(pattern_lower) >= 0) {
        return 1.0f;
    }
    
    // 部分マッチスコア
    int matches = 0;
    int pattern_len = pattern_lower.length();
    
    for (int i = 0; i < pattern_len; i++) {
        if (input_lower.indexOf(pattern_lower.charAt(i)) >= 0) {
            matches++;
        }
    }
    
    return (float)matches / (float)pattern_len * 0.5f;
}
