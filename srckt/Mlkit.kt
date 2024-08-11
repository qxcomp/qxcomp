package org.envsh.jvkt;

import com.google.mlkit.nl.languageid.LanguageIdentification
import com.google.mlkit.nl.languageid.LanguageIdentificationOptions
import com.google.mlkit.nl.translate.TranslateLanguage
import com.google.mlkit.nl.translate.Translation
import com.google.mlkit.nl.translate.Translator
import com.google.mlkit.nl.translate.TranslatorOptions
import com.google.mlkit.common.model.DownloadConditions
import com.google.android.gms.tasks.Task
import com.google.android.gms.tasks.OnCompleteListener
// https://developers.google.com/android/reference/com/google/mlkit/common/model/DownloadConditions.Builder

// Packages cannot be imported
// import com.vitorpamplona.amethyst.service.lang
import com.vitorpamplona.amethyst.service.lang.LanguageTranslatorService
import com.vitorpamplona.amethyst.service.lang.ResultOrError

public fun Translate2(cbdata: String, text: String, sl: String, tl: String) : String {
    println(sl)
    var retval = text

    var t = LanguageTranslatorService.translate(text, sl, tl)
    // println("res task:" + t)
    log.info("res task:", t, t.isSuccessful())
    // if (true) return retval;
    tranresultprocess(t, cbdata, text, sl, tl)
    return retval
}

public fun tranresultprocess(t: Task<ResultOrError>, cbdata: String, text: String, sl: String, tl: String) {
    t.addOnCompleteListener{ task -> {
            if (task.isSuccessful()) {
                var res = task.getResult()
                var resval = res.result?:""
                println("succ:" + res)
                log.info("succ:", res)
                Litffi.java2gocall(0, "aaa")
                lffi.call2go("mlkittrandone", cbdata, true,  sl, tl, text.length, resval.length, resval)
            }else{
                var ex = task.getException()
                println("err:" + ex)
                log.error("err:", ex.toString())
                Litffi.java2gocall(1, "aaa")
                lffi.call2go("mlkittrandone", cbdata, false, ex.toString())
            }
    }}

    // t.addOnCompleteListener(OnCompleteListener<ResultOrError>() {
    //     fun onComplete(task : Task<ResultOrError>) {
    //         if (task.isSuccessful()) {
    //             var res = task.getResult()
    //             var resval = res.result?:""
    //             println("succ:" + res)
    //             log.info("succ:", res)
    //             Litffi.java2gocall(0, "aaa")
    //             lffi.call2go("mlkittrandone", cbdata, true,  sl, tl, text.length, resval.length, resval)
    //         }else{
    //             var ex = task.getException()
    //             println("err:" + ex)
    //             log.error("err:", ex.toString())
    //             Litffi.java2gocall(1, "aaa")
    //             lffi.call2go("mlkittrandone", cbdata, false, ex.toString())
    //         }
    //     }
    // })
}

// sl auto
public fun Translate3(cbdata: String, text: String, sl: String, tl: String) : String {
    println(sl)
    var retval = text
    var notrans = sl.split(",").toSet()

    var t = LanguageTranslatorService.autoTranslate(text, notrans, tl)
    tranresultprocess(t, cbdata, text, sl, tl)

    return retval
}


// will hand and timeout and quit
public fun Translate(cbdata: String, text: String, sl: String, tl: String) : String {
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
        log.info("dlok", "addOnSuccessListener")
    }
    .addOnFailureListener { exception ->
        // Model couldn’t be downloaded or other internal error.
        // ...
        log.info("dlerr", exception)
    }

englishGermanTranslator.translate(text)
    .addOnSuccessListener { translatedText -> {
        // Translation successful.
        retval = translatedText
        log.info("trok", text, "=>", retval)
        }
    }
    .addOnFailureListener { exception -> {
         // Error.
         // ...
         log.info("trerr", exception)
        }
    }


    return retval
}