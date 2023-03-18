/* Copyright (C) 2019 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
package io.github.kichikuou.system3

import android.app.AlertDialog
import android.content.Context
import android.content.SharedPreferences
import android.media.MediaPlayer
import android.os.Bundle
import android.text.InputType
import android.util.Log
import android.view.ContextMenu
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.EditText
import org.libsdl.app.SDLActivity
import java.io.File
import java.io.IOException

// Intent for this activity must have the following extras:
// - EXTRA_GAME_ROOT (string): A path to the game installation.
// - EXTRA_SAVE_DIR (string): A directory where save files will be stored.
class GameActivity : SDLActivity() {
    companion object {
        const val EXTRA_GAME_ROOT = "GAME_ROOT"
        const val EXTRA_SAVE_DIR = "SAVE_DIR"
        const val PREF_USE_FM = "use_fm_sound"
    }

    private lateinit var gameRoot: File
    private lateinit var prefs: SharedPreferences
    private val midi = MidiPlayer()
    private var useFM = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        gameRoot = File(intent.getStringExtra(EXTRA_GAME_ROOT)!!)
        prefs = getSharedPreferences("system3", Context.MODE_PRIVATE)
        useFM = prefs.getBoolean(PREF_USE_FM, useFM)
        registerForContextMenu(mLayout)
    }

    override fun onStop() {
        super.onStop()
        midi.onActivityStop()
    }

    override fun onResume() {
        super.onResume()
        midi.onActivityResume()
    }

    override fun getLibraries(): Array<String> {
        return arrayOf("SDL2", "system3")
    }

    override fun getArguments(): Array<String> {
        val args = arrayListOf(
                "-gamedir", intent.getStringExtra(EXTRA_GAME_ROOT)!!,
                "-savedir", intent.getStringExtra(EXTRA_SAVE_DIR)!! + "/@",
                "-playlist", Launcher.PLAYLIST_FILE)
        if (useFM)
            args.add("-fm")
        return args.toTypedArray()
    }

    override fun setTitle(title: CharSequence?) {
        super.setTitle(title)
        val str = title?.toString()?.substringAfter(':', "")
        if (str.isNullOrEmpty())
            return
        File(gameRoot, Launcher.TITLE_FILE).writeText(str)
        Launcher.updateGameList()
    }

    private fun textInputDialog(oldVal: String, maxLen: Int, result: Array<String?>) {
        val input = EditText(this)
        input.inputType = InputType.TYPE_CLASS_TEXT
        input.setText(oldVal)
        AlertDialog.Builder(this)
                .setMessage(getString(R.string.input_dialog_message, maxLen))
                .setView(input)
                .setPositiveButton(R.string.ok) {_, _ ->
                    val s = input.text.toString()
                    result[0] = if (s.length <= maxLen) s else s.substring(0, maxLen)
                }
                .setNegativeButton(R.string.cancel) {_, _ -> }
                .setOnDismissListener {
                    synchronized(result) {
                        @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).notify()
                    }
                }
                .show()
    }

    private var menuShown = false
    private fun openMenu() {
        if (menuShown)
            return
        openContextMenu(mLayout)
        menuShown = true
    }

    override fun onCreateContextMenu(menu: ContextMenu, v: View?, menuInfo: ContextMenu.ContextMenuInfo?) {
        super.onCreateContextMenu(menu, v, menuInfo)
        menuInflater.inflate(R.menu.game_menu, menu)
        menu.findItem(if (useFM) R.id.fm_sound else R.id.midi_sound).isChecked = true
    }

    override fun onContextItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.quit_game -> {
                finish()
                true
            }
            R.id.fm_sound -> {
                selectSynthesizer(true)
                useFM = true
                prefs.edit().putBoolean(PREF_USE_FM, useFM).apply()
                true
            }
            R.id.midi_sound -> {
                selectSynthesizer(false)
                useFM = false
                prefs.edit().putBoolean(PREF_USE_FM, useFM).apply()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onContextMenuClosed(menu: Menu) {
        super.onContextMenuClosed(menu)
        menuShown = false
    }

    // C functions we call
    private external fun selectSynthesizer(use_fm: Boolean)

    // The functions below are called in the SDL thread by JNI.
    @Suppress("unused") fun midiStart(path: String, loop: Boolean) = midi.start(path, loop)
    @Suppress("unused") fun midiStop() = midi.stop()
    @Suppress("unused") fun midiCurrentPosition() = midi.currentPosition()

    @Suppress("unused") fun inputString(oldVal: String, maxLen: Int): String? {
        val result = arrayOfNulls<String?>(1)
        runOnUiThread { textInputDialog(oldVal, maxLen, result) }
        // Block the calling thread.
        synchronized(result) {
            try {
                @Suppress("PLATFORM_CLASS_MAPPED_TO_KOTLIN") (result as Object).wait()
            } catch (ex: InterruptedException) {
                ex.printStackTrace()
            }
        }
        return result[0]
    }

    @Suppress("unused") fun popupMenu() {
        runOnUiThread { openMenu() }
    }
}

private class MidiPlayer {
    private val player = MediaPlayer()
    private var playing = false
    private var playerPaused = false

    init {
        player.setOnCompletionListener { playing = false }
    }

    fun start(path: String, loop: Boolean) {
        try {
            player.apply {
                reset()
                setDataSource(path)
                isLooping = loop
                prepare()
                start()
            }
            playing = true
        } catch (e: IOException) {
            Log.e("midiStart", "Cannot play midi", e)
            player.reset()
        }
    }

    fun stop() {
        if (playing && player.isPlaying) {
            player.stop()
            playing = false
        }
    }

    fun currentPosition(): Int {
        return if (playing) player.currentPosition else 0
    }

    fun onActivityStop() {
        if (playing && player.isPlaying) {
            player.pause()
            playerPaused = true
        }
    }

    fun onActivityResume() {
        if (playerPaused) {
            player.start()
            playerPaused = false
        }
    }
}
