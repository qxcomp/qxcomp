package org.envsh.jvkt;

// todo reflect type
// todo file/line/func

public class log {

companion object {
    // todo string build, output not split
public fun printv(vararg args: Any) {
    var total = 0
    for (arg in args) {
        val cls = arg::class
        val clsstr = "$cls"
        var tystr = clsstr
        // if clsstr == "class kotlin.String" { // wow, kt bug???
        if (clsstr == "class kotlin.String") {
            tystr = "CKS"
        }else if (clsstr == "class kotlin.Boolean") {
            tystr = "CKB"
        }
        if (total == 0) {print("$arg($tystr)")}
        else {print(" $arg($tystr)")}
        total += 1
    }
    print("\n")
}
public fun info(vararg args: Any) = printv("[I]", *args)
public fun warn(vararg args: Any) = printv("[W]", *args)
public fun error(vararg args: Any) = printv("[E]", *args)
}


}
