# Game of Amazons AI & GUI-北京大学2025秋计算概论A期末大作业


<div align="center">

![Uploading 99d46a8acfa71a53a2e8fd0f2b8b8b1d.jpg…]()


<br/>

<h3>Amazon 棋 AI 对战系统 · C++ / EasyX 图形界面实现</h3>

<p>
A playable <b>Game of Amazons</b> program with graphical interface, Alpha-Beta pruning AI, save/load system, undo support, and history replay.
</p>

<p>
<img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white"/>
<img src="https://img.shields.io/badge/GUI-EasyX-7B68EE?style=for-the-badge"/>
<img src="https://img.shields.io/badge/AI-Alpha--Beta_Search-2E8B57?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white"/>
</p>

</div>

---

## Overview

This repository contains a complete C++ implementation of **Game of Amazons**, a two-player strategy board game that combines queen-like movement with territory blocking.

The project provides an interactive graphical game client and an AI opponent. The AI searches the game tree with **Minimax**, **Alpha-Beta pruning**, **iterative deepening**, and a hand-crafted evaluation function. The GUI is built with **EasyX**, supporting visual move selection, arrow placement, saving and loading, undo operations, and historical game replay.

This project was originally developed as the final project for **Peking University Introduction to Computing A, 2025 Fall**.

---

## Game Rules

Game of Amazons is played on a chessboard-like grid.

Each turn consists of three steps:

1. Select one of your queens.
2. Move it like a chess queen: horizontally, vertically, or diagonally through empty squares.
3. Shoot an arrow from the queen's new position, also in queen-like movement.

The square hit by the arrow becomes permanently blocked.
A player loses when they have no legal move left.

This makes the game highly strategic: every move both expands one's own space and reduces the opponent's future mobility.

---

## Highlights

### Complete Playable System

* Full implementation of Amazon chess rules
* Human vs AI gameplay
* Support for choosing black or white side
* Legal move visualization
* Arrow placement visualization
* Automatic win/loss detection
* Undo operation
* Save and load system
* Historical game record and replay

### AI Opponent

The built-in AI uses a classical adversarial search pipeline:

* Minimax game-tree search
* Alpha-Beta pruning
* Iterative deepening under time limit
* Heuristic move ordering
* Board evaluation based on territory, mobility, position, and center control

The AI is designed to be both playable and interpretable. Instead of relying on black-box learning, it evaluates the current board from several strategic perspectives and searches ahead to choose a move.

### Graphical Interface

The program includes a custom EasyX-based GUI:

* Background rendering
* Custom black/white queen image assets
* Interactive chessboard
* Button-based side menu
* Animated snow effect
* Visual feedback during move selection
* Responsive screen update while the AI is thinking

---

## AI Design

The AI treats Game of Amazons as a deterministic two-player zero-sum game and searches for the move that maximizes its expected advantage.

### Search Framework

The main search framework is:

```text
Iterative Deepening
        ↓
Minimax Search
        ↓
Alpha-Beta Pruning
        ↓
Heuristic Evaluation
```

Iterative deepening allows the AI to search progressively deeper while respecting the time limit. If the time limit is reached, the AI can still return the best move found at the last completed depth.

Alpha-Beta pruning reduces unnecessary branches in the game tree, making deeper search possible within limited computation time.

### Evaluation Strategy

The evaluation function combines multiple strategic features:

| Component         | Meaning                                                             |
| ----------------- | ------------------------------------------------------------------- |
| Territory control | Whether a square is more easily reachable by the AI or the opponent |
| Mobility          | Number of available movement options                                |
| Position value    | Local freedom and spatial advantage                                 |
| Center control    | Preference for more flexible central positions                      |

In Amazon chess, territory and mobility are especially important because the board becomes increasingly blocked as arrows accumulate. A strong move should not only gain space immediately, but also restrict the opponent's future options.

---

## Project Structure

```text
Game-of-Amazons-AI-and-GUI
├── mama.cpp                  # Main source file: game logic, AI search, GUI, save/load, replay
├── mylatr.vcxproj             # Visual Studio project file
├── mylatr.vcxproj.filters     # Visual Studio filter configuration
├── mylatr.vcxproj.user        # Visual Studio user configuration
├── background.jpg             # Background image used by the GUI
├── black.png                  # Black queen image asset
├── white.png                  # White queen image asset
├── savegame.txt               # Saved game state file
├── history_record.txt         # Historical game record file
└── README.md
```

---

## Environment

The project is intended to run on Windows.

Recommended setup:

| Requirement       | Version / Note      |
| ----------------- | ------------------- |
| Operating System  | Windows             |
| IDE               | Visual Studio       |
| Language Standard | C++20               |
| Graphics Library  | EasyX               |
| Build Target      | x64 Debug / Release |

Because the GUI depends on EasyX, the project is not cross-platform by default.

---

## Build and Run

### 1. Clone the repository

```bash
git clone https://github.com/wxrunofficial-bit/Game-of-Amazons-AI-and-GUI.git
cd Game-of-Amazons-AI-and-GUI
```

### 2. Install EasyX

Install the EasyX graphics library for Visual Studio.

The source code uses:

```cpp
#include <graphics.h>
```

so EasyX must be correctly configured in the Visual Studio environment.

### 3. Open the Visual Studio project

Open the project file:

```text
mylatr.vcxproj
```

Then build and run the project from Visual Studio.

### 4. Keep resource files together

The image and record files should stay in the project working directory:

```text
background.jpg
black.png
white.png
savegame.txt
history_record.txt
```

These files are used by the GUI, save/load system, and history replay system.

---

## User Interface

The right-side menu provides the main game operations:

| Button   | Function                  |
| -------- | ------------------------- |
| 新游戏（您执黑） | Start a new game as black |
| 新游戏（您执白） | Start a new game as white |
| 存盘       | Save the current game     |
| 读盘       | Load a saved game         |
| 悔棋       | Undo previous move        |
| 历史回放     | Replay historical games   |
| 退出       | Exit the program          |

During a turn, the player selects a queen, moves it to a legal square, and then chooses a square to shoot an arrow. The program highlights the interactive process visually.

---

## Save, Load, and Replay

The project includes persistent game-state support.

* `savegame.txt` stores the current board and game state.
* `history_record.txt` stores completed game records.
* The replay system can reconstruct and display previous games step by step.

This makes the project more than a one-shot demo: games can be interrupted, restored, reviewed, and analyzed.

---

## Implementation Details

The implementation is concentrated in `mama.cpp`, which contains the core modules of the project:

| Module               | Responsibility                                               |
| -------------------- | ------------------------------------------------------------ |
| Board representation | Maintains the 8 × 8 game board                               |
| Move generation      | Enumerates queen moves and arrow shots                       |
| Rule checking        | Determines legal moves and terminal states                   |
| AI search            | Performs Minimax search with Alpha-Beta pruning              |
| Evaluation           | Scores board states for the AI                               |
| GUI rendering        | Draws board, queens, background, buttons, and visual effects |
| Interaction handling | Processes mouse input and game phases                        |
| Persistence          | Handles save/load and historical record replay               |

The single-file structure keeps the project easy to compile and inspect, while still covering the major components of a complete board-game AI program.

---

## Technical Notes

Several design choices are worth noting:

* The board is represented as a fixed-size 8 × 8 array, which keeps state copying and rule checking straightforward.
* Move generation follows the movement pattern of a chess queen.
* Search uses backtracking to simulate and undo moves efficiently.
* Iterative deepening improves practical time control.
* The evaluation function is manually designed instead of learned, making the AI behavior easier to understand and debug.
* GUI refreshing is handled during AI computation so that the program remains visually responsive.

---

## Course Context

This project was completed as the final assignment for:

> Peking University
> Introduction to Computing A
> 2025 Fall

The goal of the project was to build a non-trivial C++ program that combines algorithmic design, graphical interaction, and software engineering practice.

Compared with a simple rule-based game, this project emphasizes both **AI decision-making** and **complete user interaction**.

---

## Acknowledgements

Thanks to the teaching staff and classmates of Introduction to Computing A.

Special thanks to **小小鱼粥於**.

---

## Academic Integrity

This repository is maintained as a personal project archive and learning reference.

If you are taking a similar course, please follow your course's academic integrity policy and do not submit this project as your own work.
