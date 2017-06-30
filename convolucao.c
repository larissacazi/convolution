#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define log2(NUM)	log(NUM)/log(2)

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
	int m;
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

typedef struct Class {
	int numOfVotes;
	double class;
} Class;
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

	do {
		c = fgetc(filePtr);
		auxiliar = realloc(auxiliar, (counter+1)*sizeof(char));
		if(auxiliar != NULL) {
        		auxiliar[counter++] = c;
		}
	} while(c != '\n' && c != '\r' && c != EOF);
	auxiliar[counter-1] = '\0';

	return auxiliar;
}

void printMatrixString(char **M, int n) {
	int i = 0;
	for(i=0; i<n; i++) printf("%s\n", M[i]);
}

void printDoubleVector(double *v, int n) {
	int i = 0;
	for(i=0; i<n; i++) printf("%.2lf\t", v[i]);
	printf("\n");
}

int **generateMatrix(int row, int column) {
	int **m = NULL;
	int i = 0;

	m = (int**)calloc(row, sizeof(int*));	

	for(i=0; i<row; i++) {
		m[i] = (int*)calloc(column, sizeof(int));
	}

	return m;
}

int *fillIndexVector(int n) {
	int i = 0;
	int *vector = NULL;
	vector = (int*)malloc(sizeof(int) * n);

	for(i=0; i<n; i++) {
		vector[i] = i;
	}

	return vector;
}
//=============================================================================
//Class Structure==============================================================
Class *createClass(double class) {
	Class *element = NULL;

	element = (Class*)calloc(1, sizeof(Class));
	element->class = class;
	element->numOfVotes++;

	return element;
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
		maskData->mask[i].m = m;
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

	//Read from file
	if((filePtr = fopen(filename, "rb")) == NULL) {
		exit(1);
	}

	fscanf(filePtr, "%2s", pgmType);
	
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

	return pgm;

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
		//printf("aux = %s\n", aux);
		images->pgm[i]->class = atof(aux);
		free(aux);
	}

	fclose(filePtr);
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
//Function=====================================================================
double *featureVector(int **pgm, int row, int column) {
	int i = 0, j = 0, k = 0;
	double *feature = NULL;
	double one = 0, two = 0, three = 0;
	double med = 0, var = 0, entropy = 0;

	feature = (double*)malloc(sizeof(double)*6*row);

	for(i=0; i<row; i++) {//For each line
		one = 0; two = 0; three = 0;
		med = 0; var = 0; entropy = 0;

		//printf("---- Linha [%d]:\n", i);
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

		med = med / (double)column;
		feature[k++] = med;

		for(j=0; j<column; j++) {
			var = var + (double)pow(pgm[i][j] - med, 2);
		}
		var = var / (double)column;
		feature[k++] = var;

		for(j=0; j<column; j++) {
			entropy = entropy + pgm[i][j]*(log2(abs(pgm[i][j]) + 1));
		}
		entropy = -1 * entropy;
		feature[k++] = entropy;
		//printf("\t one [%.2lf] two [%.2lf] three [%.2lf]\n", one, two, three);
		//printf("\t med [%.2lf] var [%.2lf] entropy [%.2lf]\n", med, var, entropy);
		//printf("---------------\n");
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
	int n = maskSet->mask[0].m/2;
	int **result = NULL;
	double *feature = NULL;

	//Allocate Matrix
	result = generateMatrix(images->pgm[0]->row, images->pgm[0]->column);

	//For each image
	for(i=0; i<images->num; i++) {
		//Calculate the total size of the feature Vector:
		// Number of Masks * 6 (size of one feature vector) * Number of lines from image
		images->pgm[i]->sizeVector = maskSet->num * 6 * images->pgm[i]->row; 
		//printf("convolution::Total size: %d\n", images->pgm[i]->sizeVector);
		images->pgm[i]->featureVector = (double*)malloc(sizeof(double) * images->pgm[i]->sizeVector);

		//For each mask
		for(l=0; l<maskSet->num; l++) {
			//printf("For each mask:\n");			

			//For each pixel of each image
			for(j=0; j<images->pgm[i]->row; j++) {
				for(k=0; k<images->pgm[i]->column; k++) {
					//printf("\tFor each pixel of each image: maskDimension[%d]\n", maskSet->mask[l].m);	

					//For each pixel of each mask
					for(a=0; a<maskSet->mask[l].m; a++) {
						for(b=0; b<maskSet->mask[l].m; b++) {
							//printf("\t\tFor each pixel of each mask:\n");
							//printf("j [%d] k[%d] a[%d] b[%d] n[%d]\n",j,k,a,b,n);
							if(j-n+a < 0) continue;
							if(j-n+a > 27) continue;
							if(k-n+b < 0) continue;
							if(k-n+b > 27) continue;

							//printf("Result[%d][%d] = %d\n", j,k, result[j][k]);
							//printf("j-n+a[%d] e k-n+b[%d]\n", j-n+a, k-n+b);
							result[j][k] = result[j][k] + 
							maskSet->mask[l].matrix[a][b] * images->pgm[i]->pgm[j-n+a][k-n+b];
							//printf("images->pgm[i]->pgm[%d][%d] = %d\n",j-n+a,k-n+b,images->pgm[i]->pgm[j-n+a][k-n+b]);
							//printf("maskSet->mask[l].matrix[%d][%d] = %d\n", a, b,maskSet->mask[l].matrix[a][b]);
						}
					}
					//printf("Result[%d][%d] = %d", j,k, result[j][k]);
				}
			}
			
			//printf("Calculate feature vector:\n");
			//Calculate feature vector -> 6 * row elements
			feature = featureVector(result, images->pgm[i]->row, images->pgm[i]->column);
			copyFeature(images->pgm[i]->featureVector, feature, counter, 6 * images->pgm[i]->row, images->pgm[i]->sizeVector);
			free(feature);
			counter = counter + 6*images->pgm[i]->row;
		}
		//printf("Image %d - Feature Vector:", i);
		//printDoubleVector(images->pgm[i]->featureVector, images->pgm[i]->sizeVector);

	}
}

double euclidianDistance(double *test, int testSize, double *train, int trainSize) {
	int i = 0;
	double result = 0;	

	for(i=0; i<testSize; i++) {
		result = result + pow(test[i] - train[i], 2);
	}

	result = sqrt(result);

	return result;
}

double *calculateDistance(PGM *test, ImageSet *train) {
	double *distance = NULL;
	int i = 0;
	distance = (double*)calloc(train->num, sizeof(double));
	//printf("train->num %d\n", train->num);

	for(i=0; i<train->num; i++) {
		distance[i] = euclidianDistance(test->featureVector, test->sizeVector, train->pgm[i]->featureVector, train->pgm[i]->sizeVector);
	}

	return distance;
}

void swap(int **v, int i, int j) {
	int tmp = (*v)[i];
	(*v)[i] = (*v)[j];
	(*v)[j] = tmp;
}

int *sortByDist(double *dist, int numOfData) {
	int i = 0, j = 0;
	int *index = NULL;
	index = fillIndexVector(numOfData);

	for(i=0; i<numOfData-1; i++) {
		for(j=0; j<numOfData-1-i; j++) {
			if(dist[index[j]] > dist[index[j+1]]) {
				swap(&index, j, j+1);
			}
		}
	}

	/*for(i=0; i<numOfData; i++) {
		printf("dist[%d] = %.2lf\n", index[i], dist[index[i]]);
	}*/

	return index;
}

BOOL existInClassificator(Class **classificator, int num, double class) {
	int i = 0;

	for(i=0; i<num; i++) {
		if(classificator[i]->class == class) {
			return TRUE;
		}
	}
	return FALSE;
}

int findClassificator(Class **classificator, int num, double class) {
	int i = 0;

	for(i=0; i<num; i++) {
		if(classificator[i]->class == class) {
			return i;
		}
	}
	return -1;
}

void classifyElement(ImageSet *train, double *distance, int nDist, int *index, int k) {
	Class **classificator = NULL;
	int num = 0, i = 0, major = 0, idx = 0;

	//printf("\n -----Entering! \tClassifying test\n");

	for(i=0; i<k; i++) {
		//printf("\t --I = %d\n", i);
		if(num == 0) {
			//printf("Nenhum elemento.\n");
			classificator = (Class**)calloc((num+1), sizeof(Class*));
			//printf("num %d, index[%d] %d\n", num, i, index[i]);
			//printf("1class %.2lf\n", train->pgm[index[i]]->class);
			classificator[num] = createClass(train->pgm[index[i]]->class);
			num++;
		}
		else if(existInClassificator(classificator, num, train->pgm[index[i]]->class) == FALSE) {
			//printf("NUM = %d\n", num);
			//printf("2class %.2lf\n", train->pgm[index[i]]->class);
			classificator = (Class**)realloc(classificator, sizeof(Class*) * (num+1));
			classificator[num] = createClass(train->pgm[index[i]]->class);
			num++;
		}
		else {
			//printf("passou.\n");
			//printf("3class %.2lf\n", train->pgm[index[i]]->class);
			idx = findClassificator(classificator, num, train->pgm[index[i]]->class);
			classificator[idx]->numOfVotes++;
		}
	}
	//printf("out - pasosu.\n");

	major = classificator[0]->numOfVotes; idx = 0;
	//printf("major = %d\n", major);
	//printf("num= %d\n", num);
	for(i=1; i<num; i++) {
		//printf("entrou.\n");
		if(classificator[i]->numOfVotes > major) {
			classificator[i]->numOfVotes = major;
			idx = i;
		}
	}

	//printf("Major = %d Class = %.2lf\n", major, classificator[idx]->class);
	printf("%.2lf\n", classificator[idx]->class);

	for(i=0; i<num; i++) {
		free(classificator[i]);
	}
	free(classificator);

}

void knn(ImageSet *test, ImageSet *train, int k) {
	int *index = NULL;
	double *distance = NULL;
	int i = 0;

	for(i=0; i<test->num; i++) {
		//printf("\n ----- \tCalculating.. test[%d]\n", i);
		//Calculating distance vector
		distance = calculateDistance(test->pgm[i], train);

		//Sorting distance
		//printf("\n ----- \tSorting distance[%d]\n", i);
		index = sortByDist(distance, train->num);

		//Classifying test
		//printf("\n ----- \tClassifying test[%d]\n", i);
		classifyElement(train, distance, test->pgm[i]->sizeVector, index, k);

		//Free everything
		//printf("\n ----- \tFree everything[%d]\n", i);
		free(distance);
		//free(index);
	}
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

	//Reading M number of mask
	scanf("%d", &M);
	//Reading dimension m of mask
	scanf("%d", &m);
	//Reading ALL mask
	maskSet = readMaskSet(M, m);
	//Reading K value
	scanf("%d", &K);

	//Working with Data
	printf("Working with Data\n");
	trainData = readFileSet(trainFilename);
	testData = readFileSet(testFilename);

	//Initializing ImageSet
	printf("Initializing ImageSet\n");
	test = createImageSet(testData->num);
	train = createImageSet(trainData->num);

	//Reading images
	printf("Reading images\n");
	test->pgm = readPGMVector(testData);
	train->pgm = readPGMVector(trainData);

	//Fill class of train set
	printf("Fill class of train set\n");
	fillClass(classFilename, train);

	//Fill Feature Vectors for Train Images
	printf("Fill Feature Vectors for Train Images\n");
	convolution(maskSet, train);

	//Fill Feature Vectors for Test Images
	printf("Fill Feature Vectors for Test Images\n");
	convolution(maskSet, test);

	//printf("Train Image Set:\n");
	//printImageSet(train);

	//Applying KNN for classification
	printf("Applying KNN for classification\n");
	knn(test, train, K);

	//Free everything
	printf("Free everything\n");
	freeFileSet(trainData);
	freeFileSet(testData);
	freeMaskSet(maskSet, m);

	return 0;
}
