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
import org.qtproject.qt.android.bindings.QtActivity;

// org.qtproject.example.helloworld.FedyQkActivity
// todo Manifest.xml QtActivity => 
public class FedyQkActivity extends QtActivity {

    ReceiverHandler rcvhd = new ReceiverHandler();

    public void onCreate(Bundle savedInstanceState) {
        // Intent intent0 = new Intent(getIntent());
        super.onCreate(savedInstanceState);

        // ReceiverHandler.main123(null);
        // ReceiverHandler.initNativeLibFedy();

        // Intent intent = new Intent(this, FedyQkActivity.class);
        rcvhd.setupTopbarNotification(this);
        // 似乎只需要在onResume中处理，冷启动也会调用到onResume
        // 这里如果开启，会重复处理
        // rcvhd.handleBroadcast(this, "create");
    }


    protected void onStart() {
        super.onStart();

        rcvhd.setupSystemEvent(this);
    }

    protected void onRestart() {
        super.onRestart();
    }

    protected void onResume() {
        super.onResume();
        rcvhd.handleBroadcast(this, "resume");
    }

    protected void onPause() {
        super.onPause();
    }

    protected void onStop() {
        super.onStop();

        rcvhd.unsetSystemEvent(this);
    }

    protected void onDestroy() {
        super.onDestroy();
    }

    public void setupweaklock() {
        // 需要在Activity中调用 getSystemService
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        WakeLock wakelock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MyApp::MyWakelockTag");
        wakelock.acquire();
        wakelock.release();
    }
}
