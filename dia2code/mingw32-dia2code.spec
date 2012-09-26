%define __strip %{_mingw32_strip}
%define __objdump %{_mingw32_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw32_findrequires}
%define __find_provides %{_mingw32_findprovides}
%define __os_install_post %{_mingw32_debug_install_post} \
                          %{_mingw32_install_post}

Name:           mingw32-dia2code
Summary:        Convert Dia files into source code
Version:        0.8.5
Release:        0
License:        GPLv2+
Group:          Graphics
Url:            http://dia2code.sourceforge.net
Source:         dia2code-0.8.5.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-buildroot
BuildRequires:  mingw32-cross-gcc, mingw32-libxml2-devel, mingw32-cross-pkg-config
BuildRequires:  libtool automake autoconf xz

BuildArch:      noarch
#!BuildIgnore: post-build-checks  

%description
dia2code is a tool to convert .dia files into soource code.

%{_mingw32_debug_package}

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n dia2code-%{version}


%build
libtoolize --copy --force
autoreconf -f -i
echo "lt_cv_deplibs_check_method='pass_all'" >>%{_mingw32_cache}
PATH=%{_mingw32_bindir}:$PATH \
%{_mingw32_configure} \
	--enable-shared --disable-static \
	--without-docs --disable-werror

%{_mingw32_make} %{?_smp_mflags} || %{_mingw32_make}

%install
%{_mingw32_make} DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_mingw32_bindir}/dia2code*.exe

%changelog
