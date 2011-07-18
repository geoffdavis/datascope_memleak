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
	long i=0, j=0; // record counts for datascope views
	Dbptr db; // main db pointer
	Dbptr dbsnetsta; //pointer to snetsta table
	Dbptr dbsitechan; // pointer to sitechan table
	Dbptr db_sub1,db_sub2;
	Tbl *sortkeys, *groupkeys; // keys to sort the db
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
	db = dbjoin(dbsnetsta, dbsitechan, 0, 0, 0, 0, 0);
	sortkeys = strtbl("snet", "sta", NULL);
	db = dbsort(db, sortkeys, 0, 0);
	groupkeys = strtbl("sta", NULL);
	db = dbgroup(db, groupkeys, NULL, 0);

	dbquery(db, dbRECORD_COUNT, &i);

	printf("dbRECORD_COUNT i is %ld\n", i);

	// start an infinite loop of db crunching
	while (1) {
		for (db.record = 0; db.record < i; db.record++) {
			if (dbgetv(db, 0, "sta", sub_sta, NULL) < 0)
				elog_die(1, "Can't dbgetv sta");

			snprintf( subsetstr, sizeof(subsetstr), "sta=~/%s", sub_sta);
			db_sub1 = dbsubset(db, subsetstr, 0);
			db_sub2 = dbungroup(db_sub1, 0);
			dbfree(db_sub1);


			dbquery(db_sub2, dbRECORD_COUNT, &j);

			for (db_sub2.record = 0; db_sub2.record < j; db_sub2.record++) {
				printf("dbRECORD_COUNT j is %ld\n", j);
				//no-op for the moment, need to get a list of values
			} // end for j

			dbfree(db_sub2);

		} //end for i


	} //end while (1)
	dbfree(dbsnetsta);
	dbfree(dbsitechan);
	dbclose(db);

} //end main
