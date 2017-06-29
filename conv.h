#IFNDEF _CONV_H_
#DEFINE _CONV_H_

//Structure
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
#ENDIF
