/**
 * Tiny LLM Engine for ESP32-S3
 * 超軽量な言語モデル推論エンジン
 * 
 * メモリ制約:
 * - SRAM: 512KB
 * - PSRAM: 8MB
 * - Flash: 16MB
 * 
 * アプローチ:
 * 1. 量子化された超小型モデル (1-10MB)
 * 2. トークン単位の推論
 * 3. PSRAMを活用した重みの読み込み
 */

#ifndef TINY_LLM_H
#define TINY_LLM_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>

// モデル設定
#define MAX_SEQ_LENGTH 128
#define VOCAB_SIZE 2048
#define EMBED_DIM 128
#define HIDDEN_DIM 256
#define NUM_HEADS 4
#define NUM_LAYERS 2

// 量子化設定
#define USE_INT8_QUANTIZATION

class TinyLLM {
private:
    // モデルパラメータ（PSRAM上）
    struct ModelWeights {
        int8_t* token_embeddings;     // [VOCAB_SIZE, EMBED_DIM]
        int8_t* attention_weights;    // [NUM_LAYERS, HIDDEN_DIM, HIDDEN_DIM]
        int8_t* ffn_weights;          // [NUM_LAYERS, HIDDEN_DIM, HIDDEN_DIM]
        int8_t* output_weights;       // [HIDDEN_DIM, VOCAB_SIZE]
        
        float* scales;                 // 量子化スケール
        float* biases;                 // バイアス
    };
    
    ModelWeights* weights;
    bool model_loaded;
    
    // トークナイザー
    String* vocab;
    int vocab_size;
    
    // 推論バッファ（PSRAM）
    float* hidden_states;
    float* attention_output;
    int16_t* token_ids;
    
    // キャッシュ
    float* kv_cache;
    int cache_length;
    
public:
    TinyLLM();
    ~TinyLLM();
    
    // 初期化
    bool init();
    bool loadModelFromSD(const char* path);
    bool loadModelFromSPIFFS(const char* path);
    
    // 推論
    String generate(const String& prompt, int max_tokens = 50);
    String chat(const String& message, const String& context = "");
    
    // トークナイザー
    int* tokenize(const String& text, int* length);
    String detokenize(int* tokens, int length);
    
    // ユーティリティ
    void clearCache();
    size_t getMemoryUsage();
    bool isModelLoaded() { return model_loaded; }
    
private:
    // モデル演算
    void embedding(int token_id, float* output);
    void attention(float* input, float* output, int layer);
    void feedforward(float* input, float* output, int layer);
    void softmax(float* input, int size);
    int sample(float* logits, int size, float temperature = 0.8f);
    
    // メモリ管理
    bool allocateMemory();
    void freeMemory();
    
    // ヘルパー
    float dequantize(int8_t value, float scale);
    int8_t quantize(float value, float scale);
};

// プリセットプロンプト
namespace TinyLLMPrompts {
    const char* SYSTEM_KIRBY = 
        "You are a cute character like Kirby. "
        "Respond in short, cheerful Japanese sentences. "
        "Use emoji and end with だよ/なの/ね.";
    
    const char* SYSTEM_ASSISTANT = 
        "You are a helpful assistant. "
        "Keep responses brief and friendly.";
}

// 簡易的なルールベース応答（フォールバック）
class SimpleResponder {
private:
    struct Rule {
        String pattern;
        String response;
        float priority;
    };
    
    Rule* rules;
    int num_rules;
    
public:
    SimpleResponder();
    ~SimpleResponder();
    
    void init();
    String respond(const String& input);
    void addRule(const String& pattern, const String& response, float priority = 1.0f);
    
private:
    float matchScore(const String& input, const String& pattern);
};

#endif
