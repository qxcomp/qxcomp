package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 

import android.os.SystemClock

import android.net.ConnectivityManager;
import android.net.NetworkCapabilities
import android.app.Activity;
import android.content.Context
import androidx.work.WorkManager


// 获取Jvm内存
public fun getJvmMemory(which : Int) : Long {
    var jvrt = Runtime.getRuntime();
    var totmem = jvrt.totalMemory();
    var freemem = jvrt.freeMemory();

    return if (which==0) totmem else freemem
}

// var mactx : Context // see Util2.mactx

public fun isVPNConnected(ctx : Context): Boolean {
    val connectivityManager = ctx.getSystemService(ConnectivityManager::class.java) as ConnectivityManager
    val activeNetwork = connectivityManager.activeNetwork
    val networkCapabilities = connectivityManager.getNetworkCapabilities(activeNetwork)

    return networkCapabilities?.hasTransport(NetworkCapabilities.TRANSPORT_VPN) ?: false
}
public fun isVPNConnected2(): Boolean = isVPNConnected(Util2.mactx)

// 并不喜欢kotlin的基础类型还要大写字母开头
public fun sleepms(interrupt: Boolean, ms: Long) {
    if (interrupt) {
        Thread.sleep(ms)
    }else{
        SystemClock.sleep(ms)
    }
}

// usally used in Activity.onCreate
/*
Just if anyone needs a possible solution: I had the same problem, and it seems that many WorkerCalls originated from previous enqueuings in my application.

After adding following to my Application OnCreate-method, it works fine:        
 */

public fun  wkmanCancelAll() = WorkManager.getInstance(Util2.mactx).cancelAllWork()
