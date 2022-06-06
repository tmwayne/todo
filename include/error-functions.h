// 
// -----------------------------------------------------------------------------
// error-functions.h
// -----------------------------------------------------------------------------
//
// Copyright (c) 2019 Michael Kerrisk
//
// This program is free software. You may use, modify, and redistribute it
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 or (at your option)
// any later version. This program is distributed without any warranty.
//

#ifndef ERROR_FUNCTIONS_H
#define ERROR_FUNCTIONS_H

// Print to stderr the error text corresponding to the current value of errno
void errMsg(const char *format, ...);

#ifdef __GNUC__

// This maco stops 'gcc -Wall' from complaining that
// "control reached end of non-void function" if we use the
// following functions to terminate main() or some other
// non-void function.

#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

// Print system call error text and exit the program
void sysErrExit(const char *format, ...) NORETURN;

// Print system call error text, but don't flush stdout before printing 
// error message and terminate by calling_exit(2) instead of exit(3), 
// which terminates without flushing stdout buffers and without
// invoking exit handlers
void sysErr_Exit(const char *format, ...) NORETURN;

// Print the text corresponding to the error number and exit the program
void sysErrExitEN(int errno, const char *format, ...) NORETURN;

// Diagnose general errors, including from functions that don't set errno
void errExit(const char *format, ...) NORETURN;

// Diagnose errors in command-line usage
void usageErr(const char *format, ...) NORETURN;

// Diagnose errors in command-line arguments
void cmdLineErr(const char *format, ...) NORETURN;

#endif // ERROR_FUNCTIONS_H
