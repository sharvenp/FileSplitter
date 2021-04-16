#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: assemble <meta_file> <save_file>\n");
        return 0;
    }

    // Check if meta file exists
    if(access(argv[1], F_OK) != 0) {
        fprintf(stderr, "metafile does not exist\n");
        return -1;
    } 

    struct stat st;
    stat(argv[1], &st);
    if (st.st_size == 0) {
        fprintf(stderr, "metadate is empty\n");
        return -1;
    }

    printf("Parsing meta file...\n");

    // Create meta file
    FILE* meta_file;
    if ((meta_file = fopen(argv[1], "rb")) == NULL) {
        perror(argv[1]);
        return -1;
    }

    // load meta data
    char split_dir[512];
    if (fgets(split_dir, 512, meta_file) == NULL) {
        fprintf(stderr, "metafile is corrupted\n");
        return -1;
    }
    split_dir[strcspn(split_dir, "\n")] = 0;

    char file_name[255];
    if (fgets(file_name, 255, meta_file) == NULL) {
        fprintf(stderr, "metafile is corrupted\n");
        return -1;
    }
    file_name[strcspn(file_name, "\n")] = 0;

    char file_size_str[128];
    if (fgets(file_size_str, 128, meta_file) == NULL) {
        fprintf(stderr, "metafile is corrupted\n");
        return -1;
    }
    file_size_str[strcspn(file_size_str, "\n")] = 0;
    int file_size = atoi(file_size_str);
    
    char chunks_str[128];
    if (fgets(chunks_str, 128, meta_file) == NULL) {
        fprintf(stderr, "metafile is corrupted\n");
        return -1;
    }
    chunks_str[strcspn(chunks_str, "\n")] = 0;
    int chunks = atoi(chunks_str);

    // Close the metadata file
    if (fclose(meta_file) == EOF) {
        perror(argv[1]);
        return EOF;
    }

    printf("Parsed meta file\n");

    printf("Loading chunks...\n");
    // Check if files exist
    for (int i = 0; i < chunks; i++) {
        char fn[255];
        sprintf(fn, "%s/%s-%d", split_dir, file_name, i);
        // Check if meta file exists
        if(access(fn, F_OK) != 0) {
            fprintf(stderr, "chunk mising: %s\n", fn);
            return -1;
        }
        struct stat chunk_st;
        stat(fn, &chunk_st);
        if (chunk_st.st_size == 0) {
            fprintf(stderr, "chunk[%d] is empty\n", i);
            return -1;
        }
    }

    // Load file chunks
    FILE** chunk_files;
    if ((chunk_files = malloc(sizeof(FILE*) * chunks)) == NULL) {
        perror("malloc: ");
        return -1;
    }

    int* chunk_sizes;
    if ((chunk_sizes = malloc(sizeof(int) * chunks)) == NULL) {
        perror("malloc: ");
        return -1;
    }

    for (int i = 0; i < chunks; i++) {
        char fn[255];
        sprintf(fn, "%s/%s-%d", split_dir, file_name, i);
        FILE* fp;
        if ((fp = fopen(fn, "rb")) == NULL) {
            perror(fn);
            return -1;
        }
        chunk_files[i] = fp;

        struct stat chunk_st;
        stat(fn, &chunk_st);
        chunk_sizes[i] = chunk_st.st_size;

        printf("Chunk[%d]: %d\n", i, chunk_sizes[i]);
    }

    // Reconstruct file
    printf("Reconstructing file...\n");

    // Load file chunks
    FILE* save_file;
    if ((save_file = fopen(argv[2], "wb")) == NULL) {
        perror(argv[2]);
        return -1;
    }

    int bytes = 0;
    for (int i = 0; i < chunks; i++) {
        
        char* buffer;
        if ((buffer = malloc((chunk_sizes[i]+1) * sizeof(char))) == NULL) {
            perror("malloc: ");
            return -1;
        }
        
        if (fread(buffer, 1, chunk_sizes[i], chunk_files[i]) < chunk_sizes[i]) {
            perror("fread");
            return -1;
        }

        if (fwrite(buffer, sizeof(char), chunk_sizes[i], save_file) < chunk_sizes[i]) {
            perror("fwrite");
            return -1;
        }
        
        printf("Merged chunk[%d]\n", i);

        bytes += chunk_sizes[i];

        free(buffer);
    }

    // Close the save file
    if (fclose(save_file) == EOF) {
        perror(argv[2]);
        return EOF;
    }

    if (bytes != file_size) {
        fprintf(stderr, "difference in true file size and observed file size\nexpected: %d\nobserved: %d\n", file_size, bytes);
        return -1;
    }

    printf("Done!\n");

    return 0;
}