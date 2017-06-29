#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define ASCII_MODE "P2"
#define log2(NUM)	log(NUM) / log(2)

typedef enum {
	FALSE,
	TRUE
} BOOL;

typedef struct PGM {
	int column;		//Width
	int row;		//Height
	int max;		//Maximum gray value
	unsigned char **pgm;		//Gray values
	double *featureVector; 	//Feature vector
	int sizeVector;		//Size of the vector
	double class;  		//Image class
} PGM;

typedef struct ImageSet {
	int num;	//Number of Images
	PGM **pgm;	//Images Pointer Vector
} ImageSet;

typedef struct FileSet {
	int num;
	char **file;
} FileSet;

typedef struct Mask {
	int **matrix;
} Mask;

typedef struct MaskSet {
	int num;
	Mask *mask;
} MaskSet;

//=======================================================================================================================================
//UTILS==================================================================================================================================
char *getLineFromFile(FILE *filePtr) {
	char *auxiliar = NULL;
	char c;
	int counter = 0;
	
	do {
		c = fgetc(filePtr);
		if(c == EOF) {
			//printf("Error3\n");
			return NULL;
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
//=======================================================================================================================================
//PGM====================================================================================================================================
PGM* readPGM(char *filename) {
	FILE *filePtr = NULL;
	PGM *pgm = NULL;
	char pgmType[50];
	int i, j;

	pgm = (PGM*)calloc(1, sizeof(PGM));

	//Read from file
	if((filePtr = fopen(filename, "r")) == NULL) {
		exit(1);
	}

	fscanf(filePtr, "%s", pgmType);
	//printf("PGM Type %s.\n", pgmType);
	if(strcmp(pgmType, ASCII_MODE) == 0) {
		//printf("P2 TYPE\n");
		//Read columns, rows, max
		fscanf(filePtr, "%d", &pgm->column);
		fscanf(filePtr, "%d", &pgm->row);
		fscanf(filePtr, "%d", &pgm->max);

		pgm->pgm = (int**)malloc(sizeof(int*)*pgm->row);

		for(i=0; i<pgm->row; i++) {
			pgm->pgm[i] = (int*)malloc(sizeof(int)*pgm->column);
			for(j=0; j<pgm->column; j++) {
				fscanf(filePtr, "%d" ,&pgm->pgm[i][j]);
			}
		}

		fclose(filePtr);

		return pgm;
	}

	fclose(filePtr);

	return NULL;

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

PGM **readPGMVector(FileSet *fileSet) {
	int i = 0;
	PGM **pgmVector = NULL;

	pgmVector = (PGM**)malloc(sizeof(PGM*)*fileSet->num);

	for(i=0; i<fileSet->num; i++) {
		pgmVector[i] = readPGM(fileSet->file[i]);
	}

	return pgmVector;
}

ImageSet *createImageSet(int num) {
	ImageSet *images = NULL;
	images = (ImageSet*)calloc(1, sizeof(ImageSet));
	images->num = num;
	return images;
}

void fillClass(char *filename, ImageSet *images) {
	FILE *filePtr = NULL;	
	int i = 0;
	char *aux = NULL;

	//Read from file
	if((filePtr = fopen(filename, "r")) == NULL) {
		exit(1);
	}

	for(i=0; i<images->num; i++) {
		aux = getLineFromFile(filePtr);
		images->pgm[i]->class = atof(aux);
		free(aux);
	}

	fclose(filePtr);
}

void printImageSet(ImageSet *images) {
	int i = 0;

	for(i=0; i<images->num; i++) {
		printf("---- PGM[%d]:\n", i);
		//printf("\tColumn [%d]\n", images->pgm[i]->column);
		//printf("\tRow [%d]\n", images->pgm[i]->row);
		printf("\tClass [%.2lf]\n", images->pgm[i]->class);
		//printf("\tMax [%d]\n", images->pgm[i]->max);
		printf("-------------\n");
	}
}

//=======================================================================================================================================
//FileSet================================================================================================================================
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
	char *aux = NULL;
	int len = 0;

	files = createFileSet();

	//Read from file
	if((filePtr = fopen(filename, "r")) == NULL) {
		exit(1);
	}

	while((aux = getLineFromFile(filePtr)) != NULL) {
		files->file = (char**)realloc(files->file, sizeof(char*)*(files->num+1));
		len = strlen(aux);
		files->file[files->num] = (char*)malloc(sizeof(char)*(len+1));
		strcpy(files->file[files->num], aux);
		files->num++;
		free(aux);
	}

	return files;
}

void printFileSet(FileSet *files) {
	printf("-----FileSet:: NUM [%d]\n", files->num);
	printMatrixString(files->file, files->num);
	printf("=======================\n");
}

//=======================================================================================================================================
//Mask===================================================================================================================================
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

//=======================================================================================================================================
//Function===============================================================================================================================
double *featureVector(int **pgm, int row, int column) {
	int i = 0, j = 0, k = 0;
	double *feature = NULL;
	double one = 0, two = 0, three = 0;
	double med = 0, var = 0, entropy = 0;

	feature = (double*)malloc(sizeof(double)*6*row);

	for(i=0; i<row; i++) {//For each line
		one = 0; two = 0; three = 0;
		med = 0; var = 0; entropy = 0;

		printf("---- Linha [%d]:\n", i);
		for(j=0; j<column; j++) {
			if(pgm[i][j] > 0) {
				one++;
			}
			else if(pgm[i][j] == 0) {
				two++;
			}
			else if(pgm[i][j] < 0) {
				three++;
			}
			med = med + pgm[i][j];
		}
		feature[k++] = one;
		feature[k++] = two;
		feature[k++] = three;

		med = med / column;
		feature[k++] = med;

		for(j=0; j<column; j++) {
			var = var + pow(pgm[i][j] - med, 2);
		}
		var = var / column;
		feature[k++] = var;

		for(j=0; j<column; j++) {
			entropy = entropy + pgm[i][j]*(log2(abs(pgm[i][j]) + 1));
		}
		entropy = -1 * entropy;
		feature[k++] = entropy;
		printf("\t one [%.2lf] two [%.2lf] three [%.2lf]\n", one, two, three);
		printf("\t med [%.2lf] var [%.2lf] entropy [%.2lf]\n", med, var, entropy);
		printf("---------------\n");
	}
	
	

	return feature;
}

void copyFeature(double *featureVector, double *feature, int start, int num, int max) {
	if(start >= max) return;

	int i = 0, j = 0;
	for(i=start; i<start+num; i++) {
		featureVector[i] = feature[j];
		j++;
	}
}

void convolution(MaskSet *maskSet, ImageSet *images) {
	int i = 0, j = 0, k = 0, a = 0, b = 0, l = 0;
	int counter = 0;
	int n = maskSet->num/2;
	int **result = NULL;
	double *feature = NULL;

	result = generateMatrix(images->pgm[0]->row, images->pgm[0]->column);

	for(i=0; i<images->num; i++) {//For each image
		images->pgm[i]->sizeVector = maskSet->num * 6 * images->pgm[i]->row;
		images->pgm[i]->featureVector = (double*)malloc(sizeof(double) * images->pgm[i]->sizeVector);
		
		for(l=0; l<maskSet->num; l++) {//For each mask

			for(j=0; j<images->pgm[i]->row; j++) {
				for(k=0; k<images->pgm[i]->column; k++) {//For each pixel of each image
					for(a=0; a<maskSet->num; a++) {
						for(b=0; b<maskSet->num; b++) {
							result[j][k] = result[j][k] + 
									maskSet->mask[l].matrix[a][b]*images->pgm[i]->pgm[j-n+a][k-n+b];
						}
					}
				}
			}

			//Calculate feature vector -> 6 * row elements
			feature = featureVector(result, images->pgm[i]->row, images->pgm[i]->column);
			copyFeature(images->pgm[i]->featureVector, feature, counter, 6 * images->pgm[i]->row, images->pgm[i]->sizeVector);
		}
	}
}

//=======================================================================================================================================
//Main Function==========================================================================================================================
int main() {
	char trainFilename[31], testFilename[31], classFilename[31];
	FileSet *trainData = NULL;
	FileSet *testData = NULL;
	MaskSet *maskSet = NULL;
	ImageSet *test = NULL;
	ImageSet *train = NULL;
	int M, m, K;

	//Reading filenames
	scanf("%s", trainFilename);
	scanf("%s", classFilename);
	scanf("%s", testFilename);

	//Reading M number of mask
	scanf("%d", &M);
	//Reading dimension m of mask
	scanf("%d", &m);
	//Reading ALL mask
	maskSet = readMaskSet(M, m);
	//Reading K value
	scanf("%d", &K);
	//printMaskSet(maskSet, m);
	//printf("K value [%d]\n", K);

	//Working with Data
	trainData = readFileSet(trainFilename);
	testData = readFileSet(testFilename);

	//ImageSet
	test = createImageSet(testData->num);
	train = createImageSet(trainData->num);
	test->pgm = readPGMVector(testData);
	train->pgm = readPGMVector(trainData);

	//Fill Class from Train ImageSet
	fillClass(classFilename, train);
	//printImageSet(train);;
	//printFileSet(trainData);


	//Free everything
	//freeFileSet(trainData);
	//freeFileSet(testData);
	freeMaskSet(maskSet, m);

	return 0;
}
