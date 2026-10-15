package org.envsh.jvkt;

import java.lang.*;
import java.io.IOException; 

import android.os.SystemClock

import android.net.ConnectivityManager;
import android.net.NetworkCapabilities
import android.net.NetworkRequest;
import android.net.Network;
import android.app.Activity;
import android.content.Context
import androidx.work.WorkManager


public fun getappctx() : Context = Util2.mactx

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

// network relate
val networkRequest = NetworkRequest.Builder()
        .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
        .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
        .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
        .build()

private val networkCallback = object : ConnectivityManager.NetworkCallback() {
    // network is available for use
    override fun onAvailable(network: Network) {
        super.onAvailable(network)
        var nettype = Util2.isNetworkAvailable()
        log.warn("mobnet.oked", network)
        lffi.call2go(0, "mobnet.oln", 1, nettype) // 最后参数，网络类型
    }
    // lost network connection
    override fun onLost(network: Network) {
        super.onLost(network)
        log.warn("mobnet.lost", network)
        lffi.call2go(0, "mobnet.oln", 0, 0)
    }

    // Network capabilities have changed for the network
    override fun onCapabilitiesChanged(
            network: Network,
            networkCapabilities: NetworkCapabilities
    ) {
        super.onCapabilitiesChanged(network, networkCapabilities)
        val unmetered = networkCapabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_NOT_METERED)
        log.warn("modnet.capnew", network, networkCapabilities, unmetered)
    }

}

// todo permissions:android.permission.CHANGE_NETWORK_STATE,android.permission.WRITE_SETTINGS.
// 但是我没有要change，只想读取。。。
public fun regnetupstate() {
    val connectivityManager = getappctx().getSystemService(ConnectivityManager::class.java) as ConnectivityManager
    connectivityManager.requestNetwork(networkRequest, networkCallback)
}

public class Util {
companion object {
    // 可以在同一个文件中调用regnetupstate()
    // kotlin这种机制还是挺玄妙的,不好
    // 在其他类中也是当全局函数这么调用？？？regnetupstate() // 竟然要这么调用，他喵的

    public fun regnetupstate1() = regnetupstate()
}
}
