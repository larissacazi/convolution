#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//Structures===================================================================
typedef enum {
	FALSE,
	TRUE
} BOOL;

typedef struct PGM {
	int column;		//Width
	int row;		//Height
	int max;		//Maximum gray value
	int **pgm;		//Gray values
	double *featureVector; 	//Feature vector
	int sizeVector;		//Size of the vector
	double class;  		//Image class
} PGM;

typedef struct ImageSet {
	int num;	//Number of Images
	PGM **pgm;	//Images Pointer Vector
} ImageSet;

typedef struct Mask {
	int **matrix;
} Mask;

typedef struct MaskSet {
	int num;
	Mask *mask;
} MaskSet;

//Reading Filenames
typedef struct FileSet {
	int num;
	char **file;
} FileSet;
//=============================================================================
//UTILS========================================================================
char *getSubstring(char *first, int idx, int len) {
	char *subStr = (char*)malloc(sizeof(char) * (len+1));
	
	//printf("getSubstring::first [%s], idx = %d, len = %d\n", first, idx, len);
	memcpy(subStr, &first[idx], len);
	subStr[len] = '\0';
	//printf("substr = %s\n", subStr);

	return subStr;
}

void printIntMatrix(int **m, int row, int column) {
	int i, j;

	for(i=0; i<row; i++) {
		for(j=0; j<column; j++)	{
			printf("%d ", m[i][j]);	
		}
		printf("\n");
	}
}

char *getLineFromFile(FILE *filePtr) {
	char *auxiliar = NULL;
	char c;
	int counter = 0;

	//printf("Entrou getLineFromFile.\n");
	
	do {
		c = fgetc(filePtr);
		if(c == EOF) {
			break;
		}
		//printf("counter %d\n ", counter);
		auxiliar = realloc(auxiliar, (counter+1)*sizeof(char));
		if(auxiliar != NULL) {
        		auxiliar[counter++] = c;
		}
	} while(c != '\n' && c != '\r');
	auxiliar[counter-1] = '\0';
	//printf("auxiliar %s\n", auxiliar);
	//printf("counter %d\n", counter);

	return auxiliar;
}

void printMatrixString(char **M, int n) {
	int i = 0;
	for(i=0; i<n; i++) printf("%s\n", M[i]);
}

int **generateMatrix(int row, int column) {
	int **m = NULL;
	int i = 0;

	m = (int**)malloc(sizeof(int*)*row);	

	for(i=0; i<row; i++) {
		m[i] = (int*)malloc(sizeof(int)*column);
	}

	return m;
}
//=============================================================================
//Mask Set=====================================================================
MaskSet *readMaskSet(int M, int m) {
	MaskSet *maskData = NULL;
	int i = 0, j = 0, k = 0;

	maskData = (MaskSet*)calloc(1, sizeof(MaskSet));
	maskData->num = M;
	maskData->mask = (Mask*)calloc(M, sizeof(Mask));

	for(i=0; i<M; i++) {//Reading each image
		maskData->mask[i].matrix = (int**)malloc(sizeof(int*)*m);
		for(j=0; j<m; j++) {
			maskData->mask[i].matrix[j] = (int*)malloc(sizeof(int)*m);
			for(k=0; k<m; k++) {
				scanf("%d", &maskData->mask[i].matrix[j][k]);
			}
		}
	}

	return maskData;
}

void printMaskSet(MaskSet *maskData, int m) {
	int i = 0, j = 0, k = 0;
	
	for(i=0; i<maskData->num; i++) {
		printf("----- Mask[%d]\n", i);
		for(j=0; j<m; j++) {
			for(k=0; k<m; k++) {
				printf("%d ", maskData->mask[i].matrix[j][k]);
			}
			printf("\n");
		}
		printf("-------------\n");
	}
}

void freeMaskSet(MaskSet *maskData, int m) {
	int i = 0, j = 0;
	
	for(i=0; i<maskData->num; i++) {
		for(j=0; j<m; j++) {
			free(maskData->mask[i].matrix[j]);
		}
		free(maskData->mask[i].matrix);
	}
	free(maskData->mask);
	free(maskData);
}
//=============================================================================
//FileSet======================================================================
FileSet *createFileSet() {
	FileSet *files = NULL;
	files = (FileSet*)calloc(1, sizeof(FileSet));

	return files;
}

void freeFileSet(FileSet *files) {
	int i = 0;

	for(i=0; i<files->num; i++) {
		free(files->file[i]);
	}
	free(files->file);
	free(files);
}

FileSet *readFileSet(char *filename) {
	FILE *filePtr = NULL;
	FileSet *files = NULL;
    char *line = NULL;
    char *aux = NULL;
    size_t size = 0;
    ssize_t nread;
    int len = 0;

    files = createFileSet();

	//Read from file
	if((filePtr = fopen(filename, "r")) == NULL) {
		exit(1);
	}

	while ((nread = getline(&line, &size, filePtr)) != -1) {
		files->file = (char**)realloc(files->file, sizeof(char*)*(files->num+1));
		
		len = strlen(line);
		files->file[files->num] = (char*)malloc(sizeof(char)*(len+1));
		
		if(line[len-1] == '\n') {
			aux = getSubstring(line, 0, len-1);
			strcpy(files->file[files->num], aux);
			free(aux);
		}
		else {
			strcpy(files->file[files->num], line);
		}
		files->num++;
	}

	free(line);
	fclose(filePtr);

	return files;
}

void printFileSet(FileSet *files) {
	printf("-----FileSet:: NUM [%d]\n", files->num);
	printMatrixString(files->file, files->num);
	printf("=======================\n");
}

//=============================================================================
//Image Set and PGM============================================================
ImageSet *createImageSet(int num) {
	ImageSet *images = NULL;
	images = (ImageSet*)calloc(1, sizeof(ImageSet));
	images->num = num;
	return images;
}

PGM *readPGM(char *filename) {
	FILE *filePtr = NULL;
	PGM *pgm = NULL;
	char pgmType[4];
	unsigned char c;
	int i, j;

	pgm = (PGM*)calloc(1, sizeof(PGM));

	printf("filename: %s", filename);

	//Read from file
	if((filePtr = fopen(filename, "rb")) == NULL) {
		exit(1);
	}

	printf("File[%d] opened\n", filename);
	fscanf(filePtr, "%2s", pgmType);
	printf("PGM Type %s.\n", pgmType);
	
	//Read columns, rows, max
	fscanf(filePtr, "%d", &pgm->column);
	fscanf(filePtr, "%d", &pgm->row);
	fscanf(filePtr, "%d", &pgm->max);

	pgm->pgm = (int**)malloc(sizeof(int*)*pgm->row);

	for(i=0; i<pgm->row; i++) {
		pgm->pgm[i] = (int*)malloc(sizeof(int)*pgm->column);
		for(j=0; j<pgm->column; j++) {
			fread(&c, sizeof(unsigned char), 1, filePtr);
			pgm->pgm[i][j] = c;
		}
	}

	fclose(filePtr);

	return NULL;

}

PGM **readPGMVector(FileSet *fileSet) {
	int i = 0;
	PGM **pgmVector = NULL;

	pgmVector = (PGM**)malloc(sizeof(PGM*)*fileSet->num);

	printf("readPGMVector::TrainData:\n");
	printFileSet(fileSet);

	for(i=0; i<fileSet->num; i++) {
		printf("Image number %d - filename::%s\n", i, fileSet->file[i]);
		pgmVector[i] = readPGM(fileSet->file[i]);
	}

	return pgmVector;
}

void printImageSet(ImageSet *images) {
	int i = 0;

	for(i=0; i<images->num; i++) {
		printf("---- PGM[%d]:\n", i);
		printf("\tColumn [%d]\n", images->pgm[i]->column);
		printf("\tRow [%d]\n", images->pgm[i]->row);
		printf("\tClass [%.2lf]\n", images->pgm[i]->class);
		printf("\tMax [%d]\n", images->pgm[i]->max);
		printf("\t Matrix:\n");
		printIntMatrix(images->pgm[i]->pgm, images->pgm[i]->row, images->pgm[i]->column);
		printf("\n");
		printf("-------------\n");
	}
}

void freePGM(PGM *pgm) {
	int i = 0;

	for(i=0; i<pgm->row; i++) {
		free(pgm->pgm[i]);
	}
	free(pgm->pgm);
	free(pgm);
}

void freePGMVector(PGM **pgm, int num) {
	int i = 0;

	for(i=0; i<num; i++) {
		freePGM(pgm[i]);
	}
	free(pgm);
}

//=============================================================================
//Main Function================================================================
int main() {
	char trainFilename[31], testFilename[31], classFilename[31];
	int M, m, K;
	MaskSet *maskSet = NULL;
	FileSet *trainData = NULL;
	FileSet *testData = NULL;
	ImageSet *test = NULL;
	ImageSet *train = NULL;

	//Reading filenames
	scanf("%s", trainFilename);
	scanf("%s", classFilename);
	scanf("%s", testFilename);
	//printf("trainFilename[%s] - classFilename[%s] - testFilename[%s]\n", trainFilename, classFilename, testFilename);

	//Reading M number of mask
	scanf("%d", &M);
	//Reading dimension m of mask
	scanf("%d", &m);
	//Reading ALL mask
	maskSet = readMaskSet(M, m);
	//Reading K value
	scanf("%d", &K);

	//Working with Data
	trainData = readFileSet(trainFilename);
	testData = readFileSet(testFilename);

	//ImageSet
	printf("createImageSet::\n");
	test = createImageSet(testData->num);
	train = createImageSet(trainData->num);
	//test->pgm = readPGMVector(testData);
	printf("TrainData:\n");
	printFileSet(trainData);

	printf("Reading Train Images:\n");
	train->pgm = readPGMVector(trainData);
	printf("Train Image Set:\n");
	printImageSet(train);

	//printf("TrainData:\n");
	//printFileSet(trainData);
	//printf("TestData:\n");
	//printFileSet(testData);
	//printMaskSet(maskSet, m);
	//printf("K[%d] M[%d] m[%d]\n", K, M, m);


	//Free everything
	freeMaskSet(maskSet, m);

	return 0;
}