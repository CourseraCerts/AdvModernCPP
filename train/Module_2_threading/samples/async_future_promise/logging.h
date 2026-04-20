#ifndef LOGGING_H
#define LOGGING_H

#include <mutex>
#include <ostream>

namespace logging {

inline std::mutex gLogMutex;

template <typename... Args>
void logSync(std::ostream& stream, Args&&... args) {
  std::scoped_lock lock(gLogMutex);
  (stream << ... << args);
  stream.flush();
}

} // namespace logging

#endif // LOGGING_H