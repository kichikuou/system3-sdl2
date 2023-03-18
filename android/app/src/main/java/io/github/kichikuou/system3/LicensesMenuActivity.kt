package io.github.kichikuou.system3

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.widget.ListView
import android.widget.SimpleAdapter

class LicensesMenuActivity : Activity() {
    class Entry(val displayName: String, val fileName: String, val url: String)
    private val entries: ArrayList<Entry> = arrayListOf(
        Entry("system3-sdl2", "system3", "https://github.com/kichikuou/system3-sdl2"),
        Entry("SDL", "SDL", "https://www.libsdl.org/"),
        Entry("SDL_ttf", "SDL_ttf", "https://github.com/libsdl-org/SDL_ttf"),
        Entry("SDL_mixer", "SDL_mixer", "https://github.com/libsdl-org/SDL_mixer"),
        Entry("FreeType", "freetype", "https://freetype.org/"),
        Entry("HarfBuzz", "harfbuzz", "https://harfbuzz.github.io/"),
        Entry("ymfm", "ymfm", "https://github.com/aaronsgiles/ymfm"),
        Entry("MotoyaLCedar W3 mono", "MTLc3m", "https://github.com/aosp-mirror/platform_frameworks_base/tree/lollipop-release/data/fonts"),
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_licenses_menu)

        val items = entries.map { mapOf("name" to it.displayName, "url" to it.url)}
        val listView = findViewById<ListView>(R.id.list)
        listView.adapter = SimpleAdapter(this, items, android.R.layout.simple_list_item_2,
            arrayOf("name", "url"), intArrayOf(android.R.id.text1, android.R.id.text2))
        listView.setOnItemClickListener { _, _, pos, _ ->
            val intent = Intent(this, LicensesActivity::class.java).apply {
                putExtra(LicensesActivity.EXTRA_DISPLAY_NAME, entries[pos].displayName)
                putExtra(LicensesActivity.EXTRA_FILE_NAME, entries[pos].fileName)
            }
            startActivity(intent)
        }
    }
}