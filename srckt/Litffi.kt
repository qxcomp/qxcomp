package org.envsh.jvkt;

import java.lang.*;

public class Litffi {
    companion object {
        init {
            // System.loadLibrary("hellogo");//  crash???
        }

    public var datasep : String = "$|$"; // |$|$| 这个不错
    // https://forum.qt.io/topic/97028/to-connect-android-broadcastreceiver-onreceive-method-with-cpp/4
    // 在C中的函数签名，Java_org_envsh_jvkt_Litffi_java2gocall 
    // 参数格式，使用|#|分隔的字符串吧，不搞别的序列化了
    // datafmt=0 now
    // datafmt=1 json
    // datafmt=2 ...
    public external fun java2gocall( datafmt: Int,  cmdinfo:String) :Integer ;
    }
}
