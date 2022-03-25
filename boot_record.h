#ifndef BOOT_RECORD
#define BOOT_RECORD

#include <stdio.h>
#include <iostream>

using namespace std;

class __attribute__((__packed__)) boot_record{

	public:
    unsigned short bytes_per_sector;
    char sectors_per_cluster;
    unsigned short reserved_sectors;
    char number_of_tables;
    unsigned short sectors_per_table;
    unsigned short num_entries_root_dir;
    unsigned char* garbadge;
   
    boot_record( unsigned short bytes_per_sector = 512, char sectors_per_cluster = 8, 
            unsigned short reserved_sectors = 1, char number_of_tables = 3,
            unsigned short sectors_per_table = 0, unsigned short num_entries_root_dir = 1024){

      this->bytes_per_sector = bytes_per_sector;
      this->sectors_per_cluster = sectors_per_cluster;
      this->reserved_sectors = reserved_sectors;
      this->number_of_tables = number_of_tables;
      this->sectors_per_table = sectors_per_table;
      this->num_entries_root_dir = num_entries_root_dir;
      this->garbadge = (unsigned char*) calloc( 502, sizeof(char) );

    }

  void WriteFile( FILE* file );  
  void ReadFile( FILE* file ); 
};

void boot_record::ReadFile( FILE* file ){
  fseek(file, 0, SEEK_SET);
	fread(this, 10, 1, file);
}

void boot_record::WriteFile( FILE* file ){
	fwrite( this, 10, 1, file );
	fwrite( this->garbadge, 1, 502, file );
}

#endif   
