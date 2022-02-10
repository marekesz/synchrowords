#pragma once
#include <exception>

#ifndef __INTELLISENSE__
$EXACT_DEF$
#endif

namespace synchrolib {

class OutOfMemoryException : public std::exception {
  const char* what() const throw() { return "OutOfMemoryException"; }
};

}  // namespace synchrolib

#ifndef __INTELLISENSE__
$EXACT_UNDEF$
#endif
