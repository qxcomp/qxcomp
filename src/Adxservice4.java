package org.envsh.jvkt;

import android.app.Service;
import android.app.IntentService;
import android.content.*;
import android.os.*;
import android.os.Build;
import android.widget.Toast;
import android.app.PendingIntent;
import android.app.Notification;
import android.app.NotificationManager;
import android.media.AudioAttributes;
import android.app.NotificationChannel;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;
import java.util.concurrent.TimeUnit;
// import androidx.work.impl.utils.futures.AbstractFuture;
// import com.google.common.util.concurrent.ListenableFuture;

import yti.fedim.qmlui.R; // 这个包名是在哪儿设置的都忘记了

// 怎么好像还是在UI主线程呢？
// 08-16 09:26:17.504 29391 29391 I System.out: 35[I](CKS) hehehhe(CKS) inRunnable(CKS)
public class Adxservice4 extends IntentService {

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        startForeground(62318, builtNotification());
        // timerHandler.postDelayed(timerRunnable, 0);
        log.info("onCreate", "fgservice");
    }

    public Adxservice4() {
        super("Adxservice4");
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        Toast.makeText(this, "MyCustomService Handling Intent", Toast.LENGTH_LONG).show();
        // this method is called in background thread
        // INSERT THE WORK TO BE DONE HERE
        try {
        while (true) {
            Thread.sleep(9*1000);
            log.info("hehehhe", "IntentService");
        }
        }catch(Exception e) {
            e.printStackTrace();
        }
        log.info("hehehhe", "IntentService", "doneeee");
    }

    public Notification builtNotification()
    {

        NotificationManager notificationManager = (NotificationManager)
                getSystemService(Context.NOTIFICATION_SERVICE);
        assert notificationManager != null;

        NotificationCompat.Builder builder = null;
        Uri soundUri = Uri.parse("android.resource://" +
                getApplicationContext().getPackageName()
                + "/" + R.drawable.ic_launcher);
                // + "/" + R.raw.notification_sound);

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O)
        {
            int importance = NotificationManager.IMPORTANCE_HIGH;
            NotificationChannel notificationChannel =
                    new NotificationChannel("ID", "Name", importance);
            // Creating an Audio Attribute
            AudioAttributes audioAttributes = new AudioAttributes.Builder()
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .setUsage(AudioAttributes.USAGE_ALARM)
                    .build();

            notificationChannel.setSound(soundUri,audioAttributes);
            notificationManager.createNotificationChannel(notificationChannel);
            builder = new NotificationCompat.Builder(this, notificationChannel.getId());
        } else {
            builder = new NotificationCompat.Builder(this);
        }

        builder.setDefaults(Notification.DEFAULT_LIGHTS);
        builder.setSound(soundUri);

        String message = "Forever running service";
        builder
            //.setSmallIcon(R.ic_launcher)
            .setSmallIcon(R.drawable.ic_launcher)
                .setAutoCancel(false)
                .setPriority(Notification.PRIORITY_MAX)
                .setSound(soundUri)
                .setOngoing(true)
                .setOnlyAlertOnce(true)
                .setColor(Color.parseColor("#0f9595"))
                // .setContentTitle(getString(R.string.app_name))
                .setContentTitle("R.string.app_name")
                .setContentText(message);

        Intent launchIntent = getPackageManager().getLaunchIntentForPackage(getPackageName());
        launchIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, launchIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
        builder.setContentIntent(contentIntent);

        Notification notification = builder.build();
        notification.flags = Notification.FLAG_ONGOING_EVENT;
        return notification;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        ForegroundServiceLauncher.getInstance().startService(this);
    }

    // demo Runnable
    // 好像没有执行为什么？？？ 要在manifest.xml中添加 <service>标签
    // 这个不应该是在另外的线程的吗，sleep为什么会卡UI？？？
    // https://stackoverflow.com/questions/9147195/difference-between-service-and-thread-in-android
    // In short, the main difference between Service and Thread is that, Service runs on Main(UI) thread and Thread runes on its own thread. If we are using Service for long tasks, then it may cause block Main UI Thread.
    // 看来还不能这么用啊，不能用sleep.
    // 可能需要IntentService: https://stackoverflow.com/questions/5138265/how-to-start-service-in-new-thread-in-android
    private long runcnter = 0;
    private long timerTime = TimeUnit.SECONDS.toMillis(10);
    Handler timerHandler = new Handler();
    Runnable timerRunnable = new Runnable() {
        @Override
        public void run() {
            log.info("hehehhe", "inRunnable111");
            try {
                //Sync data to and fro every 300 seconds
                // if (runcnter > 0) {
                    // Thread.sleep(9*1000);
                // }
                runcnter ++;
                // LogKt.info("hehehhe", "inRunnable");
                log.info("hehehhe", "inRunnable");
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                timerHandler.postDelayed(this, timerTime);
            }
        }
    };
}

// usage:
/*
private long timerTime = TimeUnit.SECONDS.toMillis(300);
Handler timerHandler = new Handler();
Runnable timerRunnable = new Runnable() {
    @Override
    public void run() {
        try {
            //Sync data to and fro every 300 seconds
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            timerHandler.postDelayed(this, timerTime);
        }
    }
};
// Start Runnable from Service onCreate
@Override 
public void onCreate() {
    super.onCreate();
    startForeground(62318, builtNotification());
    timerHandler.postDelayed(timerRunnable, 0);
}
*/

// see https://medium.com/@Lakshya_Punhani/background-services-running-forever-in-android-part-2-6e3a667d36fd

