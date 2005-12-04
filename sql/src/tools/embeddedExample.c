/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2005 CWI.
 * All Rights Reserved.
 */

/*
This program is meant to illustrate an embedded SQL application
and provides a baseline for footprint comparisons.
A defensive programming style.
*/
#include <Mbedded.h>

#define die(dbh,hdl) (hdl?mapi_explain_result(hdl,stderr):		\
			  dbh?mapi_explain(dbh,stderr):			\
			      fprintf(stderr,"command failed\n"),	\
		      exit(-1))

#define close_handle(X,Y) if (mapi_close_handle(X) != MOK) die(X, Y);

#define SQL1 "create table emp(name varchar(20),age int)"
#define SQL2 "insert into emp values('user%d', %d)"
#define SQL3 "select * from emp"

int
main()
{
	Mapi dbh;
	MapiHdl hdl = NULL;
	int i;

	mapi_embedded_start();

	dbh = mapi_connect("localhost", 45123, "monetdb", "monetdb", "sql");
	if (dbh == NULL || mapi_error(dbh))
		die(dbh, hdl);

	/* mapi_trace(dbh, 1); */
	/* switch off autocommit */
	if (mapi_setAutocommit(dbh, 0) != MOK || mapi_error(dbh))
		die(dbh,NULL);
	
	if ((hdl = mapi_query(dbh, SQL1)) == NULL || mapi_error(dbh))
		die(dbh, hdl);
	close_handle(dbh,hdl);

	for(i=0; i< 1000; i++){
		char query[100];
		snprintf(query, 100, SQL2, i, i % 82);
		if ((hdl = mapi_query(dbh, query)) == NULL || mapi_error(dbh))
			die(dbh, hdl);
		close_handle(dbh,hdl);
	}

	if ((hdl = mapi_query(dbh, SQL3)) == NULL || mapi_error(dbh))
		die(dbh, hdl);

	i=0;
	while (mapi_fetch_row(hdl)) {
		char *age = mapi_fetch_field(hdl, 1);
		i= i+ atoi(age);
	}
	if (mapi_error(dbh))
		die(dbh, hdl);
	close_handle(dbh,hdl);
	printf("The footprint is %d Mb \n",i);

	mapi_disconnect(dbh);
	mapi_embedded_exit();
	return 0;
}
