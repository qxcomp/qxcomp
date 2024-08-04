// package org.qtproject.example.helloworld;
package org.envsh.jvkt;

import java.util.*;
import java.lang.Class;
import java.lang.reflect.*;

import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.content.Context;
import android.app.Activity;

import org.qtproject.qt.android.bindings.QtActivity;

public class Foobar {

// protected static void setEnv(Map<String, String> newenv) throws Exception {
//   try {
//     Class<?> processEnvironmentClass = Class.forName("java.lang.ProcessEnvironment");
//     Field theEnvironmentField = processEnvironmentClass.getDeclaredField("theEnvironment");
//     theEnvironmentField.setAccessible(true);
//     Map<String, String> env = (Map<String, String>) theEnvironmentField.get(null);
//     env.putAll(newenv);
//     Field theCaseInsensitiveEnvironmentField = processEnvironmentClass.getDeclaredField("theCaseInsensitiveEnvironment");
//     theCaseInsensitiveEnvironmentField.setAccessible(true);
//     Map<String, String> cienv = (Map<String, String>) theCaseInsensitiveEnvironmentField.get(null);
//     cienv.putAll(newenv);
//   } catch (NoSuchFieldException e) {
//     Class[] classes = Collections.class.getDeclaredClasses();
//     Map<String, String> env = System.getenv();
//     for(Class cl : classes) {
//       if("java.util.Collections$UnmodifiableMap".equals(cl.getName())) {
//         Field field = cl.getDeclaredField("m");
//         field.setAccessible(true);
//         Object obj = field.get(env);
//         Map<String, String> map = (Map<String, String>) obj;
//         map.clear();
//         map.putAll(newenv);
//       }
//     }
//   }
// }
    companion object {
protected fun setEnv(newenv: Map<String, String> )  {
  try {
    var processEnvironmentClass = Class.forName("java.lang.ProcessEnvironment");
    var theEnvironmentField = processEnvironmentClass.getDeclaredField("theEnvironment");
    theEnvironmentField.setAccessible(true);
    // var env = (Map<String, String>)  theEnvironmentField.get(null);
    // env.putAll(newenv);
    // var theCaseInsensitiveEnvironmentField = processEnvironmentClass.getDeclaredField("theCaseInsensitiveEnvironment");
    // theCaseInsensitiveEnvironmentField.setAccessible(true);
    // var cienv = (Map<String, String>) theCaseInsensitiveEnvironmentField.get(null);
    // cienv.putAll(newenv);
  } catch (e:NoSuchFieldException) {
    var classes = Collections::class.java.getDeclaredClasses();
    var env = System.getenv();
    for ( cl in classes) {
        System.out.println(cl.getName());
        var s = "java.util.Collections\$UnmodifiableMap";
       if(s.equals(cl.getName())) {
        var field = cl.getDeclaredField("m");
        field.setAccessible(true);
        var obj = field.get(env);

        // var map = (Map<String, String>) obj;
        var map : Map<String, String>;
        // soshit
        @Suppress("UNCHECKED_CAST")
        map = obj as Map<String, String>;
        // map.clear();
        // map.putAll(newenv);
      }       
    }
    // for(Class cl : classes) {
    //   if("java.util.Collections$UnmodifiableMap".equals(cl.getName())) {
    //     var field = cl.getDeclaredField("m");
    //     field.setAccessible(true);
    //     var obj = field.get(env);
    //     var map = (Map<String, String>) obj;
    //     map.clear();
    //     map.putAll(newenv);
    //   }
    // }
  }
}


        init  {
            System.out.println("fsdfadsfdf");
            // System.loadLibrary("fsdfadsfdf");
            var envs = HashMap<String,String>();
            envs.put("QT_ANDROID_DEBUGGER_MAIN_THREAD_SLEEP_MS", "123");
            try {
                setEnv(envs);
            }catch(e :Exception) {
                System.out.println(e);
            }
        }
    }
//     static {
//         // os.setenv();
//         System.out.println("fsdfadsfdf");
//         Map<String,String> envs = new HashMap<String,String>();
//         envs.put("QT_ANDROID_DEBUGGER_MAIN_THREAD_SLEEP_MS", "123");
//         try {
//         setEnv(envs);
//         }catch(Exception e) {
//             System.out.println(e);
//         }
//     }
//   aijfiefj  
}
//aewkfewif
