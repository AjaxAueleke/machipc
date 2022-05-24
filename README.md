# Mach IPC in Linux

This project aims to implement the Mach inter-process communication IPC 
mechanism within a traditional Linux environment.

## Overview

The Mach IPC is a message-passing communication model. All inter-task
communication is carried out by messages. Messages are sent to, and received
from, mailboxes, which are called ports in Mach. A message is sent to one port,
and a response is sent to a separate reply port. Message passing may occur
between any two ports on the same host or on separate hosts on a distributed
system. Associated with each port is a collection of port rights that identify the
capabilities necessary for a task to interact with the port. Each task also has
access to a _central_ or _bootstrap_ server , which is responsible for
storing and distributing information of running tasks.
As messages are sent to the port, the messages are copied into a
queue, which can be read by the receiver thread.

## Technology Used

The UNIX networking and sockets APIs are used to implement the
communication channels between processes.

## How to Run Locally

Download the project on your machine.

```bash
    git clone git@github.com:AjaxAueleke/machipc.git
    cd gitmachipc/
```

Make a source folder for all object files.

```bash
    mkdir src
```

Start the central server.

```bash
    ./run.sh mach/mach_central_server
```

Start the sending process.

```bash
    ./run.sh send
```

Start the receiving process.

```bash
    ./run.sh recv
```

To close the central server:

```bash
    ./run.sh process_endserver
```

For more information, [click here](https://github.com/AjaxAueleke/machipc/blob/main/OS%20Project%20Report.pdf)

## Authors

- email : ahmed.jamil7410@gmail.com or k200388@nu.edu.pk
- github : [@ajaxaueleke](https://www.github.com/ajaxaueleke)
- email : ayyansaad46@gmail.com or k200161@nu.edu.pk
- github : [@saad0510](https://www.github.com/saad0510)
- email : moaaz88sajjad@gmail.com or k200154@nu.edu.pk
- github : [@nlzza](https://www.github.com/nlzza)

## Last updated

24 May, 2022

