/* Example C program to try to find memory leaks in the Datascope libraries
 * See https://github.com/robnewman/antelope_utilities/blob/master/tests/datascope_memory_leak
 * for a working python version of this program
 */

#include "db.h"
#include "stock.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *antelope; //$ANTELOPE
	char dbpath[260]; // path to database
	long i = 0; // record count for number of stations
	//long j = 0; // record count for rows associated with station
	unsigned long iter = 0; // number of iterations of outer loop
	Dbptr db; // master db pointer
	Dbptr dbsnetsta; //pointer to snetsta table
	Dbptr dbsitechan; // pointer to sitechan table
	Dbptr db_sub1, db_sub2; // temporary
	Tbl *sortkeys, *groupkeys; // keys to sort and group the db
	char sub_sta[25];
	char subsetstr[sizeof(sub_sta) + 5];
	// Open up demo database tables and group, then run infinite loop
	elog_init(argc, argv);

	antelope = getenv("ANTELOPE");
	if (antelope == NULL)
		elog_die(1, "The ANTELOPE environment variable is not set.");

	// Construct the dbpath
	sprintf( dbpath, "%s/demo/socalif/db/scdemo", antelope);

	if (dbopen(dbpath, "r", &db) < 0)
		elog_die(0, "Can't open demo db");

	dbsnetsta = dblookup(db, 0, "snetsta", 0, 0);
	dbsitechan = dblookup(db, 0, "sitechan", 0, 0);

	/* This would "leak" a view but it's outside of the infinite loop.
	 * Thus it's not a huge contributor to the overall memory footprint */
	db = dbjoin(dbsnetsta, dbsitechan, 0, 0, 0, 0, 0);
	sortkeys = strtbl("snet", "sta", NULL);
	/* This would "leak" a view as well, but it's outside the loop */
	db = dbsort(db, sortkeys, 0, 0);
	groupkeys = strtbl("sta", NULL);
	/* Ditto about leaking a view here. */
	db = dbgroup(db, groupkeys, NULL, 0);

	dbquery(db, dbRECORD_COUNT, &i);
	//printf("dbRECORD_COUNT i is %ld\n", i);

	/*
	 * start an infinite loop of db crunching
	 */
	while (1) {
		/* Get each station */
		for (db.record = 0; db.record < i; db.record++) {
			iter++;
			printf("Iteration: %ld\n", iter);
			if (dbgetv(db, 0, "sta", sub_sta, NULL) < 0)
				elog_die(1, "Can't dbgetv sta");

			snprintf( subsetstr, sizeof(subsetstr), "sta=~/%s", sub_sta);

			db_sub1 = dbsubset(db, subsetstr, 0);
			db_sub2 = dbungroup(db_sub1, 0);
			if (dbfree(db_sub1) == dbINVALID)
				elog_die(1, "*stack* dbfree db_sub1 failed\n");

			/*
			 * This next bit is where you would do some actual work on an
			 * actual station. Commented out since it's effectively a NO-OP in
			 * terms of causing a leak.
			 */
			/*
			dbquery(db_sub2, dbRECORD_COUNT, &j);
			for (db_sub2.record = 0; db_sub2.record < j; db_sub2.record++) {
				printf("dbRECORD_COUNT j is %ld\n", j);
				//no-op for the moment, would dbgetv and print values
			} // end for j
			*/

			if (dbfree(db_sub2) == dbINVALID)
				elog_die(1, "*stack* dbfree db_sub2 failed\n");

			/*
			 * At this point, we have dbfree'd both views that we created in
			 * this loop iteration, but ps still shows that we are increasing
			 * memory utilization on each iteration.
			 */
		} //end for i


	} //end while (1)
	dbfree(dbsnetsta);
	dbfree(dbsitechan);
	dbclose(db);

} //end main
