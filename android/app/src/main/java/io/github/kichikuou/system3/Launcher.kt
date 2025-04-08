/* Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

import android.os.Build
import android.util.Log
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.*
import java.nio.charset.Charset
import java.util.*
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream
import java.util.zip.ZipOutputStream

private var gLauncher: Launcher? = null

interface LauncherObserver {
    fun onGameListChange()
    fun onInstallProgress(path: String)
    fun onInstallSuccess(path: File)
    fun onInstallFailure(msgId: Int)
}

private const val SAVE_DIR = "save"

class Launcher private constructor(private val rootDir: File) {
    companion object {
        const val TITLE_FILE = "title.txt"
        const val PLAYLIST_FILE = "playlist2.txt"
        const val OLD_PLAYLIST_FILE = "playlist.txt"

        fun getInstance(rootDir: File): Launcher {
            if (gLauncher == null) {
                gLauncher = Launcher(rootDir)
            }
            return gLauncher!!
        }

        fun updateGameList() {
            gLauncher?.updateGameList()
        }
    }

    data class Entry(val path: File, val title: String, val timestamp: Long)
    val games = arrayListOf<Entry>()
    val titles: List<String>
        get() = games.map(Entry::title)
    var observer: LauncherObserver? = null
    var isInstalling = false
        private set
    val saveDir: File
        get() = File(rootDir, SAVE_DIR)

    init {
        updateGameList()
    }

    @OptIn(DelicateCoroutinesApi::class)
    fun install(input: InputStream) {
        val dir = createDirForGame()
        isInstalling = true
        GlobalScope.launch(Dispatchers.Main) {
            try {
                withContext(Dispatchers.IO) {
                    extractFiles(input, dir) { msg ->
                        GlobalScope.launch(Dispatchers.Main) {
                            observer?.onInstallProgress(msg)
                        }
                    }
                }
                observer?.onInstallSuccess(dir)
            } catch (e: InstallFailureException) {
                observer?.onInstallFailure(e.msgId)
            } catch (e: Exception) {
                Log.e("launcher", "Failed to extract ZIP", e)
                observer?.onInstallFailure(R.string.zip_extraction_error)
            }
            isInstalling = false
        }
    }

    fun uninstall(id: Int) {
        games[id].path.deleteRecursively()
        games.removeAt(id)
        observer?.onGameListChange()
    }

    private fun updateGameList() {
        var saveDirFound = false
        games.clear()
        for (path in rootDir.listFiles() ?: emptyArray()) {
            if (!path.isDirectory)
                continue
            if (path.name == SAVE_DIR) {
                saveDirFound = true
                continue
            }
            try {
                val titleFile = File(path, TITLE_FILE)
                val title = titleFile.readText()
                games.add(Entry(path, title, titleFile.lastModified()))
                migratePlaylist(path)
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
        }
        games.sortByDescending(Entry::timestamp)
        if (!saveDirFound) {
            saveDir.mkdir()
        }
    }

    private fun createDirForGame(): File {
        var i = 0
        while (true) {
            val f = File(rootDir, i++.toString())
            if (!f.exists() && f.mkdir()) {
                return f
            }
        }
    }

    // Throws IOException
    fun exportSaveData(output: OutputStream) {
        ZipOutputStream(output.buffered()).use { zip ->
            for (path in saveDir.walkTopDown()) {
                if (path.isDirectory)
                    continue
                val pathInZip = path.relativeTo(saveDir.parentFile!!).path
                Log.i("exportSaveData", pathInZip)
                zip.putNextEntry(ZipEntry(pathInZip))
                path.inputStream().buffered().use {
                    it.copyTo(zip)
                }
            }
        }
    }

    fun importSaveData(input: InputStream): Int? {
        try {
            var imported = false
            forEachZipEntry(input) { zipEntry, zip ->
                // Process only files under save/
                if (zipEntry.isDirectory || !zipEntry.name.startsWith("save/"))
                    return@forEachZipEntry
                Log.i("importSaveData", zipEntry.name)
                val path = File(rootDir, zipEntry.name)
                path.parentFile?.mkdirs()
                FileOutputStream(path).buffered().use {
                    zip.copyTo(it)
                }
                imported = true
            }
            return if (imported) null else R.string.no_data_to_import
        } catch (e: UTFDataFormatException) {
            // Attempted to read Shift_JIS zip in Android < 7
            return R.string.unsupported_zip
        } catch (e: IOException) {
            Log.e("launcher", "Failed to extract ZIP", e)
            return R.string.zip_extraction_error
        }
    }

    private fun extractFiles(input: InputStream, outDir: File, progressCallback: (String) -> Unit) {
        val configWriter = GameConfigWriter()
        forEachZipEntry(input) { zipEntry, zip ->
            Log.i("extractFiles", zipEntry.name)
            val entryName = File(zipEntry.name).name
            if (zipEntry.isDirectory)
                return@forEachZipEntry
            progressCallback(entryName)
            FileOutputStream(File(outDir, entryName)).buffered().use {
                zip.copyTo(it)
            }
            configWriter.maybeAdd(entryName)
        }
        configWriter.write(outDir)
    }

    // System3-sdl2 <=0.7.0 had a bug where playlist had an extra empty line at
    // the beginning of the file. This migrates playlist.txt created by an old
    // version of system3-sdl2 to playlist2.txt.
    private fun migratePlaylist(dir: File) {
        val oldPlaylist = File(dir, OLD_PLAYLIST_FILE)
        if (!oldPlaylist.exists())
            return
        var tracks = oldPlaylist.readLines()
        if (tracks.isNotEmpty())
            tracks = tracks.subList(1, tracks.size)
        File(dir, PLAYLIST_FILE).writeText(tracks.joinToString("\n"))
        oldPlaylist.delete()
    }

    class InstallFailureException(val msgId: Int) : Exception()

    // A helper class which generates playlist.txt in the game root directory.
    private class GameConfigWriter {
        private var hasAdisk = false
        private val audioRegex = """((\d+).*|.*?(\d+))\.(wav|mp3|ogg)""".toRegex(RegexOption.IGNORE_CASE)
        private val audioFiles: Array<String?> = arrayOfNulls(100)

        fun maybeAdd(path: String) {
            val name = File(path).name

            if (name.lowercase(Locale.US) == "adisk.dat") {
                hasAdisk = true
            }
            audioRegex.matchEntire(name)?.let {
                val track = it.groupValues[2].toIntOrNull() ?: it.groupValues[3].toInt()
                if (0 < track && track <= audioFiles.size)
                    audioFiles[track - 1] = path
            }
        }

        fun write(outDir: File) {
            if (!hasAdisk) {
                throw InstallFailureException(R.string.cannot_find_adisk)
            }
            val playlist = audioFiles.joinToString("\n") { it ?: "" }.trimEnd('\n')
            File(outDir, PLAYLIST_FILE).writeText(playlist)
        }
    }
}

private fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit) {
    val zip = if (Build.VERSION.SDK_INT >= 24) {
        ZipInputStream(input.buffered(), Charset.forName("Shift_JIS"))
    } else {
        ZipInputStream(input.buffered())
    }
    zip.use {
        while (true) {
            try {
                val zipEntry = zip.nextEntry ?: break
                action(zipEntry, zip)
            } catch (e: UTFDataFormatException) {
                Log.w("forEachZipEntry", "UTFDataFormatException: skipping a zip entry")
                zip.closeEntry()
            }
        }
    }
}
