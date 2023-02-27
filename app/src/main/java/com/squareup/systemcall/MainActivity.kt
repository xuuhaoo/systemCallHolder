package com.squareup.systemcall

import android.Manifest
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.squareup.systemcall.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.MANAGE_EXTERNAL_STORAGE), 1000)

    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        // Example of a call to a native method
//        binding.sampleText.text = stringFromJNI()
        val file = File(getExternalFilesDir(null), "svcTest").also {
            it.delete()
            it.createNewFile()
            it.writeText("This is test svc file content")
        }
        binding.sampleText.text = readFileSysCall(file.path)
    }

    /**
     * A native method that is implemented by the 'systemcall' native library,
     * which is packaged with this application.
     */
    private external fun readFileSysCall(filePath: String): String

    private external fun ptraceViewSvcCall()

    companion object {
        // Used to load the 'systemcall' library on application startup.
        init {
            System.loadLibrary("systemcall")
        }
    }
}