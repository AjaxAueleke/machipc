# Mach IPC on Linux

> A from-scratch implementation of **Mach-style, message-passing inter-process
> communication (IPC)** built on top of plain UNIX/BSD sockets — ports,
> mailboxes, and a bootstrap (central) server, recreated in C++ on Linux.

![Language](https://img.shields.io/badge/language-C%2B%2B-00599C)
![Platform](https://img.shields.io/badge/platform-Linux-333333)
![License](https://img.shields.io/badge/license-MIT-green)

---

## What is Mach IPC?

In a microkernel like [Mach](https://en.wikipedia.org/wiki/Mach_(kernel)), tasks
do not share memory or call each other directly — they **exchange messages**. A
message is sent to a **port**, which behaves like a mailbox: the kernel copies
the message into a queue that the owning task drains at its own pace. To find one
another, tasks consult a **bootstrap (central) server** that knows which ports
are alive and who owns them.

This project models those concepts in user space on Linux:

- **Ports / mailboxes** are TCP sockets plus a per-process message queue.
- The **bootstrap server** is a small registry process that maps a process *key*
  to its receive port.
- **Messages** are sent to a peer's receive port and queued for asynchronous
  delivery.

It is an educational re-creation of Mach's IPC *model* — not a binding to the
real Mach kernel API.

---

## Architecture

The system has three moving parts: a **central server** (the registry), a
**`mach_process`** object that each participant instantiates, and the **messages**
that flow between them.

### Components

| Component | File | Role |
|-----------|------|------|
| Central / bootstrap server | `mach/mach_central_server.cpp` | Single-threaded registry listening on **TCP :3333**. Tracks live processes and hands out peer addresses. Uses `select()` to multiplex all clients. |
| Process / endpoint | `mach/mach_process.hpp` | The per-process IPC object. Owns a **receive port**, a **service port** (to the server), a background **receiving thread**, and a **mailbox** (message queue). |
| Port | `mach/mach_port.hpp` | `{ socket, port, ip }` triple plus a binder that grabs an ephemeral port and reads it back via `getsockname()`. |
| Service record | `mach/mach_service.hpp` | The control protocol with the server: `{ type, key, port }` where `type ∈ {init, peers, exit, closeserver}`. |
| Message | `mach/mach_msg.hpp` | `{ id, data, size }` — the unit copied into a peer's mailbox. |
| Socket library | `include/` + `lib/` | Error-checked socket wrappers (`Socket/Bind/Listen/Accept/select_readable`), reliable `readn`/`writen` byte I/O, and IPv4 address conversions. |

### How a `mach_process` works

When you construct `mach_process p(key)`:

1. It binds two ephemeral ports — a **receive port** (where peers send it
   messages) and a **service port** (its connection to the central server).
2. It **registers** with the server (`init`), announcing its `key` and receive
   port.
3. It spawns a **receiving thread** that listens on the receive port, accepts
   incoming peer connections, and pushes every arriving `mach_msg` into a
   **mutex-guarded queue** — the process's mailbox.

From there:

- **`connect(remote_key)`** asks the server for the live peer list (`peers`),
  finds the target by key, and opens a **direct peer-to-peer** TCP connection to
  that peer's receive port.
- **`send(remote_key, msg)`** writes a `mach_msg` over that direct connection.
- **`receive(&msg)`** pops the next message from the local mailbox (non-blocking;
  returns `false` when empty).
- The destructor deregisters from the server (`exit`), closes sockets, and
  cancels/joins the receiving thread.

Note that **the server is only a directory** — once a sender has looked up a
receiver, message traffic flows **directly between the two processes**, not
through the server.

### Message flow

```
                 ┌────────────────────────────────────┐
                 │      Central / Bootstrap Server     │
                 │            TCP :3333                 │
                 │   registry:  key ──► receive port    │
                 └──────▲───────────────────────▲──────┘
       (1) init /       │                       │       (1) init /
       peers / exit     │                       │       peers / exit
                        │                       │
            ┌───────────┴────────┐   ┌──────────┴───────────┐
            │   Process A         │   │   Process B          │
            │   key = 1 (sender)  │   │   key = 2 (receiver) │
            └───────────┬────────┘   └──────────▲───────────┘
                        │                       │
        (2) connect(2): look up B's receive port via the server
                        │                       │
                        └──── (3) send mach_msg ─┘   direct peer-to-peer TCP
                                                 │
                                    receiving thread accepts
                                    and enqueues into B's mailbox
                                                 │
                                    (4) B's receive() pops the message
```

---

## Technology

- **Language:** C++ (lambdas, `std::queue`/`std::vector`, POSIX threads). Builds
  with `g++`/`gcc`.
- **Transport:** POSIX/BSD sockets — TCP over IPv4 on localhost, with `select()`
  for I/O multiplexing.
- **Concurrency:** one `pthread` per process for receiving; the mailbox is a
  `std::queue` guarded by a `pthread_mutex`.
- **Out-of-band payload (optional demo):** System V **shared memory**
  (`shmget`/`shmat`) to move large payloads between processes while the socket
  carries only the message header.
- **Dependencies:** none beyond `libpthread` and the C/C++ standard library.

---

## Build

Verified on **Ubuntu 24.04, g++ 13.3.0**. You only need a C++ compiler and `make`.

### Using the Makefile (recommended)

`make` compiles the central server and every demo into `build/`:

```bash
git clone git@github.com:AjaxAueleke/machipc.git
cd machipc
make            # build all targets into build/
make clean      # remove build/
```

| Binary | Source | Role |
|--------|--------|------|
| `build/cserver`   | `mach/mach_central_server.cpp` | central / bootstrap server (TCP :3333) |
| `build/send`      | `send.cpp`                     | minimal sender demo (key 1 → key 2) |
| `build/recv`      | `recv.cpp`                     | minimal receiver demo (key 2) |
| `build/endserver` | `process_endserver.cpp`        | sends the shutdown message to the server |
| `build/shm_send`  | `process1_send.cpp`            | sender demo using System V shared memory |
| `build/shm_recv`  | `process2_recv.cpp`            | receiver demo using System V shared memory |

Everything compiles cleanly under `-Wall` with no warnings.

### Using the `run.sh` quick-run helper

`run.sh` is a *build-and-run* shortcut for a single target: it compiles the
shared socket library into `src/` (creating the directory if needed), links the
target with `-lpthread`, **runs** the binary in the foreground, and removes it on
exit.

```bash
./run.sh <target>        # e.g. ./run.sh send  or  ./run.sh recv
```

### Manual build (without make)

```bash
mkdir -p build
gcc -c lib/*.cpp -Iinclude && mv *.o build/
g++ mach/mach_central_server.cpp build/*.o -lpthread -o build/cserver
g++ send.cpp              build/*.o -lpthread -o build/send
g++ recv.cpp              build/*.o -lpthread -o build/recv
g++ process_endserver.cpp build/*.o -lpthread -o build/endserver
```

---

## Demo

Build everything first with `make`, then open **three terminals** in the project
root.

**Terminal 1 — start the central server:**

```bash
./build/cserver
# listening at 3333 on localhost
```

**Terminal 2 — start the receiver** (registers as key `2`, then blocks waiting
for a message):

```bash
./build/recv
```

**Terminal 3 — start the sender** (registers as key `1`, looks up key `2`,
connects, and sends one message):

```bash
./build/send
# connecting peer from <ephemeral-port>
```

Back in **Terminal 2**, the receiver prints the message it pulled from its
mailbox and exits:

```
connection from <ephemeral-port>
rec 70
rec 4
```

`70` is the message `id` set in `send.cpp`; `4` is the `size` field
(`sizeof(int)`). (You may also see an interleaved `connection closed by remote
peer …` line — that is the sender hanging up after delivery.)

**To shut the central server down:**

```bash
./build/endserver
```

### Shared-memory variant

`process1_send.cpp` / `process2_recv.cpp` demonstrate moving a real payload
out-of-band: the sender writes a string into a System V shared-memory segment and
transmits only the message header over the socket; the receiver reattaches the
segment and reads the payload. This variant uses `ftok("shm", id)`, so it
requires a file named `shm` to exist in the working directory (`touch shm`).

---

## Project structure

```
.
├── include/                     # public headers for the socket helper library
│   ├── wrapper.hpp              #   error-checked BSD socket wrappers + select
│   ├── msg.hpp                 #   reliable readn/writen byte I/O
│   └── conv.hpp                #   IPv4 presentation <-> numeric helpers
├── lib/                         # implementations of the headers above
│   ├── wrapper.cpp
│   ├── msg.cpp
│   └── conv.cpp
├── mach/                        # the Mach IPC core
│   ├── mach_central_server.cpp #   bootstrap/registry server (TCP :3333)
│   ├── mach_process.hpp        #   per-process IPC object: ports, mailbox, recv thread
│   ├── mach_port.hpp           #   socket+port+addr struct and binder
│   ├── mach_service.hpp        #   registry record / control protocol
│   └── mach_msg.hpp            #   message struct { id, data, size }
├── send.cpp                     # demo: minimal sender (key 1 -> key 2)
├── recv.cpp                     # demo: minimal receiver (key 2)
├── process1_send.cpp            # demo: sender using System V shared memory
├── process2_recv.cpp            # demo: receiver using System V shared memory
├── process_endserver.cpp        # sends the shutdown control message to the server
├── Makefile                     # builds every target into build/
├── run.sh                       # single-target build-and-run helper
└── OS Project Report.pdf        # original course project report
```

For a deeper write-up, see the
[project report](https://github.com/AjaxAueleke/machipc/blob/main/OS%20Project%20Report.pdf).

---

## Limitations

This is a learning project that recreates Mach's IPC *concepts*; it is not
production middleware.

- **Single machine only.** All communication is TCP over IPv4 on `localhost`,
  and the server port `3333` is hardcoded.
- **Header-only messages over the socket.** `mach_msg` carries a raw `char* data`
  pointer; a pointer is only meaningful inside the sender's address space, so the
  plain `send`/`recv` flow transmits only the header (`id`, `size`). The
  shared-memory demo exists precisely to move the actual payload bytes
  out-of-band.
- **Polled receive.** `receive()` is non-blocking and the demos busy-wait on it;
  there is no condition variable or blocking receive.
- **Fail-fast error handling.** The socket wrappers call `exit(1)` on error —
  fine for a demo, not for a reusable library.
- **No framing, versioning, authentication, encryption, or backpressure.**

---

## Authors

- Muhammad Ahmed — [@AjaxAueleke](https://www.github.com/AjaxAueleke) ·
  ahmed.jamil7410@gmail.com / k200388@nu.edu.pk
- Ayyan Saad — [@saad0510](https://www.github.com/saad0510) ·
  ayyansaad46@gmail.com / k200161@nu.edu.pk
- Moaaz Sajjad — [@nlzza](https://www.github.com/nlzza) ·
  moaaz88sajjad@gmail.com / k200154@nu.edu.pk

## License

Released under the [MIT License](LICENSE).
