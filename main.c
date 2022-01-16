#include <stdio.h>
#include <string.h>

struct header{
    char namesize;
    char filename[255];
    char filesize[4];
};
union fileheader{
    char buffer[260];
    struct header header;
};
int getfilesize(FILE* file){
    int i=0;
    char c;
    while(fread(&c, 1, 1, file)){
        ++i;
    }
    fseek(file, 0, SEEK_SET);
    return i;
}

void create(char** filesnames, int filescount, FILE* archive){
    fwrite("M", 1, 1, archive);
    fwrite("Y", 1, 1, archive);
    fwrite("A", 1, 1, archive);
    fwrite("R", 1, 1, archive);
    fwrite("C", 1, 1, archive);
    char cfilescount=(char) filescount;
    fwrite(&cfilescount, 1, 1, archive);
    FILE* files;
    int allfilessize=0;
    for(int i=0; i<filescount; ++i){
        files=fopen(filesnames[i], "rb");
        int filesize;
        filesize=getfilesize(files);
        allfilessize+=filesize;
    }
    char filessizebytes[4];
    allfilessize+=filescount*260;
    for(int i=3; i>=0; --i){
        filessizebytes[i]=allfilessize%128;
        allfilessize=allfilessize/128;
    }
    for(int i=0; i<4; ++i){
        fwrite(&filessizebytes[i], 1, 1,archive);
    }
    for(int i=0; i<filescount; ++i){
        union fileheader fileheader;
        int namesize=strlen(filesnames[i]);
        fileheader.header.namesize=namesize;
        for(int j=0; j<namesize; ++j){
            fileheader.header.filename[j]=filesnames[i][j];
        }
        files=fopen(filesnames[i], "rb");
        int filesize;
        filesize=getfilesize(files);
        char filesizebytes[4];
        for(int j=3; j>=0; --j){
            filesizebytes[j]=filesize%128;
            filesize=filesize/128;
        }
        for(int j=0; j<4; ++j){
            fileheader.header.filesize[j]=filesizebytes[j];
        }
        for(int j=0; j<260; ++j){
            fwrite(&fileheader.buffer[j], 1, 1, archive);
        }
        filesize=getfilesize(files);
        char c;
        for(int j=0; j<filesize; ++j){
            fread(&c, 1, 1, files);
            fwrite(&c, 1, 1, archive);
        }
    }
}

void list(FILE* archive){
    fseek(archive, 5, SEEK_SET);
    char filescount;
    fread(&filescount, 1, 1, archive);
    fseek(archive, 4, SEEK_CUR);
    for(int i=0; i<filescount; ++i){
        char namesize;
        fread(&namesize, 1, 1, archive);
        char name[namesize];
        fread(&name, 1, namesize, archive);
        for(int j=0; j<namesize; ++j){
            printf("%c", name[j]);
        }
        printf("\n");
        fseek(archive, -1*namesize, SEEK_CUR);
        fseek(archive, 255, SEEK_CUR);
        int filesize=0;
        char sizebytes;
        for(int j=0; j<4; ++j){
            fread(&sizebytes, 1, 1, archive);
            filesize=filesize*128+sizebytes;
        }
        fseek(archive, filesize, SEEK_CUR);
    }
}

void extract(FILE* archive){
    fseek(archive, 5, SEEK_SET);
    char filescount;
    fread(&filescount, 1, 1, archive);
    fseek(archive, 4, SEEK_CUR);
    for(int i=0; i<filescount; ++i){
        char namesize;
        fread(&namesize, 1, 1, archive);
        char name[namesize];
        fread(&name, 1, namesize, archive);
        fseek(archive, -1*namesize, SEEK_CUR);
        fseek(archive, 255, SEEK_CUR);
        int filesize=0;
        char sizebytes;
        for(int j=0; j<4; ++j){
            fread(&sizebytes, 1, 1, archive);
            filesize=filesize*128+sizebytes;
        }
        FILE* fout;
        fout=fopen(name, "wb");
        char c;
        for(int j=0; j<filesize; ++j){
            fread(&c, 1, 1, archive);
            fwrite(&c, 1, 1,fout);
        }
    }
}

int main(int argv, char** argc) {
    char* arcname;
    arcname=argc[2];
    char* command;
    command=strpbrk(argc[3], "-")+2;
    if(strcmp(command, "create")==0){
        char c;
        char *cp;
        cp=&c;
        char **filesnames = &cp;
        for(int i=4; i<=argv; ++i){
            filesnames[i-4]=argc[i];
        }
        FILE* archive=fopen(arcname, "wb");
        if(archive==NULL) printf("null\n");
        create(filesnames, argv-4, archive);
    }else{
        if(strcmp(command, "list")==0){
            FILE* archive=fopen(arcname, "rb");
            list(archive);
        }else{
            if(strcmp(command, "extract")==0){
                FILE* archive=fopen(arcname, "rb");
                extract(archive);
            }else{
                printf("wrong command\n");
            }
        }
    }
    return 0;
}