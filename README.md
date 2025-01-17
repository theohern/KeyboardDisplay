# Keyboard Input Latency Project

This project aims to minimize the latency between pressing a key on the keyboard and displaying the corresponding letter on the screen. Several solutions are explored, each implemented in different folders, progressing from less to more performant methods.

## Goal

The primary goal of this project is to measure and reduce the latency of displaying a character after a key press. Each implementation explores different trade-offs between complexity and performance.

## How to Use

1. Navigate to the folder of the implementation you want to test (`python/`, `cpp_fltk/`, or `drm/`).
2. Follow the setup instructions in the respective folder to run the program.
3. Press a key on your keyboard and observe how quickly the corresponding character appears on the screen.

## Performance Comparison

The implementations are organized in increasing order of performance:
1. **Python:** Simple but higher latency.
2. **C++ with FLTK:** Improved performance with moderate complexity.
3. **DRM:** Lowest latency with advanced hardware-level optimizations.
