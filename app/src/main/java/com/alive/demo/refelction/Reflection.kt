package com.alive.demo.refelction

import android.content.Context
import android.os.Build.VERSION

object Reflection {
    fun unuseal(base:Context):Int{
        if (VERSION.SDK_INT < 28) {
            // Below Android P, ignore
            return 0
        }
        // try exempt API first.
        if (BootStrapClass().exemptAll()) {
            return 0
        }
//        if (com.boolbird.keepalive.Reflection.unsealByDexFile(context)) {
//            return 0
//        }
        return -1
    }
}