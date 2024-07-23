package com.alive.demo.main

import android.content.Intent
import android.util.Log
import com.alive.demo.keeplive.KeepAliveService

class TestServices : KeepAliveService() {
    var x: Int = 0
    private val exit = false

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Thread {
            while (!exit) {
                try {
                    Thread.sleep(1000)
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
                x++
                Log.d("TestServices", x.toString() + "")
            }
        }.start()
        return super.onStartCommand(intent, flags, startId)
    }
}