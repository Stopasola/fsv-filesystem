#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <list>
#include<tuple>
#include "boot_record.h"
#include "root_directory.h"
#include "fat_table.h"

using namespace std;

class Dvv{

  public:
		boot_record* boot_rec;
    Fat* fat_table;
    RootDirectory* root_dir;
    Dvv();
    void format(FILE* file, int sector_size);
    int findPath(FILE* file, string filepath);
    int copy_file(FILE* pendrive_file, FILE* disk_file, int cluster_found, string filepath);
    int remove_file(FILE* pendrive_file, int cluster_found, string filepath);
    int create_directory(FILE* pendrive_file, string directory_name);
    int copy_out_file(FILE* pendrive_file, FILE* output_ptr, int cluster_found, string filepath);
    int calculateFatSize(int disk_sector_size, 
      unsigned short reserved_sectors, unsigned short num_entries_root_dir,
      char sectors_per_cluster, unsigned short bytes_per_sector);
    string filenameOfFilepath(string filepath);
    void list_files(FILE* pendrive_file, string folder_name, unsigned int cluster_found);
}; 


Dvv::Dvv(){
	this->boot_rec = new boot_record;
	this->root_dir = new RootDirectory;
}

void Dvv::list_files(FILE* pendrive_file, string folder_name, unsigned int cluster_found){
  
  boot_record bt;
  bt.ReadFile(pendrive_file);

  // Read Entry Directory
  RootDirectory rd;
  
  int root_entry;
	if (cluster_found == 0){
		root_entry = 0;
	}else{
		root_entry = 2;
	}
	unsigned long entry_sector =  (bt.reserved_sectors + (bt.sectors_per_table * 3) + root_entry) * bt.bytes_per_sector;
	entry_sector += cluster_found * 4096;

  for(int i = 0; i < 1024; i+=64){
    rd.ReadEntry(pendrive_file, entry_sector + i);
    if (rd.file_attribute == 5){
      cout << "----------------" << endl;
      cout << rd.filename;
      cout << ".";
      cout << rd.extension << endl;
    }
  }
}

int Dvv::calculateFatSize(int disk_sector_size,  unsigned short reserved_sectors, unsigned short num_entries_root_dir,
      char sectors_per_cluster, unsigned short bytes_per_sector){
    
    float sectors_per_table = 0;
    sectors_per_table = disk_sector_size - reserved_sectors - (num_entries_root_dir / bytes_per_sector);
    sectors_per_table = floor (sectors_per_table / sectors_per_cluster);
    sectors_per_table =  ((sectors_per_table + 1)  * 4);
    sectors_per_table =  ceil ( sectors_per_table / bytes_per_sector);
   
    return sectors_per_table;
}

void Dvv::format( FILE* file, int disk_sector_size ){
  cout << "Formating ... " << endl;

  boot_record bt;
  bt.sectors_per_table = calculateFatSize(disk_sector_size, bt.reserved_sectors, bt.num_entries_root_dir,
                                                    bt.sectors_per_cluster, bt.bytes_per_sector);

  Fat fat(bt.sectors_per_table, bt.bytes_per_sector);
  
  // Formatação do disco

	rewind(file);
  bt.WriteFile(file); 

  fat.WriteFile(file);
  fat.WriteFile(file);
  fat.WriteFile(file);

  // Cria entradas do diretório raiz
  for(int i = 0; i < bt.num_entries_root_dir; i+=64){
		RootDirectory rd;
		rd.WriteFile(file);
	}

  int data_sectors = disk_sector_size - bt.reserved_sectors - ( bt.sectors_per_table * 3) - (bt.num_entries_root_dir / bt.bytes_per_sector);
  cout << "data_sectors: " << data_sectors << endl;
	
  // Escreve na área de dados 
  for( int sector = 0; sector < data_sectors; sector++ ){
    // cout << "sector: " << sector << endl;
    for(int i = 0; i < bt.bytes_per_sector; i+=64 ){
      RootDirectory rd;
		  rd.WriteFile(file);
    }
  }
}

string Dvv::filenameOfFilepath(string filepath){
  string delimiter = "/";
  list <string> subdir;

  size_t pos = 0;
  std::string token;
  while ((pos = filepath.find(delimiter)) != std::string::npos) {
      token = filepath.substr(0, pos);
      //std::cout << token << std::endl;
      subdir.push_back(token);
      filepath.erase(0, pos + delimiter.length());
  }
  subdir.push_back(filepath);
  subdir.remove(" ");

  // showlist(subdir);
  return subdir.back();
}

int Dvv::findPath(FILE* file, string filepath){
  std::string delimiter = "/";
  list <string> subdir;
  string directory;
  int filepath_size;
  bool found = false;

  // Split lista de strings
  size_t pos = 0;
  std::string token;
  while ((pos = filepath.find(delimiter)) != std::string::npos) {
      token = filepath.substr(0, pos);
      subdir.push_back(token);
      filepath.erase(0, pos + delimiter.length());
  }
  std::cout << filepath << std::endl;
  subdir.remove(" ");
  // Buscar first byte do entry cluster

  boot_record bt;
  bt.ReadFile(file);

  int root_dir_start = ((bt.reserved_sectors + ( bt.sectors_per_table * bt.number_of_tables)) * bt.bytes_per_sector);
  
  if (subdir.front().compare("root") != 0){
    return -1;
  }else{
    subdir.pop_front();
      
    RootDirectory rd;
    unsigned int first_cluster = 0;
    unsigned int entrys_size = bt.num_entries_root_dir;
    
    while(subdir.size() > 0){
      for(int i = 0; i < bt.num_entries_root_dir; i+=64 ){
        rd.ReadEntry(file, root_dir_start + i);
        if (rd.file_attribute != '\0'){
          found = true;
          filepath_size = subdir.size();
          directory = subdir.front();
          for(int j=0;j<filepath_size; j++){
            if(rd.filename[j] == directory.front()){
              directory.erase(0, 1);;
              continue;
            }
            else{
              found = false;
              break;
            }
          }
          if(found == true){
            first_cluster = rd.first_cluster;
            break;
          }
        }
      }
      if(found == false){
        return -1;
      }
      root_dir_start = (bt.reserved_sectors + (bt.number_of_tables * bt.sectors_per_table) + 
                       (bt.num_entries_root_dir / bt.bytes_per_sector)) * bt.bytes_per_sector;
      root_dir_start += first_cluster * 4096;
      subdir.pop_front();
    }
    return first_cluster;
  }
}

void showlistt(list <unsigned int> g){
		cout << "===" << endl;
    list <unsigned int> :: iterator it;
    for(it = g.begin(); it != g.end(); ++it)
        cout << '\t' << *it;
    cout << '\n';
}

int Dvv::remove_file(FILE* pendrive_file, int cluster_found, string filepath){

  string filename, file_ext, fileextension, delimiter = ".";
  file_ext = filenameOfFilepath(filepath);
  filename = file_ext.substr(0, file_ext.find(delimiter)); // filename
  fileextension = file_ext.erase(0, filename.length() + 1);  // ext

  cout << "dvv --->  filepath: " << filepath << endl;
  cout << "dvv --->  file_ext: " << file_ext << endl;
  cout << "dvv --->  filename: " << filename << endl;
  cout << "dvv --->  fileextension: " << fileextension << endl;
  

  boot_record bt;
  bt.ReadFile(pendrive_file);

  // Read Entry Directory
  RootDirectory rd;
  unsigned int entry_cluster = rd.GetFileFirstCluster(pendrive_file, cluster_found, bt.reserved_sectors, bt.sectors_per_table, bt.bytes_per_sector, filename);

  if (entry_cluster == 0){
    cout << "Removal file not found." << endl;
    exit(1);
  }

  cout << "dvv --->  Entry position (byte): " << entry_cluster << endl;
  rd.ReadEntry(pendrive_file, entry_cluster);

  // Get File Clusters
  Fat fat(bt.sectors_per_table, bt.bytes_per_sector);

  list <unsigned int> cluster_list = fat.GetFileCluster(pendrive_file, rd.first_cluster, bt.reserved_sectors);

  showlistt(cluster_list);

  // Free file clusters 
  fat.DeleteFileCluster(pendrive_file, bt.reserved_sectors, cluster_list);

  rewind(pendrive_file);
	fseek(pendrive_file, 512, SEEK_SET );
  fat.WriteFile(pendrive_file);
  fat.WriteFile(pendrive_file);
  fat.WriteFile(pendrive_file);

  // Free entry file   
  rewind(pendrive_file);
  fseek(pendrive_file, entry_cluster, SEEK_SET);
  rd.WriteFile2(pendrive_file);

}
    

int Dvv::copy_file(FILE* pendrive_file, FILE* disk_file, int cluster_found, string filepath){

  string file_ext, filename, fileextension, delimiter = ".";
  boot_record bt;
  bt.ReadFile(pendrive_file);
  
  file_ext = filenameOfFilepath(filepath);
  filename = file_ext.substr(0, file_ext.find(delimiter)); // filename
  fileextension = file_ext.erase(0, filename.length() + 1);  // ext
  
  cout << "filename " << filename << endl;
  cout << "fileextension " << fileextension << endl;

  if( filename.length() > 44 || fileextension.length() > 3){
    cout << "File name or extension are invalid, try again." << endl;
    return - 1;
  }
  RootDirectory rd;
  unsigned int entry_cluster = rd.GetFileFirstCluster(pendrive_file, cluster_found, bt.reserved_sectors, bt.sectors_per_table, bt.bytes_per_sector, filename);
  
  if (entry_cluster != 0){
    cout << "There is one file with the same name in this folder, pĺease change the file name." << endl;
    exit(1);
  }


  fseek(disk_file, 0, SEEK_END);
  unsigned long file_size = ftell(disk_file); 
  int number_of_clusters = (( ceil ((file_size) / bt.bytes_per_sector) + 1) / bt.sectors_per_cluster) + 1 ;

  // cout << "file_size:" << file_size <<  endl;
  // cout << "number_of_clusters:" << number_of_clusters <<  endl;
  
  Fat fat(bt.sectors_per_table, bt.bytes_per_sector);

  // Percorre a fat buscando clusters livres
  list <unsigned int> cluster_list = fat.GetFreeClusters(pendrive_file, bt.reserved_sectors, number_of_clusters);

  showlistt(cluster_list);

  // Percorre a fat escrevendo clusters 
  fat.SetFileClusters(pendrive_file, bt.reserved_sectors, cluster_list);

  // Escreve nas fats
  rewind(pendrive_file);

	fseek(pendrive_file, 512, SEEK_SET );
  fat.WriteFile(pendrive_file);
  
  fat.WriteFile(pendrive_file);
  fat.WriteFile(pendrive_file);
 

  // Busca e escreve no diretorio
  
  rd.writeFileEntry(pendrive_file, cluster_found, cluster_list.front(), filename, fileextension, 
                    file_size, bt.reserved_sectors, bt.sectors_per_table, bt.bytes_per_sector);

  // Escreve nos clusters

	rewind(disk_file);

  // 512 é o cluster reservado 0xfffffff
  
  int data_sector =  (bt.reserved_sectors + (bt.number_of_tables * bt.sectors_per_table) + 
                      (bt.num_entries_root_dir / bt.bytes_per_sector)) * bt.bytes_per_sector;
  
  // cout << "-------data_sector:" << data_sector << endl;
  
  int bytes_in_cluster = bt.bytes_per_sector * bt.sectors_per_cluster;
  

  for(unsigned long cluster = 0; cluster < number_of_clusters; cluster++){
    unsigned long offset = cluster_list.front() * bt.sectors_per_cluster * bt.bytes_per_sector;
    cluster_list.pop_front();
    fseek(pendrive_file, data_sector + offset, SEEK_SET );
    // cout << "-----------data_sector: " << data_sector << endl;
    // cout << "-----------offset: " << offset << endl;
    // cout << "--------------data_sector + offset: " << data_sector + offset << endl;
    for( unsigned long i = 0; i < bytes_in_cluster; i++ ){
      unsigned char buffer;
      fread(  &buffer, 1, 1, disk_file );
      // cout << "----------buffer:     " << buffer << endl;
      if( feof( disk_file ) )
          return 1;
      fwrite( &buffer, 1, 1, pendrive_file );
    }
  }
  
  return 1;

}

int Dvv::create_directory(FILE* pendrive_file, string directory_name){
  // cout << "create_directory" << endl;

  if( directory_name.length() > 44){
    cout << "Directory name is invalid, try again." << endl;
    return - 1;
  }

  boot_record bt;
  bt.ReadFile(pendrive_file);

  // Percorre a fat buscando clusters livres

  Fat fat(bt.sectors_per_table, bt.bytes_per_sector);

  list <unsigned int> cluster_list = 
    fat.GetFreeClusters(pendrive_file, bt.reserved_sectors, 1);

  showlistt(cluster_list);

  // Percorre a fat escrevendo clusters 
  fat.SetFileClusters(pendrive_file, bt.reserved_sectors, cluster_list);

  // Escreve nas fats
  rewind(pendrive_file);

	fseek(pendrive_file, 512, SEEK_SET );
  fat.WriteFile(pendrive_file);
  fat.WriteFile(pendrive_file);
  fat.WriteFile(pendrive_file);

  // Busca e escreve no diretorio
  RootDirectory rd;
  rd.writeDirectoryEntry(pendrive_file, cluster_list.front(), directory_name
                    , bt.reserved_sectors, bt.sectors_per_table, bt.bytes_per_sector);

  // cout << "directory_name " << directory_name << endl;
}

int Dvv::copy_out_file(FILE* pendrive_file, FILE* output_ptr, int cluster_found, string filepath){
  cout << "Entrou " << endl;
  string filename, file_ext, fileextension, delimiter = ".";
  file_ext = filenameOfFilepath(filepath);
  filename = file_ext.substr(0, file_ext.find(delimiter)); // filename
  fileextension = file_ext.erase(0, filename.length() + 1);  // ext
 

  boot_record bt;
  bt.ReadFile(pendrive_file);

  // Read Entry Directory
  RootDirectory rd;
  unsigned int entry_cluster = rd.GetFileFirstCluster(pendrive_file, cluster_found, bt.reserved_sectors, bt.sectors_per_table, bt.bytes_per_sector, filename);

  // cout << "entry_cluster" << entry_cluster << endl;
  rd.ReadEntry(pendrive_file, entry_cluster);

  // cout << "rd.filename;" << rd.filename << endl;

  // Get File Clusters
  Fat fat(bt.sectors_per_table, bt.bytes_per_sector);

  list <unsigned int> cluster_list = fat.GetFileCluster(pendrive_file, rd.first_cluster, bt.reserved_sectors);

  // le e apaga da fat

	rewind(output_ptr);  
  int data_sector =  (bt.reserved_sectors + (bt.number_of_tables * bt.sectors_per_table) + 
                      (bt.num_entries_root_dir / bt.bytes_per_sector)) * bt.bytes_per_sector;
  

  // cout << "-------data_sector:" << data_sector << endl;
  
  int bytes_in_cluster = bt.bytes_per_sector * bt.sectors_per_cluster;
  int number_of_clusters = cluster_list.size();
  unsigned int file_size = rd.file_size;

  // cout << "-------number_of_clusters:" << number_of_clusters << endl;

  for(unsigned long cluster = 0; cluster < number_of_clusters; cluster++){

    unsigned long offset = cluster_list.front() * bt.sectors_per_cluster * bt.bytes_per_sector;
    cluster_list.pop_front();
    fseek(pendrive_file, data_sector + offset, SEEK_SET );
    // cout << "-----------data_sector: " << data_sector << endl;
    // cout << "-----------offset: " << offset << endl;
    // cout << "--------------data_sector + offset: " << data_sector + offset << endl;

    for( unsigned long i = 0; i < bytes_in_cluster; i++ ){
      unsigned char buffer;
      fread(  &buffer, 1, 1, pendrive_file );
      file_size -= 1;
      if (file_size == 0){
        return 1;
      }
      fwrite( &buffer, 1, 1, output_ptr);
    }
  }
}
