/* 
 * Copyleft DieTime/CLI-Autocomplete is licensed under the MIT License
 * Base source code: https://github.com/DieTime/CLI-Autocomplete
 */

/*
 * MIT License
 * Copyright (c) 2020 Denis Glazkov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <core/pch.h>

#define MAX_OF(x, y) (((x) > (y)) ? (x) : (y))

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#ifndef OS_WINDOWS
#define OS_WINDOWS

#include <windows.h>
#include <conio.h>
#include <stdint.h>
#endif
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix) || defined(unix) || defined(__linux__)
#ifndef OS_UNIX
#define OS_UNIX

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#else
#error Unknown environment!
#endif

#if defined(OS_WINDOWS)
#define KEY_ENTER 13
#define KEY_BACKSPACE 8
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_DEL 83
#define CTRL_C 3
#define SPECIAL_SEQ_1 0
#define SPECIAL_SEQ_2 224
#define COLOR_TYPE uint16_t
#define DEFAULT_TITLE_COLOR 160
#define DEFAULT_PREDICT_COLOR 8
#define DEFAULT_MAIN_COLOR 7
#elif defined(OS_UNIX)
#define KEY_ENTER 10
#define KEY_BACKSPACE 127
#define KEY_LEFT 68
#define KEY_RIGHT 67
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_DEL 51
#define KEY_DEL_AFTER 126
#define SPECIAL_SEQ_1 27
#define SPECIAL_SEQ_2 91
#define COLOR_TYPE const char *
#define DEFAULT_TITLE_COLOR "0;30;102"
#define DEFAULT_PREDICT_COLOR "90"
#define DEFAULT_MAIN_COLOR "0"
#endif
#define KEY_SPACE 32
#define KEY_TAB 9


/**
 * Function for checking if input
 * character in ignore list
 *
 * @param ch - Input character
 *
 * @return True if character in ignore list or False
 */
inline int is_ignore_key(int ch);

/**
 * Function for checking if string
 * contains special characters
 *
 * @param str - Input string
 * @param chars - Special characters
 *
 * @return True if contains or False
 */
inline int contain_chars(const char* str, const char* chars);

/**
 * Function for getting current
 * terminal width (cols count)
 *
 * @return Count of terminal cols
 */
inline short terminal_width();

/**
 * Printing text with color in terminal
 *
 * @param text - Printable text
 * @param color - Color for printing
 */
inline void color_print(const char* text, COLOR_TYPE color);

/**
 * Function for clear all content
 * in current line
 */
inline void clear_line();

/**
 * Set cursor X position in current row
 *
 * @param x - Position for moving
 */
inline void set_cursor_x(short x);

/**
 * Function for getting current
 * cursor Y position
 *
 * @return Current cursor Y position
 */
inline short get_cursor_y();

#if defined(OS_UNIX)
/**
 * Implementation of getch() function
 * for UNIX systems
 *
 * @return Pressed keyboard character
 */
inline int _getch();
#endif



inline int is_ignore_key(int ch) {
    int ignore_keys[] =
#if defined(OS_WINDOWS)
    {1, 2, 19, 24, 26};
#elif defined(OS_UNIX)
    {
        1, 2, 4, 24
    };
#endif

    // Calculate length of ignore keys array
    unsigned len = sizeof (ignore_keys) / sizeof (int);

    // Check if character is ignore key
    for (unsigned i = 0; i < len; i++) {
        if (ch == ignore_keys[i]) {
            return 1;
        }
    }

    return 0;
}


inline int contain_chars(const char* str, const char* chars) {
    // Checking for given characters in a string
    for (unsigned i = 0; str[i] != '\0'; i++) {
        for (unsigned j = 0; chars[j] != '\0'; j++) {
            if (str[i] == chars[j]) {
                return 1;
            }
        }
    }

    return 0;
}

inline void color_print(const char* text, COLOR_TYPE color) {
#if defined(OS_WINDOWS)
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_console == NULL) {
        fprintf(stderr, "[ERROR] Couldn't handle terminal\n");
        exit(1);
    }

    CONSOLE_SCREEN_BUFFER_INFO console_info;
    COLOR_TYPE backup;

    // Save current attributes
    if (GetConsoleScreenBufferInfo(h_console, &console_info) == 0) {
        fprintf(stderr, "[ERROR] Couldn't get terminal info\n");
        exit(1);
    }
    backup = console_info.wAttributes;

    // Print colored text
    if (SetConsoleTextAttribute(h_console, color) == 0) {
        fprintf(stderr, "[ERROR] Couldn't set terminal color\n");
        exit(1);
    }

    printf("%s", text);

    // Restore original color
    if (SetConsoleTextAttribute(h_console, backup) == 0) {
        fprintf(stderr, "[ERROR] Couldn't reset terminal color\n");
        exit(1);
    }
#elif defined(OS_UNIX)
    //Set new terminal color
    printf("\033[");
    printf("%s", color);
    printf("m");

    // Print colored text
    printf("%s", text);

    //Resets the text to default color
    printf("\033[0m");
#endif
}

#if defined(OS_UNIX)

inline int _getch() {
    int character;
    struct termios old_attr, new_attr;

    // Backup terminal attributes
    if (tcgetattr(STDIN_FILENO, &old_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't get terminal attributes\n");
        exit(1);
    }

    // Disable echo
    new_attr = old_attr;
    new_attr.c_lflag &= ~(ICANON | ECHO);

    //    new_attr.c_lflag &= ~ISIG;
    //    new_attr.c_cc[VMIN] = 0;
    //    new_attr.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't set terminal attributes\n");
        exit(1);
    }

    // Get input character
    character = getchar();

    // Restore terminal attributes
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't reset terminal attributes\n");
        exit(1);
    }

    return character;
}
#endif

inline short terminal_width() {
#if defined(OS_WINDOWS)
    // Handle current terminal
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_console == NULL) {
        fprintf(stderr, "[ERROR] Couldn't handle terminal\n");
        exit(1);
    }

    // Get current attributes
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    if (GetConsoleScreenBufferInfo(h_console, &console_info) == 0) {
        fprintf(stderr, "[ERROR] Couldn't get terminal info\n");
        exit(1);
    }

    // Return current width
    return console_info.dwSize.X;
#elif defined(OS_UNIX)
    struct winsize t_size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &t_size) == -1) {
        fprintf(stderr, "[ERROR] Couldn't get terminal info\n");
        exit(1);
    }

    return (short) t_size.ws_col;
#endif
}

inline void clear_line() {
#if defined(OS_WINDOWS)
    // Get current terminal width
    short width = terminal_width();
    if (width < 1) {
        fprintf(stderr, "[ERROR] Size of terminal is too small\n");
        exit(1);
    }

    // Create long empty string
    char* empty = (char*) malloc(sizeof (char) * width);
    memset(empty, ' ', width);
    empty[width - 1] = '\0';

    // Clear line
    printf("\r%s\r", empty);

    // Free line
    free(empty);
#elif defined(OS_UNIX)
    printf("\033[2K\r");
#endif
}

inline void set_cursor_x(short x) {
#if defined(OS_WINDOWS)
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_console == NULL) {
        fprintf(stderr, "[ERROR] Couldn't handle terminal\n");
        exit(1);
    }

    // Create position
    COORD xy;
    xy.X = x - 1;
    xy.Y = get_cursor_y();

    // Set cursor position
    if (SetConsoleCursorPosition(h_console, xy) == 0) {
        fprintf(stderr, "[ERROR] Couldn't set terminal cursor position\n");
        exit(1);
    }
#elif defined(OS_UNIX)
    printf("\033[%d;%dH", get_cursor_y(), x);
#endif
}

inline short get_cursor_y() {
#if defined(OS_WINDOWS)
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_console == NULL) {
        fprintf(stderr, "[ERROR] Couldn't handle terminal\n");
        exit(1);
    }

    // Get terminal info
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    if (GetConsoleScreenBufferInfo(h_console, &console_info) == 0) {
        fprintf(stderr, "[ERROR] Couldn't get terminal Y position\n");
        exit(1);
    }

    // Return Y position
    return console_info.dwCursorPosition.Y;
#elif defined(OS_UNIX)
    struct termios old_attr, new_attr;
    char ch, buf[30] = {0};
    int i = 0, pow = 1, y = 0;

    // Backup terminal attributes
    if (tcgetattr(STDIN_FILENO, &new_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't get terminal attributes\n");
        exit(1);
    }

    // Disable echo
    old_attr = new_attr;
    old_attr.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't set terminal attributes\n");
        exit(1);
    }

    // Get info about cursor
    if (write(STDOUT_FILENO, "\033[6n", 4) != 4) {
        fprintf(stderr, "[ERROR] Couldn't get cursor information\n");
        exit(1);
    }

    // Get ^[[{this};1R value

    for (ch = 0; ch != 'R'; i++) {
        if (read(STDIN_FILENO, &ch, 1) != 1) {
            fprintf(stderr, "[ERROR] Couldn't read cursor information");
            exit(1);
        }
        buf[i] = ch;
    }

    i -= 2;
    while (buf[i] != ';') {
        i -= 1;
    }

    i -= 1;
    while (buf[i] != '[') {
        y = y + (buf[i] - '0') * pow;
        pow *= 10;
        i -= 1;
    }

    // Reset attributes
    if (tcsetattr(0, TCSANOW, &new_attr) == -1) {
        fprintf(stderr, "[ERROR] Couldn't reset terminal attributes\n");
        exit(1);
    }

    return (short) y;
#endif
}

#endif //AUTOCOMPLETE_H
