package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 
import android.content.Context;
import android.content.Intent;

public class Util2 {

    public static Context mactx;

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

// todo support url/image
// static
public static boolean sharetoapp(String text, String mime) {
    log.info("sharetoapp", mime, text);
    
     /*Create an ACTION_SEND Intent*/
    Intent intent = new Intent(Intent.ACTION_SEND);
    /*This will be the actual content you wish you share.*/
    String shareBody = "Here is the share content body" + text;
    /*The type of the content is text, obviously.*/
    intent.setType("text/plain");
    /*Applying information Subject and Body.*/
    intent.putExtra(Intent.EXTRA_SUBJECT, "getString(R.string.share_subject)");
    intent.putExtra(Intent.EXTRA_TEXT, shareBody);
    /*Fire!*/
    // startActivity(Intent.createChooser(intent, getString(R.string.share_using)));
    Util2.mactx. startActivity(Intent.createChooser(intent, "getString(R.string.share_using)"));
    return true;
}


}
