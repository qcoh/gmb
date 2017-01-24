# gmb
---
![boot sequence](docs/logo.png)

---
A heavily WIP GameBoy emulator written in C++14.

### Status

 - [x] Show boot sequence
 - [ ] Debugger
 - [x] Hotswapping the CPU
 - [ ] Different mappers
 - [ ] Audio
 - [ ] Video
 - [ ] Interrupts
 - [ ] ...

### Dependencies

  * SDL2
  * libdl (\*nix only for now)
  * [tup](http://gittup.org) for building
  * A BIOS and a ROM if you actually want to use it

### Building

	tup

### Debugger

The currently implemented commands (subject to change):

 - `next`: Execute the next instruction.
 - `print 0xnnnn`: Print memory at location `0xnnnn`.
 - `break 0xnnnn`: Set breakpoint at `0xnnnn`.
 - `breakpoints`: Show all breakpoints.
 - `clear`: Clear all breakpoints.
 - `tile 0xnnn`: Dump tile `0xnnn` to terminal.

### License

```
ISC License

Copyright (c) 2017, Jeremias Yehdegho

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
```
