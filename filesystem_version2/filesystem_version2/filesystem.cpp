#include"filesystemheader.h"

int main()
{
	char *command = (char*)malloc(sizeof(char) * 100);
	char **tokens = (char**)malloc(sizeof(char) * 6);
	for (int index = 0; index < 6; index++)
		tokens[index] = (char*)malloc(sizeof(char) * 100);
	int no_of_tokens = 0;
	struct Metadata *metadata = (struct Metadata*)malloc(sizeof(struct Metadata));
	metadata = get_the_initial_metadata(metadata);
	metadata = format(metadata);
	while (1)
	{
		gets(command);
		tokens = divide_into_tokens(command, &no_of_tokens);
		if (no_of_tokens == 1 && !strcmp(tokens[0], "list"))
			list_all_the_files(metadata);
		else if (no_of_tokens == 1 && !strcmp(tokens[0], "debug"))
			debug(metadata);
		else if (no_of_tokens == 3 && !strcmp(tokens[0], "copytodisk"))
			metadata=copy_file_to_disk(metadata,tokens[1],tokens[2]);
		else if (no_of_tokens == 3 && !strcmp(tokens[0], "copyfromdisk"))
			copy_file_from_disk(metadata, tokens[1], tokens[2]);
		else if (no_of_tokens == 2 && !strcmp(tokens[0], "delete"))
			metadata=delete_file(metadata,tokens[1]);
		else if (no_of_tokens == 1 && !strcmp(tokens[0], "format"))
			metadata=format(metadata);
		else if (no_of_tokens == 1 && !strcmp(tokens[0], "exit"))
			break;
	}
	return 0;
}