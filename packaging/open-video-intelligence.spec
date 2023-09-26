Name:      open-video-intelligence
Summary:   library for edit video contents
Version:   0.1
Release:   0
Group:     Multimedia/Libraries
License:   Apache-2.0
Source0:   %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: pkgconfig(libswscale)
BuildRequires: pkgconfig(libavcodec)
BuildRequires: pkgconfig(libavutil)
BuildRequires: pkgconfig(libavformat)
BuildRequires: pkgconfig(libswresample)
BuildRequires: pkgconfig(Imath)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(python3)
BuildRequires: OpenTimelineIO-devel
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
library for edit video contents

%package devel
Summary:  A open-video-intelligence library (Development)
Group:    Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
library for edit video contents

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} -DCMAKE_BUILD_TYPE=release -DENABLE_PYTHON=ON -DBUILD_OVI_TOOLS=OFF

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%{_libdir}/libovi.so
%license LICENSE.APLv2
/etc/ovi/ovi.ini
%{_libdir}/ovi/plugins/*

%files devel
%{_includedir}/ovi/*.h
%{_libdir}/pkgconfig/ovi.pc
