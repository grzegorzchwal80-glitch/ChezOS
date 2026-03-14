#pragma once

#include <windows.h>
#define SMGVER 1.00
#define NAME "ChezOS API"
#define AUTHOR "Mangykirb"
#define BEGIN ""

class SMG {
public:
  // no private cuz what ur hiding anyway?

  // Types from typedef idk
  typedef char32_t CHAR32;
  typedef char CHAR;
  typedef const char CONCHAR;
  typedef int INT;
  typedef long long int INT64;
  typedef unsigned long long int UINT64;
  typedef double DOUBLE;
  typedef float FLOAT;
  typedef int *INTPOINTER;
  typedef double *DOUBLEPOINTER;
  typedef float *FLOATPOINTER;
  typedef char *CHARPOINTER;
  typedef void *VOIDPOINTER;
  typedef bool *BOOLPOINTER;
  typedef bool BOOL;
  typedef long long int *INT64POINTER;
  typedef unsigned long long int *UINT64POINTER;
  typedef char16_t *CHAR16POINTER;

  // variables
  static inline SMG::CHAR Name[11] = NAME;
  static inline SMG::CHAR Author[10] = AUTHOR;
  static inline SMG::CHAR Version[10] = BEGIN;
  static inline SMG::DOUBLE SMGver = SMGVER;

  void sleep_ms(SMG::INT64 ms) {
    HANDLE t = CreateWaitableTimer(NULL, TRUE, NULL);
    LARGE_INTEGER due;
    due.QuadPart = -(ms / 100);
    SetWaitableTimer(t, &due, 0, NULL, NULL, FALSE);
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
  }
};