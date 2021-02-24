#include "ExceptionUtils.h"

// QT
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QString>
#include <QTextStream>
#include <QDir>

// C++
#include <iostream>
#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace Utils;

static int const STACK_FRAMES = 40;

const int Utils::STACK_SIZE = 8192;
uint8_t Utils::alternate_stack[Utils::STACK_SIZE];

#ifdef __linux__
#define OS_STRING "Linux"
#elif __WIN64__
#define OS_STRING "Windows"
#endif

#ifdef __linux__

#include <execinfo.h>

//-----------------------------------------------------------------------------
void signalHandler(int signal, siginfo_t *siginfo, __attribute__ ((unused)) void *context)
{
  const char *signal_text = nullptr;

  switch(signal)
  {
    case SIGSEGV:
      signal_text = "SIGSEGV: segmentation fault";
      break;
    case SIGINT:
      signal_text = "SIGINT: interactive attention signal (ctrl+c ?)";
      break;
    case SIGFPE:
      switch(siginfo->si_code)
      {
        case FPE_INTDIV:
          signal_text = "SIGFPE: integer divide by zero";
          break;
        case FPE_INTOVF:
          signal_text = "SIGFPE: integer overflow";
          break;
        case FPE_FLTDIV:
          signal_text = "SIGFPE: floating-point divide by zero";
          break;
        case FPE_FLTOVF:
          signal_text = "SIGFPE: floating-point overflow";
          break;
        case FPE_FLTUND:
          signal_text = "SIGFPE: floating-point underflow";
          break;
        case FPE_FLTRES:
          signal_text = "SIGFPE: floating-point inexact result";
          break;
        case FPE_FLTINV:
          signal_text = "SIGFPE: floating-point invalid operation";
          break;
        case FPE_FLTSUB:
          signal_text = "SIGFPE: subscript out of range";
          break;
        default:
          signal_text = "SIGFPE: arithmetic exception";
          break;
      }
      break;
    case SIGILL:
      switch(siginfo->si_code)
      {
        case ILL_ILLOPC:
          signal_text = "SIGILL: illegal opcode";
          break;
        case ILL_ILLOPN:
          signal_text = "SIGILL: illegal operand";
          break;
        case ILL_ILLADR:
          signal_text = "SIGILL: illegal addressing mode";
          break;
        case ILL_ILLTRP:
          signal_text = "SIGILL: illegal trap";
          break;
        case ILL_PRVOPC:
          signal_text = "SIGILL: privileged opcode";
          break;
        case ILL_PRVREG:
          signal_text = "SIGILL: privileged register";
          break;
        case ILL_COPROC:
          signal_text = "SIGILL: coprocessor error";
          break;
        case ILL_BADSTK:
          signal_text = "SIGILL: internal stack error";
          break;
        default:
          signal_text = "SIGILL: illegal instruction";
          break;
      }
      break;
    case SIGTERM:
      signal_text = "SIGTERM: explicit termination request";
      break;
    case SIGABRT:
      signal_text = "SIGABRT: probably assert()";
      break;
    default:
      signal_text = "Unidentified signal.";
      break;
  }

  auto date     = QDate::currentDate();
  auto time     = QTime::currentTime();
  auto fileName = QObject::tr("ViSimpl-dump-%1_%2_%3-%4_%5_%6.txt").arg(date.year(),   4, 10, QChar('0'))
                                                                   .arg(date.month(),  2, 10, QChar('0'))
                                                                   .arg(date.day(),    2, 10, QChar('0'))
                                                                   .arg(time.hour(),   2, 10, QChar('0'))
                                                                   .arg(time.minute(), 2, 10, QChar('0'))
                                                                   .arg(time.second(), 2, 10, QChar('0'));
  QFile file{QDir::home().filePath(fileName)};
  if(file.open(QIODevice::Truncate|QIODevice::ReadWrite))
  {
    QTextStream out(&file);
    out << "-- VISIMPL CRASH ------------------------------------------\n";
    out << "OS: " << OS_STRING << "\n";
    out << "WHEN: " << date.toString() << " " << time.toString() << "\n";
    out << "SIGNAL: " << signal_text << "\n";
    Utils::backtrace_stack_print(out);
  }

  std::_Exit(1);
}

#endif // Linux

#ifdef __WIN64__

  // definitions needed by bfd.h apparently
  #define PACKAGE "ViSimpl"
  #define PACKAGE_VERSION "Win64"

  #include <windows.h>
  #include <excpt.h>
  #include <imagehlp.h>
  #include <bfd.h>
  #include <psapi.h>
  #include <windef.h>
  #include <libloaderapi.h>
  #include <csignal>
  #include <cfloat>
  #include <process.h>

//--------------------------------------------------------------------
void win_sig_action(int signal)
{
  const char *signal_text = nullptr;

  switch(signal)
  {
    case SIGSEGV:
      signal_text = "SIGSEGV: segmentation fault";
      break;
    case SIGINT:
      signal_text = "SIGINT: interactive attention signal (ctrl+c ?)";
      break;
    case SIGFPE:
      signal_text = "SIGFPE: floating point exception";
      break;
    case SIGILL:
      signal_text = "SIGILL: illegal instruction";
      break;
    case SIGTERM:
      signal_text = "SIGTERM: explicit termination request";
      break;
    case SIGABRT:
      signal_text = "SIGABRT: probably assert()";
      break;
    default:
      signal_text = "Unidentified signal.";
      break;
  }

  auto date     = QDate::currentDate();
  auto time     = QTime::currentTime();
  auto fileName = QObject::tr("ViSimpl-dump-%1_%2_%3-%4_%5_%6.txt").arg(date.year(),   4, 10, QChar('0'))
                                                                   .arg(date.month(),  2, 10, QChar('0'))
                                                                   .arg(date.day(),    2, 10, QChar('0'))
                                                                   .arg(time.hour(),   2, 10, QChar('0'))
                                                                   .arg(time.minute(), 2, 10, QChar('0'))
                                                                   .arg(time.second(), 2, 10, QChar('0'));
  QFile file{QDir::home().filePath(fileName)};
  if(file.open(QIODevice::Truncate|QIODevice::ReadWrite))
  {
    QTextStream out(&file);
    out << "-- VISIMPL CRASH ------------------------------------------\n";
    out << "OS: " << OS_STRING << "\n";
    out << "WHEN: " << date.toString() << " " << time.toString() << "\n";
    out << "SIGNAL: " << signal_text << "\n";
    out.flush();
    Utils::backtrace_stack_print(out);
  }

  std::_Exit(1);
}

#define BFD_ERR_OK          (0)
#define BFD_ERR_OPEN_FAIL   (1)
#define BFD_ERR_BAD_FORMAT  (2)
#define BFD_ERR_NO_SYMBOLS  (3)
#define BFD_ERR_READ_SYMBOL (4)

static const char *const bfd_errors[] = {
  "Empty",
  "Failed to open bfd",
  "Bad format",
  "No symbols",
  "Failed to read symbols",
};

struct bfd_ctx
{
  bfd * handle;
  asymbol ** symbol;
};

struct bfd_set
{
  char * name;
  struct bfd_ctx * bc;
  struct bfd_set *next;
};

struct find_info
{
  asymbol **symbol;
  bfd_vma counter;
  const char *file;
  const char *func;
  unsigned line;
};

//--------------------------------------------------------------------
void lookup_section(bfd *abfd, asection *sec, void *opaque_data)
{
  struct find_info *data = reinterpret_cast<struct find_info *>(opaque_data);

  if (data->func) return;

  if (!(bfd_get_section_flags(abfd, sec) & SEC_ALLOC)) return;

  bfd_vma vma = bfd_get_section_vma(abfd, sec);
  if (data->counter < vma || vma + bfd_get_section_size(sec) <= data->counter) return;

  bfd_find_nearest_line(abfd, sec, data->symbol, data->counter - vma, &(data->file), &(data->func), &(data->line));
}

//--------------------------------------------------------------------
void find(struct bfd_ctx * b, DWORD offset, const char **file, const char **func, unsigned int *line)
{
  struct find_info data;
  data.func    = nullptr;
  data.symbol  = b->symbol;
  data.counter = offset;
  data.file    = nullptr;
  data.func    = nullptr;
  data.line    = 0;

  bfd_map_over_sections(b->handle, &lookup_section, &data);
  if (file)
  {
    *file = data.file;
  }
  if (func)
  {
    *func = data.func;
  }
  if (line)
  {
    *line = data.line;
  }
}

//--------------------------------------------------------------------
int init_bfd_ctx(struct bfd_ctx *bc, const char * procname, int *err)
{
  bc->handle = nullptr;
  bc->symbol = nullptr;

  bfd *b = bfd_openr(procname, 0);
  if (!b)
  {
    if(err) { *err = BFD_ERR_OPEN_FAIL; }
    return 1;
  }

  if(!bfd_check_format(b, bfd_object))
  {
    bfd_close(b);
    if(err) { *err = BFD_ERR_BAD_FORMAT; }
    return 1;
  }

  if(!(bfd_get_file_flags(b) & HAS_SYMS))
  {
    bfd_close(b);
    if(err) { *err = BFD_ERR_NO_SYMBOLS; }
    return 1;
  }

  void *symbol_table;

  unsigned dummy = 0;
  if (bfd_read_minisymbols(b, FALSE, &symbol_table, &dummy) == 0)
  {
    if (bfd_read_minisymbols(b, TRUE, &symbol_table, &dummy) < 0)
    {
      free(symbol_table);
      bfd_close(b);
      if(err) { *err = BFD_ERR_READ_SYMBOL; }
      return 1;
    }
  }

  bc->handle = b;
  bc->symbol = reinterpret_cast<asymbol **>(symbol_table);

  if(err) { *err = BFD_ERR_OK; }
  return 0;
}

//--------------------------------------------------------------------
void close_bfd_ctx(struct bfd_ctx *bc)
{
  if (bc) {
    if (bc->symbol) {
      free(bc->symbol);
    }
    if (bc->handle) {
      bfd_close(bc->handle);
    }
  }
}

//--------------------------------------------------------------------
struct bfd_ctx *get_bc(struct bfd_set *set , const char *procname, int *err)
{
  while (set->name)
  {
    if (strcmp(set->name, procname) == 0)
    {
      return set->bc;
    }
    set = set->next;
  }
  struct bfd_ctx bc;
  if (init_bfd_ctx(&bc, procname, err))
  {
    return NULL;
  }
  set->next = reinterpret_cast<struct bfd_set *>(calloc(1, sizeof(*set)));
  set->bc   = reinterpret_cast<struct bfd_ctx *>(malloc(sizeof(struct bfd_ctx)));
  memcpy(set->bc, &bc, sizeof(bc));
  set->name = strdup(procname);

  return set->bc;
}

//--------------------------------------------------------------------
void release_set(struct bfd_set *set)
{
  while(set) {
    struct bfd_set * temp = set->next;
    free(set->name);
    close_bfd_ctx(set->bc);
    free(set);
    set = temp;
  }
}

#endif // Windows

//-----------------------------------------------------------------------------
void exceptionHandler()
{
  auto exptr = std::current_exception();

  const char *message = nullptr;
  const char *details = nullptr;

  try
  {
    std::rethrow_exception(exptr);
  }
  catch (const std::exception &e)
  {
    message = e.what();
  }
  catch(...)
  {
    message = "Unidentified exception.\n";
  }

  auto date     = QDate::currentDate();
  auto time     = QTime::currentTime();
  auto fileName = QObject::tr("ViSimpl-dump-%1_%2_%3-%4_%5_%6.txt").arg(date.year(),   4, 10, QChar('0'))
                                                                  .arg(date.month(),  2, 10, QChar('0'))
                                                                  .arg(date.day(),    2, 10, QChar('0'))
                                                                  .arg(time.hour(),   2, 10, QChar('0'))
                                                                  .arg(time.minute(), 2, 10, QChar('0'))
                                                                  .arg(time.second(), 2, 10, QChar('0'));
  QFile file{QDir::home().filePath(fileName)};
  if(file.open(QIODevice::Truncate|QIODevice::ReadWrite))
  {
    QTextStream out(&file);
    out << "-- VISIMPL CRASH ------------------------------------------\n";
    out << "OS: " << OS_STRING << "\n";
    out << "WHEN: " << date.toString() << " " << time.toString() << "\n";
    out << "EXCEPTION MESSAGE: " << message << "\n";
    if(details) out << "EXCEPTION DETAILS: " << details << "\n";
    out.flush();
    Utils::backtrace_stack_print(out);
  }

  std::_Exit(1);
}

//-----------------------------------------------------------------------------
void Utils::installSignalHandler()
{
#ifdef __linux__

  /* setup alternate stack */
  stack_t ss;
  ss.ss_sp = (void*) alternate_stack;
  ss.ss_size = STACK_SIZE;
  ss.ss_flags = 0;

  if (sigaltstack(&ss, NULL) != 0)
  {
    auto message = QObject::tr("Error setting the alternative stack for handling signals.");

    throw std::runtime_error(message.toStdString().c_str());
  }

  /* register signal handlers */
  struct sigaction sig_action;
  sig_action.sa_sigaction = signalHandler;
  sigemptyset(&sig_action.sa_mask);

  sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

  if ((sigaction(SIGSEGV, &sig_action, NULL) != 0) ||
      (sigaction(SIGFPE,  &sig_action, NULL) != 0) ||
      (sigaction(SIGINT,  &sig_action, NULL) != 0) ||
      (sigaction(SIGILL,  &sig_action, NULL) != 0) ||
      (sigaction(SIGTERM, &sig_action, NULL) != 0) ||
      (sigaction(SIGABRT, &sig_action, NULL) != 0))
  {
    auto message = QObject::tr("Error setting the handlers for signals.");

    throw std::runtime_error(message.toStdString().c_str());
  }

#endif // Linux

#ifdef __WIN64__

  if ((std::signal(SIGSEGV, &win_sig_action) == SIG_ERR) ||
      (std::signal(SIGFPE,  &win_sig_action) == SIG_ERR) ||
      (std::signal(SIGINT,  &win_sig_action) == SIG_ERR) ||
      (std::signal(SIGILL,  &win_sig_action) == SIG_ERR) ||
      (std::signal(SIGTERM, &win_sig_action) == SIG_ERR) ||
      (std::signal(SIGABRT, &win_sig_action) == SIG_ERR))
  {
    auto message = QObject::tr("Error setting the handlers for signals.");

    throw std::runtime_error(message.toStdString().c_str());
  }

#endif // Windows
}

//-----------------------------------------------------------------------------
void Utils::installExceptionHandler()
{
  std::set_terminate(exceptionHandler); // in Win64 SetUnhandledEventHandler alternative?
}

//-----------------------------------------------------------------------------
void Utils::backtrace_stack_print(QTextStream &stream)
{
  stream << "-- STACK TRACE -------------------------------------------\n";

#ifdef __linux__

  void *stack_traces[STACK_FRAMES];
  auto trace_size = backtrace(stack_traces, STACK_FRAMES);
  auto messages = backtrace_symbols(stack_traces, trace_size);

  size_t funcnamesize = 1024;
  char funcname[1024];
  for (int i = 0; i < trace_size - 1; i++)
  {
    char* begin_name = nullptr;
    char* begin_pos  = nullptr;
    char* end_pos    = nullptr;

    // find parentheses and +address offset surrounding the mangled name
    // ./module(function+0x15c) [0x8048a6d]
    for (char *p = messages[i]; *p; ++p)
    {
      if (*p == '(')
      {
        begin_name = p;
      }
      else
      {
        if (*p == '+')
        {
          begin_pos = p;
        }
        else
        {
          if (*p == ')' && (begin_pos || begin_name))
          {
            end_pos = p;
          }
        }
      }
    }

    if (begin_name && end_pos && (begin_name < end_pos))
    {
      *begin_name++ = '\0';
      *end_pos++ = '\0';
      if (begin_pos)
      {
        *begin_pos++ = '\0';
      }

      // apply __cxa_demangle with the current positions.
      int status = 0;
      auto ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
      auto func_name = begin_name;
      if (status == 0)
      {
        func_name = ret;
      }

      stream << messages[i] << " (";
      stream << ((func_name[0] != '\0') ? func_name : "unidentified") << ",";
      stream << (begin_pos ? begin_pos : "unknown");
      stream << ") " << end_pos << "\n";
    }
    else
    {
      // couldn't parse the line? print the whole line.
      stream << messages[i] << "\n";
    }
  }

  if (messages) free(messages);

#endif // Linux

#ifdef __WIN64__

  if (!SymInitialize(GetCurrentProcess(), 0, true))
  {
    stream << "Failed to init symbol context\n";
    return;
  }

  CONTEXT context;
  memset(&context, 0, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext(&context);
  bfd_init();
  struct bfd_set *set = reinterpret_cast<struct bfd_set *>(calloc(1,sizeof(*set)));

  char procname[MAX_PATH];
  GetModuleFileNameA(nullptr, procname, sizeof procname);

  struct bfd_ctx *bc = nullptr;
  int err = BFD_ERR_OK;

  STACKFRAME64 frame;
  memset(&frame,0,sizeof(frame));
  frame.AddrPC.Offset    = context.Rip;
  frame.AddrPC.Mode      = AddrModeFlat;
  frame.AddrStack.Offset = context.Rsp;
  frame.AddrStack.Mode   = AddrModeFlat;
  frame.AddrFrame.Offset = context.Rbp;
  frame.AddrFrame.Mode   = AddrModeFlat;

  auto process = GetCurrentProcess();
  auto thread  = GetCurrentThread();
  char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255];
  char module_name_raw[MAX_PATH];

  while(StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame, &context, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
  {
    auto symbol = reinterpret_cast<IMAGEHLP_SYMBOL *>(symbol_buffer);
    symbol->SizeOfStruct = (sizeof *symbol) + 255;
    symbol->MaxNameLength = 254;

    auto module_base = SymGetModuleBase(process, frame.AddrPC.Offset);

    if (module_base && GetModuleFileNameA(reinterpret_cast<HMODULE>(module_base), module_name_raw, MAX_PATH))
    {
      const char * module_name = module_name_raw;
      bc = get_bc(set, module_name, &err);
    }

    const char * file = nullptr;
    const char * func = nullptr;

    unsigned int line = 0;
    int status = 0;
    unsigned long long int size = 1024;
    char funcname[1024];

    if (bc)
    {
      find(bc,frame.AddrPC.Offset,&file,&func,&line);
    }

    if (!file)
    {
      PDWORD64 dummy = 0;
      if (SymGetSymFromAddr(process, frame.AddrPC.Offset, dummy, symbol))
      {
        // Need to prepend '_' to symbols to be demangled correctly. If it fails it's not a symbol to demangle.
        char prepended[1024];
        prepended[0] = '_';
        std::strcpy(prepended + 1, symbol->Name);
        auto ret = abi::__cxa_demangle(prepended, funcname, &size, &status);

        if (status == 0)
        {
          file = ret;
        }
        else
        {
          file = symbol->Name;
        }
      }
      else
      {
        file = "[unknown file]";
      }
    }

    if(func)
    {
      auto ret = abi::__cxa_demangle(func, funcname, &size, &status);

      if (status == 0)
      {
        func = ret;
      }
    }

    stream << file << " (" << (func ? func : bfd_errors[err]) << ":" << dec << static_cast<int>(line) << ") " << hex << "Addr: " <<  frame.AddrPC.Offset << "\n";
  }

  release_set(set);
  SymCleanup(GetCurrentProcess());

#endif // Windows
}
