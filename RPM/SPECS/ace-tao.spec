%define _enable_valgrind %{?_with_valgrind:1}%{?!_with_valgrind:0}

Summary:        The ADAPTIVE Communication Environment (ACE) and The ACE ORB (TAO)
Name:           ace-tao%{?_with_valgrind:-valgrind}
Conflicts:      ace-tao%{?!_with_valgrind:-valgrind}
Version:        %{_version}
Release:        %{_release}%{?dist}
Group:          Development/Libraries/C and C++
URL:            http://www.cs.wustl.edu/~schmidt/ACE.html
License:        DOC License
Source0:        ace-tao-%{_version}-%{_release}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-buildroot
BuildRequires:  redhat-rpm-config elfutils 
BuildRequires:  gcc-c++ libstdc++-devel openssl-devel zlib-devel bzip2-devel lzo-devel
BuildRequires:  perl-interpreter perl(Data::Dumper)
Provides:       ace-tao = %{version}

%description
The ADAPTIVE Communication Environment (ACE) is a freely available,
open-source object-oriented (OO) framework that implements many core
patterns for concurrent communication software. ACE provides a rich
set of reusable C++ wrapper facades and framework components that
perform common communication software tasks across a range of OS
platforms. The communication software tasks provided by ACE include
event demultiplexing and event handler dispatching, signal handling,
service initialization, interprocess communication, shared memory
management, message routing, dynamic (re)configuration of distributed
services, concurrent execution and synchronization.

TAO is a real-time implementation of CORBA built using the framework
components and patterns provided by ACE. TAO contains the network
interface, OS, communication protocol, and CORBA middleware components
and features. TAO is based on the standard OMG CORBA reference model,
with the enhancements designed to overcome the shortcomings of
conventional ORBs for high-performance and real-time applications.

%package -n     ace-tao%{?_with_valgrind:-valgrind}-devel
Conflicts:      ace-tao%{?!_with_valgrind:-valgrind}-devel
Summary:        Header files and development components for ACE
Version:        %{_version}
Release:        %{_release}%{?dist}
Group:          Development/Libraries/C and C++
Requires:       ace-tao%{?_with_valgrind:-valgrind} = %{version}
Requires:       openssl-devel
Provides:       ace-tao-devel = %{version}

%description -n ace-tao%{?_with_valgrind:-valgrind}-devel
This package contains the components needed for developing programs
using ACE and TAO.

%prep
%setup -q -n ace-tao-%{_version}-%{_release}/

%build
export ACE_TAO_MPC_ROOT=$(pwd)
export ACE_ROOT=${ACE_TAO_MPC_ROOT}/ACE_TAO/ACE
export TAO_ROOT=${ACE_TAO_MPC_ROOT}/ACE_TAO/TAO
export MPC_ROOT=${ACE_TAO_MPC_ROOT}/MPC
cd "${TAO_ROOT}"
"${ACE_ROOT}/bin/mwc.pl" rTAO.mwc -type gnuace
make valgrind=%{_enable_valgrind} %{?_smp_mflags}

%install
export ACE_TAO_MPC_ROOT=$(pwd)
export ACE_ROOT=${ACE_TAO_MPC_ROOT}/ACE_TAO/ACE
export TAO_ROOT=${ACE_TAO_MPC_ROOT}/ACE_TAO/TAO
export MPC_ROOT=${ACE_TAO_MPC_ROOT}/MPC
cd "${TAO_ROOT}"
make valgrind=%{_enable_valgrind} %{?_smp_mflags} \
  install DESTDIR=%{buildroot} INSTALL_PREFIX=/usr INSTALL_LIB=lib64

%files
%defattr(-,root,root,-)
%{_libdir}/libACE*.so.7.0.9
%{_libdir}/libTAO*.so.3.0.9
%{_libdir}/libKokyu.so.7.0.9
#%doc AUTHORS COPYING README THANKS VERSION
/usr/share/ace/*
/usr/share/tao/*

%files -n ace-tao%{?_with_valgrind:-valgrind}-devel
%{_bindir}/*
%{_includedir}/*
%{_libdir}/pkgconfig
%{_libdir}/lib*.so
%{_datadir}/*

%post
/sbin/ldconfig

%post -n ace-tao%{?_with_valgrind:-valgrind}-devel
/sbin/ldconfig

%postun
/sbin/ldconfig

%postun -n ace-tao%{?_with_valgrind:-valgrind}-devel
/sbin/ldconfig

%changelog

