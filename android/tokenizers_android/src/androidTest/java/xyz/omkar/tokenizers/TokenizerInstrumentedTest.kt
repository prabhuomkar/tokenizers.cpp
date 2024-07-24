package xyz.omkar.tokenizers

import android.content.Context
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith


private const val FILE_NAME = "bert-base-uncased.json"
private val EXPECTED_ENCODED = intArrayOf(101, 1855, 100, 100, 1817, 100, 1746, 1861, 1636, 9353, 18100, 2099, 1041, 9986, 2063, 1012, 102)
private val EXPECTED_DECODED = "我 学 中 文 。 acucar e doce."

@RunWith(AndroidJUnit4::class)
class TokenizerInstrumentedTest {
    @Test
    fun testEncodeAndDecode() {
        val tokenizerConfig = getTokenizerConfig(InstrumentationRegistry.getInstrumentation().context)
        val tokenizer = Tokenizer(tokenizerConfig)
        val gotEncoded = tokenizer.encode("我喜欢学习中文。Açúcar é doce.", true)
        assertArrayEquals(EXPECTED_ENCODED, gotEncoded)
        val gotDecoded = tokenizer.decode(gotEncoded, true)
        assertEquals(EXPECTED_DECODED, gotDecoded)
    }

    private fun getTokenizerConfig(context: Context): String {
        val inputStream = context.assets.open(FILE_NAME)
        val reader = inputStream.bufferedReader(Charsets.UTF_8)
        return reader.useLines { it.joinToString("\n") }
    }
}