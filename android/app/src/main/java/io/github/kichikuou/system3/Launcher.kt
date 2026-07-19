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
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
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

sealed class InstallState {
    object Idle : InstallState()
    data class Installing(val progress: String?) : InstallState()
    data class Succeeded(val path: File) : InstallState()
    data class Failed(val msgId: Int) : InstallState()
}

private const val SAVE_DIR = "save"

class Launcher private constructor(private val rootDir: File) {
    companion object {
        const val TITLE_FILE = "title.txt"
        const val PLAYLIST_FILE = "playlist2.txt"

        fun getInstance(rootDir: File): Launcher {
            if (gLauncher == null) {
                gLauncher = Launcher(rootDir)
            }
            return gLauncher!!
        }
    }

    data class Entry(val path: File, val title: String, val timestamp: Long)
    val games = arrayListOf<Entry>()
    val titles: List<String>
        get() = games.map(Entry::title)
    var observer: LauncherObserver? = null
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main)
    private var installJob: Job? = null
    var installState: InstallState = InstallState.Idle
        private set
    val saveDir: File
        get() = File(rootDir, SAVE_DIR)

    init {
        refreshGameList()
    }

    fun install(input: InputStream) {
        if (installJob?.isActive == true) {
            input.close()
            return
        }
        installState = InstallState.Installing(null)
        installJob = scope.launch {
            try {
                val dir = withContext(Dispatchers.IO) {
                    input.use {
                        extractFilesTransactionally(it) { msg ->
                            withContext(Dispatchers.Main) {
                                setInstallProgress(msg)
                            }
                        }
                    }
                }
                setInstallSucceeded(dir)
            } catch (e: InstallFailureException) {
                setInstallFailed(e.msgId)
            } catch (e: Exception) {
                Log.e("launcher", "Failed to extract ZIP", e)
                setInstallFailed(R.string.zip_extraction_error)
            }
        }
    }

    fun consumeInstallResult() {
        if (installState is InstallState.Succeeded || installState is InstallState.Failed) {
            installState = InstallState.Idle
        }
    }

    private fun setInstallProgress(progress: String) {
        installState = InstallState.Installing(progress)
        observer?.onInstallProgress(progress)
    }

    private fun setInstallSucceeded(path: File) {
        installState = InstallState.Succeeded(path)
        observer?.onInstallSuccess(path)
    }

    private fun setInstallFailed(msgId: Int) {
        installState = InstallState.Failed(msgId)
        observer?.onInstallFailure(msgId)
    }

    fun uninstall(id: Int) {
        games[id].path.deleteRecursively()
        games.removeAt(id)
        observer?.onGameListChange()
    }

    fun refreshGameList() {
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
            } catch (e: IOException) {
                // Incomplete game installation. Delete it.
                path.deleteRecursively()
            }
        }
        games.sortByDescending(Entry::timestamp)
        if (!saveDirFound) {
            saveDir.mkdir()
        }
        observer?.onGameListChange()
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
                val path = resolveOutputPath(saveDir, zipEntry.name.removePrefix("save/"))
                Log.i("importSaveData", zipEntry.name)
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

    private suspend fun extractFilesTransactionally(
        input: InputStream,
        progressCallback: suspend (String) -> Unit
    ): File {
        val dir = createDirForGame()
        var committed = false
        try {
            extractFiles(input, dir, progressCallback)
            committed = true
            return dir
        } finally {
            if (!committed && !dir.deleteRecursively()) {
                Log.w("launcher", "Failed to delete incomplete install directory: $dir")
            }
        }
    }

    private suspend fun extractFiles(
        input: InputStream,
        outDir: File,
        progressCallback: suspend (String) -> Unit
    ) {
        val configWriter = GameConfigWriter()
        forEachZipEntrySuspending(input) { zipEntry, zip ->
            Log.i("extractFiles", zipEntry.name)
            val entryName = File(zipEntry.name).name
            if (zipEntry.isDirectory)
                return@forEachZipEntrySuspending
            progressCallback(entryName)
            FileOutputStream(resolveOutputPath(outDir, entryName)).buffered().use {
                zip.copyTo(it)
            }
            configWriter.maybeAdd(entryName)
        }
        configWriter.write(outDir)
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

private fun resolveOutputPath(baseDir: File, relativePath: String): File {
    val canonicalBase = baseDir.canonicalFile
    val file = File(canonicalBase, relativePath).canonicalFile
    if (!file.path.startsWith(canonicalBase.path + File.separator)) {
        throw IOException("Output path is outside target directory: $relativePath")
    }
    return file
}

private fun forEachZipEntry(input: InputStream, action: (ZipEntry, ZipInputStream) -> Unit) =
    runBlocking {
        forEachZipEntrySuspending(input) { zipEntry, zip ->
            action(zipEntry, zip)
        }
    }

private suspend fun forEachZipEntrySuspending(
    input: InputStream,
    action: suspend (ZipEntry, ZipInputStream) -> Unit
) {
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
