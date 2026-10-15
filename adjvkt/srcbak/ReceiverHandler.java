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

import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.content.Context;
import android.app.Activity;
// import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Uri;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
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

    // deprecated not work
    public interface LibC { // A representation of libC in Java
        int puts(String s); // mapping of the puts function, in C `int puts(const char *s);`
    }
    // deprecated not work
    public static void main123(String[] args) {
        // 缺少 libjffi-1.2.so，没找到给arm编译好的，不好用啊
        LibC libc = LibraryLoader.create(LibC.class).load("c"); // load the "c" library into the libc variable

        libc.puts("Hello World!"); // prints "Hello World!" to console
    }

    // deprecated not work
    public interface LibFedy {
        int java2gocall(int f, String s);
    }

    // deprecated not work
    static LibFedy fdyh = null;

    // deprecated not work
    public static void initNativeLibFedy() {
        fdyh = LibraryLoader.create(LibFedy.class).load("hellogo");
        System.out.println("LibFedy init:" + fdyh);
    }

    AirplaneModeChangeReceiver airplaneModeChangeReceiver = new AirplaneModeChangeReceiver();
    MyBroadcastReceiver myBroadcastReceiver = new MyBroadcastReceiver();

    public void setupTopbarNotification(Activity activity) {
        if (false) {
        // notification
        // Bitmap bm = Bitmap.createScaledBitmap(BitmapFactory.decodeResource(getResources(), R.drawable.avatar), 
        //         getResources().getDimensionPixelSize(android.R.dimen.notification_large_icon_width),
        //         getResources().getDimensionPixelSize(android.R.dimen.notification_large_icon_height), 
        //         true);
        Intent intent = new Intent(activity, FedyQkActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(activity, 01, intent, Intent.FLAG_ACTIVITY_CLEAR_TASK | PendingIntent.FLAG_IMMUTABLE);
        Notification.Builder builder = new Notification.Builder(activity.getApplicationContext());
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
        Notification notification = builder.build();
        NotificationManager notificationManger = 
                (NotificationManager) activity.getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManger.notify(01, notification);   
        }

    }

    public void setupSystemEvent(Activity activity) {
        if (false) {
        IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        activity.registerReceiver(airplaneModeChangeReceiver, filter);

        // filter = new IntentFilter(Intent.ACTION_DATE_CHANGED);
        // registerReceiver(airplaneModeChangeReceiver, filter);

        // filter = new IntentFilter(Intent.ACTION_CONNECTIVITY_CHANGE);
        // registerReceiver(airplaneModeChangeReceiver, filter);

        IntentFilter filter2 = new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
        filter2.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter2.addAction(Intent.ACTION_BOOT_COMPLETED);
        // filter2.addAction(Intent.ACTION_CHARGING);
        filter2.addAction(Intent.ACTION_DATE_CHANGED);
        activity.registerReceiver(myBroadcastReceiver, filter2);
        }
    }
    // todo
    public void unsetSystemEvent(Activity activity) {
        if (false) {
        activity.unregisterReceiver(airplaneModeChangeReceiver);
        activity.unregisterReceiver(myBroadcastReceiver);
        }
    }

    public void handleBroadcast(Activity activity, String where) {
        if (true) {
        // share message/data
        // Get intent, action and MIME type 
        Intent intent2 = activity.getIntent();
        // Intent intent2 = intent;
        // Intent intent2 = new Intent(this, FedyQkActivity.class);
        if (intent2 == null) {
            Toast.makeText(activity, "cannot got Intent???", Toast.LENGTH_LONG).show();
            System.out.println(where.toString() +"Fedy " + "cannot got Intent???");
        }
        if (intent2 != null) {
        String action = intent2.getAction();
        String type = intent2.getType();

        Bundle data = intent2.getExtras();
        if (data != null) {
            // good works
            for (String key : data.keySet()) {
                Object value = data.get(key);
                // System.out.println("Fedy " + key + ":" + (value != null ? value : "NULL") + ":" + value.getClass().getName());
                // Log.e("Fedy " + value );
            }
        }

        Toast.makeText(activity, type + " ooo " + action, Toast.LENGTH_LONG).show();
        // System.out.println("Fedy " + type + " ooo " + action + "ddd" + "" + "xxx" +intent2.getExtras()  +"eee"  + data.size() );
        // 和 intent.getStringExtra(Intent.EXTRA_TEXT) 结果一样的，费半天劲。
        // 就是要注意多行的文本，System.out.println会输出为多行
        // 有时候只看到第一行以为没有收到数据。
        Object oetext = data.get(Intent.EXTRA_TEXT);
        Object oesubject = data.get(Intent.EXTRA_SUBJECT);
        String etext = oetext != null ? oetext.toString() : "EXTRA_TEXT NULL";
        String esubject = oesubject != null ? oesubject.toString() : "EXTRA_TEXT NULL";
        System.out.println("EXTRA_TEXT" + data.get(Intent.EXTRA_TEXT));
        System.out.println("EXTRA_SUBJECT" + data.get(Intent.EXTRA_SUBJECT));

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

    void handleSendText(Activity activity, Intent intent, String where) {
        String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
        if (sharedText != null) {
            // Update UI to reflect text being shared
            Toast.makeText(activity, sharedText, Toast.LENGTH_LONG).show();
            System.out.println(where +"Fedy133 "  + " ooo " + sharedText);
            String cmdpack = "sendsharedata"+Litffi.datasep+sharedText;
            Litffi.java2gocall(0, cmdpack);
            System.out.println(where +"Fedy133 "  + " kkk " + cmdpack);
        }
    }

    void handleSendImage(Activity activity, Intent intent) {
        Uri imageUri = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
        if (imageUri != null) {
            // Update UI to reflect image being shared
        }
    }

    void handleSendMultipleImages(Activity activity, Intent intent) {
        ArrayList<Uri> imageUris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
        if (imageUris != null) {
            // Update UI to reflect multiple images being shared
        }
    }


}