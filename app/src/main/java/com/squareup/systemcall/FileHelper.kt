package com.squareup.systemcall

import android.content.Context
import java.io.File

object FileHelper {
    lateinit var file: File
    fun initFile(context: Context) {
        file = File(context.getExternalFilesDir(null), "svcTest").also {
            it.delete()
            it.createNewFile()
            it.writeText("This is test svc file content")
        }
    }
}