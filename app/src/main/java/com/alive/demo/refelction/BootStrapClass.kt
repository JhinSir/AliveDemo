package com.alive.demo.refelction

import android.os.Build
import android.os.Build.VERSION
import android.util.Log
import java.lang.reflect.Method

class BootStrapClass {

    companion object{
        @JvmStatic
        val TAG="BootStrapClass"
        @JvmStatic
        var sVmRuntime: Any? = null
        @JvmStatic
        var setHiddenApiExemptions: Method? = null
    }

    init {
        if (VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            try {
                val forName =
                    Class::class.java.getDeclaredMethod("forName", String::class.java)
                val getDeclaredMethod = Class::class.java.getDeclaredMethod(
                    "getDeclaredMethod",
                    String::class.java,
                    Array<Any>::class.java
                )

                val vmRuntimeClass = forName.invoke(null, "dalvik.system.VMRuntime") as Class<*>
                val getRuntime =
                    getDeclaredMethod.invoke(vmRuntimeClass, "getRuntime", null) as Method
                BootStrapClass.setHiddenApiExemptions =
                    getDeclaredMethod.invoke(
                        vmRuntimeClass, "setHiddenApiExemptions", arrayOf<Class<*>>(
                            Array<String>::class.java
                        )
                    ) as Method
                BootStrapClass.sVmRuntime = getRuntime.invoke(null)
            } catch (e: Throwable) {
               Log.e(TAG,"refection error ${e.toString()}")
            }
        }
    }



    /**
     * make the method exempted from hidden API check.
     *
     * @param method the method signature prefix.
     * @return true if success.
     */
    fun exempt(method: String): Boolean {
        return exempt(*arrayOf(method))
    }

    /**
     * make specific methods exempted from hidden API check.
     *
     * @param methods the method signature prefix, such as "Ldalvik/system", "Landroid" or even "L"
     * @return true if success
     */
    fun exempt(vararg methods: String?): Boolean {
        if (BootStrapClass.sVmRuntime == null || BootStrapClass.setHiddenApiExemptions == null) {
            return false
        }

        try {
            BootStrapClass.setHiddenApiExemptions?.invoke(
                BootStrapClass.sVmRuntime,
                *arrayOf<Any>(methods)
            )
            return true
        } catch (e: Throwable) {
            return false
        }
    }

    /**
     * Make all hidden API exempted.
     *
     * @return true if success.
     */
    fun exemptAll(): Boolean {
        return exempt(*arrayOf("L"))
    }
}