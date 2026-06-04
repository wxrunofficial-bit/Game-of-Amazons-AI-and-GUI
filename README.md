# Game of Amazons AI & GUI-北京大学2025秋计算概论A期末大作业

<p align="center">
  <img src="background.jpg" alt="Game Background" width="720">
</p>

<p align="center">
  <b>A C++ / EasyX implementation of the Game of Amazons, featuring an Alpha-Beta search AI and an interactive graphical interface.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++20">
  <img src="https://img.shields.io/badge/GUI-EasyX-blueviolet?style=flat-square" alt="EasyX">
  <img src="https://img.shields.io/badge/AI-Alpha--Beta%20Search-brightgreen?style=flat-square" alt="Alpha-Beta">
  <img src="https://img.shields.io/badge/Course-PKU%20IntroCS-red?style=flat-square" alt="PKU">
</p>

---

## Introduction

This repository contains my final project for **Peking University Introduction to Computing A, 2025 Fall**.

The project implements the board game **Game of Amazons** with a complete playable GUI and a built-in AI opponent. The AI combines move generation, heuristic board evaluation, iterative deepening, and Alpha-Beta pruning to make decisions under a limited thinking time.

The project is written in **C++** and uses **EasyX** for graphical rendering.

---

## Features

### Game System

* Complete implementation of the Game of Amazons rules
* Human vs AI gameplay
* Support for playing as either black or white
* Legal move highlighting
* Arrow-shooting phase visualization
* Win/loss detection
* Undo support
* Save and load game state
* Automatic game history recording
* Historical game replay

### AI Engine

The AI is based on a classical game-tree search framework:

* Minimax search
* Alpha-Beta pruning
* Iterative deepening
* Time-limited search
* Move ordering based on rough heuristic scores
* Board evaluation using:

  * territory control
  * mobility
  * positional advantage
  * center control

The design goal is not only to make the AI playable, but also to keep the implementation readable enough for future students who want to learn from a compact board-game AI example.

### Graphical Interface

The GUI is implemented with EasyX and includes:

* Interactive chessboard
* Custom queen images
* Background image
* Move and shooting hints
* Button-based menu system
* Snow animation effect
* UI refresh during AI thinking

---

## Project Structure

```text
.
├── mama.cpp                 # Main source file: game logic, AI, GUI
├── mylatr.vcxproj            # Visual Studio project file
├── black.png                 # Black queen image asset
├── white.png                 # White queen image asset
├── background.jpg            # Background image asset
├── savegame.txt              # Saved game state
├── history_record.txt        # Historical game records
└── README.md
```

---

## Environment

Recommended environment:

* Windows
* Visual Studio
* EasyX graphics library
* C++20

This project depends on EasyX, so it is intended to be built and run on Windows with Visual Studio.

---

## How to Run

### 1. Clone the repository

```bash
git clone https://github.com/wxrunofficial-bit/Game-of-Amazons-AI-and-GUI.git
cd Game-of-Amazons-AI-and-GUI
```

### 2. Install EasyX

Download and install EasyX for Visual Studio from the official EasyX website.

After installation, make sure Visual Studio can correctly include:

```cpp
#include <graphics.h>
```

### 3. Open the Visual Studio project

Open:

```text
mylatr.vcxproj
```

Then build and run the project in Visual Studio.

### 4. Keep assets in the project root

The following files should stay in the same directory as the executable/project root:

```text
black.png
white.png
background.jpg
savegame.txt
history_record.txt
```

Otherwise, the GUI may fail to load images or records correctly.

---

## Controls

The game is operated mainly through mouse clicks.

| Action            | Description                                     |
| ----------------- | ----------------------------------------------- |
| New Game as Black | Start a new game with the human player as black |
| New Game as White | Start a new game with the human player as white |
| Save              | Save the current game state                     |
| Load              | Load a saved game                               |
| Undo              | Undo recent moves                               |
| History Replay    | View and replay previous games                  |
| Exit              | Close the game                                  |

During replay:

| Mouse Action | Description             |
| ------------ | ----------------------- |
| Left click   | Pause / continue replay |
| Right click  | Exit replay             |

---

## AI Design

The AI searches possible moves and chooses the best move according to a heuristic evaluation function.

### Search

The search procedure uses:

```text
Iterative Deepening + Minimax + Alpha-Beta Pruning
```

The AI starts from shallow depth and keeps deepening the search while the time limit allows. If the current depth is not fully searched before timeout, the AI keeps the best move found from the last completed depth.

### Evaluation

The evaluation function considers several factors:

1. **Territory control**
   A square is more valuable if the AI can reach it faster than the opponent.

2. **Mobility**
   Pieces with more reachable squares are generally better.

3. **Position score**
   Nearby controllable positions contribute to local advantage.

4. **Center control**
   Positions closer to the board center are mildly preferred.

This makes the AI prefer moves that expand space, restrict the opponent, and maintain long-term mobility.

---

## Implementation Notes

This project is intentionally kept as a mostly single-file C++ implementation. Although this is not ideal for large-scale software engineering, it makes the project easier to inspect as a course assignment archive.

Some implementation highlights:

* Board state is represented by a fixed `8 × 8` array.
* Move generation simulates queen movement and arrow shooting.
* Backtracking is used during game-tree search.
* Static arrays are used in BFS for speed.
* GUI state and game state are separated enough to support save/load and replay.
* The UI keeps refreshing during AI thinking so the program does not look frozen.

---

## Screenshots

A demo screenshot or GIF can be added here later.

```markdown
![demo](docs/demo.gif)
```

Recommended future additions:

* opening menu screenshot
* mid-game screenshot
* AI thinking screenshot
* replay demo GIF

---

## Possible Improvements

This project is a course project and still has room for improvement:

* Refactor the single source file into multiple modules
* Add stronger move ordering
* Add transposition table / Zobrist hashing
* Tune evaluation weights more systematically
* Add Monte Carlo Tree Search as an alternative AI
* Improve cross-platform support
* Provide a packaged executable release
* Add more polished screenshots and demo videos

---

## Acknowledgements

This project was completed as the final assignment for **Peking University Introduction to Computing A**.

Thanks to the course staff and classmates for their help and inspiration.

Special thanks to 小小鱼粥於.

---

## Academic Integrity Notice

This repository is kept as a personal course project archive and learning reference. If you are taking a similar course, please follow your course's academic integrity rules and do not submit this project as your own work.
