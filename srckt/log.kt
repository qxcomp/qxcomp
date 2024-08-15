package org.envsh.jvkt;

// todo reflect type
// todo file/line/func

public class log {

companion object {
    var cnter : Long = 0
    // todo string build, output not split
public fun printv(vararg args: Any) {
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
public fun info(vararg args: Any) = printv("[I]", *args)
public fun warn(vararg args: Any) = printv("[W]", *args)
public fun error(vararg args: Any) = printv("[E]", *args)
}


}
