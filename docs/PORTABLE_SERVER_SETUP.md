# Portable / Standalone LegionCore Server Layout

This guide explains how to turn compiled LegionCore outputs into a standalone
folder you can move/copy as one unit. It covers the server binaries, configs,
client data, MariaDB database folder, users/passwords, and the config settings
that usually need changing.

> Current verified build milestone: `bnetserver` has been successfully built on
> Linux with Boost 1.83. A full `worldserver`/scripts build still needs separate
> verification. The layout below assumes both `bnetserver` and `worldserver` are
> available after a successful build.

---

## 1. Recommended portable folder structure

Create one folder, for example:

```text
LegionPortable/
├── bin/
│   ├── bnetserver.exe              # or bnetserver on Linux
│   ├── worldserver.exe             # or worldserver on Linux
│   ├── libmariadb.dll              # Windows: required runtime DLL
│   ├── libssl-3-x64.dll            # Windows/OpenSSL runtime DLL
│   ├── libcrypto-3-x64.dll         # Windows/OpenSSL runtime DLL
│   └── other required .dll files   # copy if Windows reports missing DLLs
├── configs/
│   ├── bnetserver.conf
│   └── worldserver.conf
├── data/
│   └── ClientData/
│       ├── dbc/
│       ├── db2/
│       ├── maps/
│       ├── vmaps/
│       ├── mmaps/                  # optional but recommended
│       ├── gt/                     # if produced by the extractor
│       └── cameras/                # if produced by the extractor
├── database/
│   └── mariadb/
│       ├── bin/
│       ├── data/
│       ├── logs/
│       ├── tmp/
│       └── my.ini
├── logs/
├── sql/                            # optional copy of repo sql/ for reference/imports
├── tools/                          # optional extractors / connection_patcher
├── start_bnetserver.bat            # Windows helper
├── start_worldserver.bat           # Windows helper
├── start_bnetserver.sh             # Linux helper
└── start_worldserver.sh            # Linux helper
```

You can choose different folder names, but keeping `bin`, `configs`, `data`,
`database`, and `logs` separate makes upgrades and backups much easier.

---

## 2. Copy compiled server binaries

### Windows Visual Studio preset output

After a successful Windows build, outputs are expected under something like:

```text
build/windows-msvc-release/bin/RelWithDebInfo/
```

Copy these to `LegionPortable/bin/`:

```text
bnetserver.exe
worldserver.exe
```

If you built tools (`TOOLS=1`), optionally copy:

```text
mapextractor.exe
vmap4extractor.exe
vmap4assembler.exe
mmaps_generator.exe
connection_patcher.exe
```

### Linux targeted bnetserver test output

The verified targeted Linux bnetserver test produced:

```text
build/bnetserver-boost183/src/server/bnetserver/bnetserver
```

For a portable Linux layout, copy it to:

```text
LegionPortable/bin/bnetserver
```

When `worldserver` is successfully built, copy it similarly:

```text
LegionPortable/bin/worldserver
```

---

## 3. Copy runtime DLLs/libraries

### Windows

At minimum, Windows normally needs these beside the `.exe` files or on `PATH`:

```text
libmariadb.dll
libssl-3-x64.dll
libcrypto-3-x64.dll
```

Typical source locations:

```text
compile_deps/openssl/OpenSSL-Win64/bin/
compile_deps/mariadb/.../lib/ or MariaDB install directory
C:\Program Files\OpenSSL-Win64\bin\
C:\Program Files\MariaDB 10.6\lib\
```

If Windows says another DLL is missing, copy that DLL into `bin/` too or add its
folder to `PATH`.

### Linux

Linux usually uses system libraries. If you want a truly portable Linux bundle,
check dependencies with:

```bash
ldd bin/bnetserver
ldd bin/worldserver
```

Then either install the required packages on the target machine or copy the
needed `.so` files and use an appropriate `LD_LIBRARY_PATH` wrapper script.

---

## 4. Copy and rename configs

Copy:

```text
src/server/bnetserver/bnetserver.conf.dist   -> LegionPortable/configs/bnetserver.conf
src/server/worldserver/worldserver.conf.dist -> LegionPortable/configs/worldserver.conf
```

Do not edit the `.dist` files in your source tree for a live server. Edit only
the copied `.conf` files inside your portable server folder.

Because the compiled default config path may point to the build/install prefix,
the most portable method is to start the servers with explicit `-c` config paths.

Windows start scripts:

```bat
:: start_bnetserver.bat
@echo off
cd /d %~dp0
bin\bnetserver.exe -c configs\bnetserver.conf
pause
```

```bat
:: start_worldserver.bat
@echo off
cd /d %~dp0
bin\worldserver.exe -c configs\worldserver.conf
pause
```

Linux start scripts:

```bash
#!/usr/bin/env bash
cd "$(dirname "$0")"
./bin/bnetserver -c configs/bnetserver.conf
```

```bash
#!/usr/bin/env bash
cd "$(dirname "$0")"
./bin/worldserver -c configs/worldserver.conf
```

Make Linux scripts executable:

```bash
chmod +x start_bnetserver.sh start_worldserver.sh
```

---

## 5. Client data folder

`worldserver.conf` contains:

```ini
DataDir = "./ClientData"
```

For the portable layout above, change it to:

```ini
DataDir = "./data/ClientData"
```

Then put extracted Legion 7.3.5 build 26972 client data under:

```text
LegionPortable/data/ClientData/
```

Expected subfolders include:

```text
dbc/
db2/
maps/
vmaps/
mmaps/
gt/       # if produced
cameras/  # if produced
```

The exact extractor output depends on which tools are built and run. `vmaps` and
`mmaps` are strongly recommended for proper line-of-sight/pathing behavior.

---

## 6. Portable MariaDB folder

You can use either an installed MariaDB service or a portable MariaDB folder.
For a standalone bundle, use a folder like:

```text
LegionPortable/database/mariadb/
```

### Windows portable MariaDB ZIP approach

1. Download a MariaDB 10.6.x or 10.11.x Windows x64 ZIP from MariaDB.
2. Extract it into:

```text
LegionPortable/database/mariadb/
```

3. Create these folders:

```text
LegionPortable/database/mariadb/data/
LegionPortable/database/mariadb/logs/
LegionPortable/database/mariadb/tmp/
```

4. Create `LegionPortable/database/mariadb/my.ini`:

```ini
[mysqld]
basedir=.
datadir=./data
port=3306
bind-address=127.0.0.1
character-set-server=utf8mb4
collation-server=utf8mb4_unicode_ci
max_allowed_packet=64M
sql_mode=NO_ENGINE_SUBSTITUTION
log_error=./logs/mariadb.err
tmpdir=./tmp

[client]
port=3306
host=127.0.0.1
default-character-set=utf8mb4
```

5. Initialize the database directory.

MariaDB ZIP packages vary by version. Try these in order from inside
`database/mariadb/`:

```bat
bin\mariadb-install-db.exe --datadir=data
```

If that program does not exist, try:

```bat
bin\mysql_install_db.exe --datadir=data
```

If neither exists, use the server initializer:

```bat
bin\mariadbd.exe --initialize-insecure --datadir=data
```

6. Start MariaDB manually for testing:

```bat
bin\mariadbd.exe --defaults-file=my.ini --console
```

Leave that console open while importing databases and running the server.

### Linux local MariaDB

On Linux it is usually easier to use the system service:

```bash
sudo systemctl enable --now mariadb
sudo mariadb
```

A fully portable Linux MariaDB is possible, but service-based MariaDB is much
simpler and safer.

---

## 7. Create databases and a database user

Log into MariaDB as root:

Windows portable:

```bat
cd LegionPortable\database\mariadb
bin\mariadb.exe -uroot -h127.0.0.1 -P3306
```

Linux service:

```bash
sudo mariadb
```

Create the databases and a dedicated user. Replace `CHANGE_ME_STRONG_PASSWORD`
with your own password.

```sql
CREATE DATABASE IF NOT EXISTS auth       DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE DATABASE IF NOT EXISTS characters DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE DATABASE IF NOT EXISTS world      DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE DATABASE IF NOT EXISTS hotfixes   DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

CREATE USER IF NOT EXISTS 'legion'@'127.0.0.1' IDENTIFIED BY 'CHANGE_ME_STRONG_PASSWORD';
CREATE USER IF NOT EXISTS 'legion'@'localhost' IDENTIFIED BY 'CHANGE_ME_STRONG_PASSWORD';

GRANT ALL PRIVILEGES ON auth.*       TO 'legion'@'127.0.0.1';
GRANT ALL PRIVILEGES ON characters.* TO 'legion'@'127.0.0.1';
GRANT ALL PRIVILEGES ON world.*      TO 'legion'@'127.0.0.1';
GRANT ALL PRIVILEGES ON hotfixes.*   TO 'legion'@'127.0.0.1';

GRANT ALL PRIVILEGES ON auth.*       TO 'legion'@'localhost';
GRANT ALL PRIVILEGES ON characters.* TO 'legion'@'localhost';
GRANT ALL PRIVILEGES ON world.*      TO 'legion'@'localhost';
GRANT ALL PRIVILEGES ON hotfixes.*   TO 'legion'@'localhost';

FLUSH PRIVILEGES;
```

Test the new user:

```bash
mariadb -ulegion -p -h127.0.0.1 -P3306 auth
```

---

## 8. Import base databases

From the repository root or from a copied `sql/` folder:

```bash
mariadb -ulegion -p -h127.0.0.1 -P3306 auth       < sql/base/auth_database.sql
mariadb -ulegion -p -h127.0.0.1 -P3306 characters < sql/base/characters_database.sql
```

The repo currently includes a compressed world database archive:

```text
sql/base/TDB_full_735.00_2018_02_19.7z
```

Extract it with 7-Zip, then import the extracted `.sql` file into `world`:

```bash
7z x sql/base/TDB_full_735.00_2018_02_19.7z -osql/base/TDB_full_735_extracted
mariadb -ulegion -p -h127.0.0.1 -P3306 world < path/to/extracted_world.sql
```

Hotfix base data is not a standalone obvious SQL file in `sql/base/` in the
current tree. If your database pack provides a hotfixes base dump, import it:

```bash
mariadb -ulegion -p -h127.0.0.1 -P3306 hotfixes < hotfixes_base.sql
```

The server update system can apply files from `sql/updates/`, but it cannot
replace a missing compatible base database dump.

Optional custom SQL can be reviewed/imported from:

```text
sql/custom/
sql/optional/
sql/fix_dberrors.sql
sql/fix_legion_assault_bonus_objectives.sql
```

Do not blindly import optional patch-enable/disable files unless you understand
what content phase you want.

---

## 9. Configure database login strings

Both config files use semicolon-separated database strings:

```text
host;port;user;password;database
```

### bnetserver.conf

Change:

```ini
LoginDatabaseInfo = "127.0.0.1;3306;root;admin;auth"
```

to:

```ini
LoginDatabaseInfo = "127.0.0.1;3306;legion;CHANGE_ME_STRONG_PASSWORD;auth"
```

### worldserver.conf

Change:

```ini
LoginDatabaseInfo     = "127.0.0.1;3306;root;admin;auth"
CharacterDatabaseInfo = "127.0.0.1;3306;root;admin;characters"
HotfixDatabaseInfo    = "127.0.0.1;3306;root;admin;hotfixes"
WorldDatabaseInfo     = "127.0.0.1;3306;root;admin;world"
```

to:

```ini
LoginDatabaseInfo     = "127.0.0.1;3306;legion;CHANGE_ME_STRONG_PASSWORD;auth"
CharacterDatabaseInfo = "127.0.0.1;3306;legion;CHANGE_ME_STRONG_PASSWORD;characters"
HotfixDatabaseInfo    = "127.0.0.1;3306;legion;CHANGE_ME_STRONG_PASSWORD;hotfixes"
WorldDatabaseInfo     = "127.0.0.1;3306;legion;CHANGE_ME_STRONG_PASSWORD;world"
```

---

## 10. Configure ports and addresses

### bnetserver.conf

Common settings:

```ini
Game.Build.Version = 26972
BattlenetPort = 1119
LoginREST.Port = 8081
LoginREST.ExternalAddress = 127.0.0.1
LoginREST.LocalAddress = 127.0.0.1
BindIP = "0.0.0.0"
```

For LAN/public hosting, change `LoginREST.ExternalAddress` to the address clients
should use to reach the server.

### worldserver.conf

Common settings:

```ini
Game.Build.Version = 26972
RealmID = 1
DataDir = "./data/ClientData"
WorldServerPort = 8085
InstanceServerPort = 8086
BnetServer.Address = 127.0.0.1
BnetServer.Port = 1118
BindIP = "0.0.0.0"
```

Keep `Game.Build.Version` identical in both configs. This project targets Legion
7.3.5 build 26972.

If you change database user/password/port, update every `*DatabaseInfo` line.
If you change public IP/domain, update both the config and the `realmlist` table
in the auth database as needed.

---

## 11. Create a game account

Preferred method after `worldserver` is running:

```text
account create user@example.com MyStrongGamePassword
account set gmlevel user@example.com 3 -1
```

Use an email-like account name because this Legion/BNet-era core treats account
name and email as effectively the same in several places.

If account commands differ in your build, use `.help account` or `help account`
from the server console.

Avoid manually inserting account rows unless you know the expected hash format.

---

## 12. Start order

1. Start MariaDB.
2. Start `bnetserver`.
3. Start `worldserver`.
4. Watch both consoles/log files for database, port, or missing-data errors.

Windows:

```bat
start_bnetserver.bat
start_worldserver.bat
```

Linux:

```bash
./start_bnetserver.sh
./start_worldserver.sh
```

---

## 13. Backups and upgrades

Back up these folders before updating binaries or SQL:

```text
configs/
database/mariadb/data/
data/ClientData/       # large, but avoids re-extracting
logs/                  # optional
```

Database backup examples:

```bash
mariadb-dump -ulegion -p auth       > backup_auth.sql
mariadb-dump -ulegion -p characters > backup_characters.sql
mariadb-dump -ulegion -p world      > backup_world.sql
mariadb-dump -ulegion -p hotfixes   > backup_hotfixes.sql
```

Characters are the most important live data to protect.

---

## 14. Common startup errors

### Cannot connect to database

Check:

* MariaDB is running.
* User/password in config are correct.
* The user has grants for all four databases.
* Host is `127.0.0.1` if you granted `legion`@`127.0.0.1`.

### Missing dbc/db2/maps/vmaps/mmaps

Check:

```ini
DataDir = "./data/ClientData"
```

and confirm the extracted folders exist under that path.

### Port already in use

Change the relevant port in config or stop the other process using it.
Common ports:

```text
1119  BattlenetPort
8081  LoginREST.Port
8085  WorldServerPort
8086  InstanceServerPort
3306  MariaDB
```

### Client cannot connect

Check:

* Client is exactly Legion 7.3.5 build 26972.
* `connection_patcher.exe` was applied if required by your client setup.
* Firewall allows the configured ports.
* `LoginREST.ExternalAddress` and auth `realmlist` address are correct.

---

## 15. Minimal checklist

```text
[ ] bnetserver and worldserver copied to bin/
[ ] required DLLs copied to bin/ on Windows
[ ] bnetserver.conf and worldserver.conf copied to configs/
[ ] DataDir points to ./data/ClientData
[ ] ClientData has dbc/db2/maps/vmaps/mmaps
[ ] MariaDB starts from database/mariadb/ or system service
[ ] auth, characters, world, hotfixes databases created
[ ] legion database user created and granted privileges
[ ] base SQL imported
[ ] config database strings changed from root/admin to legion/password
[ ] bnetserver starts cleanly
[ ] worldserver starts cleanly
[ ] account created
[ ] client patched/configured for build 26972
```
