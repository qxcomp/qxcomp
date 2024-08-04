// package org.qtproject.example.helloworld;
package org.envsh.jvkt;

// https://www.geeksforgeeks.org/broadcast-receiver-in-android-with-example/
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.widget.Toast;
 
public class AirplaneModeChangeReceiver extends BroadcastReceiver {
 
    @Override
    public void onReceive(Context context, Intent intent) {
 
        if (isAirplaneModeOn(context.getApplicationContext())) {
            Toast.makeText(context, "AirPlane mode is on", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(context, "AirPlane mode is off", Toast.LENGTH_SHORT).show();
        }
    }
 
    private static boolean isAirplaneModeOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }
}

// class AirplaneModeChangeReceiver extends BroadcastReceiver {

//        public void onReceive( Context context, Intent intent) {

//             // logic of the code needs to be written here

//       }

//     //   IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED).also {

//     //                  // receiver is the broadcast receiver that we have registered

//     //                 // and it is the intent filter that we have created

//     //                 registerReceiver(receiver,it)

//     //   }

// }
