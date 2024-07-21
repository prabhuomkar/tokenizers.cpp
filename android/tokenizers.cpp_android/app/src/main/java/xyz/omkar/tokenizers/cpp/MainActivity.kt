package xyz.omkar.tokenizers.cpp

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import xyz.omkar.tokenizers.cpp.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private lateinit var tokenizer: Tokenizer;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        tokenizer = Tokenizer(getTokenizerConfig(this, "bert-base-uncased/tokenizer.json"));

        var encodings = tokenizer.encode("我喜欢学习中文。Açúcar é doce.", true)

        binding.encoded.text = "Encoded: " + encodings.contentToString()
        binding.decoded.text = "Decoded: " + tokenizer.decode(encodings, true)
    }

    fun getTokenizerConfig(context: Context, fileName: String): String {
        val inputStream = context.assets.open(fileName)
        val reader = inputStream.bufferedReader(Charsets.UTF_8)
        return reader.useLines { it.joinToString("\n") }
    }
}