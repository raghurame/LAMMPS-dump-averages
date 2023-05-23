/*
compute         bondTypes all property/local batom1 batom2
compute         bondDist all bond/local engpot dist force engvib engrot engtrans omega velvib
dump            bondStats all local 1000 dump.bonds c_bondTypes[*] c_bondDist[*]
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define INPUTDUMPNAME "dumpFirst.bonds"
#define NTIMETOSKIP 0

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
	nTimesteps -= 9;
	nTimesteps /= nAtoms;

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

	for (int i = NTIMETOSKIP; i < nTimesteps; ++i)
	{
		sumColumns = init1dfloat (sumColumns, nColumns);

		for (int j = 0; j < nColumns; ++j)
		{
			for (int k = 0; k < nAtoms; ++k)
			{
				sumColumns[j] += dumpValues[i][j][k];
			}

			ensembleAvg[i][j] = sumColumns[j] / (float)nAtoms;
		}
	}

	return ensembleAvg;
}

float **computeTimeAvg (float **timeAvg, int nTimesteps, float ***dumpValues, int nColumns, int nAtoms)
{
	float *sumColumns;
	sumColumns = (float *) malloc (nColumns * sizeof (float));

	for (int i = 0; i < nAtoms; ++i)
	{
		sumColumns = init1dfloat (sumColumns, nColumns);

		for (int j = 0; j < nColumns; ++j)
		{
			for (int k = NTIMETOSKIP; k < nTimesteps; ++k)
			{
				sumColumns[j] += dumpValues[k][j][i];
			}

			timeAvg[i][j] = sumColumns[j] / (float)nColumns;
		}
	}

	return timeAvg;
}

int main(int argc, char const *argv[])
{
	FILE *inputDump;
	inputDump = fopen (INPUTDUMPNAME, "r");

	int nAtoms = findNAtoms (nAtoms, inputDump), nColumns = findNColumns (nColumns, inputDump), nTimeframes = 0, nTimesteps = countNTimesteps (nTimesteps, inputDump, nAtoms, INPUTDUMPNAME);

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
	timeAvg = computeTimeAvg (timeAvg, nTimesteps, dumpValues, nColumns, nAtoms);

	fclose (inputDump);
	return 0;
}