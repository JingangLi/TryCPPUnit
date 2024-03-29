#include <cpput/dllproxy.h>

#ifdef CPPUT_DLL_SUPPORT
# include <cpput/translate.h>
# ifdef CPPUT_USE_WIN32_DLL
#  define WIN32_LEAN_AND_MEAN 
#  define NOGDI
#  define NOUSER
#  define NOKERNEL
#  define NOSOUND
#  define NOMINMAX
#  include <windows.h>
# endif // # ifdef CPPUT_USE_WIN32_DLL

namespace CppUT {

   // //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Dll API Common to all OS/API
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

DllProxyError::DllProxyError( const std::string &message )
   : std::runtime_error( message )
{
}


DllProxy::DllProxy( const std::string &path )
   : handle_( NULL )
   , path_( path )
{
   loadLibrary( path );
}


DllProxy::~DllProxy()
{
   releaseLibrary();
}


void *
DllProxy::findSymbol( const std::string &symbol )
{
   try
   {
      void *symbolPointer = doFindSymbol( symbol );
      if ( symbolPointer != 0 )
         return symbolPointer;
   }
   catch ( ... )
   {
   }

   return 0;
}


void
DllProxy::loadLibrary( const std::string &path )
{
   try
   {
      releaseLibrary();
      handle_ = doLoadLibrary( path );
      if ( handle_ != 0 )
         return;
   }
   catch (...)
   {
   }

   CppTL::ConstString message = translate( "Failed to load dynamic library: " ) 
      + path + "\n" + getLastErrorDetail();
   throw DllProxyError( message.c_str() );
}


void 
DllProxy::releaseLibrary()
{
   if ( handle_ != 0 )
   {
      doReleaseLibrary();
      handle_ = 0;
   }
}

# ifdef CPPUT_USE_WIN32_DLL
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Dll API for WIN32 (using LoadLibrary)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

DllProxy::LibraryHandle 
DllProxy::doLoadLibrary( const std::string &path )
{
  return ::LoadLibrary( path.c_str() );
}


void 
DllProxy::doReleaseLibrary()
{
  ::FreeLibrary( (HINSTANCE)handle_ );
}


DllProxy::Symbol 
DllProxy::doFindSymbol( const std::string &symbol )
{
  return DllProxy::Symbol( ::GetProcAddress( 
                              (HINSTANCE)handle_, 
                              symbol.c_str() ) );
}


std::string 
DllProxy::getLastErrorDetail() const
{
  LPVOID lpMsgBuf;
  ::FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0,
      NULL 
  );

  std::string message = (LPCTSTR)lpMsgBuf;

  // Display the string.
//  ::MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

  // Free the buffer.
  ::LocalFree( lpMsgBuf );

  return message;
}

#elif defined(CPPUT_USE_DL_DLL)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Dll API for Unix (using dlopen)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


DllProxy::LibraryHandle 
DllProxy::doLoadLibrary( const std::string &libraryName )
{
  return ::dlopen( libraryName.c_str(), RTLD_NOW | RTLD_GLOBAL );
}


void 
DllProxy::doReleaseLibrary()
{
  ::dlclose( handle_ );
}


DllProxy::Symbol 
DllProxy::doFindSymbol( const std::string &symbol )
{
  return ::dlsym( handle_, symbol.c_str() );
}


std::string 
DllProxy::getLastErrorDetail() const
{
  return "";      // ???
}

#elif defined(CPPUT_USE_SHL_DLL)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Dll API for Unix (using shl_load)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

DllProxy::LibraryHandle 
DllProxy::doLoadLibrary( const std::string &libraryName )
{
   return ::shl_load(libraryName.c_str(), BIND_IMMEDIATE, 0L);
}


void 
DllProxy::doReleaseLibrary()
{
  ::shl_unload( (shl_t)m_libraryHandle);
}


DllProxy::Symbol 
DllProxy::doFindSymbol( const std::string &symbol )
{
   DllProxy::Symbol L_symaddr = 0;
   if ( ::shl_findsym( (shl_t*)(&m_libraryHandle), 
                       symbol.c_str(), 
                       TYPE_UNDEFINED, 
                       &L_symaddr ) == 0 )
   {
      return L_symaddr;
   } 

   return 0;
}


std::string 
DllProxy::getLastErrorDetail() const
{
  return "";
}


#elif defined(CPPUT_USE_BEOS_DLL)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Dll API for BeOS (using load_add_on)
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

DllProxy::LibraryHandle 
DllProxy::doLoadLibrary( const std::string &libraryName )
{
  return (LibraryHandle)::load_add_on( libraryName.c_str() );
}


void 
DllProxy::doReleaseLibrary()
{
  ::unload_add_on( (image_id)m_libraryHandle );
}


DllProxy::Symbol 
DllProxy::doFindSymbol( const std::string &symbol )
{
  void *symbolPointer;
  if ( ::get_image_symbol( (image_id)m_libraryHandle, 
                           symbol.c_str(), 
                           B_SYMBOL_TYPE_TEXT, 
                           &symbolPointer ) == B_OK )
    return symnolPointer;
  return NULL;
}


std::string 
DllProxy::getLastErrorDetail() const
{
  return "";
}


# endif // #elif defined(CPPUT_USE_BEOS_DLL)

} // namespace CppUT

#endif // #ifdef CPPUT_DLL_SUPPORT
