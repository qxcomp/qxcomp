package org.envsh.jvkt;

// todo reflect type
// todo file/line/func

public class log {

companion object {
    var cnter : Long = 0
    // todo string build, output not split
@JvmStatic public fun printv(vararg args: Any) {
    var total = 0
    cnter++
    var line = cnter.toString()
    for (arg in args) {
        val cls = arg::class
        val clsstr = "$cls"
        var tystr = clsstr
        // if clsstr == "class kotlin.String" { // wow, kt bug???
        if (clsstr == "class kotlin.String") {
            tystr = "CKS" 
        } else if (clsstr == "class kotlin.Int") {
            tystr = "CKI"
        } else if (clsstr == "class kotlin.Long") {
            tystr = "CKI"
        }else if (clsstr == "class kotlin.Boolean") {
            tystr = "CKB"
        }
        if (total == 0) {}
        else {
            // print(" $arg($tystr)")
            line += " "
        }
        line += "$arg($tystr)"
        total += 1
    }
    // line += "\n"
    // print("\n")
    println(line)
}

// for java use: @JvmStatic
// log.info
@JvmStatic public fun info(vararg args: Any) = printv("[I]", *args)
@JvmStatic public fun warn(vararg args: Any) = printv("[W]", *args)
@JvmStatic public fun error(vararg args: Any) = printv("[E]", *args)
}
}

// for java use
// LogKt.info
public fun info(vararg args: Any) = log.printv("[I]", *args)
public fun warn(vararg args: Any) = log.printv("[W]", *args)
public fun error(vararg args: Any) = log.printv("[E]", *args)
