include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
# include(CheckPrototypeExists)
# include(CheckStructMember)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

macro_bool_to_01(X11_Xrandr_FOUND HAVE_XRANDR) # kwin

macro_bool_to_01(X11_Xcomposite_FOUND HAVE_XCOMPOSITE) # kicker

macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE) # kwin

macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR) # many uses in workspace/

macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES) # klipper, kicker

macro_bool_to_01(X11_Xxf86misc_FOUND HAVE_XF86MISC) # kdesktop and kcontrol/lock

macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER) # kcontrol/style, konsole, kicker

macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER) # kdesktop

macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST) # khotkeys, kxkb, kdm

macro_bool_to_01(FONTCONFIG_FOUND HAVE_FONTCONFIG) # kcontrol/{fonts,kfontinst}

macro_bool_to_01(X11_dpms_FOUND HAVE_DPMS) # kdesktop

#now check for dlfcn.h using the cmake supplied CHECK_include_FILE() macro
# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if (WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
endif (WIN32)

check_include_files(devinfo.h HAVE_DEVINFO_H)     # kcontrol/infocenter
check_include_files(fstab.h HAVE_FSTAB_H)         # kcontrol/infocenter
check_include_files("sys/types.h;libutil.h" HAVE_LIBUTIL_H) # fish
check_include_files(limits.h HAVE_LIMITS_H)       # kdm, ksmserver
check_include_files(linux/raw.h HAVE_LINUX_RAW_H) # kcontrol/infocenter
check_include_files(mntent.h HAVE_MNTENT_H)       # kcontrol/infocenter
check_include_files(paths.h HAVE_PATHS_H)         # kcheckpass
check_include_files(pty.h HAVE_PTY_H)             # fish
check_include_files(sasl.h HAVE_SASL_H)           # kioslave/ldap
check_include_files(sasl/sasl.h HAVE_SASL_SASL_H) # kioslave/ldap
check_include_files(string.h HAVE_STRING_H)       # kioslave/floppy kioslave/man (but is this really needed?)
check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H) # fish, kcontrol/infocenter
check_include_files(sys/loadavg.h HAVE_SYS_LOADAVG_H) # kasbar
check_include_files(sys/raw.h HAVE_SYS_RAW_H)     # kcontrol/infocenter
check_include_files(sys/select.h HAVE_SYS_SELECT_H) # kdesu and kioslaves (pop3 sftp thumbnail)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H) # kioslave/smtp kcontrol(kio/socks)
check_include_files(sys/sockio.h HAVE_SYS_SOCKIO_H) # kcontrol/infocenter
check_include_files(sys/stat.h HAVE_SYS_STAT_H)     # kioslave/tar (needed?)
check_include_files(sys/time.h HAVE_SYS_TIME_H)     # ksmserver, ksplashml, sftp
check_include_files(sys/types.h HAVE_SYS_TYPES_H)   # kioslave, kcontrol(kio/socks)
check_include_files(sys/wait.h HAVE_SYS_WAIT_H)     # kdesu
check_include_files(termios.h HAVE_TERMIOS_H)       # fish, kdm
check_include_files(termio.h HAVE_TERMIO_H)         # kdm
check_include_files(unistd.h HAVE_UNISTD_H)         # kioslave/man - unneeded
check_include_files(util.h HAVE_UTIL_H)             # fish
check_include_files(linux/cdrom.h COMPILE_LINUXCDPOLLING) # kioslave/media
check_include_files(Alib.h HAVE_ALIB_H) # kcontrol/infocenter
check_include_files("sys/time.h;time.h" TIME_WITH_SYS_TIME) # ksplashml (unneeded?)
check_include_files(X11/fonts/fontenc.h HAVE_X11_FONTS_FONTENC_H)

check_symbol_exists(gethostname     "unistd.h"                 HAVE_GETHOSTNAME) # many uses of gethostname
check_symbol_exists(getdomainname   "unistd.h"                 HAVE_GETDOMAINNAME) # kdm
check_symbol_exists(mallinfo        "malloc.h"                 KDE_MALLINFO_MALLOC) # konq
check_symbol_exists(mallinfo        "stdlib.h"                 KDE_MALLINFO_STDLIB) # konq
# TODO KDE_MALLINFO_FIELD_hblkhd   # konq
# TODO KDE_MALLINFO_FIELD_uordblks # konq
# TODO KDE_MALLINFO_FIELD_usmblks  # konq

# check_prototype_exists(gethostname "stdlib.h;unistd.h" HAVE_GETHOSTNAME_PROTO) # many uses of gethostname
# check_prototype_exists(getdomainname "stdlib.h;unistd.h" HAVE_GETDOMAINNAME_PROTO) # kdm
# check_prototype_exists(unsetenv stdlib.h HAVE_UNSETENV_PROTO) # drkonqi, kdesu, trash use unsetenv
# check_prototype_exists(usleep unistd.h HAVE_USLEEP_PROTO)   # various uses of usleep

# check_prototype_exists(DPMSInfo "X11/Xlib.h;X11/extensions/dpms.h"  HAVE_DPMSINFO_PROTO) #kdesktop

# check_prototype_exists(DPMSCapable "X11/Xlib.h;X11/extensions/dpms.h" HAVE_DPMSCAPABLE_PROTO)

check_symbol_exists(getnameinfo   "sys/socket.h;netdb.h" HAVE_GETNAMEINFO) # kcontrol/infocenter
check_symbol_exists(vsnprintf     "stdio.h"              HAVE_VSNPRINTF) # config.h (ksysguardd uses vsnprintf)

check_library_exists(Xss XScreenSaverAllocInfo "" HAVE_SCREENSAVER)
check_library_exists(GL  glXChooseVisual "" HAVE_GLXCHOOSEVISUAL)
check_library_exists(Xxf86misc XF86MiscSetGrabKeysState "" HAVE_XF86MISCSETGRABKEYSSTATE)
check_library_exists(ICE _IceTransNoListen "" HAVE__ICETRANSNOLISTEN) #ksmserver

check_function_exists(getpeereid  HAVE_GETPEEREID) # kdesu
check_function_exists(getpt       HAVE_GETPT)      # fish
check_function_exists(grantpt     HAVE_GRANTPT)    # fish
check_function_exists(getifaddrs  HAVE_GETIFADDRS) # kcontrol/infocenter, kdm
check_function_exists(getloadavg  HAVE_GETLOADAVG) # kdm
check_function_exists(nice        HAVE_NICE)       # kioslave/thumbnail
check_function_exists(setpriority HAVE_SETPRIORITY) # kdesktop, kcontrol/screensaver
check_function_exists(setproctitle HAVE_SETPROCTITLE) # kdm
check_function_exists(strnlen     HAVE_STRNLEN)    # kdm
check_function_exists(unsetenv    HAVE_UNSETENV)   # drkonqi, kdesu, trash use unsetenv
check_function_exists(usleep      HAVE_USLEEP)     # various uses of usleep


check_library_exists(util  openpty "" HAVE_OPENPTY) # fish
check_library_exists(util  isastream "" HAVE_ISASTREAM) #fish

FIND_FILE(UTMP_FILE utmp PATHS /var/run/ /var/adm/ /etc/ )


# TODO: COMPILE_HALBACKEND (kioslave/media)
# TODO: HAVE_COREAUDIO (kcontrol/infocenter, for OSX)
# TODO: HAVE_KSTAT (ksysguardd/{Solaris,Tru64})
# TODO: HAVE_LIBDEVINFO_H (kcontrol/infocenter, for Solaris)
# TODO: HAVE_SIGACTION HAVE_SIGSET (kdeprint)
# TODO: HAVE_XFT (kfontinst)
# TODO: PATH_JAVA (konqueror and kcontrol/konqhtml)

check_type_size("long" SIZEOF_LONG) # infocenter

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" HAVE_STRUCT_UCRED) # kdesu

# check_struct_member("struct sockaddr" "sa_len" "sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN) # kcontrol/infocenter
# check_struct_member("struct sockaddr_in" "sin_len" "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN_SIN_LEN) # kdm
# check_struct_member("struct sockaddr_in6" "sin6_len" "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN) # kdm

if (UNIX)
  find_path(X11_XKB_INCLUDE_PATH X11/XKBlib.h "${X11_INC_SEARCH_PATH}")
  if (X11_XKB_INCLUDE_PATH)
    MACRO_PUSH_REQUIRED_VARS()
    set(CMAKE_REQUIRED_LIBRARIES "${X11_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES}")
    check_library_exists(X11 XkbLockModifiers "" HAVE_XKB) # konsole, kxkb, kdm
    check_library_exists(X11 XkbSetPerClientControls "" HAVE_XKBSETPERCLIENTCONTROLS) # kdm
    MACRO_POP_REQUIRED_VARS()
  endif (X11_XKB_INCLUDE_PATH)
endif (UNIX)

check_cxx_source_compiles("
  class A { public: virtual A* me(); };
  class X { public: int x; virtual void ff() {}; };
  class B : public X, public A { public: virtual B* me(); };
  int foo( A* a )
    { 
    A* aa = a->me();
    return a == aa;
    } 
  int main()
    { 
    B* b = new B;
    return foo( b ) == 0;
    } 
  A* A::me() { return this; }
  B* B::me() { return this; }
" HAVE_COVARIANT_RETURN ) # khotkeys
