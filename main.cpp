#include <iostream>
#include <string.h>
#include "boot_record.h"
#include "root_directory.h"
#include "fat_table.h"
#include "dvv_system.h"

using namespace std;

// Compile project: g++ main.cpp -o main
// Run project: ./main

void input_validations(int argc, int argsize){
  if(argc <= argsize){
    cout << "Wrong number of arguments" << endl;
		exit(-1);
	}
}

void format_operation(char* pendrive, int pendrive_sector_size){
  FILE* ptr;
  Dvv dvv;
  ptr = fopen(pendrive, "w+");
  if( ptr == NULL ){
    cout << "argc: " << endl;  
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }
  dvv.format(ptr, pendrive_sector_size);
}

void copy_file_in(char* pendrive, char* diskpath){
  FILE* pendrive_ptr;
  FILE* disk_ptr;
  string filename;
  Dvv dvv;
  unsigned int cluster_found;


  pendrive_ptr = fopen(pendrive, "r+");
  if( pendrive_ptr == NULL ){
    cout << "argc: " << endl;  
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }

  cluster_found = dvv.findPath(pendrive_ptr, diskpath);
  cout << "cluster_found: " << cluster_found << endl;
  

  if (cluster_found == -1){
    cout << "Wrong Path, try again. \n";
    exit(-1);
  }else{

    filename = dvv.filenameOfFilepath(diskpath);
    cout << "filename:  " << filename << endl;
    disk_ptr = fopen(filename.c_str(), "r+");
    if( disk_ptr == NULL ){
      cout << "argc: " << endl;  
      cout << "File in disk not found, try again. \n"; 
      exit(-1);
    }

    dvv.copy_file(pendrive_ptr, disk_ptr, cluster_found, diskpath);
  }

  cout << "Arquivo copiado para o DVV. " << endl;  
}

void copy_file_out(char* pendrive, char* dvvpath, char* newfile_name){
  
  cout << "copy_file_out " << endl;
  FILE* pendrive_ptr;
  FILE* output_ptr;
  Dvv dvv;
  string filename;
  unsigned int cluster_found;
  
  pendrive_ptr = fopen(pendrive, "r+");
  if( pendrive_ptr == NULL ){
    cout << "argc: " << endl;  
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }

  cluster_found = dvv.findPath(pendrive_ptr, dvvpath);
  cout << "cluster_founddd: " << cluster_found << endl;  

  if (cluster_found == -1){
    cout << "Wrong Path, try again. \n";
    exit(-1);
  }else{
    filename = dvv.filenameOfFilepath(dvvpath);
    cout << "filename:  " << filename << endl;
    
    output_ptr = fopen(newfile_name, "w+");
    if( output_ptr == NULL ){
      cout << "File in disk not found, try again. \n"; 
      exit(-1);
    }

    dvv.copy_out_file(pendrive_ptr, output_ptr, cluster_found, dvvpath);
  }
  cout << "Arquivo copiado para o disco rígido. " << endl;  
}

void list_all_files(char* pendrive, char* folder_name){

  FILE* pendrive_ptr;
  Dvv dvv;
  unsigned int cluster_found;
  pendrive_ptr = fopen(pendrive, "r+");
  if( pendrive_ptr == NULL ){
    cout << "argc: " << endl;  
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }
  
  cluster_found = dvv.findPath(pendrive_ptr, folder_name);

  if (cluster_found == -1){
    cout << "Wrong Path, try again. \n";
    exit(-1);
  }else{
    dvv.list_files(pendrive_ptr, folder_name, cluster_found);
  }
}

void make_directory(char* pendrive, char* directory_name){
  FILE* pendrive_ptr;
  Dvv dvv;

  pendrive_ptr = fopen(pendrive, "r+");
  if( pendrive_ptr == NULL ){
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }
  dvv.create_directory(pendrive_ptr, directory_name);
  cout << "Directory created. " << endl;
}

void removefile(char* pendrive, char* diskpath){
  FILE* pendrive_ptr;
  FILE* disk_ptr;
  string filename;
  Dvv dvv;
  unsigned int cluster_found;

  pendrive_ptr = fopen(pendrive, "r+");
  if( pendrive_ptr == NULL ){
    cout << "argc: " << endl;  
    cout << "Pendrive not found, try again. \n"; 
    exit(-1);
  }

  cluster_found = dvv.findPath(pendrive_ptr, diskpath);
  cout << "cluster_found: " << cluster_found << endl;
  
  if (cluster_found == -1){
    cout << "Wrong Path, try again. \n";
    exit(-1);
  }else{

    filename = dvv.filenameOfFilepath(diskpath);
    cout << "main --> filename:  " << filename << endl;
    dvv.remove_file(pendrive_ptr, cluster_found, filename);
  }

  cout << "Arquivo removido no DVV. " << endl;  
}

int main( int argc, char *argv[] ){
  
  char* pendrive;
  char* disk_path_to_file;
  char* directory_name;
  char* dvv_path_to_file;
  char* newfile_name;
  char* folder_name;

  input_validations(argc, 3);

  pendrive = argv[1];


  if( !strcmp( argv[2], "format" ) ){
    int pendrive_sector_size;
    
    pendrive_sector_size = atoi(argv[3]);

    format_operation(pendrive, pendrive_sector_size);
  }
  else if( !strcmp( argv[2], "cpin" ) ){
    disk_path_to_file = argv[3]; 
    input_validations(argc, 3); // quando receber caminho path vai ser 4
    copy_file_in(pendrive, disk_path_to_file);
  }
  else if( !strcmp( argv[2], "cpout" ) ){

    dvv_path_to_file = argv[3]; 
    newfile_name = argv[4];
    input_validations(argc, 4); // quando receber caminho path vai ser 4
    copy_file_out(pendrive, dvv_path_to_file, newfile_name);

  }
  else if( !strcmp( argv[2], "ls" ) && !strcmp( argv[3], "-la" ) ){
    folder_name = argv[4];
    list_all_files(pendrive, folder_name);
  }
  else if( !strcmp( argv[2], "mkdir" ) ){
    directory_name = argv[3]; 
    
    make_directory(pendrive, directory_name);
  }
  else if( !strcmp( argv[2], "rm" ) ){
    disk_path_to_file = argv[3];
    input_validations(argc, 3); // quando receber caminho path vai ser 4
    removefile(pendrive, disk_path_to_file);
  }
  else{
    cout << "Command not found ... \n" << endl;
  }

}
