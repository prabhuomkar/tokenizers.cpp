package xyz.omkar.tokenizers;

public class Tokenizer {

    private final long nativePtr;

    public Tokenizer(String config) {
        nativePtr = newTokenizer(config);
    }

    public int[] encode(String sequence, boolean addSpecialTokens) {
        return encode(nativePtr, sequence, addSpecialTokens);
    }

    public String decode(int[] ids, boolean skipSpecialTokens) {
        return decode(nativePtr, ids, skipSpecialTokens);
    }

    private native long newTokenizer(String config);

    private native int[] encode(long nativePtr, String sequence, boolean addSpecialTokens);

    private native String decode(long nativePtr, int[] ids, boolean skipSpecialTokens);

    static {
        System.loadLibrary("tokenizers_android");
    }
}
