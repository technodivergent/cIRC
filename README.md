# cIRC
A simplistic implementation of IRC in C.

## Build:
Run `make` to build

## Run:
Run `./server <port> [-v]`

| Option | Description |
|  ---   |     ---     |
| `<port>` | Typical IRC port is 6667 |
| `-v`     | Verbose mode enables socket connection debugging info |

## Usage:
To use cIRC, open another terminal and connect via `telnet localhost <port>`

## Commands:
The following commands are available:

| Command | Description |
|   ---   |     ---     |
| `HELO`  | A test command to verify connection to the server |
| `LIST`  | A list of currently connected users |
| `NICK`  | Display your current nickname |
| `NICK <nick>` | Change your nickname to *<nick>* |
| `MESG <msg>` | Send *<msg>* to all connected hosts | 
| `QUIT` | Disconnect from cIRC |
