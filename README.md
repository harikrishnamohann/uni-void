# Uni-Void
> A sliding puzzle game inside your terminal

Hey all! This is a super simple tui-based game that I made for learning purposes. Let me explain what the game is. You're presented with an n×n matrix with shuffled numbers and one empty tile. you are tasked with sorting the matrix in ascending order by sliding that empty tile around inside the matrix. You can choose a difficulty at the beginning and a leaderboard is displayed on game completion. That's all!

#### demo:
![demo](demo.gif)

---

## 🔧 Build Instructions

### Requirements

- `gcc` or any C compiler
- `ncurses` development library
- `make` build system for compiling all files

### Compile

```bash
git clone https://github.com/harikrishnamohann/uni-void.git
cd uni-void
make release
```
An executable named uni-void should be produced inside the target directory 

### Run the game
```bash
make run
```
or
```bash
./target/uni-void
```

---

### 🎮 Controls

| Key | Action |
|-----|--------|
| Arrow Keys / WASD /vim-keys | Move void tile |
| `?` | Show usage/help |
| `Enter` | Select menu item |
| `u` | Undo last move |
| `r` | Redo last undone move |
| `q` | Quit game |
| `Q` | Force quit (no save) |

---

### 📑 Todo
- ~~highlight characters that are in place~~
- ~~a menu for choosing difficulty~~
- ~~detect when game is completed~~
- ~~status line for showing some informations such as counter, difficulty etc~~
- ~~undo redo mechanism~~
- ~~help window~~
- ~~ensure the solvability of puzzle~~
- ~~save and load functionality~~
- ~~add a mode with limited number of moves~~
- ~~display leaderboard on game completion~~
- sound effects

### 🤫 pro-tip: 
you can actually edit the leaderboards

---
