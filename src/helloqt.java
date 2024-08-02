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

class Foobar {

protected static void setEnv(Map<String, String> newenv) throws Exception {
  try {
    Class<?> processEnvironmentClass = Class.forName("java.lang.ProcessEnvironment");
    Field theEnvironmentField = processEnvironmentClass.getDeclaredField("theEnvironment");
    theEnvironmentField.setAccessible(true);
    Map<String, String> env = (Map<String, String>) theEnvironmentField.get(null);
    env.putAll(newenv);
    Field theCaseInsensitiveEnvironmentField = processEnvironmentClass.getDeclaredField("theCaseInsensitiveEnvironment");
    theCaseInsensitiveEnvironmentField.setAccessible(true);
    Map<String, String> cienv = (Map<String, String>) theCaseInsensitiveEnvironmentField.get(null);
    cienv.putAll(newenv);
  } catch (NoSuchFieldException e) {
    Class[] classes = Collections.class.getDeclaredClasses();
    Map<String, String> env = System.getenv();
    for(Class cl : classes) {
      if("java.util.Collections$UnmodifiableMap".equals(cl.getName())) {
        Field field = cl.getDeclaredField("m");
        field.setAccessible(true);
        Object obj = field.get(env);
        Map<String, String> map = (Map<String, String>) obj;
        map.clear();
        map.putAll(newenv);
      }
    }
  }
}

    static {
        // os.setenv();
        System.out.println("fsdfadsfdf");
        Map<String,String> envs = new HashMap<String,String>();
        envs.put("QT_ANDROID_DEBUGGER_MAIN_THREAD_SLEEP_MS", "123");
        try {
        setEnv(envs);
        }catch(Exception e) {
            System.out.println(e);
        }
    }
//   aijfiefj  
}
//aewkfewif
