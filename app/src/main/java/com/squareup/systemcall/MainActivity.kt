package com.squareup.systemcall

import android.Manifest
import android.os.Bundle
import android.widget.Toast
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

    ActivityCompat.requestPermissions(
      this,
      arrayOf(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
      ),
      1000
    )
  }

  override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
    super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    binding.btn.setOnClickListener {
      binding.sampleText.text = Native.readFileSysCall(FileHelper.file.path)
      Toast.makeText(this@MainActivity, File(FileHelper.file.path).readText(), Toast.LENGTH_LONG).show()
    }
  }
}