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

// org.qtproject.example.helloworld.FedyQkActivity
// todo Manifest.xml QtActivity => 
public class FedyQkActivity : QtActivity () {

    var rcvhd = ReceiverHandler();

    override fun onCreate(savedInstanceState: Bundle?) {
        // setContentView(R.layout.activity_main)
        var intent0 = Intent(getIntent());
        super.onCreate(savedInstanceState);

        // ReceiverHandler.main123(null);
        // ReceiverHandler.initNativeLibFedy();

        var intent = Intent(this, FedyQkActivity::class.java);
        rcvhd.setupTopbarNotification(this);
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

    override protected fun  onResume() {
        super.onResume();
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
