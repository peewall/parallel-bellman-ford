/*
Peter Wallace
Wednesday, March 21
Parallel Programming
*/


#include <iostream>
#include <fstream>
#include <omp.h>
#include <cstdlib>
#include <limits.h>
#include <time.h>

using namespace std;

int infAdd(int a, int b ){ //Used to prevent integer wrapping
	if (a == INT_MAX || b == INT_MAX){
		return INT_MAX;
	}
	else{
		return a + b;
	}
return 0;
}

int relax(int ** adjMtx, int numVertc, int srcVtx, int numThds){

	int d[numVertc];//shared cost array
	bool change;
	int count;

	for (int i = 0; i < numVertc; ++i)
	{
		if(i != srcVtx){
			d[i] = INT_MAX;
		}
		else{
			d[i] = 0;
		}
	}

	//iterate while cost matrix is still updating
#pragma omp parallel shared (numVertc,change, srcVtx) private (count) num_threads(numThds)
	{
		change = true;
		count = 0; //because not guaranteed initialized

		while(count < numVertc && change)
		{
			#pragma omp barrier
			#pragma omp single
			{change = false;}
			//iterate over all destination nodes
			for(int i = 0; i < numVertc; ++i){
				//iterate over all source nodes
				#pragma omp for
				//distributed iteration over all 
				for (int j = 0; j < numVertc; ++j)
				{
					/*if source + actual cost in adjacency matrix < current cost to destination 
					& source != destination, update*/
				    if(infAdd(d[j], adjMtx[i][j]) < d[i] && i != j){
						d[i] = d[j] + adjMtx[i][j];
						change  = true;

				    }
				}
			}
			count++;
			#pragma omp barrier
		}
	}//end parallel

	ofstream ofs("shortestPath.txt", ofstream::out);
	for (int i = 0; i < numVertc; ++i)//print cost matrix
	{
		ofs << i << ": " << d[i] << "\n";
	}
	
    return 0;
}

void procBinFile(int srcVtx, int numThds, char * fileName){

	int numVertc = 0;
	int temp = 0;
	clock_t time;
	int elapsedTime;

	//read number of vertices in graph
	ifstream in(fileName,ios_base::binary);
	in.read(reinterpret_cast<char*>(&numVertc), sizeof(int));

	int ** tempMtx = new int *[numVertc];
	int ** adjMtx = new int *[numVertc];
	for (int i = 0; i < numVertc; ++i)
	{
		tempMtx[i] = new int [numVertc];
		adjMtx[i] = new int [numVertc];
	}

	//proccess linear adjacency matrix into 2D-array
	for (int i = 0; i < numVertc; ++i)
		{
			for (int j = 0; j < numVertc; j++)
			  {
			  	in.read(reinterpret_cast<char*>(&temp), sizeof(int));
			  	tempMtx[i][j] = temp;

			  }  

		}
	in.close();

	//convert row major to column major
	for (int i = 0; i < numVertc; ++i)
	{
		for (int j = 0; j < numVertc; ++j)
		{
			adjMtx[i][j] = tempMtx[j][i];
		}
	}

	//keep track of relax algorithm speed
	time = clock();
	//relax loops through adjacency matrix and updates cost array
	relax(adjMtx, numVertc, srcVtx, numThds);
	elapsedTime = ((double)time)/CLOCKS_PER_SEC;
	cout << "Elapsed Time: " << elapsedTime << "\n";

}

int main(int argc, char * argv[]){

	int srcVtx;
	int numThds;
	char * fileName;

	if (argc < 4){
		cerr << "4 arg. requirement: <exe> <file> <source vertex> <num threads>";
		exit(1);
	}

	srcVtx = atoi(argv[2]);
	numThds = atoi(argv[3]);
	fileName = argv[1];

	procBinFile(srcVtx, numThds, fileName);

	return 0;
}

