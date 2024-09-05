package org.envsh.jvkt;

import java.lang.*;

// 没法在外部引用啊，使用Litffi.java2gocall和LitffiKt.java2gocall都不行
    // https://forum.qt.io/topic/97028/to-connect-android-broadcastreceiver-onreceive-method-with-cpp/4
    // 在C中的函数签名，Java_org_envsh_jvkt_Litffi_java2gocall 
    // 参数格式，使用|#|分隔的字符串吧，不搞别的序列化了
    // datafmt=0 now
    // datafmt=1 json
    // datafmt=2 ...
// todo 返回字符串比较好
// 应该会是 Java_org_envsh_jvkt_LitffiKt_java2gocall ???
public external fun java2gocall(datafmt: Int, cmdinfo: String) : Int;
public external fun callanytest( vararg args: Any) :Any;

public class lffi {
    companion object {
        init {
            // System.loadLibrary("hellogo");//  crash???
        }

        public const val datasep = "|$|$|"; // |$|$| 这个不错

        public fun call2go(vararg args : Any) : Int {
            val argc = args.size
            var data = ""
            var cnter = 0
            for (arg in args) {
                data += arg
                if (cnter == argc-1) {} else { data += datasep }
                cnter += 1
            }
            val rv = java2gocall(0, data)
            return rv
        }

    }
    // kotlin 的签名更长啊，不用工具更没法搞
    // 不要在companion中声明external/native函数好了
    //	kotlin 需要的签名， (tried Java_org_envsh_jvkt_Litffi_00024Companion_java2gocall and Java_org_envsh_jvkt_Litffi_00024Companion_java2gocall__ILjava_lang_String_2)
}
