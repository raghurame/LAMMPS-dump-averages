#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int findNAtoms (int nAtoms, FILE *inputDump)
{
	char lineString[2000];

	for (int i = 0; i < 4; ++i)
	{
		fgets (lineString, 2000, inputDump);
	}

	sscanf (lineString, "%d\n", &nAtoms);

	rewind (inputDump);
	return nAtoms;
}

/*int stringCount (int nNeedles, char haystack[], char needle[])
{
	char *tempString = needle;
	nNeedles = 0;

	printf("Initial value for tempString\n");
	printf("'%s'\n", tempString);

	while (*tempString != '\0' && (tempString = strstr (haystack, tempString)))
	{
		nNeedles++;
		++tempString;
		printf("%s\n", tempString);
		sleep (1);
	}

	return nNeedles;
}
*/

int countSpaces (int nColumns, char lineString[])
{
	int stringLength = strlen (lineString);
	nColumns = 0;

	for (int i = 0; i < stringLength; ++i)
	{
		if (lineString[i] == ' ')
		{
			nColumns++;
		}
	}

	return nColumns;
}

int findNColumns (int nColumns, FILE *inputDump)
{
	char lineString[2000];

	/*snprintf (space, 10, " ");*/

	for (int i = 0; i < 10; ++i)
	{
		fgets (lineString, 2000, inputDump);
	}

	/*nColumns = stringCount (nColumns, lineString, space);*/
	nColumns = countSpaces (nColumns, lineString);

	rewind (inputDump);
	return nColumns;
}

float ***readDumpfile (float ***dumpValues, int nColumns, int nAtoms, FILE *inputDump, int *nTimeframes)
{
	char lineString[2000];
	(*nTimeframes) = 0;

	int fileStatus = fgetc (inputDump);

	while (fileStatus != EOF)
	{
		for (int i = 0; i < 9; ++i) {
			fgets (lineString, 2000, inputDump); }

		for (int i = 0; i < nAtoms; ++i)
		{
			for (int j = 0; j < nColumns; ++j) {
				fscanf (inputDump, "%f ", &dumpValues[(*nTimeframes)][j][i]); }

			fscanf (inputDump, "\n");
		}

		(*nTimeframes)++;
		fileStatus = fgetc (inputDump);
	}

	return dumpValues;
}

int countNTimesteps (int nTimesteps, FILE *inputDump, int nAtoms, const char inputFilename[])
{
	FILE *pipeLineCount;
	char *pipeString, lineString[2000];

	pipeString = (char *) malloc (100 * sizeof (char));
	snprintf (pipeString, 100, "wc -l %s", inputFilename);

	pipeLineCount = popen (pipeString, "r");
	fgets (lineString, 2000, pipeLineCount);
	sscanf (lineString, "%d", &nTimesteps);

	return nTimesteps;
}

float *init1dfloat (float *floatArray, int arrayLength)
{
	for (int i = 0; i < arrayLength; ++i) {
		floatArray[i] = 0; }

	return floatArray;
}

float **computeEnsembleAvg (float **ensembleAvg, int nTimesteps, float ***dumpValues, int nColumns, int nAtoms)
{
	float *sumColumns;
	sumColumns = (float *) malloc (nColumns * sizeof (float));


	for (int i = 0; i < nTimesteps; ++i)
	{
		sumColumns = init1dfloat (sumColumns, nColumns);

		for (int j = 0; j < nColumns; ++j)
		{
			for (int k = 0; k < nAtoms; ++k)
			{
				sumColumns[j] += dumpValues[i][j][k];
			}

			ensembleAvg[i][j] = sumColumns[j] / nColumns;
		}
	}
	return ensembleAvg;
}

int main(int argc, char const *argv[])
{
	FILE *inputDump;
	inputDump = fopen (argv[1], "r");

	int nAtoms = findNAtoms (nAtoms, inputDump), nColumns = findNColumns (nColumns, inputDump), nTimeframes = 0, nTimesteps = countNTimesteps (nTimesteps, inputDump, nAtoms, argv[1]);

	float ***dumpValues;
	dumpValues = (float ***) malloc (nTimesteps * sizeof (float **));

	for (int i = 0; i < nTimesteps; ++i) {
		dumpValues[i] = (float **) malloc (nColumns * sizeof (float *)); }

	for (int i = 0; i < nTimesteps; ++i)
	{
		for (int j = 0; j < nColumns; ++j) {
			dumpValues[i][j] = (float *) malloc (nAtoms * sizeof (float)); }
	}

	dumpValues = readDumpfile (dumpValues, nColumns, nAtoms, inputDump, &nTimeframes);

/*	for (int i = 0; i < nAtoms; ++i)
	{
		printf("%f\n", dumpValues[0][3][i]);
		usleep (100000);
	}
*/

	float **ensembleAvg, **ensembleStdev, **timeAvg, **timeStdev;

	ensembleAvg = (float **) malloc (nTimesteps * sizeof (float *));
	ensembleStdev = (float **) malloc (nTimesteps * sizeof (float *));
	timeAvg = (float **) malloc (nAtoms * sizeof (float *));
	timeStdev = (float **) malloc (nAtoms * sizeof (float *));

	for (int i = 0; i < nTimesteps; ++i)
	{
		ensembleAvg[i] = (float *) malloc (nColumns * sizeof (float));
		ensembleStdev[i] = (float *) malloc (nColumns * sizeof (float));
	}

	for (int i = 0; i < nAtoms; ++i)
	{
		timeAvg[i] = (float *) malloc (nColumns * sizeof (float));
		timeStdev[i] = (float *) malloc (nColumns * sizeof (float));
	}

	ensembleAvg = computeEnsembleAvg (ensembleAvg, nTimesteps, dumpValues, nColumns, nAtoms);

	for (int i = 0; i < nTimesteps; ++i)
	{
		printf("%f\n", ensembleAvg[i][3]);
		usleep (100000);
	}

	/*timeAvg = computeTimeAvg (timeAvg, nTimesteps, dumpValues, nColumns, nAtoms);*/

	fclose (inputDump);
	return 0;
}