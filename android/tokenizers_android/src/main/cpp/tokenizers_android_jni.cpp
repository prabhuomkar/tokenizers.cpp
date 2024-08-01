#include <jni.h>

#include <string>
#include <vector>

#include "tokenizers/tokenizer.h"

extern "C" JNIEXPORT jlong JNICALL
Java_xyz_omkar_tokenizers_Tokenizer_newTokenizer(JNIEnv *env, jobject thiz,
                                                 jstring config) {
  const char *config_str = env->GetStringUTFChars(config, nullptr);
  Tokenizer *tokenizer = new Tokenizer("", std::string(config_str));
  env->ReleaseStringUTFChars(config, config_str);
  return (jlong)tokenizer;
}
extern "C" JNIEXPORT jintArray JNICALL
Java_xyz_omkar_tokenizers_Tokenizer_encode(JNIEnv *env, jobject thiz,
                                           jlong native_ptr, jstring sequence,
                                           jboolean add_special_tokens) {
  Tokenizer *tokenizer = reinterpret_cast<Tokenizer *>(native_ptr);
  const jchar *sequence_str = env->GetStringChars(sequence, nullptr);
  jsize sequence_length = env->GetStringLength(sequence);
  std::wstring wsequence(sequence_str, sequence_str + sequence_length);
  env->ReleaseStringChars(sequence, sequence_str);
  Encoding encoding = tokenizer->encode(wsequence, add_special_tokens);
  jintArray ids_array = env->NewIntArray(encoding.ids.size());
  env->SetIntArrayRegion(ids_array, 0, encoding.ids.size(),
                         encoding.ids.data());
  return ids_array;
}
extern "C" JNIEXPORT jstring JNICALL Java_xyz_omkar_tokenizers_Tokenizer_decode(
    JNIEnv *env, jobject thiz, jlong native_ptr, jintArray ids,
    jboolean skip_special_tokens) {
  Tokenizer *tokenizer = reinterpret_cast<Tokenizer *>(native_ptr);
  jsize length = env->GetArrayLength(ids);
  jint *ids_array = env->GetIntArrayElements(ids, nullptr);
  std::vector<int> ids_vector(ids_array, ids_array + length);
  env->ReleaseIntArrayElements(ids, ids_array, 0);
  std::string decoded = tokenizer->decode(ids_vector, skip_special_tokens);
  return env->NewStringUTF(decoded.c_str());
}