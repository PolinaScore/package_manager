# package_manager

# Description
The package manager is a program package management system that supports three types of packages: main applications (MAIN), libraries (LIBRARY), and meta-packages (META). The system automatically manages dependencies between packages.

# General commands available
| Command | Description | Example |
|---------|-------------|---------|
| **`help`** | Display this help message with all available commands | `help` |
| **`exit`** | Exit the package manager application | `exit` |
| **`list [type]`** | List packages by type<br>• `all` - all packages<br>• `main` - main applications<br>• `lib` - libraries<br>• `meta` - meta-packages | `list main`<br>`list lib` |
| **`info <package>`** | Show detailed information about a specific package | `info myapp` |
| **`stats`** | Display system statistics | `stats` |
| **`check`** | Verify system integrity and dependencies | `check` |

|---------|-------------|---------|
| **`init <pkg1> [pkg2...]`** | Initialize the system with one or more packages | `init app1`<br>`init app1 app2 app3` |
| **`install <package>`** | Install a package along with all its dependencies | `install myapp` |
| **`remove <package>`** | Remove a package (automatically removes unused libraries) | `remove oldapp` |
| **`update-all`** | Update all installed packages to their latest versions | `update-all` |
| **`cleanup`** | Remove all unused libraries from the system | `cleanup` |

|---------|-------------|---------|
| **`add <name> <type> <version> [publisher]`** | Add a new package to the repository<br>**Types:** `MAIN`, `LIBRARY`, `META`<br> **Note:** For META packages, use version `latest` | `add mylib LIBRARY 1.0`<br>`add myapp MAIN 2.1.0`<br>`add mymeta META latest` |
| **`delete <package>`** | Remove a package from the repository | `delete oldlib` |
| **`update <package> <version>`** | Update a package to a new version | `update myapp 2.0` |
| **`dep <from> <to>`** | Add a dependency relationship between packages | `dep app1 lib1` |
| **`link <meta> <target>`** | Link a META package to its target package | `link myapp-meta myapp` |

|---------|-------------|---------|
| **`split <library> <part1> [part2...]`** | Split a library into multiple parts | `split oldlib newlib1 newlib2` |
| **`merge <pkg1> <pkg2> <new_name> <type>`** | Merge two packages into a new one | `merge lib1 lib2 newlib LIBRARY` |
| **`performance`** | Compare single-threaded vs multi-threaded performance | `performance` |



