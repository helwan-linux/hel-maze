# Helwan Linux Maze  🎮

**Helwan Linux Maze** is a fast-paced, terminal-inspired graphical puzzle game built with C and GTK-3.0. Originally developed by Saeed Badreldin, it serves as the official built-in game for Helwan Linux, an Arch-based distribution designed for education and development.

---

## 🕹️ User Guide: How to Play

### 🎯 Objective

The goal is simple but challenging: Navigate your player through a randomly generated maze to reach the **Green Exit Square** before the timer runs out.

### 🎮 Controls

* **Arrow Keys** → Move the blue player (Up, Down, Left, Right)
* **Enter Key** → Restart the game or proceed to the next level after a win/loss
* **Options Menu** → Access the *How to Play* manual and *About* info directly from the game window

### ⚙️ Game Features

* **Fog of War** → You can only see a small radius around your player. Explore carefully!
* **Yellow Coins** → Collect these to gain **+10 Score** and **+5 seconds** of bonus time
* **Red Enemy** → A smart AI entity that chases you. It gets faster and more aggressive as you reach higher levels
* **Infinite Levels** → Each time you win, a new, more difficult maze is generated

---

## 🛠️ Developer Guide: Join the Project

As Helwan Linux is an open-source project focused on development and education, we welcome contributors to help improve this game or the distribution itself.

### 📦 Prerequisites

To compile the game on Helwan Linux (or any Arch-based system), ensure you have the following installed:

* gcc (GNU Compiler Collection)
* gtk3 (GTK+ 3 development libraries)
* pkg-config

### ⚡ Compilation

You can compile the source code using the following command in your terminal:

```bash
gcc main.c -o helwan-maze `pkg-config --cflags --libs gtk+-3.0` -lm
```

### 🤝 Contribution Areas

We are looking for help in the following areas:

* **AI Improvement** → Enhancing the enemy's pathfinding algorithms (currently basic DFS/Random tracking)
* **Theming** → Adding support for custom GTK themes matching:

  * Home (GNOME)
  * Edu (Xfce)
  * Dev (Cinnamon)
* **Features** → Implementing a High Score system saved to:

```
/home/$USER/.config/helwan-maze/
```

* **Localization** → Translating the UI into other languages while maintaining English as the primary technical standard

---

## 📜 Project Philosophy

All contributions should follow the professional standards of the Helwan Linux project. We prioritize clean, direct code and global-standard technical implementations.

---

## 📜 License

This project is licensed under the **MIT License**.

```
MIT License

Copyright (c) 2026 Saeed Badreldin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 👨‍💻 Credits

* **Developed by:** Saeed Badreldin
* **Part of:** Helwan Linux OS Project
* **Website:** [helwan-linux.org](https://helwan-linux.github.io/helwanlinux/index.html)
