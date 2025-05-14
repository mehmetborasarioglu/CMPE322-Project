# CMPE 322 – Virtual Video‑Game Console  
**Standalone Game Collection & Plug‑and‑Play Cartridge**

---

## 1  Project Overview
This repository contains three classic terminal games implemented in pure **C99** plus the build artifacts required by **CMPE 322 Operating Systems – Project 1**.  
Each game can be run directly from the shell *or* packaged into an **ext4 disk image** (`storage_vgc.img`) that your custom launcher (`main-screen`) automatically discovers at runtime.

| Game | Source file | Compiled binary | Objective |
|------|-------------|-----------------|-----------|
| Snake | `snake.c` | `game_snake` | Survive and grow by eating bait on a 15 × 15 grid. |
| Burger Builder | `burger.c` | `game_burger` | Catch falling ingredients to stack a 10‑layer burger. |
| Battleship (local 2‑player) | `battleship.c` | `game_battleship` | Sink both of your opponent’s ships on a 5 × 5 board. |

The **PDF** specification (`CMPE322‑HW1.pdf`) is included for reference.

---

## 2  Directory Layout
```
.
├── battleship.c          # Battleship source
├── burger.c              # Burger Builder source
├── snake.c               # Snake source
├── CMPE322-HW1.pdf       # Full project description
├── scripts/              # Loop‑device helper scripts (created in §5)
└── README.md             # You are here
```

---

## 3  Prerequisites
| Requirement | Notes |
|-------------|-------|
| Any modern Linux (tested on **Debian 12.8.0**) | Must support loop devices & ext4. |
| GCC 9 or newer | `sudo apt install build-essential` |
| `make`, `bash`, `e2fsprogs`, `util-linux` | For image creation & mounting |
| Super‑user privileges | Needed only for attaching/detaching the loop device |

---

## 4  Quick Start — Run a Single Game
Compile and launch **Snake** (repeat similarly for the other two):

```bash
gcc -std=c99 -Wall -O2 -o game_snake snake.c
./game_snake          # Quit with ‘q’ or Ctrl‑C
```

All games trap **SIGINT / SIGTERM** and restore the terminal before exit.

### 4.1 Default Controls
| Game | Movement | Action / Quit |
|------|----------|---------------|
| Snake | `w a s d` | `q` |
| Burger | `a` = move bun left, `d` = right | Ctrl‑C = abort |
| Battleship | `w a s d` to move cursor | `v`/`h` place ship, **Enter** fire |

---

## 5  Building the Plug‑and‑Play Console Cartridge

The project rubric requires delivering the games on a **16 MiB ext4 “cartridge”** named `storage_vgc.img`, mounted via loop device, and containing:

```
bin/
├── main-screen
├── game_snake
├── game_burger
└── game_battleship
```

Follow the steps below; everything can be automated with four helper scripts.

### 5.1 Create Helper Scripts
Create a folder called **`scripts/`** and add the following:

<details>
<summary><code>scripts/initialize.sh</code></summary>

```bash
#!/usr/bin/env bash
set -e
IMG=storage_vgc.img
SIZE=16M                # Adjust if rubric changes
[ -f "$IMG" ] && rm -f "$IMG"
dd if=/dev/zero of="$IMG" bs=$SIZE count=1
mkfs.ext4 -F "$IMG"
echo "[initialize] $IMG created and formatted (ext4)."
```
</details>

<details>
<summary><code>scripts/startup.sh</code></summary>

```bash
#!/usr/bin/env bash
set -e
IMG=storage_vgc.img
DEV=$(sudo losetup -Pf --show "$IMG")   # e.g. /dev/loop7
echo "$DEV" > .loopdev
mkdir -p mount
sudo mount "$DEV" mount
sudo ln -sf "$DEV" device-file          # symbolic link required by spec
echo "[startup] $IMG mounted at ./mount via $DEV."
```
</details>

<details>
<summary><code>scripts/terminate.sh</code></summary>

```bash
#!/usr/bin/env bash
set -e
DEV=$(cat .loopdev)
sudo umount ./mount
sudo losetup -d "$DEV"
rm -f .loopdev device-file
echo "[terminate] $DEV detached and mount cleared."
```
</details>

<details>
<summary><code>scripts/purge.sh</code></summary>

```bash
#!/usr/bin/env bash
set -e
./scripts/terminate.sh || true
rm -f storage_vgc.img
echo "[purge] Cartridge image removed."
```
</details>

> Make the scripts executable:
> ```bash
> chmod +x scripts/*.sh
> ```

### 5.2 Compile Everything

```bash
gcc -std=c99 -O2 -o game_snake       snake.c
gcc -std=c99 -O2 -o game_burger      burger.c
gcc -std=c99 -O2 -o game_battleship  battleship.c

gcc -std=c99 -O2 -o main-screen      main-screen.c
```

### 5.3 Prepare the Cartridge

```bash
./scripts/initialize.sh    
./scripts/startup.sh       
sudo mkdir -p mount/bin
sudo cp game_* main-screen mount/bin/
ls -l mount/bin            
```

### 5.4 Run the Console

```bash
./mount/bin/main-screen    
```

* **`w` / `s`** – move cursor  
* **Enter** – launch highlighted game  
* **`q`** – quit launcher

When you are finished:

```bash
./scripts/terminate.sh
```

To rebuild from scratch:

```bash
./scripts/purge.sh && ./scripts/initialize.sh
```

---

## 6  Troubleshooting

| Issue | Resolution |
|-------|------------|
| *“Permission denied \*/dev/loopX\*”* | Run the script with `sudo` or add your user to the `disk` group. |
| Terminal shows garbage after abort | Ensure each game calls `tcsetattr()` to restore settings in its `atexit()` handler. |
| Games not detected by launcher | File names **must** start with `game_` and have execute permission (`chmod +x`). |

---

## 7  Extending the Console
* Add new games by following the **`game_<title>`** naming convention and copying binaries into `./mount/bin/`.
* Update `main-screen.c` to display metadata (high scores, icons) or improve UI animations.
* Feel free to rewrite the helper scripts in **Python**, **Make**, or **Ansible** as long as they provide identical behaviour.

---