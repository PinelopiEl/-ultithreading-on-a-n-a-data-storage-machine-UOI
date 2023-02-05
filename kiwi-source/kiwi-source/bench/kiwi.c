#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

DB* db;
//function to control db
//when flag=1 open the db
//when flag=0 close the db
void controlDb(int flagOper){
	if(flagOper == 1){
		db = db_open(DATAS);
	}else if(flagOper == 0){
		db_close(db);
	}
}

double computeCost(long long firstTime,long long lastTime,double res,pthread_mutex_t myMut){
	long long addTime;
	//diafora telikou meion arxikou xronou
	addTime= lastTime-firstTime;
	pthread_mutex_lock(&myMut);
	//prosthesi tis diaforas
	res= res + addTime;
	pthread_mutex_unlock(&myMut);
	return res;

}

void _write_test(long int count, int r)
{
	int i;
	long long start,end;
	Variant sk, sv;


	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	

	start = get_ustime_sec();
	
	for (i = 0; i < count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();
	

	wrCost = computeCost(start,end,wrCost,writeCmut);
	

}

void _read_test(long int count, int r)
{
	int i;
	int ret;
	int found = 0;
	long long start,end;
	Variant sk;
	Variant sv;
	char key[KSIZE + 1];

	start = get_ustime_sec();
	
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		//sleep(0.10);
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}


	end = get_ustime_sec();
	
	rdCost = computeCost(start,end,rdCost,readCmut);
	

	
}
