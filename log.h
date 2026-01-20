/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef LOG_MAX_CALLBACKS
#define LOG_MAX_CALLBACKS 16
#endif

typedef struct
{
  va_list ap;
  const char *fmt;
  const char *file;
  uint32_t time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef uint32_t (*log_TimeFn)(void);
typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

typedef enum
{
  LOG_TRACE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
} log_Level;

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char *log_level_string(log_Level level);
void log_set_time(log_TimeFn fn);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(log_Level level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, log_Level level);
int log_add_fp(FILE *fp, log_Level level);

void log_log(log_Level level, const char *file, int line, const char *fmt, ...);

#ifdef LOG_IMPLEMENTATION
#include <stdio.h>

typedef struct
{
  log_LogFn fn;
  void *udata;
  log_Level level;
} Callback;

static struct
{
  void *udata;
  log_TimeFn time;
  log_LockFn lock;
  log_Level level;
  bool quiet;
  Callback callbacks[LOG_MAX_CALLBACKS];
} L;

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"};
#endif

static void stdout_callback(log_Event *ev)
{
  if (L.time)
  {
#ifdef LOG_USE_COLOR
    fprintf(ev->udata, "[%02u:%02u:%02u.%03u] %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", ev->time / 3600000, (ev->time / 60000) % 60, (ev->time / 1000) % 60, ev->time % 1000, level_colors[ev->level],
            level_strings[ev->level], ev->file, ev->line);
#else
    fprintf(ev->udata, "[%02u:%02u:%02u.%03u] %-5s %s:%d: ", ev->time / 3600000, (ev->time / 60000) % 60, (ev->time / 1000) % 60, ev->time % 1000, level_strings[ev->level], ev->file, ev->line);
#endif
  }
  else
  {
#ifdef LOG_USE_COLOR
    fprintf(ev->udata, "%s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", level_colors[ev->level],
            level_strings[ev->level], ev->file, ev->line);
#else
    fprintf(ev->udata, "%-5s %s:%d: ", level_strings[ev->level], ev->file, ev->line);
#endif
  }
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}

static void file_callback(log_Event *ev)
{
  if (L.time)
  {
    fprintf(
        ev->udata, "[%02u:%02u:%02u.%03u] %-5s %s:%d: ",
        ev->time / 3600000, (ev->time / 60000) % 60, (ev->time / 1000) % 60, ev->time % 1000, level_strings[ev->level], ev->file, ev->line);
  }
  else
  {
    fprintf(ev->udata, "%-5s %s:%d: ", level_strings[ev->level], ev->file, ev->line);
  }
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}

static void lock(void)
{
  if (L.lock)
  {
    L.lock(true, L.udata);
  }
}

static void unlock(void)
{
  if (L.lock)
  {
    L.lock(false, L.udata);
  }
}

const char *log_level_string(log_Level level) { return level_strings[level]; }

void log_set_time(log_TimeFn fn) { L.time = fn; }

void log_set_lock(log_LockFn fn, void *udata)
{
  L.lock = fn;
  L.udata = udata;
}

void log_set_level(log_Level level) { L.level = level; }

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(log_LogFn fn, void *udata, log_Level level)
{
  for (int i = 0; i < LOG_MAX_CALLBACKS; i++)
  {
    if (!L.callbacks[i].fn)
    {
      L.callbacks[i] = (Callback){fn, udata, level};
      return 0;
    }
  }
  return -1;
}

int log_add_fp(FILE *fp, log_Level level)
{
  return log_add_callback(file_callback, fp, level);
}

static void init_event(log_Event *ev, void *udata)
{
  if (!ev->time)
  {
    ev->time = L.time ? L.time() : 0;
  }
  ev->udata = udata;
}

void log_log(log_Level level, const char *file, int line, const char *fmt, ...)
{
  // Strip away path for file
  const char *slash = file;
  for (const char *p = file; *p; p++)
  {
    if (*p == '/' || *p == '\\')
    {
      slash = p + 1;
    }
  }
  file = slash;

  log_Event ev = {
      .fmt = fmt,
      .file = file,
      .line = line,
      .level = level,
  };

  lock();

  if (!L.quiet && level >= L.level)
  {
    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    stdout_callback(&ev);
    va_end(ev.ap);
  }

  for (int i = 0; i < LOG_MAX_CALLBACKS && L.callbacks[i].fn; i++)
  {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level)
    {
      init_event(&ev, cb->udata);
      va_start(ev.ap, fmt);
      cb->fn(&ev);
      va_end(ev.ap);
    }
  }

  unlock();
}
#endif // LOG_IMPLEMENTATION

#endif // LOG_H