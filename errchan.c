#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <pcre.h>
#include "hiredis.h"


#define OVECCOUNT 30        /* should be a multiple of 3 */

int switchDatabase( redisContext *c, char *db )
{
	redisReply *reply;
	int replytype;

	reply = redisCommand(c,"select %s", db);
	printf("Select database:%s %s\n", db, reply->str);

	replytype = reply->type;
	freeReplyObject(reply);
	return replytype;
}


int main( int argc, char **argv )
{
	int i=0, j=0, failed;
	redisContext *c;
	redisReply *reply, *tmpreply;
	char *YYYYMM, *ERRCODE;
	long int count=0, sumcount=0, sumerr=0;

	/* for getopt */
	int cc;
	int carg=0, darg=0, larg=0, parg=0, noneoptions=0;
	char *copt = NULL, *dopt = NULL;
	static struct option long_options[] = {
		{"database", 1, 0, 'd'},
		{"code", 1, 0, 'c'},
		{"listcode", 0, 0, 'l'},
		{"pattern", 1, 0, 'p'},
		{NULL, 0, NULL, 0}
	};
	int option_index = 0;

	/* for pcre Regexp matching */
    const char *error;
    int erroffset;
	pcre *re=0;
	char *pattern=0;
	int length;
	int rc;
	int ovector[OVECCOUNT];

	/* Procceed command line args */
	while ((cc = getopt_long(argc, argv, "d:c:lp:",
				long_options, &option_index)) != -1){
	    switch (cc){
			case 0:
			  printf ("option %s", long_options[option_index].name);
			  if (optarg)
			    printf (" with arg %s", optarg);
			  printf ("\n");
			  break;
			case 'l':
			  larg = 1;
			  break;
			case 'c':
			  printf ("option c with value '%s'\n", optarg);
			  carg = 1;
			  copt = optarg;
			  break;
			case 'd':
			  printf ("option d with value '%s'\n", optarg);
			  darg = 1;
			  dopt = optarg;
			  break;
			case 'p':
			  printf ("option p with value '%s'\n", optarg);
			  parg = 1;
			  pattern = optarg;
			  break;
			case '?':
			  break;
			default:
			  printf ("?? getopt returned character code 0%o ??\n", cc);
		}
    }
	
	if (optind < argc)
    	printf ("non-option ARGV-elements: ");
	else{
		printf("Usage: %s [-l/--listcode] [-c/--code <ErrorCode>] [-p/--pattern <RegexpString>] [-d/--database <dbindex>] <YYYY-MM>\n", argv[0] );
		exit(1);
	}

	YYYYMM = argv[optind];
	ERRCODE = copt ? copt : argv[optind+1];

    while (optind < argc){
		printf ("%s ", argv[optind++]);
		noneoptions ++;
	}
	printf("\n");

	/* Connect to redis server ... */
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
	if (c->err) {
		printf("Connection error: %s\n", c->errstr);
		exit(1);
	}

	/* PING server */
	reply = redisCommand(c,"PING");
	printf("PING: %s\n", reply->str);
	freeReplyObject(reply);

	/* Now switch to database (1), or the one pointed by command line option -d or --database */
	if(switchDatabase(c, darg?dopt:"1") == REDIS_REPLY_ERROR){
		printf("Can't switch to database %s\n", darg?dopt:"1" );
		exit(1);
	}

	/* precompile the Regexp(if -p argument present) */
	if( parg ) {
		re = pcre_compile(pattern,	/* the pattern */
			  0,	/* default options */
			  &error,	/* for error message */
			  &erroffset,	/* for error offset */
			  NULL);	/* use default character tables */

		/* Compilation failed: print the error message and exit */

		if (re == NULL) {
			printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
			return 1;
		}
	}


	/* 
	 *  List status codes (that matches the pattern from -p, if present )
	 *
	 */

	if( larg ){

		/* get the statuscode set from redis */
		reply = redisCommand(c, "SMEMBERS MT:%s:statuscode", YYYYMM );

		if (reply->type == REDIS_REPLY_ARRAY){    /* for every statuscode */
			for (i = 0; i < reply->elements; i++){
				if( parg ){
					length = strlen(reply->element[i]->str);
					rc = pcre_exec(re,	/* the compiled pattern */
		  	 				NULL,		/* no extra data - we didn't study the pattern */
							reply->element[i]->str,	/* the subject string */
							length,		/* the length of the subject */
							0,			/* start at offset 0 in the subject */
							0,			/* default options */
							ovector,	/* output vector for substring information */
							OVECCOUNT);	/* number of elements in the output vector */

					if(rc<0)			/* Not match */
						continue;
				}

				tmpreply = redisCommand(c, "GET MT:%s:%s:Count", YYYYMM, reply->element[i]->str);

				printf( "%3d) %s %10s\n", ++j, reply->element[i]->str, tmpreply->str );
				freeReplyObject(tmpreply);
			}
		}
		freeReplyObject(reply);
		exit(0);
	}

	printf("\nDate %s  code:\"%s\"\n\n", YYYYMM, ERRCODE );

	/*
	 *   Error code report
	 *
	 */

	j = 0;

	/* Get channel set within the given month */
	reply = redisCommand(c, "SMEMBERS MT:%s:%s:Channel", YYYYMM, ERRCODE );

	if (reply->type == REDIS_REPLY_ARRAY){

		printf("No.  CHAN   Total  %7s     %%\n", ERRCODE);
		printf("=== ===== ======== ======= ====== \n\n");

        for (i=j=0; i < reply->elements; i++) {

			if( parg ){
				length = strlen(reply->element[i]->str);
			    rc = pcre_exec(re,	/* Run the compiled pattern */
		  	 				NULL,		/* no extra data - we didn't study the pattern */
							reply->element[i]->str,	/* the subject string */
							length,		/* the length of the subject */
							0,			/* start at offset 0 in the subject */
							0,			/* default options */
							ovector,	/* output vector for substring information */
							OVECCOUNT);	/* number of elements in the output vector */

				if(rc<0)				/* Not match */
					continue;
			}
		
			/* Monthly error code number per channel    MT:month:statuscode:channel:Count */
			tmpreply = redisCommand(c, "GET MT:%s:%s:%s:Count", YYYYMM, ERRCODE, reply->element[i]->str);
		
			failed = atoi(tmpreply->str);
			freeReplyObject(tmpreply);

			/* Monthly MT Count per channel      MT:month:channel:Count */
			tmpreply = redisCommand(c, "GET MT:%s:%s:Count", YYYYMM, reply->element[i]->str );
			count = atoi(tmpreply->str);

			freeReplyObject(tmpreply);

			printf("%3d %s %8ld %7d %5.1f%%\n", ++j, reply->element[i]->str, count, failed, ((float)failed/count )*100);
			sumcount += count;
			sumerr += failed;
		}
	}else
		printf( "Code %s not found\n", ERRCODE );

	printf("\n=== ===== ======== ======= ====== \n");

	freeReplyObject(reply);
	printf("Sum ----- %7ld %8ld %5.1f%%\n", sumcount, sumerr, ((float)sumerr/sumcount)*100);
	pcre_free(re);

	return 0;
}
