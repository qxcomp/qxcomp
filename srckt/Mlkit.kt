package org.envsh.jvkt;

import com.google.mlkit.nl.languageid.LanguageIdentification
import com.google.mlkit.nl.languageid.LanguageIdentificationOptions
import com.google.mlkit.nl.translate.TranslateLanguage
import com.google.mlkit.nl.translate.Translation
import com.google.mlkit.nl.translate.Translator
import com.google.mlkit.nl.translate.TranslatorOptions
import com.google.mlkit.common.model.DownloadConditions
// https://developers.google.com/android/reference/com/google/mlkit/common/model/DownloadConditions.Builder

// Packages cannot be imported
// import com.vitorpamplona.amethyst.service.lang
import com.vitorpamplona.amethyst.service.lang.LanguageTranslatorService

public fun Translate2(text: String, sl: String, tl: String) : String {
    println(sl)
    var retval = text

    var t = LanguageTranslatorService.translate(text, sl, tl)

    return retval
}

// sl auto
public fun Translate3(text: String, sl: String, tl: String) : String {
    println(sl)
    var retval = text
    // var notrans = Set<String>("fff") // not work

    // var t = LanguageTranslatorService.translate(text, sl, tl)

    return retval
}


// will hand and timeout and quit
public fun Translate(text: String, sl: String, tl: String) : String {
    println(sl)
    var retval = text

// Create an English-German translator:
val options = TranslatorOptions.Builder()
    .setSourceLanguage(TranslateLanguage.ENGLISH)
    .setTargetLanguage(TranslateLanguage.CHINESE)
    .build()
val englishGermanTranslator = Translation.getClient(options)

var conditions = DownloadConditions.Builder()
    .requireWifi()
    .build()
englishGermanTranslator.downloadModelIfNeeded(conditions)
    .addOnSuccessListener {
        // Model downloaded successfully. Okay to start translating.
        // (Set a flag, unhide the translation UI, etc.)
    }
    .addOnFailureListener { exception ->
        // Model couldn’t be downloaded or other internal error.
        // ...
        println(exception)
    }

englishGermanTranslator.translate(text)
    .addOnSuccessListener { translatedText ->
        // Translation successful.
        retval = translatedText
    }
    .addOnFailureListener { exception ->
         // Error.
         // ...
         println(exception)
    }


    return retval
}