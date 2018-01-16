Usage: escapefn [options] [file ...]
  Read NUL-separated raw filename list from files (or stdin), and
  convert to LF-separated escaped filename list.

Options:
  -m, --minimum     escape LF and backslash only (default)
  -c, --cstyle      escape all C0 control chars and DEL in C style
  -o, --octal       escape all C0 control chars and DEL in octal
  -C, --cstylefull  escape all C0, DEL and Non-ASCII chars in C style
  -O, --octalfull   escape all C0, DEL and Non-ASCII chars in octal
  -u, --unescape    unescape
  -h, --help        show this help


Copyright (c) 2016-2018 Yohei Endo <yoheie@gmail.com>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
   not claim that you wrote the original software. If you use this
   software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must
   not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
