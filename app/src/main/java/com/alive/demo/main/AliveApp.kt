package com.alive.demo.main

import android.app.Application
import android.content.Context
import android.content.Intent
import android.util.Log
import com.alive.demo.keeplive.KeepAlive
import com.alive.demo.keeplive.KeepAliveConfigs
import com.alive.demo.refelction.Reflection

class AliveApp : Application() {
    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        Reflection.unuseal(base!!)
        val configs = KeepAliveConfigs(
            KeepAliveConfigs.Config(
                "$packageName:test",
                TestServices::class.java.getCanonicalName()
            )
        )
        //configs.ignoreBatteryOptimization()
        // configs.rebootThreshold(10*1000, 3);
        configs.setOnBootReceivedListener(object : KeepAliveConfigs.OnBootReceivedListener {
            override fun onReceive(context: Context, intent: Intent?) {
                context.startService(Intent(context, TestServices::class.java))
            }
        })
        KeepAlive.init(base, configs)
    }

    override fun onCreate() {
        super.onCreate()
    }
}