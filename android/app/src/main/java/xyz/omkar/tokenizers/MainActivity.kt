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
    private lateinit var module: Module
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

        module = LiteModuleLoader.loadModuleFromAsset(assets, "$MODEL_NAME.ptl")
        tokenizer = Tokenizer(getTokenizerConfig(applicationContext))
        val input : EditText = findViewById(R.id.input)
        val output : TextView = findViewById(R.id.output)

        val submitButton : Button = findViewById(R.id.submit)
        submitButton.setOnClickListener {
            val ids = tokenizer.encode(input.text.toString(), true)
            val attentionMask = IntArray(ids.size) { 1 }
            val maskedIdx = ids.indexOf(103)
            val idsTensor: Tensor =
                Tensor.fromBlob(ids, longArrayOf(1, ids.size.toLong()))
            val attentionMaskTensor: Tensor =
                Tensor.fromBlob(attentionMask, longArrayOf(1, attentionMask.size.toLong()))
            val outputs = module.runMethod("forward", IValue.from(idsTensor), IValue.from(attentionMaskTensor)).toTuple()
            val predictions = outputs[0].toTensor()
            val predictionForMaskedIndex = predictions.dataAsFloatArray.sliceArray(maskedIdx*predictions.shape().last().toInt() until (maskedIdx+1)*predictions.shape().last().toInt())
            var predictedIndex = 0
            var maxVal = predictionForMaskedIndex[0]
            for (i in predictionForMaskedIndex.indices) {
                if (predictionForMaskedIndex[i] > maxVal) {
                    maxVal = predictionForMaskedIndex[i]
                    predictedIndex = i
                }
            }
            val predictedText = tokenizer.decode(intArrayOf(predictedIndex), false)
            output.setText("Output: " + predictedText)
        }
    }

    private fun getTokenizerConfig(context: Context): String {
        val inputStream = context.assets.open("$MODEL_NAME.json")
        val reader = inputStream.bufferedReader(Charsets.UTF_8)
        return reader.useLines { it.joinToString("\n") }
    }
}