#include "readconfig.h"

bool
readConfigFile(FILE* configFile, Matrix* mtr)
{

    // check the file is NULL or not
    if (configFile == NULL){
        fprintf(stderr, "Error: config file is NULL\n");
        return false;
    }

    // read address count
    mtr->ac = readAddressCount(configFile);
    if (mtr->ac == 0){
        fprintf(stderr, "Error: Can not read address count\n");
        return false;
    }

    // read character count
    mtr->cc = readCharacterCount(configFile);
    if (mtr->cc == 0){
        fprintf(stderr, "Error: can not read character count\n");
        return false;
    }

    // read the thres
    mtr->thres = readThres(configFile);
    if (mtr->thres == 0){
        mtr->thres = 0;
    }

    // read the addresses
    readAddresses(configFile, mtr->ac, &mtr->addresses);
    if(mtr->addresses == NULL){
        fprintf(stderr, "Error: can not read addresses from config file.\n");
        return false;
    }

    // read thresholds
    readThresholds(configFile, mtr->ac, &mtr->thresholds);
    if (mtr->thresholds == NULL){
        fprintf(stderr, "Error: can not read thresholds from config file.\n");
        return false;
    }

    // read characters
    readCharacters(configFile, mtr->cc, &mtr->characters);
    if (mtr->characters == NULL){
        fprintf(stderr, "Error: can not read characters from config file.\n");
        return false;
    }

    // read Matrix values
    readMatrixValues(configFile, mtr->ac, mtr->cc, &mtr->matrixvalues);
    if (mtr->matrixvalues == NULL){
        fprintf(stderr, "Error: can not read matrixvalues from config file.\n");
        return false;
    }

    return true;
}

int
getOffset(FILE* file, char* scanstring)
{
    int t,offset = 0;
    char tmpstr[LINECOUNT];
    bool findstr = false;

    if (file == NULL){
        return -1;
    }

    t = ftell(file);

    fseek(file,0,SEEK_SET);
    while(fgets(tmpstr, LINECOUNT,file)!=NULL){
        if (strstr(tmpstr, scanstring) != NULL){
            offset += strlen(scanstring);
            findstr = true;
            break;
        }else{
            offset += strlen(tmpstr);
        }
    }

    if(!findstr)
        offset = -1;

    fseek(file,t,SEEK_SET);

    return offset;
}

int
readNextInt(FILE* file)
{
    char c;
    int t;
    while((t=fgetc(file))!=EOF){
        c = (char) t;
        if (c == '-'||c =='+'||( c <= '9' && c >= '0')){
            fseek(file,-1,SEEK_CUR);
            fscanf(file,"%d",&t);
            return t;
        }
    }
    return 0;
}

int64_t
readNextHexInt(FILE* file)
{
    char c;
    unsigned int t;
    int tc;
    while((tc=fgetc(file))!=EOF){
        c = (char) tc;
        if (c == '-'||c=='+'||(c<='9'&&c>='0')||(c>= 'a'&&c<='f')){
            fseek(file,-1,SEEK_CUR);
            fscanf(file,"%x",&t);
            return (int64_t)t;
        }
    }
    return 0;
}

char*
readNextWord(FILE* file)
{
    char c,*word;
    int wc=0,tc;
    word = (char*)malloc(sizeof(char)*WORDLENGTH);
    while((tc=fgetc(file))!=EOF){
        c = (char) tc;
        if ( c >= 'a' && c <= 'z'){
            word[wc++]=c;
        }else if(wc!=0){
            word[wc] = '\0';
            break;
        }
    }
    if (wc ==0){
        free(word);
        word = NULL;
    }
    return word;

}

size_t
readAddressCount(FILE* configFile)
{
    fseek(configFile, getOffset(configFile, AC_HEADER), SEEK_SET);
    return (size_t) readNextInt(configFile);
}

size_t
readCharacterCount(FILE* configFile)
{
    fseek(configFile, getOffset(configFile, CC_HEADER), SEEK_SET);
    return (size_t) readNextInt(configFile);    
}

uint64_t
readThres(FILE* configFile)
{
    fseek(configFile, getOffset(configFile, THRES_HEADER), SEEK_SET);
    return (uint64_t) readNextInt(configFile);
}

void
readAddresses(FILE* configFile, size_t addressCount, size_t** addresses)
{
    size_t i;
    *addresses = (size_t *)malloc(sizeof(size_t)*addressCount);
    fseek(configFile, getOffset(configFile, ADDRESSES_HEADER), SEEK_SET);
    for (i = 0; i < addressCount; ++i)
    {
        (*addresses)[i] = (size_t)readNextHexInt(configFile);
        if ((*addresses)[i] == 0){
            free(*addresses);
            *addresses = NULL;
            return;
        }
    }

    return;
}

void
readThresholds(FILE* configFile, size_t addressCount, uint64_t** thresholds)
{
    size_t i;
    *thresholds = (uint64_t *)malloc(sizeof(uint64_t)*addressCount);
    fseek(configFile, getOffset(configFile, THRESHOLDS_HEADER), SEEK_SET);
    for (i = 0; i < addressCount; ++i)
    {
        (*thresholds)[i] = (uint64_t)readNextInt(configFile);
        if ((*thresholds)[i] == 0){
            free(*thresholds);
            *thresholds = NULL;
            return;
        }
    }

    return;
}

void
readCharacters(FILE* configFile, size_t characterCount, char*** characters)
{
    size_t i;
    *characters = (char**)malloc(sizeof(char*)*characterCount);
    fseek(configFile, getOffset(configFile, CHARACTERS_HEADER), SEEK_SET);
    for (i = 0; i < characterCount; ++i)
    {
        (*characters)[i] = readNextWord(configFile);
        if ((*characters)[i] == NULL){
            while(true){
                --i;
                free((*characters)[i]);
                (*characters)[i] = NULL;
                if(i==0)
                    break;
            }
            free(*characters);
            *characters = NULL;
            return;
        }
    }

    return;
}

void
readMatrixValues(FILE* configFile, size_t addressCount, size_t characterCount, size_t*** matrixValues)
{
    size_t i, j;
    *matrixValues = (size_t **)malloc(sizeof(size_t*)*characterCount);
    fseek(configFile, getOffset(configFile, MATRIXVALUES_HEADER), SEEK_SET);
    for (i = 0; i < characterCount; ++i)
    {
        (*matrixValues)[i] = (size_t *)malloc(sizeof(size_t)*addressCount);
        for (j = 0; j < addressCount; j++)
        {
            (*matrixValues)[i][j]= readNextInt(configFile);
        }
    }

    return;
}

