package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 

// 获取Jvm内存
public fun getJvmMemory(which : Int) : Long {
    var jvrt = Runtime.getRuntime();
    var totmem = jvrt.totalMemory();
    var freemem = jvrt.freeMemory();

    return if (which==0) totmem else freemem
}
