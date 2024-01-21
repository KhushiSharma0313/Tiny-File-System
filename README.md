# Tiny-File-System
In Tiny File System (TFS), data files can be moved from the real file system to the virtual disk. Each command make its changes to the memory resident Drive data structure, and then write the Drive to the TFS file. In this way the TFS file is always up to date, with no pending changes. 
Commands are -

import LP tp
Copy a file stored in the regular file system to the current TFS-Drive.


export tp LP
copy a file stored in the current virtual disk to the regular file system.

ls tp
list the contents of the given directory of the TFS-Drive

display
shown the raw contents of the current TFS-Drive, as a table of 16 rows and columns where each entry is a 2 character hexadecimal number.

open LP
open an existing TFS-Drive file, and close the TFS-file currently in use, if any. Fails if the file does not exist.

create LP
create a new, empty TFS-Drive file. (You must initialize the root directory) Fails if the file already exists.

mkdir tp
Create a new directory in the TFS-Drive.

rm tp
Remove a file or directory from the TFS-Drive. Fails if the file or directory does not exist, or if the directory is not empty.

