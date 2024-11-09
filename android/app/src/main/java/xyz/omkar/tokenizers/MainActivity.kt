package xyz.omkar.tokenizers

import android.content.Context
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import org.pytorch.IValue
import org.pytorch.LiteModuleLoader
import org.pytorch.Module
import org.pytorch.Tensor


class MainActivity : AppCompatActivity() {

    private val MODEL_NAME = "mobilebert-uncased"
    private lateinit var tokenizer: Tokenizer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        tokenizer = Tokenizer(getTokenizerConfig(applicationContext))
        val input : EditText = findViewById(R.id.input)
        val output : TextView = findViewById(R.id.output)

        val submitButton : Button = findViewById(R.id.submit)
        submitButton.setOnClickListener {
            val ids = tokenizer.encode(input.text.toString(), true)
            val decoded = tokenizer.decode(ids, false)
            output.text = "Encoded: ${ids.toList()}\nDecoded: $decoded"
        }
    }

    private fun getTokenizerConfig(context: Context): String {
        val inputStream = context.assets.open("$MODEL_NAME.json")
        val reader = inputStream.bufferedReader(Charsets.UTF_8)
        return reader.useLines { it.joinToString("\n") }
    }
}