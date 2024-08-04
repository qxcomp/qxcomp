// package org.qtproject.example.helloworld;
package org.envsh.jvkt;

// https://www.geeksforgeeks.org/broadcast-receiver-in-android-with-example/
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.widget.Toast;
 
public class MyBroadcastReceiver : BroadcastReceiver() {

    // @Override
    override public fun onReceive(context: Context , intent: Intent ) {
        var sb =  StringBuilder();
        sb.append("Action: " + intent.getAction() + "\n");
        sb.append("URI: " + intent.toUri(Intent.URI_INTENT_SCHEME).toString() + "\n");
        var log = sb.toString();
        Toast.makeText(context, log, Toast.LENGTH_LONG).show();
    }
}