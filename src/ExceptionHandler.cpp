#include "pch.hpp"
#ifdef _WIN32
#include <DbgHelp.h>
#endif
#include <boost/stacktrace.hpp>
#include <boost/predef.h>

#ifdef _WIN32
LONG WINAPI SehCrashHandler(EXCEPTION_POINTERS* e)
{
    std::string error = "SE Exception: ";
    switch(e->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        error += "Access Violation";
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        error += "Array bounds exceeded";
        break;
    case EXCEPTION_BREAKPOINT:
        error += "Breakpoint";
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        error += "Datatype misalignment";
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        error += "FLT Denormal Operand";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        error += "FLT Divide by zero";
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        error += "FLT Inexact result";
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        error += "AFLT Invalid operation";
        break;
    case EXCEPTION_FLT_OVERFLOW:
        error += "FLT Overflow";
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        error += "FLT Stack check";
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        error += "FLT Underflow";
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        error += "Illegal instruction";
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        error += "In page error";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        error += "INT Divide by zero";
        break;
    case EXCEPTION_INT_OVERFLOW:
        error += "INT Overflow";
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        error += "Invalid disposition";
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        error += "Noncontinuable exception";
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        error += "Priv instruction";
        break;
    case EXCEPTION_SINGLE_STEP:
        error += "Single stop";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        error += "Stack overflow";
        break;
    default:
        error += "Unknown";
    }
    const std::string stacktrace = boost::stacktrace::to_string(boost::stacktrace::stacktrace());

    std::string ret = std::format("{} (0x{:X}) at address: 0x{:X}\n", error, e->ExceptionRecord->ExceptionCode, (uint64_t)e->ExceptionRecord->ExceptionAddress);
#if BOOST_ARCH_X86_64
    ret += std::format("RAX: 0x{:X}, RCX: 0x{:X}, RDX: 0x{:X}, RBX: 0x{:X}, RSP: 0x{:X}\n",
        e->ContextRecord->Rax, e->ContextRecord->Rcx, e->ContextRecord->Rdx, e->ContextRecord->Rbx, e->ContextRecord->Rsp);
    ret += std::format("RBP: 0x{:X}, RSI: 0x{:X}, RDI: 0x{:X}, R8: 0x{:X}, R9: 0x{:X}\n",
        e->ContextRecord->Rbp, e->ContextRecord->Rsi, e->ContextRecord->Rdi, e->ContextRecord->R8, e->ContextRecord->R9);
    ret += std::format("RIP: 0x{:X}, EFlags: 0x{:X}\n", e->ContextRecord->Rip, e->ContextRecord->EFlags);
#elif BOOST_ARCH_X86_32
    ret += std::format("EAX: 0x{:X}, ECX: 0x{:X}, EDX: 0x{:X}, EBX: 0x{:X}, ESP: 0x{:X}\n",
        e->ContextRecord->Eax, e->ContextRecord->Ecx, e->ContextRecord->Edx, e->ContextRecord->Ebx, e->ContextRecord->Esp);
    ret += std::format("EBP: 0x{:X}, ESI: 0x{:X}, EDI: 0x{:X}\n",
        e->ContextRecord->Ebp, e->ContextRecord->Esi, e->ContextRecord->Edi);
    ret += std::format("EIP: 0x{:X}, EFlags: 0x{:X}\n", e->ContextRecord->Eip, e->ContextRecord->EFlags);
#else

#endif
    ret += "\n\n";
    ret += "Stacktrace:";
    ret += "\n";
    ret += stacktrace;

    std::ofstream out("crash.txt", std::ofstream::binary);
    out << ret;
    out.flush();
    out.close();

    wxTextEntryDialog d(wxGetApp().GetTopWindow(), "CustomKeyboard has crashed. The error reported has been generated below.", "Unhandled exception - CustomKeyboard", ret, wxTE_MULTILINE | wxOK);
    d.SetSize(wxSize(800, 600));
    d.ShowModal();

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void ExceptionHandler::Register()
{
#ifdef _WIN32
    ::SetUnhandledExceptionFilter(SehCrashHandler);
#endif
};