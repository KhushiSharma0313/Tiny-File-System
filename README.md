# Tiny-File-System
In Tiny File System (TFS), data files can be moved from the real file system to the virtual disk. Each command make its changes to the memory resident Drive data structure, and then write the Drive to the TFS file. In this way the TFS file is always up to date, with no pending changes. 
