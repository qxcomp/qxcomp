package org.envsh.jvkt;

import android.app.Service;
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


import yti.fedim.qmlui.R; // 这个包名是在哪儿设置的都忘记了

public class Adxservice3 extends Service {

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
        timerHandler.postDelayed(timerRunnable, 0);
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
                PendingIntent.FLAG_UPDATE_CURRENT);
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
    private long timerTime = TimeUnit.SECONDS.toMillis(300);
    Handler timerHandler = new Handler();
    Runnable timerRunnable = new Runnable() {
        @Override
        public void run() {
            try {
                //Sync data to and fro every 300 seconds
                Thread.sleep(90*1000);
                // log.info("hehehhe", "inRunnable");
                System.out.println("inRunnabe");
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

