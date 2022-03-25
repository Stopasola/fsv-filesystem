#ifndef FAT_TABLE
#define FAT_TABLE

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>

using namespace std;


class Fat{
	public:
		unsigned int bytes_per_sector;
		unsigned int num_of_sectors;
	  unsigned int* fat_table;
		int fat_entries;

		Fat(unsigned int num_sectors_fat, unsigned short bytes_per_sector);
		void WriteFile( FILE* file );
		std::list<unsigned int> GetFreeClusters( FILE* file, int reserved_sectors, int number_of_clusters);
		void SetFileClusters(FILE* file, int reserved_sectors, list<unsigned int> cluster_list);
		std::list<unsigned int> GetFileCluster(FILE* file, unsigned int first_cluster, int reserved_sectors);
		void DeleteFileCluster(FILE* file, int reserved_sectors, list<unsigned int> cluster_list);
};

Fat::Fat( unsigned int num_sectors_fat, unsigned short bytes_per_sector){

	this->num_of_sectors = num_sectors_fat;
	this->bytes_per_sector = bytes_per_sector;
	this->fat_entries = ( num_sectors_fat * bytes_per_sector) / 4;
	
	fat_table = (unsigned int*) calloc(fat_entries, sizeof(unsigned int));


}

void Fat::WriteFile( FILE* file ){
	this->fat_table[0] = 0xFFFFFFFF;

	fwrite( this->fat_table, sizeof(unsigned int), this->fat_entries, file );
}

std::list<unsigned int> Fat::GetFreeClusters( FILE* file, int reserved_sectors, int number_of_clusters){
	list <unsigned int> cluster_list;
	int found = 0;
	fseek(file, reserved_sectors * 512, SEEK_SET);
	fread(this->fat_table, sizeof(unsigned int), this->fat_entries, file );

	
	for(int i=1; i <= this->fat_entries;i++){
		if( this->fat_table[i] == 0){
			found = 1;
			cluster_list.push_back(i);
			if(cluster_list.size() == number_of_clusters ){
				break;
			} 
		}
	}
	if (found = 0){
		cout << "No pendrive space!!" << endl;
		exit(-1);
	}

	return cluster_list;
}

void Fat::SetFileClusters(FILE* file, int reserved_sectors, list<unsigned int> cluster_list){
	// pensar caso n seja linear !!! sort

	for(int i=1; i <= this->fat_entries;i++){
		if(cluster_list.front() == i){
			cluster_list.pop_front();
			if( cluster_list.size() >= 1){
				this->fat_table[i] = cluster_list.front();
			}else{
				this->fat_table[i] = 0xFFFFFFFF;
			}
		}
	}
}

void showlist(list <unsigned int> g){
    list <unsigned int> :: iterator it;
    for(it = g.begin(); it != g.end(); ++it)
        cout << '\t' << *it;
    cout << '\n';
}

std::list<unsigned int> Fat::GetFileCluster(FILE* file, unsigned int first_cluster, int reserved_sectors){

	list<unsigned int> cluster_list;
	cluster_list.push_back(first_cluster);
	fseek(file, reserved_sectors * 512, SEEK_SET);
	fread(this->fat_table, sizeof(unsigned int), this->fat_entries, file );

	while (this->fat_table[cluster_list.back()] != 0xFFFFFFFF){

		cluster_list.push_back(this->fat_table[cluster_list.back()]);
	}
	showlist(cluster_list);
	return cluster_list;
}

void Fat::DeleteFileCluster(FILE* file, int reserved_sectors, list<unsigned int> cluster_list){
	for(int i=1; i <= this->fat_entries;i++){
		if(cluster_list.front() == i){
			cluster_list.pop_front();
			this->fat_table[i] = 0x00000000;
		}
	}
}

#endif
