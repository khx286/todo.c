# todo.c
program for managing tasks

# build
`make` - build the program

`make install` - install to `/usr/local/bin`

`make uninstall` - remove from `/usr/local/bin`

`make clean` - remove the executable from the source directory

# how to use
| key      | description                                                                 |
| -------- | --------------------------------------------------------------------------- |
| `TAB`    | switch the context                                                          |
| `k`, `j` | up, down                                                                    |
| `K`, `J` | move the current task up, down                                              |
| `g`, `G` | go to the first & last task of the current context                          |
| `o`, `O` | insert a new task below, above the current task (only for the TODO context) |
| `r`      | rename the current task (only for the TODO context)                         |
| `d`      | delete the current task                                                     |
| `x`      | mark the current task as done or not done (related to the current context)  |
| `w`      | write to file                                                               |
| `q`, `Q` | exit, forced exit                                                           |

use `./todo --help` for more information
