# worldserver.conf.dist translation workspace

This folder exists for one purpose: if you want to replace my script-translated
English with a more natural-reading Google-translated version.

## What's here

| File | Source | What it is |
|---|---|---|
| `worldserver.conf.dist.original-french` | Pulled from `Psychostout/LegionCore-Reforged` upstream `main` | The raw French original (2,841 lines, 83 KB) |
| `worldserver.conf.dist.english` | My script-translated version (currently active in `src/server/worldserver/`) | 2,881 lines, 82 KB. 100% English, all 474 setting lines byte-identical to French original |

Difference between the two files: only **one** added setting — `Solocraft.Enable = 0`
— which was added when I restored the SoloCraft script. Otherwise both files have
the same 473 setting names with the same default values in the same order.

## If you want to Google-Translate the French version

1. Open `worldserver.conf.dist.original-french` and copy its contents.
2. Paste into Google Translate (French → English).
3. Paste the result back into a new file under this folder
   (e.g. `worldserver.conf.dist.english-google`).
4. Tell me; I'll merge:
   - Use your Google-translated **comments** verbatim.
   - Keep the **setting lines (NAME = VALUE)** byte-identical to the live file
     so no runtime behaviour changes.
   - Re-add the SoloCraft configuration block at the end.
5. Replace `src/server/worldserver/worldserver.conf.dist` with the merged result.

## Important: do NOT modify these lines

```
# Setting name (no leading #)
SettingName = value
```

Anything that looks like `Foo.Bar = 123` is a real setting that the worldserver
parses at startup. Google Translate can mangle these (it loves to "translate"
identifier names and add weird whitespace). If you let me do the merge in step 4
I'll guarantee these stay intact. If you want to do the merge yourself, run:

```bash
diff <(grep -E "^[A-Z][A-Za-z0-9._]*[A-Za-z0-9]\s*=" worldserver.conf.dist.english) \
     <(grep -E "^[A-Z][A-Za-z0-9._]*[A-Za-z0-9]\s*=" worldserver.conf.dist.YOUR-VERSION)
```

The output should be **empty** (apart from the `Solocraft.Enable = 0` line at the
end).

## Current state

My script-translated `worldserver.conf.dist.english` has zero French markers
remaining (verified via grep for `defaut`, `fichier`, `repertoire`, `royaume`,
`connexion`, etc.). The phrasing is mechanically translated and reads slightly
clunky in places (e.g. "Maximum players displayed in the /who list" instead of
the more natural "Max players shown in /who"). It's functional and clear, just
not polished.

Google Translate would produce more idiomatic English. Both are equally valid for
the purpose of being readable English server configuration.
