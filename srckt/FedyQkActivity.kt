// package org.qtproject.example.helloworld;
package org.envsh.jvkt;

// import org.qtproject.example.helloworld.R;
// import android.R;
// 原来叫这个名字
// find . -name R.jar
import yti.fedim.qmlui.R; // 这个包名是在哪儿设置的都忘记了

import java.util.*;
import java.lang.Class;
import java.lang.reflect.*;

import android.util.Log;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.Activity;
// import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.Uri;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.provider.Settings;
import android.widget.Toast;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;


// import jnr.ffi.LibraryLoader;
import org.qtproject.qt.android.bindings.QtActivity;


import android.app.NotificationChannel
import android.content.pm.PackageManager
import android.graphics.Color
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.app.NotificationCompat
// import com.google.android.material.dialog.MaterialAlertDialogBuilder
import androidx.work.*
import androidx.lifecycle.Observer
import androidx.annotation.RequiresApi



// org.qtproject.example.helloworld.FedyQkActivity
// todo Manifest.xml QtActivity => 
public class FedyQkActivity : QtActivity () {

    var rcvhd = ReceiverHandler();

    // public val notificationPermissionLauncher =
    //     registerForActivityResult(ActivityResultContracts.RequestPermission()) { isGranted ->
    //         hasNotificationPermissionGranted = isGranted
    //         if (!isGranted) {
    //             if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
    //                 if (Build.VERSION.SDK_INT >= 33) {
    //                     if (shouldShowRequestPermissionRationale(android.Manifest.permission.POST_NOTIFICATIONS)) {
    //                         showNotificationPermissionRationale()
    //                     } else {
    //                         showSettingDialog()
    //                     }
    //                 }
    //             }
    //         } else {
    //             Toast.makeText(applicationContext, "notification permission granted", Toast.LENGTH_SHORT)
    //                 .show()
    //         }
    //     }

    // private fun showSettingDialog() {
    //     MaterialAlertDialogBuilder(this, com.google.android.material.R.style.MaterialAlertDialog_Material3)
    //         .setTitle("Notification Permission")
    //         .setMessage("Notification permission is required, Please allow notification permission from setting")
    //         .setPositiveButton("Ok") { _, _ ->
    //             val intent = Intent(ACTION_APPLICATION_DETAILS_SETTINGS)
    //             intent.data = Uri.parse("package:$packageName")
    //             startActivity(intent)
    //         }
    //         .setNegativeButton("Cancel", null)
    //         .show()
    // }

    // private fun showNotificationPermissionRationale() {

    //     MaterialAlertDialogBuilder(this, com.google.android.material.R.style.MaterialAlertDialog_Material3)
    //         .setTitle("Alert")
    //         .setMessage("Notification permission is required, to show notification")
    //         .setPositiveButton("Ok") { _, _ ->
    //             if (Build.VERSION.SDK_INT >= 33) {
    //                 notificationPermissionLauncher.launch(android.Manifest.permission.POST_NOTIFICATIONS)
    //             }
    //         }
    //         .setNegativeButton("Cancel", null)
    //         .show()
    // }

    var hasNotificationPermissionGranted = false

    // override fun onCreate(savedInstanceState: Bundle?) {
    //     super.onCreate(savedInstanceState)
    //     setContentView(R.layout.activity_main)

    //     findViewById<Button>(R.id.btnRequestPermission).setOnClickListener {
    //         if (Build.VERSION.SDK_INT >= 33) {
    //             notificationPermissionLauncher.launch(android.Manifest.permission.POST_NOTIFICATIONS)
    //         } else {
    //             hasNotificationPermissionGranted = true
    //         }
    //     }
    //     findViewById<Button>(R.id.btnShowNotification).setOnClickListener {
    //         if (checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) {
    //             showNotification()
    //         }
    //     }
    // }

    private fun showNotification() {

        val channelId = "12345"
        val description = "Test Notification"

        val notificationManager =
            getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        log.info("notychan creating", Build.VERSION.SDK_INT ,Build.VERSION_CODES.O)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val notificationChannel =
                NotificationChannel(channelId, description, NotificationManager.IMPORTANCE_HIGH)
            // NotificationManager.IMPORTANCE_DEFAULT
            notificationChannel.lightColor = Color.BLUE

            notificationChannel.enableVibration(true)
            notificationManager.createNotificationChannel(notificationChannel)
            log.info("notychan created")
        }

        val  builder = NotificationCompat.Builder(this, channelId)
            .setOngoing(true) // permanent
            .setContentTitle("Hello World")
            .setContentText("Test Notification")
            .setSmallIcon(R.drawable.ic_launcher)
            // .setSmallIcon(R.drawable.ic_launcher_foreground)
            // .setLargeIcon(
            //     BitmapFactory.decodeResource(
            //         this.resources, R.drawable
            //             .ic_launcher_background
            //     )
            // )
        notificationManager.notify(12345, builder.build())
    }

    /////////
    override fun onCreate(savedInstanceState: Bundle?) {
        // setContentView(R.layout.activity_main)
        var intent0 = Intent(getIntent());
        super.onCreate(savedInstanceState);
        Util2.mactx = getApplicationContext()
        // UtilKt.regnetupstate() // kotlin 还是有问题，为什么这么调用不行呢？
        // Util.regnetupstate1() // 这样才能够调用
        regnetupstate() // 竟然要这么调用，他喵的

        // startService(Intent(this, Adxservice2::class.java));
        // ForegroundServiceLauncher.getInstance().startService(this);
        // ForegroundServiceLauncher4.getInstance().startService(this);

/*
Just if anyone needs a possible solution: I had the same problem, and it seems that many WorkerCalls originated from previous enqueuings in my application.

After adding following to my Application OnCreate-method, it works fine:        
 */
        WorkManager.getInstance(Util2.mactx).cancelAllWork();
        setupLongtimeDownload(this)

        showNotification()

        // ReceiverHandler.main123(null);
        // ReceiverHandler.initNativeLibFedy();

        var intent = Intent(this, FedyQkActivity::class.java);
        // rcvhd.setupTopbarNotification(this);
        // 似乎只需要在onResume中处理，冷启动也会调用到onResume
        // 这里如果开启，会重复处理
        rcvhd.handleBroadcast(this, getIntent(), "create");
    }

    override protected fun onStart() {
        super.onStart();

        rcvhd.setupSystemEvent(this);
    }

    override protected fun  onRestart() {
        super.onRestart();
    }

    // ??? bug Activity top resumed state loss timeout???
    override protected fun  onResume() {
        log.info("onResume111"); // 这个也不会执行
        super.onResume();
        log.info("onResume222");
        rcvhd.handleBroadcast(this, getIntent(), "resume");
    }
    override protected fun onNewIntent(p0: Intent) {
        super.onNewIntent(p0);
        var  data = p0.getExtras();
        if (data != null) {
            // good works
            for (key in data.keySet()) {
                var value = data.get(key) as Object;
                // Unresolved reference 'getClass'
                System.out.println("Fedy " + key + ":" + (if (value != null)  value else  "NULL") + ":" + value.getClass().getName());
                // Log.e("Fedy " + value );
            }
        }
        System.out.println("share by newIntent???" + p0.getExtras());

        rcvhd.handleBroadcast(this, p0, "newIntent");
        
    }

    override protected fun  onPause() {
        super.onPause();
    }

    override protected fun  onStop() {
        super.onStop();

        rcvhd.unsetSystemEvent(this);
    }

    override protected fun  onDestroy() {
        super.onDestroy();
    }

    public fun  setupweaklock() {
        // 需要在Activity中调用 getSystemService
        var pm = this.getSystemService(Context.POWER_SERVICE) as PowerManager;
        var wakelock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MyApp::MyWakelockTag");
        wakelock.acquire();
        wakelock.release();
        System.out.println("hehehe");
    }
}
