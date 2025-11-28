# Talons

Talons is a GTK3 email client
supporting managing mailboxes via IMAP and on a filesystem.

Talons is a fork of [Claws Mail],
which itself is a fork of [Sylpheed].
It started out life as an experiment of using [Zig] to maintain an old C codebase.

OpenBSD is supported; other OSs untested.

## Getting started

Talons has direct dependencies on the following libraries:

- cairo
- gdk-3
- gdk_pixbuf-2.0
- gio-2.0
- glib-2.0
- gmp
- gnutls
- gtk-3
- hogweed
- libidn2
- nettle
- p11-kit
- pango-1.0
- tasn1
- unistring

Install these using pkg_add(1).

TODO symlink workarounds for https://github.com/ziglang/zig/pull/18475

Install zig (`pkg_add zig`) then build the project:

	cd src
	zig build

To run the app:

	zig build run

## Goals

Many features of Claws Mail have been removed to make it easier to maintain by a single person.

To be honest I can't even remember how many or which features have been removed.
Off the top of my head:

- no autotools
- no Windows support
- no plugins
- no localisation (sorry)
- no actions
- no client-side mail filtering

[Zig]: https://ziglang.org
[Claws Mail]: https://claws-mail.org
[Sylpheed]: https://sylpheed.sraoss.jp/en/
