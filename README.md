# Neonland

Neonland is a top-down shooter game for Windows and macOS where your goal is to survive waves of enemies by defeating them with an arsenal of neon-colored weapons.

This game is a school project that I made for a course during my first year majoring in game applications at Metropolis UAS. It's programmed in C++, excluding the macOS renderer which is written in Swift, interfacing with the C++ codebase via C interoperability.

The game uses native APIs on both Windows and macOS.

| Platform   | Windowing and Input | Rendering   | Audio                        |
| ---------- | ------------------- | ----------- | ---------------------------- |
| Windows    | UWP with C++/WinRT  | DirectX 12  | XAudio2, Media Foundation    |
| macOS      | AppKit              | Metal       | AVFoundation                 |

The game is built in a data-driven way, allowing the systems to efficiently query and manipulate the data (entities and components). It's by no means perfect (collision detection could have used some spatial partitioning etc. etc.), but I learned a lot while making it, and have continued to learn ever since.

![gameplay](./Assets/gameplay.gif)
