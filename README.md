# FSV FileSystem

![GitHub top language](https://img.shields.io/github/languages/top/stopasola/fsv-filesystem)

## Introduction

   A file system is a set of logical structures and routines that allow the operating system to control access to the hard disk. Different operating systems use different file systems. As the disk capacity grows and the volume of files and accesses increases, this task becomes more and more complicated, requiring the use of increasingly complex and robust file systems. There are several different file systems, and the one addressed in this work will be the DVV System.  
   The FSV system is based directly on FAT but with some changes, such as having a third FAT table for file protection, a variation to FAT28 and having a 64-byte root directory.


## Methodology

![image](https://user-images.githubusercontent.com/17886190/160159835-b9d4126a-c721-4d4b-9dd0-16146c3e3c6b.png)

Allocation method: linked list

Free block management: Linked list via file allocation table

#### Boot Record: 512 bytes - 1 sector

The Boot Record is the boot sector that contains information that the file system uses to access other areas of the system. The next table describes the Boot Record fields.

|Offaset (decimal)|Size (bytes)|Meaning|Value|
| :---: | :---: | :---: |:---: |
|0|2|Número de bytes por setor|512 bytes|
|2|1|Sectors per cluster|8 sectors|
|3|2|Number of reserved sectos (beguin of 1º FAT )|1 sectors|
|5|1|Number of tables |3 tables|
|6|2|Number of setores por tabela fat |--|
|8|2|Number of entries in root directory (amount of files that can be saved in roo directory )|1024|

#### File management table (FAT28) - 155 sectors:

To protect the stored data, the FAT table is used, in which 3 tables will be used in this project as protection to have full guarantee of the security of the files.
We have 4 bytes per entry (32 bits ) and the first Cluster, Cluster 0, is not used, so it is marked 0xFFFFFF.


#### Root Directory 64 bytes:

Here you have the area where the system files are stored where each entry corresponds to a file in the root directory. The next table describes the Root Directory fields.

|Offset (decimal)|Size (bytes)|Meaning|
| :---: | :---: | :---: |
|0|47|44.3 filename|
|49|1|file_atribute directory ->  0x10 file -> 0x5|
|50|2|creation time|
|52|2|creation date|
|54|2|hour of last change|
|56|2|hour of last change|
|58|4|first cluster|
|60|4|size of file in bytes|

   To define subdirectories we use the same structure, in this case it is necessary to read the file_attribute field, which serves to indicate if it is a directory or file, if the previous and current entry have 0x10 in the file attribute field, it means that we are working with a subdirectory, this will be limited to a single cluster.

#### Conventions

> Indicate an excluded entry: 0xE5

> Indicate a free cluster: 0x00000000

> Indicate end of file (last cluster): 0xFFFFFFFF

- The root directory can be found using the following formula:

> ```boot_record->reserved_sectors + (boot_record->sectors_by_table * boot_record-number_of_tables)```

- The root directory size can be calculated with the following formula:

> ```(boot_record->dir_root_entries * 32) / boot_record->bytes_per_sector```

- Data Sector

The data area can be found using the following formula

> ```boot_record->reserved_sectors + (boot_record->sectors_per_table * boot_record->number_of_tables) + (boot_record->entries_root_dir * 32) / boot_record->bytes_per_sector)```

## File System User Manual

   The DVV file system works by receiving commands via terminal, following a defined logical pattern. Before executing the commands, it is necessary to compile the project through the command.

> g++ main.cpp -o main

Example of the structure to follow:

> ./main disk.img operation params

- ./main: name of the executable file.
- disk.img: name of the image that simulates the pendrive.
- operation: name of the operation to be performed on disk
- params: parameters necessary for the desired operation to be performed, may not exist or have - more than one depending on the command.

#### Behaved operations


#### Disk Formatting

> ./main floppy.img format disk_size

This operation formats the passed image file, information such as boot record, fat table, and root directory entries and data area are defined. DVV writes 0x0000 to all bytes in the data area and root directory, except for reserved fields.

Parameters: to format the file, it is necessary to define its size in sectors, remembering that a sector is defined as 512 bytes.

#### Copying a file to the file system

> ./main floppy.img cpin dvvFilePath file.ext

The cpin operation copies a file from the hard disk into the dvv file system, that is, the passed file has its data stored in the defined directory entry, it will define the clusters used in the fat table according to the size of the file, and for end writes the content to the defined clusters.

Parameters: to copy the file, just define the path where the file will be stored on the file system, and the path from where the file comes from the hard disk respectively.

The file system has implemented a functionality that does not allow inserting files with the same name in the same folder, by design decision it cannot have files with the same name and different extensions.

#### Copying a file from the file system to the hard disk

> ./main floppy.img cpout dvvFilePath/file.ext DiskFilePath/newfile.ext

The cout operation copies a file from the dvv file system to the hard disk, that is, the passed file has its data deleted in the defined directory entry and the clusters defined in the fat table are deleted.

Parameters: to copy the file, just define the path from where the file comes from the DVV, and the path where the file will be stored on the hard disk respectively.


#### List of files stored in the file system

> ./main floppy.img ls -la filepath/

This operation simply lists all files within all file system directories. No need to send any parameters.

#### Directory creation

> ./main floppy.img mkdir folderPath/folderName

In the mkdir operation, a new directory is created within the file system, that is, the path where the directory will be stored is searched, its data will be stored in the entry of its parent directory, a cluster will be defined in the fat, so that the new directory have its own entry to save new files or other directories.

Parameters: just enter the path and name of the directory to be entered.

#### File removal

 > ./main floppy.img rm folderPath/folderName

In the rm operation the specified file is removed from the file system, this process erases the addresses for the clusters within the FAT and remove erases the directory entry, i.e. all 64 bytes of the entry are set to 0.
  
Parameters: just enter the path and name of the directory to be removed.

