#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("Usage: split <file_path> <number_of_chunks> <save_dir>\n");
        return 0;
    }

    // Check if save directory exists
    DIR* dir = opendir(argv[3]);
    if (dir) {
        closedir(dir);
    } else if (ENOENT == errno) {
        perror(argv[3]);
        return ENOENT;
    } else {
        perror(argv[3]);
        return 1;
    }

    // Open the file
    FILE* file;
    if ((file = fopen(argv[1], "rb")) == NULL) {
        perror(argv[1]);
        return 1;
    }
    
    // Hold the data in the buffer
    struct stat st;
    stat(argv[1], &st);
    int file_size = st.st_size;
    int chunks = atoi(argv[2]);
    int chunk_size = file_size / chunks;

    printf("Splitting: %s\n", argv[1]);
    printf("File size: %dB\n", file_size);
    printf("Chunk count: %d\n", chunks);
    printf("Chunk size limit: %dB\n", chunk_size);
    printf("** Starting Splitting Process **\n");
    
    int bytes = file_size; // Keep track of bytes read so far
    int chunk_i = 0; // Keep track of chunk index
    char *buffer; // Store the chunk

    fseek(file, 0, SEEK_SET);

    // Write the chunks
    while (bytes > 0) {
        
        int buffer_size;
        if (bytes > chunk_size) {
            buffer_size = chunk_size;
        } else {
            buffer_size = bytes;
        }

        if ((buffer = malloc((buffer_size+1) * sizeof(char))) == NULL) {
            perror("malloc: ");
            fclose(file);
            return 1;
        }
        
        fread(buffer, (buffer_size), sizeof(char), file);

        // Create file name
        char chunk_dest[512];
        char chunk_name[255];
        memcpy(chunk_dest, argv[3], 255);
        sprintf(chunk_name, "/%s-%d", argv[1], chunk_i);
        strncat(chunk_dest, chunk_name, 512);

        // Write to the file
        FILE* chunk_file;
        if ((chunk_file = fopen(chunk_dest, "wb")) == NULL) {
            perror(chunk_dest);
            fclose(file);
            return 1;
        }
        
        int written = fwrite(buffer, sizeof(char), sizeof(char) * buffer_size, chunk_file);
        if (written < sizeof(char) * buffer_size) {
            perror(chunk_dest);
            fclose(file);
            fclose(chunk_file);
            return 1;
        }

        // Close the file
        if (fclose(chunk_file) == EOF) {
            perror(argv[1]);
            fclose(file);
            return EOF;
        }

        printf("Chunk[%d]: %s-%d (Size: %d B)\n", chunk_i, argv[1], chunk_i, buffer_size);

        bytes -= buffer_size;
        chunk_i++;

        free(buffer);
    }

    // Close the file
    if (fclose(file) == EOF) {
        perror(argv[1]);
        return EOF;
    }

    printf("** Done! **\n");

    return 0;
}