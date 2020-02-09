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

import android.app.*
import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.ListView
import android.widget.Toast
import java.io.*

private const val CONTENT_TYPE_ZIP = "application/zip"
private const val INSTALL_REQUEST = 1
private const val SAVEDATA_EXPORT_REQUEST = 2
private const val SAVEDATA_IMPORT_REQUEST = 3

class LauncherActivity : ListActivity(), AdapterView.OnItemLongClickListener, LauncherObserver {
    private lateinit var launcher: Launcher
    private var progressDialog: ProgressDialogFragment? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        launcher = Launcher.getInstance(filesDir)
        launcher.observer = this
        if (launcher.isInstalling) {
            showProgressDialog()
        }

        onGameListChange()
        listView.onItemLongClickListener = this
    }

    override fun onDestroy() {
        launcher.observer = null
        super.onDestroy()
    }

    override fun onListItemClick(l: ListView?, v: View?, position: Int, id: Long) {
        super.onListItemClick(l, v, position, id)
        if (position < launcher.games.size) {
            startGame(launcher.games[position].path)
        } else {
            val i = Intent(Intent.ACTION_GET_CONTENT)
            i.type = CONTENT_TYPE_ZIP
            startActivityForResult(Intent.createChooser(i, getString(R.string.choose_a_file)), INSTALL_REQUEST)
        }
    }

    override fun onItemLongClick(a: AdapterView<*>?, v: View?, position: Int, id: Long): Boolean {
        if (position < launcher.games.size) {
            AlertDialog.Builder(this).setTitle(R.string.uninstall_dialog_title)
                    .setMessage(getString(R.string.uninstall_dialog_message, launcher.games[position].title))
                    .setPositiveButton(R.string.ok) {_, _ -> uninstall(position)}
                    .setNegativeButton(R.string.cancel) {_, _ -> }
                    .show()
        }
        return true
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.launcher_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.export_savedata -> {
                val i = Intent(Intent.ACTION_CREATE_DOCUMENT)
                i.type = CONTENT_TYPE_ZIP
                i.putExtra(Intent.EXTRA_TITLE, "savedata.zip")
                startActivityForResult(i, SAVEDATA_EXPORT_REQUEST)
                true
            }
            R.id.import_savedata -> {
                val i = Intent(Intent.ACTION_GET_CONTENT)
                i.type = CONTENT_TYPE_ZIP
                startActivityForResult(Intent.createChooser(i, getString(R.string.choose_a_file)),
                                       SAVEDATA_IMPORT_REQUEST)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (resultCode != RESULT_OK)
            return
        val uri = data?.data ?: return
        when (requestCode) {
            INSTALL_REQUEST -> {
                val input = contentResolver.openInputStream(uri) ?: return
                showProgressDialog()
                launcher.install(input)
            }
            SAVEDATA_EXPORT_REQUEST -> try {
                launcher.exportSaveData(contentResolver.openOutputStream(uri)!!)
                Toast.makeText(this, R.string.save_data_export_success, Toast.LENGTH_SHORT).show()
            } catch (e: IOException) {
                Log.e("launcher", "Failed to export savedata", e)
                errorDialog(R.string.save_data_export_error)
            }
            SAVEDATA_IMPORT_REQUEST -> {
                val input = contentResolver.openInputStream(uri) ?: return
                val errMsgId = launcher.importSaveData(input)
                if (errMsgId == null) {
                    Toast.makeText(this, R.string.save_data_import_success, Toast.LENGTH_SHORT).show()
                } else {
                    errorDialog(errMsgId)
                }
            }
        }

    }

    override fun onGameListChange() {
        val items = launcher.titles.toMutableList()
        items.add(getString(R.string.install_from_zip))
        listAdapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, items)
    }

    override fun onInstallProgress(path: String) {
        progressDialog?.setProgress(getString(R.string.install_progress, path))
    }

    override fun onInstallSuccess(path: File) {
        dismissProgressDialog()
        startGame(path)
    }

    override fun onInstallFailure(msgId: Int) {
        dismissProgressDialog()
        errorDialog(msgId)
    }

    private fun startGame(path: File) {
        val i = Intent()
        i.setClass(applicationContext, GameActivity::class.java)
        i.putExtra(GameActivity.EXTRA_GAME_ROOT, path.path)
        startActivity(i)
    }

    private fun uninstall(id: Int) {
        launcher.uninstall(id)
    }

    private fun showProgressDialog() {
        progressDialog = ProgressDialogFragment()
        progressDialog!!.show(fragmentManager, "progress_dialog")
    }

    private fun dismissProgressDialog() {
        progressDialog?.dismiss()
        progressDialog = null
    }

    private fun errorDialog(msgId: Int) {
        AlertDialog.Builder(this).setTitle(R.string.error_dialog_title)
                .setMessage(msgId)
                .setPositiveButton(R.string.ok) {_, _ -> }
                .show()
    }
}

@Suppress("DEPRECATION") // for ProgressDialog
class ProgressDialogFragment : DialogFragment() {
    private lateinit var dialog: ProgressDialog
    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        dialog = ProgressDialog(activity)
        return dialog.apply {
            setTitle(R.string.install_dialog_title)
            setCancelable(true)
        }
    }
    fun setProgress(msg: String) {
        dialog.setMessage(msg)
    }
}