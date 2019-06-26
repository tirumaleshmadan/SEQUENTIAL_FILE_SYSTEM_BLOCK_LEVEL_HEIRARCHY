#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>

#define size (16 * 1024)

struct file_record
{
	char filename[20];
	int startblock;
	int no_of_blocks;
	unsigned size_of_file;
};

struct Metadata
{
	int code;
	struct file_record files[20];
	int no_of_files;
	int free_blocks;
	char vector[6400];
};

char* get_block(int block_number)
{
	char *buffer = (char*)malloc(sizeof(char)*(size));
	FILE *fstream = fopen("harddisk.hdd", "rb+");
	if (fstream)
	{
		if (!fseek(fstream,block_number*size,SEEK_SET))
		{
			if (!fread(buffer, 1, size, fstream))
				printf("FILE NOT READ\n");
			fclose(fstream);
		}
		else
			printf("SEEKING UNSUCCESSFULL\n");
	}
	else
		printf("FILE NOT OPENED\n");
	return buffer;
}

void set_block(int block_number,char *buffer)
{
	FILE *fstream = fopen("harddisk.hdd", "rb+");
	if (fstream)
	{
		if (!fseek(fstream, block_number*size, SEEK_SET))
		{
			if (!fwrite(buffer, 1, size, fstream))
				printf("FILE NOT WRITTEN\n");
			fclose(fstream);
		}
		else
			printf("SEEKING UNSUCCESSFULL\n");
	}
	else
		printf("FILE NOT OPENED\n");
}

struct Metadata* get_the_initial_metadata(struct Metadata *metadata)
{
	char *buffer = (char*)malloc(sizeof(char)*size);
	buffer = get_block(0);
	memcpy(metadata,buffer,sizeof(struct Metadata));
	return metadata;
}

void set_the_metadata_to_disk(struct Metadata *metadata)
{
	char *buffer = (char*)malloc(sizeof(char)*size);
	memcpy(buffer, metadata, sizeof(struct Metadata));
	set_block(0, buffer);
}

int get_the_free_starting_block(struct Metadata *metadata,int no_of_blocks)
{
	struct Metadata *temp = metadata;
	int count = 0;
	for (int index = 1; index < 6400; index++)
	{
		if (temp->vector[index] == 48)
			count++;
		else
			count = 0;
		if (count == no_of_blocks)
			return index - no_of_blocks+1;
	}
	return -1;
}

struct Metadata* set_the_metadata(struct Metadata* metadata,char *destination,int start_block,int no_of_blocks,unsigned file_size)
{
	struct Metadata* temp = metadata;
	strcpy(temp->files[temp->no_of_files].filename, destination);
	temp->files[temp->no_of_files].no_of_blocks = no_of_blocks;
	temp->files[temp->no_of_files].startblock = start_block;
	temp->files[temp->no_of_files].size_of_file = file_size;
	for (int index = start_block; index < start_block + no_of_blocks; index++)
	{
		temp->vector[index] = 49;
	}
	temp->no_of_files+=1;
	temp->free_blocks -= no_of_blocks;
	return temp;
}

void debug(struct Metadata *metadata)
{
	struct Metadata *temp = metadata;
	for (int file = 0; file < temp->no_of_files; file++)
	{
		printf("%d ", file+1);
		printf("%s ",temp->files[file].filename);
		printf("%u ",temp->files[file].startblock);
		printf("%u ",temp->files[file].no_of_blocks);
		printf("%u\n ",temp->files[file].size_of_file);
	}
	printf("NUMBER OF FILES : %d\n", metadata->no_of_files);
	printf("NUMBER OF FREE BLOCKS : %d\n", metadata->free_blocks);
}

void list_all_the_files(struct Metadata *metadata)
{
	struct Metadata *temp = metadata;
	for (int file = 0; file < temp->no_of_files; file++)
	{
		printf("%s\n", temp->files[file].filename);
	}
}

struct Metadata* copy_file_to_disk(struct Metadata *metadata,char *source,char *destination)
{
	char *buffer = (char*)malloc(sizeof(char)*size);
	unsigned file_size;
	int no_of_blocks;
	unsigned offset;
	int start_block;
	int count = 0;
	FILE *source_stream = fopen(source, "rb+");
	if (source_stream)
	{
		if (!fseek(source_stream, 0, SEEK_END))
		{
			file_size = ftell(source_stream);
			no_of_blocks = file_size / size;
			offset = file_size%size;
			start_block = get_the_free_starting_block(metadata,no_of_blocks+(offset?1:0));
			if (start_block == -1 || metadata->no_of_files==20)
			{
				printf("NO SPACE LEFT IN THE DISK\n");
				return metadata;
			}
			if (!fseek(source_stream, 0, SEEK_SET))
			{
				while (count < no_of_blocks)
				{
					if (!fread(buffer, 1, size, source_stream))
						printf("FILE NOT READ\n");
					set_block(start_block + count, buffer);
					count++;
				}
				if (offset)
				{
					if (!fread(buffer, 1, offset, source_stream))
						printf("FILE NOT READ\n");
					set_block(start_block + count, buffer);
					count++;
				}
				metadata = set_the_metadata(metadata, destination, start_block, no_of_blocks + (offset ? 1 : 0),file_size);
				set_the_metadata_to_disk(metadata);
				fclose(source_stream);
			}
			else
				printf("SEEKING UNSUCCESSFULL\n");
		}
		else
			printf("SEEKING UNSUCCESSFULL\n");
	}
	else
		printf("FILE NOT FOUND\n");
	return metadata;
}

void copy_file_from_disk(struct Metadata *metadata, char *source, char *destination)
{
	struct Metadata *temp = metadata;
	char *buffer = (char*)malloc(sizeof(char)*size);
	unsigned file_size;
	int no_of_blocks;
	unsigned offset;
	int start_block;
	int count = 0;
	for (int file = 0; file < temp->no_of_files; file++)
	{
		if (!strcmp(temp->files[file].filename, source))
		{
			file_size = temp->files[file].size_of_file;
			no_of_blocks = temp->files[file].no_of_blocks;
			start_block = temp->files[file].startblock;
			offset = file_size%size;
			no_of_blocks--;
			FILE *fstream;
			fstream = fopen(destination, "wb");
			if (fstream)
			{
				while (count<no_of_blocks)
				{
					buffer = get_block(start_block+count);
					if (!fwrite(buffer, 1, size, fstream))
						printf("FILE NOT WRITTEN\n");
					count++;
				}
				if (offset)
				{
					buffer = get_block(start_block + count);
					if (!fwrite(buffer, 1, offset, fstream))
						printf("FILE NOT WRITTEN\n");
					count++;
				}
				else
				{
					buffer = get_block(start_block + count);
					if (!fwrite(buffer, 1, size, fstream))
						printf("FILE NOT WRITTEN\n");
					count++;
				}
				fclose(fstream);
			}
			else
				printf("FILE NOT OPENED");
			break;
		}
	}
}

struct Metadata* delete_file(struct Metadata *metadata,char *filename)
{
	struct Metadata *temp = metadata;
	for (int file = 0; file < temp->no_of_files; file++)
	{
		if (!strcmp(temp->files[file].filename, filename))
		{
			temp->no_of_files--;
			temp->free_blocks += temp->files[file].no_of_blocks;
			for (int index = temp->files[file].startblock; index < temp->files[file].no_of_blocks + temp->files[file].startblock; index++)
			{
				temp->vector[index] = 48;
			}
			memcpy(&temp->files[file], &temp->files[temp->no_of_files], sizeof(struct file_record));
			set_the_metadata_to_disk(metadata);
			break;
		}
	}
	return metadata;
}

struct Metadata* initialize_the_metadata(struct Metadata *metadata)
{
	char* buffer = (char*)malloc(sizeof(char)*size);
	struct Metadata *temp = metadata;
	temp->code = 0x444E524D;
	temp->no_of_files = 0;
	for (int index = 1; index < 6400; index++)
		temp->vector[index] = 48;
	temp->free_blocks = 6399;
	memcpy(buffer, temp, sizeof(struct Metadata));
	set_block(0, buffer);
	return temp;
}

struct Metadata* format(struct Metadata *metadata)
{
	char *command = (char*)malloc(sizeof(char) * 10);
	struct Metadata* temp = metadata;
	if (temp->code != 0x444E524D)
	{
		printf("DO YOU WANT TO FORMAT THE DISK : yes/no\n");
		gets(command);
		if (!strcmp(command, (char*)"yes"))
		{
			return initialize_the_metadata(temp);
		}
		return format(metadata);
	}
	return metadata;
}

char ** divide_into_tokens(char *command, int *no_of_tokens)
{
	char **tokens = (char**)malloc(sizeof(char) * 6);
	for (int index = 0; index < 6; index++)
		tokens[index] = (char*)malloc(sizeof(char) * 100);
	int token_index = 0;
	int token_point = 0;
	for (int index = 0; command[index] != '\0'; index++)
	{
		if (command[index] == 32)
		{
			tokens[token_index][token_point] = '\0';
			token_index++;
			token_point = 0;
		}
		else
		{
			tokens[token_index][token_point++] = command[index];
		}
	}
	tokens[token_index][token_point] = '\0';
	*no_of_tokens = token_index + 1;
	return tokens;
}

