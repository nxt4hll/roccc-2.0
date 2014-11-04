#include "common/system_specific.h"
#include "dll_subsystem.h"
#include "common/suif_list.h"
#include "suif_exception.h"
//#include "error_macros.h"

/* This is for placing a debug hook */
static void dll_init_hook(const char *name);

#if defined( UNIX_DLL_INTERFACE )
#include <dlfcn.h>
#elif defined( WIN32_DLL_INTERFACE )
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
//#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>
#endif

#include <stdio.h>
#include <assert.h>

#include "suifkernel_messages.h"



DLLSubSystem::DLLSubSystem( SuifEnv* suif_env ) :
  SubSystem( suif_env ),
  _loaded( new list<LString> )
{
}

DLLSubSystem::~DLLSubSystem()
{
  delete _loaded;
}

void DLLSubSystem::require_DLL( const LString& libraryName ) {
  if (!is_DLL_loaded(libraryName))
    load_and_initialize_DLL( libraryName );
}

bool DLLSubSystem::is_DLL_loaded( const LString& libraryName ) {
  for (list<LString>::iterator iter = _loaded->begin();
       iter != _loaded->end(); iter++) {
    if ((*iter) == libraryName) return(true);
  }
  return(false);
}

void DLLSubSystem::load_and_initialize_DLL( const LString& libraryName ) {
  String fileName;
  String initName;

  if (!is_DLL_loaded(libraryName))
    _loaded->push_back(libraryName);

  void (*initFunction)( SuifEnv* );

#ifdef UNIX_DLL_INTERFACE
  // code partially copied from dynasty.cc (Chris Wilson)
    /*
     *  In this case we assume that there is a <dlfcn.h> header file
     *  and that #include'ing <stdio.h> and <dlfcn.h> gives us the
     *  following two functions:
     *
     *      void *dlopen(char *filename, int mode);
     *      void *dlsym(void *handle, char *name);
     *
     *  and the RTLD_LAZY macro.  Acording to man pages on relatively
     *  current versions of Linux, Digital Unix 4.3, Solaris 2.5, and
     *  Irix 5.3 (all the UNIX systems I have available to me right
     *  now that support dynamic linking at all) at the time of this
     *  writing (January 1998), exactly this interface is provided on
     *  these four systems.  So it seems to be at least a de facto
     *  standard interface to program-controlled dynamic linking on
     *  UNIX systems.
     */

#ifdef __APPLE__
  fileName = String("lib") + libraryName + ".dylib" ;
#else
  fileName = String("lib") + libraryName + ".so" ;
#endif

  //    fileName = String("lib") + libraryName + ".so";
  //  fileName = String("lib") + libraryName + ".dylib" ;
    void* dlhandle = dlopen(fileName.c_str(), RTLD_LAZY);
    if (!dlhandle) {
      char *msg = dlerror();
      String erm;
      if (msg) {
	erm = msg;
      } else {
	erm = String("dlopen: ") + fileName + ": Could not open DLL";
      }
      SUIF_THROW(SuifException(erm));
    }
    initName = String("init_") + libraryName;
    initFunction =
            (void (*)(SuifEnv*))(dlsym(dlhandle, initName.c_str() ));

    if (!initFunction) {
      SUIF_THROW(SuifException(String("dlopen: '") + fileName +
			       "': Could not find function: '" +
			       initName + "'\n"));
    }

#elif defined( WIN32_DLL_INTERFACE )

    /*
     *  I found this interface described in two different third-party
     *  books on MS Visual C++.  Presumably it would work with an
     *  Win32 API compiler.
     */
    fileName = libraryName + ".DLL";
    HINSTANCE dlhandle = ::LoadLibrary( fileName.c_str() );
#ifdef MSVC
    if(!dlhandle){
        LPVOID lpMsgBuf;
        int err_num = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err_num,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );
        fprintf(stderr, (LPCTSTR)lpMsgBuf);
        kernel_assert_message(dlhandle, ("Can't load library %s, %d", fileName.c_str(), err_num));
        
        LocalFree( lpMsgBuf );
   }
#else 
    assert(dlhandle);
#endif

    initName =
     String("init_") + libraryName;

    initFunction =
            (void (*)(SuifEnv*))(::GetProcAddress(dlhandle,initName.c_str()));


#else

#error "SET ONE OF THE FOLLOWING MACROS: UNIX_DLL_INTERACE or WIN32_DLL_INTERFACE"

#endif
    assert( initFunction );
    dll_init_hook(initName.c_str());
    (*initFunction)( _suif_env );

}

void dll_init_hook(const char *name) {
  
}

