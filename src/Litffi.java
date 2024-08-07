package org.envsh.jvkt;

import java.lang.*;
// import com.vitorpamplona.amethyst.service.lang;

public class Litffi {
    static {
        // System.loadLibrary("hellogo");//  crash???
    }
    public static final String datasep = "|$|$|";
    // https://forum.qt.io/topic/97028/to-connect-android-broadcastreceiver-onreceive-method-with-cpp/4
    // 在C中的函数签名，Java_org_envsh_jvkt_Litffi_java2gocall 
    // 参数格式，使用|#|分隔的字符串吧，不搞别的序列化了
    // datafmt=0 now
    // datafmt=1 json
    // datafmt=2 ...
    public native  static String java2gocall(int datafmt, String cmdinfo);
    // public external fun callanytest( a0: Any,  a1: Any, a2 : Any, a3: Any, a4: Any) :Any ;

    // public static void ggmlkittranslate() {
    //     lang.LanguageTranslatorService.translate();
    // }
}
