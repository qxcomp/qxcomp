package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 
import android.content.Context;
import android.content.Intent;
import 	android.net.ConnectivityManager;
import 	android.net.NetworkInfo;

public class Util2 {

    public static Context mactx = null;

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
public static boolean sharetoapp(String text, String subject, String mime) {
    log.info("sharetoapp", mime, subject, text);

    if (text.startsWith("text/")) {
        sharetoapptext(text, subject, mime);
    }else if (text.startsWith("image/")) {
        sharetoappimage(text, subject, mime);
    }else{
        log.error("not support:" + mime);
        return false;
    }

    return true;
}
// todo
public static boolean sharetoappimage(String text, String subject, String mime) {
    log.info("sharetoapp", mime, subject, text);

     /*Create an ACTION_SEND Intent*/
    Intent intent = new Intent(Intent.ACTION_SEND);
    // intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
    /*This will be the actual content you wish you share.*/
    String shareBody = "Here is the share content body" + text;
    shareBody = text;
    /*The type of the content is text, obviously.*/
    intent.setType("text/plain");
    /*Applying information Subject and Body.*/
    // intent.putExtra(Intent.EXTRA_SUBJECT, "getString(R.string.share_subject)");
    intent.putExtra(Intent.EXTRA_SUBJECT, subject);
    intent.putExtra(Intent.EXTRA_TEXT, shareBody);
    /*Fire!*/
    // startActivity(Intent.createChooser(intent, getString(R.string.share_using)));
    Intent chooserIntent = Intent.createChooser(intent, "Choose Share APP");
    chooserIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    Util2.mactx. startActivity(chooserIntent);
    return true;
}

public static boolean sharetoapptext(String text, String subject, String mime) {
    log.info("sharetoapp", mime, subject, text);

     /*Create an ACTION_SEND Intent*/
    Intent intent = new Intent(Intent.ACTION_SEND);
    // intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
    /*This will be the actual content you wish you share.*/
    String shareBody = "Here is the share content body" + text;
    shareBody = text;
    /*The type of the content is text, obviously.*/
    intent.setType("text/plain");
    /*Applying information Subject and Body.*/
    // intent.putExtra(Intent.EXTRA_SUBJECT, "getString(R.string.share_subject)");
    intent.putExtra(Intent.EXTRA_SUBJECT, subject);
    intent.putExtra(Intent.EXTRA_TEXT, shareBody);
    /*Fire!*/
    // startActivity(Intent.createChooser(intent, getString(R.string.share_using)));
    Intent chooserIntent = Intent.createChooser(intent, "Choose Share APP");
    chooserIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    Util2.mactx. startActivity(chooserIntent);
    return true;
}

// 0, off, 1: wifi, 2: mobile data
public static int isNetworkAvailable() {
    boolean haveConnectedWifi = false;
    boolean haveConnectedMobile = false;

    ConnectivityManager cm
          = (ConnectivityManager) mactx.getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo activeNetworkInfo = cm != null ? cm.getActiveNetworkInfo() : null;
    NetworkInfo[] netInfo = cm.getAllNetworkInfo();

    for (NetworkInfo ni : netInfo) {
        if (ni.getTypeName().equalsIgnoreCase("WIFI"))
            if (ni.isConnected())
                haveConnectedWifi = true;
        if (ni.getTypeName().equalsIgnoreCase("MOBILE"))
            if (ni.isConnected())
                haveConnectedMobile = true;
    }

    boolean online = activeNetworkInfo != null && activeNetworkInfo.isConnected();
    if ( online) {
        if (haveConnectedWifi) return 1;
        else if (haveConnectedMobile) return 2;
    }

    return 0;
}


public static boolean reqrootperm() {
    boolean ok = ExecuteAsRootBase.canRunRootCommands();
    return ok;
}

}
