package org.envsh.jvkt;

// todo reflect type
// todo file/line/func

public class log {

companion object {
public fun printv(vararg args: Any) {
    var total = 0
    for (arg in args) {
        val cls = arg::class
        if (total == 0) {print("$arg($cls)")}
        else {print(" $arg($cls)")}
        total += 1
    }
    print("\n")
}
public fun info(vararg args: Any) = printv("[I]", *args)
public fun warn(vararg args: Any) = printv("[W]", *args)
public fun error(vararg args: Any) = printv("[E]", *args)
}


}
