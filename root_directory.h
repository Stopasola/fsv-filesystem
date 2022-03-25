#ifndef ROOT_DIRECTORY
#define ROOT_DIRECTORY
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <list>
#include <ctime>
#include <stdio.h>

class __attribute__((__packed__)) RootDirectory{
		
  public:
		unsigned char* filename;
		unsigned char* extension;
		unsigned char file_attribute;
		unsigned short creation_time;
		unsigned short creation_date;
		unsigned short last_update_time;
		unsigned short last_update_date;
		unsigned int first_cluster;
		unsigned int file_size;

		RootDirectory(){

			this->filename = (unsigned char*) calloc( 44, sizeof(char) );
			this->extension = (unsigned char*) calloc( 3, sizeof(char) );
			this->file_attribute = 0;
			this->creation_time = 0;
			this->creation_date = 0;
			this->last_update_time = 0;
			this->last_update_date = 0;
			this->first_cluster = 0;
			this->file_size = 0;
		}

		void WriteFile( FILE* file );
		void WriteFile2( FILE* file );
		void writeDirectoryEntry(FILE* file, unsigned int first_cluster, string directory_name, 
																	 unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector);

		void ReadEntry( FILE* file, unsigned long  entry_sector);
		void writeFileEntry(FILE* file, unsigned int entry_cluster, unsigned int first_cluster, string filename, string fileextension, unsigned long file_size, 
																	 unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector);
		unsigned int GetFileFirstCluster(FILE* file, unsigned int cluster_found, unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector, string filepath);
		unsigned int FreeFileEntry(FILE* file, unsigned int cluster_found, unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector, string filepath);
};

void RootDirectory::WriteFile( FILE* file ){
	fwrite(this->filename, 1, 44, file);
	fwrite(this->extension, 1, 3, file);
	fwrite(&this->file_attribute, sizeof(char), 1, file);
	fwrite(&this->creation_time, sizeof(short), 1, file );
	fwrite(&this->creation_date, sizeof(short), 1, file );
	fwrite(&this->last_update_time, sizeof(short), 1, file );
	fwrite(&this->last_update_date, sizeof(short), 1, file );
	fwrite(&this->first_cluster, sizeof(int), 1, file );
	fwrite(&this->file_size, sizeof(int), 1, file );
}

void RootDirectory::writeFileEntry(FILE* file, unsigned int entry_cluster, unsigned int first_cluster, string filename, string fileextension, unsigned long file_size, 
																	 unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector){

	int root_entry;
	// cout << "====== entry_cluster " << entry_cluster << endl;
	if (entry_cluster == 0){
		root_entry = 0;
	}else{
		root_entry = 2;
	}
	unsigned long entry_sector =  (reserved_sectors + (sectors_per_table * 3) + root_entry) * bytes_per_sector;
	// cout << "1 - entry_sector" << entry_sector << endl;

	entry_sector += entry_cluster * 4096;

	
	// cout << "2 - entry_sector" << entry_sector << endl;

	unsigned char* verification_byte;
	
	for(int i = 0; i < 1024; i+=64){

		this->ReadEntry(file, entry_sector + i);

		if (this->file_attribute == '\0'){
			int filename_size = filename.size();
			for(int i = 0; i < filename_size; i++){
				this->filename[i] = filename.front();
				filename.erase(0, 1);

			}
			int fileextension_size = fileextension.size();
			for(int i = 0; i < fileextension_size; i++){
				this->extension[i] = fileextension.front();
				fileextension.erase(0, 1);
			}
			this->file_attribute = 5;
			this->creation_time = 0;
			this->creation_date = 0;
			this->last_update_time = 0;
			this->last_update_date = 0;
			this->first_cluster = first_cluster;
			this->file_size = file_size;
			cout << "entry_sector + offset" << entry_sector + i << endl;
			fseek(file, entry_sector + i, SEEK_SET);
			this->WriteFile(file);
			break;
		}

	}
	// cout << "this->filename: " << this->filename << endl;
	// cout << "this->fileextension: " << this->extension << endl;
	// cout << "this->first_cluster: " << this->first_cluster << endl;
	// cout << "this->file_size: " << this->file_size << endl;
}

void RootDirectory::writeDirectoryEntry(FILE* file, unsigned int first_cluster, string directory_name, 
																	 unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector){

	unsigned long entry_sector =  (reserved_sectors + (sectors_per_table * 3)) * bytes_per_sector;

	// cout << "reserved_sectors" << reserved_sectors << endl;
	// cout << "sectors_per_table" << sectors_per_table << endl;
	// cout << "bytes_per_sector" << bytes_per_sector << endl;
	// cout << "entry_sector" << entry_sector << endl;

	for(int i = 0; i < 1024; i+=64){

		this->ReadEntry(file, entry_sector + i);

		if (this->file_attribute == '\0'){

			int directory_size = directory_name.size();

			for(int i = 0; i < directory_size; i++){
				this->filename[i] = directory_name.front();
				directory_name.erase(0, 1);
			}
			this->file_attribute = 10;
			this->first_cluster = first_cluster;

			fseek(file, entry_sector + i, SEEK_SET);
			this->WriteFile(file);
			break;
		}

	}
	// cout << "this->filename: " << this->filename << endl;
	// cout << "this->fileextension: " << this->extension << endl;
	// cout << "this->first_cluster: " << this->first_cluster << endl;
	// cout << "this->file_size: " << this->file_size << endl;
}

void RootDirectory::ReadEntry( FILE* file, unsigned long entry_sector ){
	fseek(file, entry_sector, SEEK_SET);

	fread(this->filename, 1, 44, file);
	fread(this->extension, 1, 3, file);
	fread(&this->file_attribute, sizeof(char), 1, file);
	fread(&this->creation_time, sizeof(short), 1, file );
	fread(&this->creation_date, sizeof(short), 1, file );
	fread(&this->last_update_time, sizeof(short), 1, file );
	fread(&this->last_update_date, sizeof(short), 1, file );
	fread(&this->first_cluster, sizeof(int), 1, file );
	fread(&this->file_size, sizeof(int), 1, file );
}

void RootDirectory::WriteFile2( FILE* file ){

	this->filename = (unsigned char*) calloc( 44, sizeof(char) );
	this->extension = (unsigned char*) calloc( 3, sizeof(char) );
	this->file_attribute = 0;
	this->creation_time = 0;
	this->creation_date = 0;
	this->last_update_time = 0;
	this->last_update_date = 0;
	this->first_cluster = 0;
	this->file_size = 0;

	fwrite(this->filename, 1, 44, file);
	fwrite(this->extension, 1, 3, file);
	fwrite(&this->file_attribute, sizeof(char), 1, file);
	fwrite(&this->creation_time, sizeof(short), 1, file );
	fwrite(&this->creation_date, sizeof(short), 1, file );
	fwrite(&this->last_update_time, sizeof(short), 1, file );
	fwrite(&this->last_update_date, sizeof(short), 1, file );
	fwrite(&this->first_cluster, sizeof(int), 1, file );
	fwrite(&this->file_size, sizeof(int), 1, file );
}

unsigned int RootDirectory::GetFileFirstCluster(FILE* file, unsigned int cluster_found, unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector, string filename){
	
	int root_entry;
	string filename_aux;
	if (cluster_found == 0){
		root_entry = 0;
	}else{
		root_entry = 2;
	}

	unsigned long entry_sector =  (reserved_sectors + (sectors_per_table * 3) + root_entry) * bytes_per_sector;
	entry_sector += cluster_found * 4096;

	cout << "root --->  Entry position (byte): " << entry_sector << endl;


	bool found = true;
	int filepath_size = 0; 
	
	std::cout << "root ---> file_ext --- " << filename << "\n";


	for(int i = 0; i < 1024; i+=64){
		this->ReadEntry(file, entry_sector + i);
		if (this->file_attribute != '\0'){
			found = true;
			filename_aux = filename;
			filepath_size = filename.size();
			// std::cout << "root ---> filepath_size --- " << filepath_size << "\n";
			// std::cout << "root ---> this->filename --- " << this->filename << "\n";

			// std::cout << "GetFileFirstCluster --- file_attribute --- " << file_attribute << "\n";
			for(int j=0;j<filepath_size; j++){
				// cout << "==========> this->filename[j]: " << this->filename[j] << endl;
				// cout << "==========> filename.front(): " << filename_aux.front() << endl;
				if(this->filename[j] == filename_aux.front()){
					filename_aux.erase(0, 1);;
					continue;
				}
				else{
					found = false;
					break;
				}
			}
			if(found == true){
				return entry_sector + i;
			}
		}
	}
	return 0;
}

unsigned int RootDirectory::FreeFileEntry(FILE* file, unsigned int cluster_found, unsigned short reserved_sectors, unsigned short sectors_per_table, unsigned short bytes_per_sector, string filepath){
	
	int root_entry;
	// cout << "====== cluster_found " << cluster_found << endl;
	if (cluster_found == 0){
		root_entry = 0;
	}else{
		root_entry = 2;
	}

	unsigned long entry_sector =  (reserved_sectors + (sectors_per_table * 3) + root_entry) * bytes_per_sector;
	entry_sector += cluster_found * 4096;

	// cout << "entry_cluster: " << entry_sector << endl;


	bool found = true;
	int filepath_size = 0; 

	for(int i = 0; i < 1024; i+=64){
		this->ReadEntry(file, entry_sector + i);
		if (this->file_attribute != '\0'){
			found = true;
			filepath_size = filepath.size();
			for(int j=0;j<filepath_size; j++){
				// cout << "==========> this->filename[j]: " << this->filename[j] << endl;
				// cout << "==========> filepath.front(): " << filepath.front() << endl;
				if(this->filename[j] == filepath.front()){
					filepath.erase(0, 1);;
					continue;
				}
				else{
					found = false;
					break;
				}
			}
			if(found == true){
				return entry_sector + i;
			}
		}
	}
	return 0;
}

#endif
