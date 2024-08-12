# Sudoku Validator using Multithreading

### Introduction:
This C++ program validates a Sudoku puzzle using multithreading. It checks each row, column, and 3x3 sub-grid for duplicate numbers or numbers outside the range of 1 to 9. The program utilizes POSIX threads (pthreads) to perform these checks concurrently, improving performance.
### Features:
* Validates Sudoku puzzles with 9x9 grids
* Checks rows, columns, and 3x3 sub-grids for errors
* Detects duplicate numbers and out-of-range values
* Utilizes multithreading for concurrent checks
* Displays detailed error reports
### Usage:
* Compile the program using g++ with the -pthread flag: g++ sudoku_validator.cpp -o sudoku_validator -pthread
* Run the program: ./Sudoku_Validator
* The program will print the puzzle and validation results (valid or invalid with details)

#### Note:
The puzzle is hardcoded in the program for simplicity. You can modify the puzzle array to test different Sudoku puzzles.
This program uses POSIX threads, which are available on Unix-based systems (e.g., Linux, macOS). For Windows, consider using a different threading library or modifying the program to use a single thread.