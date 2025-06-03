#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Define a simple inode structure
typedef struct {
    char filename[256];
    uint32_t size;
    uint32_t block_start;
} inode_t;

// Define a simple filesystem structure
typedef struct {
    inode_t *inodes;
    uint32_t inode_count;
} filesystem_t;

// Initialize the filesystem
filesystem_t* init_filesystem() {
    filesystem_t *fs = (filesystem_t*)malloc(sizeof(filesystem_t));
    fs->inodes = (inode_t*)malloc(sizeof(inode_t) * 100); // Allocate space for 100 inodes
    fs->inode_count = 0;
    return fs;
}

// Create a new file
void create_file(filesystem_t *fs, const char *filename, uint32_t size, uint32_t block_start) {
    if (fs->inode_count < 100) {
        inode_t *inode = &fs->inodes[fs->inode_count++];
        strncpy(inode->filename, filename, 256);
        inode->size = size;
        inode->block_start = block_start;
    }
}

int main() {
    filesystem_t *fs = init_filesystem();
    create_file(fs, "test.txt", 1024, 0);
    // Add more filesystem operations here
    free(fs->inodes);
    free(fs);
    return 0;
}


// Define a simple inode structure
typedef struct {
    char filename[256];
    uint32_t size;
    uint32_t block_start;
} inode_t;

// Define a simple filesystem structure
typedef struct {
    inode_t *inodes;
    uint32_t inode_count;
} filesystem_t;

// Initialize the filesystem
filesystem_t* init_filesystem() {
    filesystem_t *fs = (filesystem_t*)malloc(sizeof(filesystem_t));
    if (!fs) {
        fprintf(stderr, "Failed to allocate memory for filesystem.\n");
        exit(EXIT_FAILURE);
    }

    fs->inodes = (inode_t*)malloc(sizeof(inode_t) * 100); // Allocate space for 100 inodes
    if (!fs->inodes) {
        fprintf(stderr, "Failed to allocate memory for inodes.\n");
        free(fs);
        exit(EXIT_FAILURE);
    }

    fs->inode_count = 0;
    return fs;
}

// Create a new file
void create_file(filesystem_t *fs, const char *filename, uint32_t size, uint32_t block_start) {
    if (fs->inode_count < 100) {
        inode_t *inode = &fs->inodes[fs->inode_count++];
        strncpy(inode->filename, filename, sizeof(inode->filename) - 1);
        inode->filename[sizeof(inode->filename) - 1] = '\0';  // Ensure null termination
        inode->size = size;
        inode->block_start = block_start;
    } else {
        fprintf(stderr, "Inode limit reached. Cannot create more files.\n");
    }
}

// Print all files in the filesystem
void list_files(filesystem_t *fs) {
    printf("Filesystem contents:\n");
    for (uint32_t i = 0; i < fs->inode_count; ++i) {
        inode_t *inode = &fs->inodes[i];
        printf("File: %s | Size: %u bytes | Block Start: %u\n",
               inode->filename, inode->size, inode->block_start);
    }
}

int main() {

    filesystem_t *fs = init_filesystem();

    // Create some files
    create_file(fs, "test.txt", 1024, 0);
    create_file(fs, "image.png", 2048, 10);
    create_file(fs, "video.mp4", 4096, 20);

    // List files
    list_files(fs);

    // Clean up
    free(fs->inodes);
    free(fs);

    return 0;
}
