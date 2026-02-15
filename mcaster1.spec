Name:		mcaster1
Version:	2.1.0-kh
Release:	1
Summary:	Xiph Streaming media server that supports multiple audio formats.

Group:		Applications/Multimedia
License:	GPL
URL:		http://www.mcaster1.org/
Vendor:		Xiph.org Foundation <team@mcaster1.org>
Source:     http://www.mcaster1.org/files/%{name}-%{version}.tar.gz
Prefix:		%{_prefix}
BuildRoot:	%{_tmppath}/%{name}-root

Requires:       libvorbis >= 1.0
BuildRequires:	libvorbis-devel >= 1.0
Requires:       libogg >= 1.0
BuildRequires:	libogg-devel >= 1.0
Requires:       curl >= 7.10.0
BuildRequires:	curl-devel >= 7.10.0
Requires:       libxml2
BuildRequires:	libxml2-devel
Requires:       libxslt
BuildRequires:	libxslt-devel

%description
Mcaster1 is a streaming media server which currently supports Ogg Vorbis 
and MP3 audio streams. It can be used to create an Internet radio 
station or a privately running jukebox and many things in between. 
It is very versatile in that new formats can be added relatively 
easily and supports open standards for commuincation and interaction.

%prep
%setup -q -n %{name}-%{version}

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix} --mandir=%{_mandir} --sysconfdir=/etc
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/%{name}

%clean 
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README AUTHORS COPYING NEWS TODO
%doc doc/*.html
%doc doc/*.jpg
%doc doc/*.css
%config(noreplace) /etc/%{name}.xml
%{_bindir}/mcaster1
%{_prefix}/share/mcaster1/*

%changelog
