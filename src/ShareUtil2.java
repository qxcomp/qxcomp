package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 
import android.content.Context;
import android.content.Intent;

public class ShareUtil2 {
// static
public static void sharetoapp(String text) {
     /*Create an ACTION_SEND Intent*/
    Intent intent = new Intent(Intent.ACTION_SEND);
    /*This will be the actual content you wish you share.*/
    String shareBody = "Here is the share content body";
    /*The type of the content is text, obviously.*/
    intent.setType("text/plain");
    /*Applying information Subject and Body.*/
    intent.putExtra(Intent.EXTRA_SUBJECT, "getString(R.string.share_subject)");
    intent.putExtra(Intent.EXTRA_TEXT, shareBody);
    /*Fire!*/
    // startActivity(Intent.createChooser(intent, getString(R.string.share_using)));
    Util2.mactx.startActivity(Intent.createChooser(intent, "getString(R.string.share_using)"));
}
}
