#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

int main() {
    DIR* dir;
    struct dirent* curr;
    struct stat buf;
    long long total_size = 0;
    long long size;
    
    dir = opendir(".");
    rewinddir(dir);

    while((curr = readdir(dir))) {
        stat(curr->d_name, &buf);
        if(!S_ISDIR(buf.st_mode)) {
            size = buf.st_size;
            printf("\nRozmiar pliku: %lld\t Nazwa pliku: %s", size, curr->d_name);
            total_size += size;
        }
    }

    printf("\n\nSumaryczny rozmiar plikow: %lld\n\n", total_size);

    closedir(dir);
    return 0;
}