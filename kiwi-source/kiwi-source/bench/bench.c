#include "bench.h"



void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

//splits work to my threads
long int splitCountToThreads(long int count,int numthreads){
	long int countNew = count/numthreads;
	return countNew; 
}

void *callWrite(void *arg)
{	struct args*d = (struct args*)arg;
	long int countArg = d->countArg;
	int rArg = d->rArg;
	int numthr = d->numthr;
	long int newcount = splitCountToThreads(countArg,numthr);
	_write_test(newcount, rArg);
	return 0;
}

void *callRead(void *arg)
{	struct args*d = (struct args*)arg;
	long int countArg = d->countArg;
	int rArg = d->rArg;
	int numthr = d->numthr;
	long int newcount = splitCountToThreads(countArg,numthr);
	_read_test(newcount, rArg,numthr);
	return 0;
}


//Gia leitoyrgia add kai get mazi 
void createThreadsforAddGet(int wrthreads,int rdthreads,long int writeCount,long int readCount,int r,int wrperc,int rdperc){

	//create tids for threads
	pthread_t addId[1500];
	pthread_t getId[1500];
	
	//ftiaxnoume structures (san enimerosh) gia write kai gia read antistoixa
	struct args writeargs;
	struct args readargs;
	
	writeargs.rArg=r;
	writeargs.countArg=writeCount;
	writeargs.numthr=wrthreads;
	
	readargs.rArg=r;
	readargs.countArg=readCount;
	readargs.numthr= rdthreads;
	
	//create threads and joins for write and read
	for(int i=0; i<wrthreads; i++){
		pthread_create(&addId[i],NULL,callWrite,(void*) &writeargs);
	}
	
	for(int i=0; i<rdthreads; i++){
		pthread_create(&getId[i],NULL,callRead,(void*) &readargs);
	}
	for(int i=0; i<wrthreads; i++){
		pthread_join(getId[i],NULL);
	}
	for(int i=0; i<rdthreads; i++){
		pthread_join(addId[i],NULL);
	}
	
		
}


//print statistics of the performance 
//when flag =1  -> write operation
//when flag =2  -> read operation
void performanceStats(long int count ,double operCost,int flagOper){
	printf(LINE);
	printf(LINE1);
	if(flagOper==1){
		printf("|Random-WRITE	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
			,count,(double)(operCost / count)
			,(double)(count/ operCost)
			,operCost);
	}else if(flagOper==2){
		printf("|Random-READ	(done:%ld): %.6f sec/op; %.1f reads/sec(estimated); cost:%.3f(sec);\n"
			,count,(double)(operCost / count)
			,(double)(count/ operCost)
			,operCost);
	}


}


int main(int argc,char** argv)
{
	long int count;
	int numthreads,i;
	struct args strArgs;

	pthread_mutex_init(&writeCmut,NULL);
    pthread_mutex_init(&readCmut,NULL);
	
	wrCost=0;
	rdCost=0;

	srand(time(NULL));
	if( strcmp(argv[1],"addget")!=0 ){
		
		// READ OR WRITE : check for correct usage
		if (argc < 4 ) {
		//--------------------------------[0]----------[1]---------[2]------[3]------
			fprintf(stderr,"Usage: db-bench < write | read > <count> <numThreads> \n");
			exit(1);
		}
	}else{
		
		// READWRITE :check for correct usage
		if (argc < 6 ) {
		//-------------------------------[0]------[1]-----[2]------[3]---------[4]--[5]---
			fprintf(stderr,"Usage: db-bench <addget> <count> <numThreads> <add> <get> \n");
			exit(1);
		}
	
	}
	
	if (strcmp(argv[1], "write") == 0) {
		
		int r = 0;
		////assign given num of requests to count Var
		count = atoi(argv[2]);

		//prints
		_print_header(count);
		_print_environment();

		//argc = 5 - > 0..4
		if (argc == 5)
			r = 1;

		//synolika threads apo ton xristi
		numthreads = atoi(argv[3]);
		
		pthread_t thrIds[numthreads];
		controlDb(1);

		strArgs.rArg=r;
		strArgs.countArg=count;
		strArgs.numthr=numthreads;
		for(i=0;i<numthreads;i++)
			pthread_create(&thrIds[i],NULL,callWrite,(void *)&strArgs);
		for(i=0;i<numthreads;i++)
			pthread_join(thrIds[i],NULL);

		controlDb(0);

		
		performanceStats(count,wrCost,1);
	
	} else if (strcmp(argv[1], "read") == 0) {

		int r = 0;
		//assign given num of requests to count Var
		count = atoi(argv[2]);

		//prints
		_print_header(count);
		_print_environment();

		//argc=5 ->0..4
		if (argc == 5)
			r = 1;

		//synolika threads apo ton xristi
		numthreads = atoi(argv[3]);
		pthread_t thrIds[numthreads];
		controlDb(1);

		strArgs.rArg=r;
		strArgs.countArg=count;
		strArgs.numthr=numthreads;
		for(i=0;i<numthreads;i++)
			pthread_create(&thrIds[i],NULL,callRead,(void *)&strArgs);
		for(i=0;i<numthreads;i++)
			pthread_join(thrIds[i],NULL);

		controlDb(0);


		performanceStats(count,rdCost,2);

	

	} else if (strcmp(argv[1], "addget") == 0) {
		int wrperc = 0;//pososto gia write
		int rdperc = 0;//pososto gia write

		long int writeCount;
		long int readCount; 

		//given num of threads for each operation
		int wrnumthreads = atoi(argv[4]);
		int rdnumthreads = atoi(argv[5]);
		
		int r = 0;
		//assign given num of requests to count Var
		count = atoi(argv[2]);

		//prints
		_print_header(count);
		_print_environment();


		//argc=7 -> 0..6
		if (argc == 7)
			r = 1;

		//synolika threads apo ton xristi
		numthreads = atoi(argv[3]);


		controlDb(1);//anoigma mixanis
		
		wrperc= (wrnumthreads * 100 / numthreads);
		rdperc= (rdnumthreads * 100 / numthreads);
		
		
	
		//pososto count gia kathe leitoyrgia
		writeCount = (long int) (count*wrperc / 100); //synolika write poy ua ginoyn
		readCount = (long int) (count*rdperc / 100); //synolika read poy ua ginoyn
	
		
		createThreadsforAddGet(wrnumthreads,rdnumthreads,writeCount,readCount,r,wrperc,rdperc);

		controlDb(0);
		
		performanceStats(writeCount,wrCost,1);
		performanceStats(readCount,rdCost,2);
		

		
	} else {
		fprintf(stderr,"Usage: db-bench <write | read | addget> <count> <numthreads> <r> \n");
		exit(1);
	}

	return 1;
}
