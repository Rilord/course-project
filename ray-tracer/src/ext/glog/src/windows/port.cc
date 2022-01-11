

#ifndef _WIN32
# error You should only be including windows/port.cc in a windows environment!
#endif

#include "config.h"
#include <stdarg.h>    // for va_list, va_start, va_end
#include <string.h>    // for strstr()
#include <assert.h>
#include <string>
#include <vector>
#include "port.h"

using std::string;
using std::vector;

// These call the windows _vsnprintf, but always NUL-terminate.
int safe_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  if (size == 0)        // not even room for a \0?
    return -1;          // not what C99 says to do, but what windows does
  str[size-1] = '\0';
  return _vsnprintf(str, size-1, format, ap);
}

#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  const int r = vsnprintf(str, size, format, ap);
  va_end(ap);
  return r;
}
#endif
