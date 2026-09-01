Name:           grm
Version:        0.9.0
Release:        %autorelease
Summary:        Fast native Telegram CLI client powered by TDLib and C++23

License:        GPL-3.0-or-later AND GFDL-1.3-or-later
URL:            https://gitlab.com/renich/%{name}
Source0:        %{url}/-/archive/v%{version}/%{name}-v%{version}.tar.gz

BuildRequires:  clang >= 18
BuildRequires:  cmake >= 3.25
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(json-c)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(zlib)

%description
High-performance, type-safe Telegram command-line client written in C++23
and powered by TDLib. Supports full CRUD operations across chats, messages,
forum topics, media uploads/downloads, folders, stories, and emoji statuses.

%prep
%autosetup -p1

%build
%cmake -G Ninja -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
%ctest

%files
%license LICENSE LICENSE-DOCS
%doc README.rst CHANGELOG.rst
%{_bindir}/%{name}
%{_mandir}/man1/%{name}.1*

%changelog
%autochangelog
