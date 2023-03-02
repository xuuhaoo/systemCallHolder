package com.squareup.systemcall

import android.Manifest
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.syscallholder.SysCallHolder
import com.squareup.systemcall.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

  private lateinit var binding: ActivityMainBinding

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)
    binding.btn.setOnClickListener {
      binding.sampleText.text = SysCallHolder.readFileSysCall(FileHelper.file.path).trim()
      Toast.makeText(this@MainActivity, File(FileHelper.file.path).readText().trim(), Toast.LENGTH_LONG).show()
    }
  }
}