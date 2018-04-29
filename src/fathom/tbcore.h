/*
  Copyright (c) 2011-2015 Ronald de Man
  Copyright 2017-2018 Jon Dart
*/

#ifndef TBCORE_H
#define TBCORE_H

#if defined(__cplusplus) && defined(TB_USE_ATOMIC)
#include <atomic>
#endif

#ifndef _WIN32
#include <pthread.h>
#define SEP_CHAR ':'
#define FD int
#define FD_ERR -1
#else
#include <windows.h>
#define SEP_CHAR ';'
#define FD HANDLE
#define FD_ERR INVALID_HANDLE_VALUE
#endif

#ifndef TB_NO_THREADS
#if defined(__cplusplus) && (__cplusplus >= 201103L)

#include <mutex>
#define LOCK_T std::mutex
#define LOCK_INIT(x)
#define LOCK(x) x.lock()
#define UNLOCK(x) x.unlock()

#else
#ifndef _WIN32
#define LOCK_T pthread_mutex_t
#define LOCK_INIT(x) pthread_mutex_init(&(x), NULL)
#define LOCK(x) pthread_mutex_lock(&(x))
#define UNLOCK(x) pthread_mutex_unlock(&(x))
#else
#define LOCK_T HANDLE
#define LOCK_INIT(x) do { x = CreateMutex(NULL, FALSE, NULL); } while (0)
#define LOCK(x) WaitForSingleObject(x, INFINITE)
#define UNLOCK(x) ReleaseMutex(x)
#endif

#endif
#else /* TB_NO_THREADS */
#define LOCK_T          int
#define LOCK_INIT(x)    /* NOP */
#define LOCK(x)         /* NOP */
#define UNLOCK(x)       /* NOP */
#endif

#define WDLSUFFIX ".rtbw"
#define DTZSUFFIX ".rtbz"
#define WDLDIR "RTBWDIR"
#define DTZDIR "RTBZDIR"
#define TBPIECES 6

#define WDL_MAGIC 0x5d23e871
#define DTZ_MAGIC 0xa50c66d7

#define TBHASHBITS 10

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned char ubyte;
typedef unsigned short ushort;

struct TBHashEntry;

#ifdef DECOMP64
typedef uint64 base_t;
#else
typedef uint32 base_t;
#endif

struct PairsData {
  char *indextable;
  ushort *sizetable;
  ubyte *data;
  ushort *offset;
  ubyte *symlen;
  ubyte *sympat;
  int blocksize;
  int idxbits;
  int min_len;
  base_t base[1]; // C++ complains about base[]...
};

struct TBEntry {
  char *data;
  uint64 key;
  uint64 mapping;
  ubyte ready;
  ubyte num;
  ubyte symmetric;
  ubyte has_pawns;
}
#ifdef __GNUC__
__attribute__((__may_alias__));
#else
;
#endif

struct TBEntry_piece {
  char *data;
  uint64 key;
  uint64 mapping;
#if defined(__cplusplus) && defined(TB_USE_ATOMIC)
  atomic<ubyte> ready;
#else
  ubyte ready;
#endif
  ubyte num;
  ubyte symmetric;
  ubyte has_pawns;
  ubyte enc_type;
  struct PairsData *precomp[2];
  int factor[2][TBPIECES];
  ubyte pieces[2][TBPIECES];
  ubyte norm[2][TBPIECES];
};

struct TBEntry_pawn {
  char *data;
  uint64 key;
  uint64 mapping;
#if defined(__cplusplus) && (__cplusplus >= 201103L)
  atomic<ubyte> ready;
#else
  ubyte ready;
#endif
  ubyte num;
  ubyte symmetric;
  ubyte has_pawns;
  ubyte pawns[2];
  struct {
    struct PairsData *precomp[2];
    int factor[2][TBPIECES];
    ubyte pieces[2][TBPIECES];
    ubyte norm[2][TBPIECES];
  } file[4];
};

struct DTZEntry_piece {
  char *data;
  uint64 key;
  uint64 mapping;
  ubyte ready;
  ubyte num;
  ubyte symmetric;
  ubyte has_pawns;
  ubyte enc_type;
  struct PairsData *precomp;
  int factor[TBPIECES];
  ubyte pieces[TBPIECES];
  ubyte norm[TBPIECES];
  ubyte flags; // accurate, mapped, side
  ushort map_idx[4];
  ubyte *map;
};

struct DTZEntry_pawn {
  char *data;
  uint64 key;
  uint64 mapping;
  ubyte ready;
  ubyte num;
  ubyte symmetric;
  ubyte has_pawns;
  ubyte pawns[2];
  struct {
    struct PairsData *precomp;
    int factor[TBPIECES];
    ubyte pieces[TBPIECES];
    ubyte norm[TBPIECES];
  } file[4];
  ubyte flags[4];
  ushort map_idx[4][4];
  ubyte *map;
};

struct TBHashEntry {
  uint64 key;
  struct TBEntry *ptr;
};

struct DTZTableEntry {
  uint64 key1;
  uint64 key2;
  struct TBEntry *entry;
};

#endif

