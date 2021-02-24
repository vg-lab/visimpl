
#ifndef UTILS_EXCEPTION_H_
#define UTILS_EXCEPTION_H_

// Qt
#include <QString>

// C++
#include <exception>
#include <signal.h>

class QTextStream;

namespace Utils
{
  /** \brief Installs the signal handler and reserves the memory for an
   *         alternate stack for tracing.
   *
   */
  void installSignalHandler();

  /** \brief Installs the handler for unmanaged exceptions.
   *
   */
  void installExceptionHandler();

  /** \brief Helper function to trace the stack and print method names.
   * \param[in] stream text stream where the stack info will be written.
   *
   */
  void backtrace_stack_print(QTextStream &stream);

  extern const int STACK_SIZE;
  extern uint8_t alternate_stack[];
} // namespace ESPINA

#endif // UTILS_EXCEPTION_H_
