#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 50

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
    char FileName[30];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int Permission;     //1   2   3
    struct inode *next;
}INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;           //1   2   3
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

int strlenX(char *str)
{
    int iRet = 0;

    while(*str != '\0')
    {
        iRet++;
        str++;
    }

    return iRet;
}

void man(char *name)
{
    if(name == NULL)
    {
        printf("Please Enter Valid Name\n");
        return;
    }

    if(strcmp(name,"create") == 0)
    {
        printf("Description: Used To Create New Regular File\n");
        printf("Usage: create file_Name permission\n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Description: Used To Read Data From Regular File\n");
        printf("Usage: read file_Name number_of_bytes_to_read\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description: Used To Write Into Regular File\n");
        printf("Usage: write file_Name\nAfter this enter the data that you want to write\n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description: Used To List All Information Of File\n");
        printf("Usage: ls\n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description: Used To Display Information Of File\n");
        printf("Usage: stat file_Name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description: Used To Display Information Of File\n");
        printf("Usage: fstat file_descriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description: Used To Remove Data From File\n");
        printf("Usage: truncate file_Name\n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description: Used To Open Existing File\n");
        printf("Usage: open file_Name mode\n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description: Used To Close Open File\n");
        printf("Usage: close file_Name\n");
    }
    else if(strcmp(name,"closeall") == 0)
    {
        printf("Description: Used To Close All Open Files\n");
        printf("Usage: closeall\n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description: Used To Delete The File\n");
        printf("Usage: rm file_Name\n");
    }
    else if(strcmp(name,"lseek") == 0)
    {
        printf("Description: Used To Change File Offset\n");
        printf("Usage: lseek file_name ChangeInOffset StartPoint\n");
    }
    else
    {
        printf("ERROR: No Manual Entry Available\n");
    }
}

void DisplayHelp()
{
    printf("create: To Create New File\n");
    printf("man: To Get Manual Of All Files\n");
    printf("ls: To List Out All Files\n");
    printf("clear: To Clear Console\n");
    printf("open: To Open The File\n");
    printf("close: To Close The File\n");
    printf("closeall: To Close All Open Files\n");
    printf("read: To Read The Content From Files\n");
    printf("write: To Write Contents Into File\n");
    printf("exit: To Termnate File System\n");
    printf("stat: To Display Information Of File Using Name\n");
    printf("fstat: To Display Information Of File Using File Description\n");
    printf("truncate: To Remove All Data From File\n");
    printf("rm: To Delete The File\n");
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
            break;
        }
        i++;
    }

    if(i == 50)
    {
        return -1;
    }
    else
    {
        return i;
    }
    return 0;
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
    {
        return NULL;
    }

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;
    }

    return temp;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while( i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    printf("DILB Created Successfully!\n");
}

void InitialiseSuperBlock()
{
    int i = 0;
    while( i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }
}

int CreateFile(char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if((name == NULL) || (permission == 0) || (permission > 3))
    {
        return -1;
    }

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        return -2;
    }

    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name) != NULL)
    {
        return -3;
    }

    while(temp != NULL)
    {
        if(temp->FileType == 0)
        {
            break;
        }
        temp = temp->next;
    }

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;
}

int rm_File(char *name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if(fd == -1)
    {
        return -1;
    }

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable);
        UFDTArr[fd].ptrfiletable = NULL;
    }
    return 0;
}

int ReadFile(int fd, char *arr, int iSize)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }
    
    if((UFDTArr[fd].ptrfiletable->mode != READ) && (UFDTArr[fd].ptrfiletable->mode != READ + WRITE))
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ) && (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE))
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
    {
        return -3;
    }

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
    {
        return -4;
    }

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if(read_size < iSize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),iSize);

        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + iSize;
    }

    return iSize;
}

int WriteFile(int fd, char *arr, int iSize)
{
    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
    {
        return -3;
    }

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, iSize);
    
    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize + 1;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;

    return iSize;
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
    {
        return -1;
    }

    temp = Get_Inode(name);
    if(temp == NULL)
    {
        return -2;
    }

    if(temp->Permission < mode)
    {
        return -3;
    }

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)
    {
        printf("Unable To Allocate Memory\n");
        return -1;
    }
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;

    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

void CloseFileByName(int fd)
{}

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);

    if( i == -1)
    {
        return -1;
    }

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

void CloseAllFile()
{
    int i = 0;
    while( i < 50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable -> readoffset = 0;
            UFDTArr[i].ptrfiletable -> writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
        }
        i++;
    }
}

int LseekFile(int fd, int iSize, int from)
{
    if((fd < 0) || (from > 2))
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + iSize) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->readoffset) + iSize) < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + iSize;
        }
        else if(from == START)
        {
            if(iSize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                return -1;
            }
            if(iSize < 0)
            {
                return -1;
            }
            (UFDTArr[fd].ptrfiletable->readoffset) = iSize;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize > MAXFILESIZE)
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize) < 0)
            {
                return -1;
            }

            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + iSize;
        }
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize ) > MAXFILESIZE)
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize ) < 0)
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize ) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;
                (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;
            }
        }
        else if(from == START)
        {
            if(iSize > MAXFILESIZE)
            {
                return -1;
            }
            if(iSize < 0)
            {
                return -1;
            }
            if(iSize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = iSize;
                (UFDTArr[fd].ptrfiletable->writeoffset) = iSize;
            }
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize < MAXFILESIZE)
            {
                return -1;
            }
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + iSize) < 0)
            {
                return -1;
            }
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;
        }
    }
    return 0;
}

void ls_file()
{
    int i = 0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("ERROR: There are no files left\n");
        return;
    }

    printf("\nFile Name\tInode number\tFile Size\tLink Count\n");
    printf("--------------------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("--------------------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if(fd < 0 || fd > MAXINODE)
    {
        return -1;
    }

    if(UFDTArr[i].ptrfiletable == NULL)
    {
        return -2;
    }

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n---------------------Statistical Information About File---------------------\n");
    printf("File Name: %s\n",temp->FileName);
    printf("Inode Number: %d\n",temp->InodeNumber);
    printf("File Size: %d\n",temp->FileSize);
    printf("Actual File Size: %d\n",temp->FileActualSize);
    printf("Link Count: %d\n",temp->LinkCount);
    printf("Reference Count: %d\n",temp->ReferenceCount);

    if(temp->Permission == 1)
    {
        printf("File Permission: Read Only\n");
    }
    else if(temp->Permission == 2)
    {
        printf("File Permission: Write Only\n");
    }
    else if(temp->Permission == 3)
    {
        printf("File Permission: Read & Write\n");
    }
    printf("------------------------------------------------------------------------------\n\n");

    return 0;
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
    {
        return -1;
    }

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;
    }

    if(temp == NULL)
    {
        return -2;
    }

    printf("\n---------------------Statistical Information About File---------------------\n");
    printf("File Name: %s\n",temp->FileName);
    printf("Inode Number: %d\n",temp->InodeNumber);
    printf("File Size: %d\n",temp->FileSize);
    printf("Actual File Size: %d\n",temp->FileActualSize);
    printf("Link Count: %d\n",temp->LinkCount);
    printf("Reference Count: %d\n",temp->ReferenceCount);

    if(temp->Permission == 1)
    {
        printf("File Permission: Read Only\n");
    }
    else if(temp->Permission == 2)
    {
        printf("File Permission: Write Only\n");
    }
    else if(temp->Permission == 3)
    {
        printf("File Permission: Read & Write\n");
    }
    printf("------------------------------------------------------------------------------\n\n");

    return 0;
}

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);

    if(fd == -1)
    {
        return -1;
    }

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,50);
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

int main()
{
    char *ptr = NULL;
    int iRet = 0, iFD = 0, iCount = 0;
    char command[4][80], str[80], arr[50];

    InitialiseSuperBlock();
    CreateDILB();

    while(1)
    {
        fflush(stdin);
        strcpy(str,"");

        printf("\nMaverick VFS :>");

        fgets(str,80,stdin);    //scanf("%[^'\n']s",str); also can be used

        iCount = sscanf(str,"%s %s %s %s", command[0], command[1], command[2], command[3]);

        if(iCount == 1)
        {
            if(strcmp(command[0],"ls") == 0)
            {
                ls_file();
            }
            else if(strcmp(command[0],"closeall") == 0)
            {
                CloseAllFile();
                printf("All Files Closed Successfully\n");
                continue;
            }
            else if(strcmp(command[0],"clear") == 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0],"help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0],"exit") == 0)
            {
                printf("Terminating Maverick Virtual File System\n");
                break;
            }
            else
            {
                printf("Command Not Found\n");
                continue;
            }
        }
        else if(iCount == 2)
        {
            if(strcmp(command[0],"stat") == 0)
            {
                iRet = stat_file(command[1]);

                switch(iRet)
                {
                    case (-1):
                        printf("ERROR: Please Enter Name\n");
                        break;

                    case (-2):
                        printf("ERROR: There is no such file\n");
                        break;
                }
                continue;
            }
            else if(strcmp(command[0],"fstat") == 0)
            {
                iRet = fstat_file(atoi(command[1]));

                switch(iRet)
                {
                    case (-1):
                        printf("ERROR: Please Enter Valid Parameters\n");
                        break;

                    case (-2):
                        printf("ERROR: There is no such file\n");
                        break;
                }
                continue;
            }
            else if(strcmp(command[0],"close") == 0)
            {
                iRet = CloseFileByName(command[1]);

                if(iRet == -1)
                {
                    printf("ERROR: There Is No Such File\n");
                }
                continue;
            }
            else if(strcmp(command[0],"rm") == 0)
            {
                iRet = rm_File(command[1]);

                if(iRet == -1)
                {
                    printf("ERROR: There Is No Such File\n");
                }
                continue;
            }
            else if(strcmp(command[0],"man") == 0)
            {
                man(command[1]);
            }
            else if(strcmp(command[0],"write") == 0)
            {
                iFD = GetFDFromName(command[1]);
                if(iFD == -1)
                {
                    printf("ERROR: Incorrect Parameter");
                    continue;
                }
                printf("Enter The Data:\n");
                scanf("%[^'\n']s",arr);

                iRet = strlenX(arr);
                if(iRet == 0)
                {
                    printf("ERROR: String Is Empty\n");

                    continue;
                }

                iRet = WriteFile(iFD, arr, iRet);

                switch(iRet)
                {
                    case -1:
                        printf("ERROR: File Does Not Have Permission To Write\n");
                        break;
                    case -2:
                        printf("ERROR: There Is No Sufficient Memory To Write\n");
                        break;
                    case -3:
                        printf("ERROR: It Is Not Regular File\n");
                        break;
                }
            }
            else if(strcmp(command[0],"truncate") == 0)
            {
                iRet = truncate_File(command[1]);

                if(iRet == -1)
                {
                    printf("ERROR: There Is No Such File\n");
                }
            }
            else
            {
                printf("ERROR: Command Not Found\n");
                continue;
            }
        }
        else if(iCount == 3)
        {
            if(strcmp(command[0],"create") == 0)
            {
                iRet = CreateFile(command[1], atoi(command[2]));
                if(iRet >= 0)
                {
                    printf("File %s Sucessfully Created With File Desciptor: %d\n",command[1] ,iRet);
                }

                if(iRet == -1)
                {
                    printf("ERROR: Incorrect Parameter\n");
                }

                else if(iRet == -2)
                {
                    printf("ERROR: There Are No Inodes Left\n");
                }

                else if(iRet == -3)
                {
                    printf("ERROR: File Already Exist\n");
                }

                else if(iRet == -4)
                {
                    printf("ERROR: Memory Allocation Failure\n");
                }

                continue;
            }
            else if(strcmp(command[0],"open") == 0)
            {
                iRet = OpenFile(command[1],atoi(command[2]));

                if(iRet >= 0)
                {
                    printf("File %s Is Successfully Opened With File Descriptor %d",command[1] ,iRet);
                }
                
                if(iRet == -1)
                {
                    printf("ERROR: Incorrect Parameter\n");
                }


                if(iRet == -2)
                {
                    printf("ERROR: File Not Present\n");
                }

                if(iRet == -3)
                {
                    printf("ERROR: Permission Denied\n");
                }
                continue;
            }
            else if(strcmp(command[0],"read") == 0)
            {
                iFD = GetFDFromName(command[1]);
                if(iFD == -1)
                {
                    printf("ERROR: File Does Not Exits\n");
                    continue;
                }

                ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
                if(ptr == NULL)
                {
                    printf("ERROR: Memory Allocation Failure\n");
                    continue;
                }

                iRet = ReadFile(iFD, ptr, atoi(command[2]));
                if(iRet == -1)
                {
                    printf("ERROR: File Does Not Exits\n");
                }
                if(iRet == -2)
                {
                    printf("ERROR: Permission Denied\n");
                }
                if(iRet == -3)
                {
                    printf("ERROR: Reached At End Of File\n");
                }
                if(iRet == -4)
                {
                    printf("ERROR: It Is Not Regular File\n");
                }
                if(iRet == 0)
                {
                    printf("ERROR: File Empty\n");
                }

                if(iRet > 0)
                {
                    write(2, ptr, iRet);
                }

                continue;
            }
            else
            {
                printf("\nERROR: Command Not Found!\n");
                continue;
            }
        }
        else if(iCount == 4)
        {
            if(strcmp(command[0],"lseek") == 0)
            {
                iFD = GetFDFromName(command[1]);
                if(iFD == -1)
                {
                    printf("ERROR: Incorrect Parameter\n");
                }
                iRet = LseekFile(iFD, atoi(command[2]), atoi(command[3]));
                if(iRet == -1)
                {
                    printf("ERROR: Unable To Perform lseek\n");
                }
            }
            else
            {
                printf("\nERROR: Command Not Found\n");
                continue;
            }
        }
        else
        {
            printf("ERROR: Command Not Found\n");
        }
    }
    return 0;
}