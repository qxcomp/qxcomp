package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 

public class Util2 {

// 获取Jvm内存
public static long getJvmMemory(int which) {
    var jvrt = Runtime.getRuntime();
    var totmem = jvrt.totalMemory();
    var freemem = jvrt.freeMemory();

    if (which == 0) {
        return totmem;
    }else if(which == 1) {
        return freemem;
    }else{
        System.out.println("which not support" + which);
    }
    if (false) {
        return UtilKt.getJvmMemory(which);
    }
    return -1;
}

}
