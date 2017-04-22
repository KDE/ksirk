# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   A turn by turn strategy game
Name:      ksirk
Version:   1.8beta1
Release:   0
Copyright: GPL
Vendor:     <The Ksirk Teal>
Url:       http://home.gna.org/ksirk

Packager:   <Kleag>
Group:     application/games
Source:    ksirk-1.8beta1.tar.bz2
BuildRoot: /tmp

%description


%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                 \
                $LOCALFLAGS
%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.ksirk
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.ksirk
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.ksirk

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/ksirk
rm -rf ../file.list.ksirk


%files -f ../file.list.ksirk
