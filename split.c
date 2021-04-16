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
        return -1;
    }

    // Open the file
    FILE* file;
    if ((file = fopen(argv[1], "rb")) == NULL) {
        perror(argv[1]);
        return 1;
    }
    
    // Calculate sizes
    struct stat st;
    stat(argv[1], &st);
    int file_size = st.st_size;
    int chunks = atoi(argv[2]);
    int chunk_size = file_size / chunks;

    if (chunks == 0) {
        perror("number_of_chunks is 0");
        return -1;
    }

    printf("Splitting: %s\n", argv[1]);
    printf("File size: %dB\n", file_size);
    printf("Chunk size limit: %dB\n", chunk_size);
    printf("Chunk count: %d\n", chunks);
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
            return -1;
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
            return -1;
        }
        
        int written = fwrite(buffer, sizeof(char), sizeof(char) * buffer_size, chunk_file);
        if (written < sizeof(char) * buffer_size) {
            perror(chunk_dest);
            return -1;
        }

        // Close the file
        if (fclose(chunk_file) == EOF) {
            perror(argv[1]);
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

    // Create meta file
    printf("Creating metafile...\n");
    FILE* meta_file;
    char meta_file_name[255];
    sprintf(meta_file_name, "./%s-meta", argv[1]);
    if ((meta_file = fopen(meta_file_name, "wb")) == NULL) {
        perror(meta_file_name);
        return -1;
    }

    // Save meta data
    char* full_save_path = realpath(argv[3], NULL);
    fprintf(meta_file, "%s\n", full_save_path);
    fprintf(meta_file, "%s\n", argv[1]);
    fprintf(meta_file, "%d\n", file_size);
    fprintf(meta_file, "%d\n", chunk_i);

    // Close the file
    if (fclose(meta_file) == EOF) {
        perror(meta_file_name);
        return EOF;
    }

    printf("** Done! **\n");

    return 0;
}