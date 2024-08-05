// package org.qtproject.example.helloworld;
package org.envsh.jvkt;

// import org.envsh.jvkt.R;
// import android.R;
// todo 这个引入包放在库里不合适
// 原来叫这个名字
// find . -name R.jar
import yti.fedim.qmlui.R;

import java.util.*;
import java.lang.Class;
import java.lang.reflect.*;


import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
// import androidx.appcompat.app.AppCompatActivity;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Uri;

import android.content.BroadcastReceiver;
import android.provider.Settings;
import android.widget.Toast;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;

import jnr.ffi.LibraryLoader;



public class ReceiverHandler  {

    // // deprecated not work
    // public interface LibC { // A representation of libC in Java
    //     int puts(String s); // mapping of the puts function, in C `int puts(const char *s);`
    // }
    // // deprecated not work
    // public static void main123(String[] args) {
    //     // 缺少 libjffi-1.2.so，没找到给arm编译好的，不好用啊
    //     LibC libc = LibraryLoader.create(LibC.class).load("c"); // load the "c" library into the libc variable

    //     libc.puts("Hello World!"); // prints "Hello World!" to console
    // }

    // // deprecated not work
    // public interface LibFedy {
    //     int java2gocall(int f, String s);
    // }

    // // deprecated not work
    // static LibFedy fdyh = null;

    // // deprecated not work
    // public static void initNativeLibFedy() {
    //     fdyh = LibraryLoader.create(LibFedy.class).load("hellogo");
    //     System.out.println("LibFedy init:" + fdyh);
    // }

    var airplaneModeChangeReceiver =  AirplaneModeChangeReceiver();
    var myBroadcastReceiver = MyBroadcastReceiver();

    public fun setupTopbarNotification(activity: Activity ) {
        if (false) {
        // notification
        // Bitmap bm = Bitmap.createScaledBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.avatar), 
        //         getResources().getDimensionPixelSize(android.R.dimen.notification_large_icon_width),
        //         getResources().getDimensionPixelSize(android.R.dimen.notification_large_icon_height), 
        //         true);
        var  intent = Intent(activity, FedyQkActivity::class.java);
        var  pendingIntent = PendingIntent.getActivity(activity, 1, intent, Intent.FLAG_ACTIVITY_CLEAR_TASK or PendingIntent.FLAG_IMMUTABLE);
        var  builder =  Notification.Builder(activity.getApplicationContext());
        builder.setContentTitle("This is the title");
        builder.setContentText("This is the text");
        builder.setSubText("Some sub text");
        builder.setNumber(101);
        builder.setContentIntent(pendingIntent);
        builder.setTicker("Fancy Notification");
        // Invalid notification (no valid small icon):
        builder.setSmallIcon(R.drawable.ic_launcher);
        // builder.setLargeIcon(bm);
        builder.setAutoCancel(true);
        builder.setPriority(0);
        var  notification = builder.build();
        var  notificationManger = 
                 activity.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager;
        notificationManger.notify(1, notification);   
        }

    }

    public fun setupSystemEvent(activity:Activity ) {
        if (false) {
        var  filter =  IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        // activity.registerReceiver(airplaneModeChangeReceiver, filter);

        // filter = new IntentFilter(Intent.ACTION_DATE_CHANGED);
        // registerReceiver(airplaneModeChangeReceiver, filter);

        // filter = new IntentFilter(Intent.ACTION_CONNECTIVITY_CHANGE);
        // registerReceiver(airplaneModeChangeReceiver, filter);

        var  filter2 = IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
        filter2.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter2.addAction(Intent.ACTION_BOOT_COMPLETED);
        // filter2.addAction(Intent.ACTION_CHARGING);
        filter2.addAction(Intent.ACTION_DATE_CHANGED);
        activity.registerReceiver(myBroadcastReceiver, filter2);
        }
    }
    // todo
    public fun unsetSystemEvent(activity:Activity ) {
        if (false) {
        activity.unregisterReceiver(airplaneModeChangeReceiver);
        activity.unregisterReceiver(myBroadcastReceiver);
        }
    }

    public fun handleBroadcast(activity:Activity, intent :Intent, where:String ) {
        if (true) {
        // share message/data
        // Get intent, action and MIME type 
        var  intent2 = if (where == "newIntent") intent else activity.getIntent();
        // Intent intent2 = intent;
        // Intent intent2 = new Intent(this, FedyQkActivity.class);
        if (intent2 == null) {
            Toast.makeText(activity, "cannot got Intent???", Toast.LENGTH_LONG).show();
            System.out.println(where.toString() +"Fedy " + "cannot got Intent???");
        }
        if (intent2 != null) {
        var  action = intent2.getAction();
        var  type = intent2.getType();

        var  data = intent2.getExtras();
        if (data != null) {
            // good works
            for (key in data.keySet()) {
                var value = data.get(key) as Object;
                // Unresolved reference 'getClass'
                System.out.println("Fedy " + key + ":" + (if (value != null)  value else  "NULL") + ":" + value.getClass().getName());
                // Log.e("Fedy " + value );
            }
        }

        Toast.makeText(activity, type + " ooo " + action, Toast.LENGTH_LONG).show();
        System.out.println("Fedy " + type + " ooo " + action + "ddd" + "" + "xxx" +intent2.getExtras()  +"eee"  + data?.size() );
        // 和 intent.getStringExtra(Intent.EXTRA_TEXT) 结果一样的，费半天劲。
        // 就是要注意多行的文本，System.out.println会输出为多行
        // 有时候只看到第一行以为没有收到数据。
        var  oetext = data?.get(Intent.EXTRA_TEXT);
        var  oesubject = data?.get(Intent.EXTRA_SUBJECT);
        var  etext = if (oetext != null)  oetext.toString() else "EXTRA_TEXT NULL";
        var  esubject = if ( oesubject != null)  oesubject.toString() else "EXTRA_TEXT NULL";
        System.out.println("EXTRA_TEXT" + data?.get(Intent.EXTRA_TEXT));
        System.out.println("EXTRA_SUBJECT" + data?.get(Intent.EXTRA_SUBJECT));

        if (Intent.ACTION_SEND.equals(action) && type != null) {
            if ("text/plain".equals(type)) {
                handleSendText(activity, intent2, where); // Handle text being sent
            } else if (type.startsWith("image/")) {
                handleSendImage(activity, intent2); // Handle single image being sent
            }
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action) && type != null) {
            if (type.startsWith("image/")) {
                handleSendMultipleImages(activity, intent2); // Handle multiple images being sent
            }
        } else {
            // Handle other intents, such as being started from the home screen
        }   
        }     
        }     
    }

    fun handleSendText(activity:Activity , intent: Intent , where: String ) {
        var  sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
        if (sharedText != null) {
            // Update UI to reflect text being shared
            Toast.makeText(activity, sharedText, Toast.LENGTH_LONG).show();
            System.out.println(where +"Fedy133 "  + " ooo " + sharedText);
            var  cmdpack = "sendsharedata"+Litffi.datasep+sharedText;
            Litffi.java2gocall(0, cmdpack);
            System.out.println(where +"Fedy133 "  + " kkk " + cmdpack);
        }
    }

    fun handleSendImage(activity: Activity , intent: Intent) {
        var imageUri = intent.getParcelableExtra(Intent.EXTRA_STREAM) as? Uri;
        if (imageUri != null) {
            // Update UI to reflect image being shared
        }
    }

    fun handleSendMultipleImages(activity:Activity , intent: Intent ) {
        // kotlin cannot as!!!
        // soshit syntax
        var imageUris = intent.getParcelableArrayListExtra<Uri>(Intent.EXTRA_STREAM);
        if (imageUris != null) {
            // Update UI to reflect multiple images being shared
        }
    }


}