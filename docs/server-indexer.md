# Colony Indexer Guide

`colony_indexer` is a command-line tool that builds the `.idx` file Colony Finder needs to search.
This guide shows how to run it on a server and share the result with other players.

## Why would I run it?

Building an index from the full Spansh dump takes a long time and a lot of disk space.
If you run the indexer once on a server, you can publish the small `.idx` file and let other
people download it through Colony Finder's **Download Prebuilt Index** screen. They skip the
big download and start searching in minutes.

## Get the tool

Grab the binary for your OS from the [GitHub Releases](https://github.com/zGiuly/Colony-Finder/releases) page:

- Windows: `colony_indexer-windows-x64.exe`
- Linux:   `colony_indexer-linux-x64`

On Linux, make it executable:

```bash
chmod +x colony_indexer-linux-x64
```

## Quick start

### Windows (PowerShell)

```powershell
.\colony_indexer-windows-x64.exe -d 1month --download-dir C:\spansh --search-dir C:\spansh
```

### Linux

```bash
./colony_indexer-linux-x64 -d 1month --download-dir /var/spansh --search-dir /var/spansh
```

This will:
1. Download `galaxy_1month.json.gz` from Spansh into the download directory.
2. Decompress and validate it.
3. Produce `galaxy_1month.idx` in the search directory.

When it finishes, the file you want to publish is `galaxy_1month.idx`.

## Useful options

| Option | What it does |
|---|---|
| `-d 1month` | Download the 1-month galaxy (recommended, ~16 GB compressed). |
| `-d full` | Download the full galaxy (~100 GB compressed, takes hours). |
| `-d <url>` | Download from any custom URL. |
| `-l <file>` | Skip the download and index a local `.json` or `.json.gz` file you already have. |
| `-b <MB>` | Decompression buffer size in MB (1–64). Default is 16. Higher = faster but more RAM. |
| `--download-dir <path>` | Where the dump is saved. |
| `--search-dir <path>` | Where the `.idx` is written. |
| `-s` | Just refresh the Spansh schema and exit. |
| `-h` | Show all options. |

Run with no arguments to enter an interactive menu.

## What you need on the server

- Around **30 GB free disk** for the 1-month workflow (dump + decompressed JSON + idx).
- For the full galaxy, plan on **200 GB+**.
- A few GB of RAM. The tool streams the data, it does not load everything in memory.
- A stable internet connection for the initial download.

Windows and Linux produce the same `.idx` file — they are interchangeable. Players on either
OS can use an index built on either OS.

## Sharing the index

The **Official source** button inside Colony Finder is reserved for the project's own release.
If you want to distribute an index you built yourself (for your wing, your Discord, your
community), host it anywhere that returns the file over HTTPS and share the direct link.
Your users paste it into Colony Finder's **Custom URL** field and download it from there.

Any of these work fine:

- A static web server (nginx, Caddy, Apache) serving the file from a folder.
- An object store: AWS S3, Cloudflare R2, Backblaze B2, etc. — use the public object URL.
- A file on your own GitHub repository's Releases (in **your** fork or a separate repo you own).
- Any cloud storage that exposes a direct download URL (not a preview page).

Two rules:

1. The URL must point straight at the bytes of the `.idx`. Pages that show a "Download" button
   in the browser will not work — Colony Finder does an HTTP GET, not a click.
2. Serve it over HTTPS. Plain HTTP will fail.

Tip: keep the filename `galaxy_1month.idx` (or `galaxy.idx` for full-galaxy builds) so it
matches what Colony Finder expects locally. The tool saves it under that name in the search
directory regardless of the URL path.

## Keeping it fresh

Spansh updates the 1-month dump regularly. A good rhythm is to rebuild the index once a week
and publish a new release. You can automate this with a scheduled GitHub Actions workflow or
a cron job on your server.

## Index versions

The `.idx` format has a version number embedded in the file header. If Colony Finder is
updated and the format changes, old indexes become incompatible. The app will detect this
and prompt the user to rebuild or download a fresh one. When you publish a new release of
Colony Finder, rebuild the index too.

## Troubleshooting

- **"Disk full" during indexing**: the JSON is decompressed to disk. Free up space or pick a
  different `--search-dir`.
- **Index file rejected by Colony Finder**: the file is corrupted or built with a different
  format version. Rebuild with a matching `colony_indexer` version.
- **Stuck at "validating schema"**: run `colony_indexer -s` first to refresh the schema, then
  retry.
