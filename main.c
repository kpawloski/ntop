/*
 *  Copyright (C) 1998-2002 Luca Deri <deri@ntop.org>
 *
 *		 	    http://www.ntop.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Copyright (c) 1994, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ntop.h"
#include "globals-report.h"

/*
  Ntop options list
  -- converted to getopts_long, Burton M. Strauss III (BStrauss@acm.org)
*/
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif

#if defined(WIN32) && defined(__GNUC__)	/* mingw compiler */
 /* we're using the winpcap getopt() implementation
  * which has the globals inside the dll, so a simple
  * extern declaration is insufficient on win32
  *
  * Scott Renfro <scott@renfro.org>
  *
  */
extern __attribute__((dllimport)) char *optarg;
#else  /* !WIN32 */
extern char *optarg;
#endif


/*
 * local variables
 */
static int enableDBsupport=0;
static int enableThUpdate=1;
static int enableIdleHosts=1;

static char *devices = NULL;
static char *localAddresses = NULL;
static char *protoSpecs = NULL;

#ifndef WIN32
static int userId=0;
static int groupId=0;
#endif

static char *webAddr = NULL;
static char *flowSpecs = NULL;
static char rulesFile[128] = {0};
static char *sslAddr = NULL;


/*
 * Yoy are welcome
 */
static void welcome (FILE * fp)
{
  fprintf (fp, "%s v.%s %s [%s] (%s build)\n",
	   myGlobals.program_name, version, THREAD_MODE, osName, buildDate);

  fprintf (fp, "Copyright 1998-2002 by %s.\n", author);
  fprintf (fp, "Get the freshest ntop from http://www.ntop.org/\n");
}


/*
 * Wrong. Please try again accordingly to ....
 */
static void usage (FILE * fp)
{
  welcome(fp);

  fprintf(fp, "\nUsage: %s\n", myGlobals.program_name);

#ifdef HAVE_GETOPT_LONG

  fprintf(fp, "    [-a <path>      | --access-log-path <path>            Path for ntop web server access log\n");

#ifdef HAVE_MYSQL
  fprintf(fp, "    [-b <host:port> | --sql-host <host:port>]             SQL host for ntop database\n");
#endif

  fprintf(fp, "    [-c             | --sticky-hosts]                     Idle hosts are not purged from hash\n");

#ifndef WIN32
  fprintf(fp, "    [-d             | --daemon ]                          Run ntop in daemon mode\n");
#endif

#ifndef MICRO_NTOP
  fprintf(fp, "    [-e <number>    | --max-table-rows <number>]          Maximum number of table rows to report\n");
#endif

  fprintf(fp, "    [-f <file>      | --traffic-dump-file <file>]         Traffic dump file (see tcpdump)\n");
  fprintf(fp, "    [-g <host:port> | --cisco-netflow-host <host:port>]   Cisco NetFlow host and port\n");

#ifndef WIN32
  fprintf(fp, "    [-i <name>      | --interface <name>]                 Interface name or names to monitor\n");
#else
  fprintf(fp, "    [-i <number>    | --interface <number>]               Interface index number to monitor\n");
#endif

  fprintf(fp, "    [-j             | --border-sniffer-mode]              Set ntop in border/gateway sniffing mode\n");
  fprintf(fp, "    [-k             | --filter-expression-in-extra-frame] Show kernel filter expression in extra frame\n");
  fprintf(fp, "    [-l <path>      | --pcap-log <path>                   Dump packets captured to a file (debug only!)\n");
  fprintf(fp, "    [-m <addresses> | --local-subnets <addresses>]        Local subnetwork(s) (see man page)\n");
  fprintf(fp, "    [-n             | --numeric-ip-addresses]             Numeric IP addresses - no DNS resolution\n");
  fprintf(fp, "    [-p <list>      | --protocols <list>]                 List of IP protocols to monitor (see man page)\n");
  fprintf(fp, "    [-q             | --create-suspicious-packets         Create file ntop-suspicious-pkts.XXX.pcap file\n");
  fprintf(fp, "    [-r <number>    | --refresh-time <number>]            Refresh time in seconds, default is %d\n", REFRESH_TIME);
  fprintf(fp, "    [-s <number>    | --max-hash-size <number>]           Maximum hash table size, default = %d\n", MAX_HASH_SIZE);
  fprintf(fp, "    [-t <number>    | --trace-level <number>]             Trace level [0-5]\n");

#ifndef WIN32
  fprintf(fp, "    [-u <user>      | --user <user>]                      Userid/name to run ntop under (see man page)\n");
#endif /* WIN32 */

#ifdef HAVE_MYSQL
  fprintf(fp, "    [-v <username:password:dbName> | --mysql-host <username:password:dbName>] MySQL host for ntop database\n");
#endif

  fprintf(fp, "    [-w <port>      | --http-server]                      Web server (http:) port (or address:port) to listen on\n");
  fprintf(fp, "    [-A <number>    | --accuracy-level <number>]          Accuracy level [0-2]\n");
  fprintf(fp, "    [-B <filter>]   | --filter-expression                 Packet filter expression, like tcpdump\n");
  fprintf(fp, "    [-D <name>      | --domain <name>]                    Internet domain name\n");

#ifndef WIN32
  fprintf(fp, "    [-E             | --enable-external-tools]            Enable lsof/nmap integration (if present)\n");
#endif

  fprintf(fp, "    [-F <spec>      | --flow-spec <specs>]                Flow specs (see man page)\n");

#ifndef WIN32
  fprintf(fp, "    [-K             | --enable-debug]                     Enable debug mode\n");
  fprintf(fp, "    [-L             | --enable-syslog]                    Enable logging via syslog\n");
#endif

  fprintf(fp, "    [-M             | --no-interface-merge]               Don't merge network interfaces (see man page)\n");
  fprintf(fp, "    [-N             | --no-nmap]                          Don't use nmap even if installed\n");
  fprintf(fp, "    [-P <path>      | --db-file-path <path>               Path for ntop internal database files\n");
  fprintf(fp, "    [-R <file>      | --filter-rule <file>]               Matching rules file\n");
  fprintf(fp, "    [-S <number>    | --store-mode <number>]              Persistent storage mode [0-none, 1-local, 2-all\n");
  fprintf(fp, "    [-U <URL>       | --mapper <URL>]                     URL (mapper.pl) for displaying host location\n");

#ifdef HAVE_OPENSSL
  fprintf(fp, "    [-W <port>      | --https-server]                     Web server (https:) port (or address:port) to listen on\n");
#endif

  fprintf(fp, "    [-1             | --no-throughput-update>] \n");
  fprintf(fp, "    [-2             | --no-idle-hosts>] \n");


#else /* !HAVE_GETOPT_LONG */

  fprintf(fp, "    [-a <path> path for ntop web server access log]\n");

#ifdef HAVE_MYSQL
  fprintf(fp, "    [-b <client:port (ntop DB client)>]\n");
#endif

  fprintf(fp, "    [-c <sticky hosts: idle hosts are not purged from hash>]\n");

#ifndef WIN32
  fprintf(fp, "    [-d (run ntop in daemon mode)]\n");
#endif

#ifndef MICRO_NTOP
  fprintf(fp, "    [-e <max # table rows)]\n");
#endif

  fprintf(fp, "    [-f <traffic dump file (see tcpdump)>]\n");
  fprintf(fp, "    [-g <client:port (Cisco NetFlow client)>]\n");

#ifndef WIN32
  fprintf(fp, "    [-i <interface>]\n");
#else
  fprintf(fp, "    [-i <interface index>]\n");
#endif

  fprintf(fp, "    [-j (set ntop in border gateway sniffing mode)]\n");
  fprintf(fp, "    [-k <show kernel filter expression in extra frame>]\n");
  fprintf(fp, "    [-l <path> (dump packets captured on a file: debug only!)]\n");
  fprintf(fp, "    [-m <local addresses (see man page)>]\n");
  fprintf(fp, "    [-n (numeric IP addresses)]\n");
  fprintf(fp, "    [-p <IP protocols to monitor> (see man page)]\n");
  fprintf(fp, "    [-q <create file ntop-suspicious-pkts.XXX.pcap>]\n");

#ifdef WIN32
  fprintf(fp, "    [-r <refresh time (web = %d sec)>]\n", REFRESH_TIME);
#else
  fprintf(fp, "    [-r <refresh time (interactive = %d sec/web = %d sec)>]\n",
	   ALARM_TIME, REFRESH_TIME);
#endif

  fprintf(fp, "    [-s <max hash size (default 32768)>]\n");
  fprintf(fp, "    [-t (trace level [0-5])]\n");

#ifndef WIN32
  fprintf(fp, "    [-u <userid> | <username> (see man page)]\n");
#endif

#ifdef HAVE_MYSQL
  fprintf(fp, "    [-v <username:password:dbName (ntop mySQL client)>]\n");
#endif

  fprintf(fp, "    [-w <HTTP port>]\n");

  fprintf(fp, "    [-A (accuracy level [0-2])]\n");
  fprintf(fp, "    [-B <filter expression (like tcpdump)>]\n");
  fprintf(fp, "    [-D <Internet domain name>]\n");

#ifndef WIN32
  fprintf(fp, "    [-E <enable lsof/nmap integration (if present)>]\n");
#endif

  fprintf(fp, "    [-F <flow specs (see man page)>]\n");

#ifndef WIN32
  fprintf(fp, "    [-K <enable application debug (no fork() is used)>]\n");
  fprintf(fp, "    [-L <use syslog instead of stdout>]\n");
#endif

  fprintf(fp, "    [-M <don't merge network interfaces (see man page)>]\n");
  fprintf(fp, "    [-N <don't use nmap if installed>]\n");
  fprintf(fp, "    [-P <path for db-files>]\n");
  fprintf(fp, "    [-R <matching rules file>]\n");
  fprintf(fp, "    [-S <store mode> (store persistently host stats)]\n");
  fprintf(fp, "    [-U <mapper.pl URL> | \"\" for not displaying host location]\n");

#ifdef HAVE_OPENSSL
  fprintf(fp, "    [-W <HTTPS port>]\n");
#endif

  fprintf(fp, "    [-1 <no throughput update>] \n");
  fprintf(fp, "    [-2 <no purge of idle hosts>] \n");

#endif /* HAVE_GETOPT_LONG */
}


/*
 * Parse the command line options
 */
static void parseoptions (int argc, char * argv [])
{
  int len;

#ifdef WIN32
  int optind=0;
#endif

#ifdef HAVE_GETOPT_LONG
  int getop_long_index;

  static struct option long_options[] = {
    { "access-log-path",                  required_argument, NULL, 'a' },

#ifdef HAVE_MYSQL
    { "sql-host",                         required_argument, NULL, 'b' },
#endif

    { "sticky-hosts",                     no_argument,       NULL, 'c' },

#ifndef WIN32
    { "daemon",                           no_argument,       NULL, 'd' },
#endif

#ifndef MICRO_NTOP
    { "max-table-rows",                   required_argument, NULL, 'e' },
#endif

    { "traffic-dump-file",                required_argument, NULL, 'f' },
    { "cisco-netflow-host",               required_argument, NULL, 'g' },
    { "help",                             no_argument,       NULL, 'h' },
    { "interface",                        required_argument, NULL, 'i' },
    { "border-sniffer-mode",              no_argument,       NULL, 'j' },
    { "filter-expression-in-extra-frame", no_argument,       NULL, 'k' },
    { "pcap-log",                         required_argument, NULL, 'l' },
    { "local-subnets",                    required_argument, NULL, 'm' },
    { "numeric-ip-addresses",             no_argument,       NULL, 'n' },
    { "protocols",                        required_argument, NULL, 'p' },
    { "create-suspicious-packets",        no_argument,       NULL, 'q' },
    { "refresh-time",                     required_argument, NULL, 'r' },
    { "max-hash-size",                    required_argument, NULL, 's' },
    { "trace-level",                      required_argument, NULL, 't' },

#ifndef WIN32
    { "user",                             required_argument, NULL, 'u' },
#endif

#ifdef HAVE_MYSQL
    { "mysql-host",                       required_argument, NULL, 'v' },
#endif

    { "http-server",                      required_argument, NULL, 'w' },
    { "accuracy-level",                   required_argument, NULL, 'A' },
    { "filter-expression",                required_argument, NULL, 'B' },
    { "domain",                           required_argument, NULL, 'D' },

#ifndef WIN32
    { "enable-external-tools",            no_argument,       NULL, 'E' },
#endif

    { "flow-spec",                        required_argument, NULL, 'F' },

#ifndef WIN32
    { "interactive-mode",                 no_argument,       NULL, 'I' }, /* interactive mode no longer used */
    { "debug",                            no_argument,       NULL, 'K' },
    { "use-syslog",                       no_argument,       NULL, 'L' },
#endif

    { "no-interface-merge",               no_argument,       NULL, 'M' },

#ifndef WIN32
    { "no-nmap",                          no_argument,       NULL, 'N' },
#endif

    { "db-file-path",                     required_argument, NULL, 'P' },
    { "filter-rule",                      required_argument, NULL, 'R' },
    { "store-mode",                       required_argument, NULL, 'S' },
    { "mapper",                           required_argument, NULL, 'U' },

#ifdef HAVE_OPENSSL
    { "https-server",                     required_argument, NULL, 'W' },
#endif

    { "no-throughput-update",             no_argument,       NULL, '1' },
    { "no-idle-host-purge",               no_argument,       NULL, '2' },

    /*
     * long ONLY options - put these here with numeric arguments,
     *  over 127 (i.e. > ascii max char)
     * (since op is unsigned this is fine)
     *  add corresponding case nnn: below
     */
#ifdef HAVE_GDCHART
    { "throughput-bar-chart",             no_argument,       NULL, 129 },
#endif

    {0, 0, 0, 0}
  };
#endif /* HAVE_GETOPT_LONG */

  /*
   * Please keep the array sorted
   */
#ifdef WIN32
  char * theOpts = "a:ce:f:g:hi:jkl:m:np:qr:s:t:w:A:D:F:MP:R:S:U:W:12";
#else
  char * theOpts = "a:b:cde:f:g:hi:jkl:m:np:qr:s:t:u:v:w:A:D:EF:IKLMNP:R:S:U:W:12";
#endif
  int opt;

  /*
   * Parse command line options to the application via standard system calls
   */
#ifdef HAVE_GETOPT_LONG
  while((opt = getopt_long(argc, argv, theOpts, long_options, &getop_long_index)) != EOF) {
#else
  while((opt = getopt(argc, argv, theOpts)) != EOF) {
#endif
    switch (opt) {

    case 'a': /* ntop access log path */
      stringSanityCheck(optarg);
      strncpy(myGlobals.accessLogPath, optarg,
	      sizeof(myGlobals.accessLogPath)-1)[sizeof(myGlobals.accessLogPath)-1] = '\0';
      break;

    case 'b': /* host:port */
      stringSanityCheck(optarg);
      handleDbSupport(optarg, &enableDBsupport);
      break;

      /* Courtesy of Ralf Amandi <Ralf.Amandi@accordata.net> */
    case 'c': /* Sticky hosts = hosts that are not purged when idle */
      myGlobals.stickyHosts = 1;
      break;

#ifndef WIN32
    case 'd':
      myGlobals.daemonMode=1;
      break;
#endif

#ifndef MICRO_NTOP
    case 'e':
      maxNumLines = atoi(optarg);
      break;
#endif

    case 'f':
      myGlobals.isLsofPresent = 0; /* Don't make debugging too complex */
      myGlobals.rFileName = optarg;
      break;

    case 'g': /* host:port */
      stringSanityCheck(optarg);
      handleNetFlowSupport(optarg);
      break;

    case 'i':
      stringSanityCheck(optarg);
      devices = optarg;
      break;

    case 'j':
      /*
	In this mode ntop sniffs from an interface on which
	 the traffic has been mirrored hence:
	 - MAC addresses are not used at all but just IP addresses
	 - ARP packets are not handled
      */
      myGlobals.borderSnifferMode = 1;
      break;

    case 'k':
      /* update info of used kernel filter expression in extra frame */
      myGlobals.filterExpressionInExtraFrame=1;
      break;

    case 'l':
      stringSanityCheck(optarg);
      myGlobals.pcapLog = optarg;
      break;

    case 'm':
      stringSanityCheck(optarg);
      localAddresses = strdup(optarg);
      break;

    case 'n':
      myGlobals.numericFlag++;
      break;

    case 'p':
      stringSanityCheck(optarg);
      len = strlen(optarg);
      if(len > 2048) len = 2048;
      protoSpecs = (char*)malloc(len+1);
      memset(protoSpecs, 0, len+1);
      strncpy(protoSpecs, optarg, len);
      break;

    case 'q': /* allow ntop to save suspicious packets
		 in a file in pcap (tcpdump) format */
      myGlobals.enableSuspiciousPacketDump=1;
      break;

    case 'r':
      if(!isdigit(optarg[0])) {
	printf("FATAL ERROR: flag -r expects a numeric argument.\n");
	exit(-1);
      }
      refreshRate = atoi(optarg);
      break;

    case 's':
      myGlobals.maxHashSize = atoi(optarg);
      if(myGlobals.maxHashSize < 64) {
	myGlobals.maxHashSize = 64;
	traceEvent(TRACE_INFO, "Max hash size set to 64 (minimum hash size)");
      }
      break;

    case 't':
      /* Trace Level Initialization */
      myGlobals.traceLevel = atoi(optarg);
      if(myGlobals.traceLevel > DETAIL_TRACE_LEVEL)
	myGlobals.traceLevel = DETAIL_TRACE_LEVEL;
      break;

#ifndef WIN32
    case 'u':
      stringSanityCheck(optarg);
      if(strOnlyDigits(optarg))
	userId = atoi(optarg);
      else {
	struct passwd *pw;
	pw = getpwnam(optarg);
	if(pw == NULL) {
	  printf("FATAL ERROR: Unknown user %s.\n", optarg);
	  exit(-1);
	}
	userId = pw->pw_uid;
	groupId = pw->pw_gid;
	endpwent();
      }
      break;
#endif /* WIN32 */

#ifdef HAVE_MYSQL
    case 'v': /* username:password:dbname:host */
      stringSanityCheck(optarg);
      handlemySQLSupport(optarg, &enableDBsupport);
      break;
#endif

    case 'w':
      stringSanityCheck(optarg);
      if(!isdigit(optarg[0])) {
	printf("FATAL ERROR: flag -w expects a numeric argument.\n");
	exit(-1);
      }

      /* Courtesy of Daniel Savard <daniel.savard@gespro.com> */
      if ((webAddr = strchr(optarg,':'))) {
	/* DS: Search for : to find xxx.xxx.xxx.xxx:port */
	/* This code is to be able to bind to a particular interface */
	*webAddr = '\0';
	webPort = atoi(webAddr+1);
	webAddr = optarg;
      } else {
	webPort = atoi(optarg);
      }
      break;

    case 'A':
      /* Accuracy Level */
      myGlobals.accuracyLevel = atoi(optarg);
      if(myGlobals.accuracyLevel > HIGH_ACCURACY_LEVEL)
	myGlobals.accuracyLevel = HIGH_ACCURACY_LEVEL;
      break;

    case 'B':
      stringSanityCheck(optarg);
      myGlobals.currentFilterExpression = (char*)malloc(strlen(optarg)+1);
      strcpy(myGlobals.currentFilterExpression, optarg);
      break;


    case 'D': /* domain */
      stringSanityCheck(optarg);
      strncpy(myGlobals.domainName, optarg,
	      sizeof(myGlobals.domainName)-1)[sizeof(myGlobals.domainName)-1] = '\0';
      break;

    case 'E':
      myGlobals.isLsofPresent  = checkCommand("lsof");
      myGlobals.isNmapPresent  = checkCommand("nmap");
      break;

    case 'F':
      stringSanityCheck(optarg);
      len = strlen(optarg);
      if(len > 2048) len = 2048;
      flowSpecs = (char*)malloc(len+1);
      memset(flowSpecs, 0, len+1);
      strncpy(flowSpecs, optarg, len);
      break;

#ifndef WIN32
    case 'I': /* Interactive mode */
      printf("intop provides you curses support. ntop -I is no longer used.\n");
      exit(-1);
#endif

#ifndef WIN32
    case 'K':
      myGlobals.debugMode = 1;
      break;

    case 'L':
      myGlobals.useSyslog = 1;
      break;
#endif

    case 'M':
      myGlobals.mergeInterfaces = 0;
      break;

    case 'N':
      myGlobals.isNmapPresent = 0;
      break;

    case 'P': /* DB-Path */
      stringSanityCheck(optarg);
      strncpy(myGlobals.dbPath, optarg, sizeof(myGlobals.dbPath)-1)[sizeof(myGlobals.dbPath)-1] = '\0';
      break;

    case 'R':
      stringSanityCheck(optarg);
      strncpy(rulesFile, optarg,
	      sizeof(rulesFile)-1)[sizeof(rulesFile)-1] = '\0';
      break;

    case 'S':
      /*
	Persitent storage only for 'local' machines
	Courtesy of Joel Crisp <jcrisp@dyn21-126.trilogy.com>

	0 = no storage
	1 = store all hosts
	2 = store only local hosts
      */
      myGlobals.usePersistentStorage = atoi(optarg);
      if((myGlobals.usePersistentStorage > 2)
	 || (myGlobals.usePersistentStorage < 0)){
	printf("FATAL ERROR: -S flag accepts value in the 0-2 range.\n");
	exit(-1);
      }
      break;

    case 'U': /* host:port */
      if(strlen(optarg) >= (sizeof(myGlobals.mapperURL)-1)) {
	strncpy(myGlobals.mapperURL, optarg, sizeof(myGlobals.mapperURL)-2);
	myGlobals.mapperURL[sizeof(myGlobals.mapperURL)-1] = '\0';
      } else
	strcpy(myGlobals.mapperURL, optarg);
      break;

#ifdef HAVE_OPENSSL
    case 'W':
      stringSanityCheck(optarg);
      if(!isdigit(optarg[0])) {
	printf("FATAL ERROR: flag -W expects a numeric argument.\n");
	exit(-1);
      }

      /*
	lets swipe the same address binding code from -w above
	Curtis Doty <Curtis@GreenKey.net>
      */
      if((sslAddr = strchr(optarg,':'))) {
	*sslAddr = '\0';
	myGlobals.sslPort = atoi(sslAddr+1);
	sslAddr = optarg;
      } else {
	myGlobals.sslPort = atoi(optarg);
      }

      break;
#endif

    case '1': /* disable throughput update */
      enableThUpdate=0;
      break;

    case '2': /* disable purging of idle hosts */
      enableIdleHosts=0;
      break;

    default:
      usage(stdout);
      exit(-1);
    }
  }
}



/* That's the meat */
int main(int argc, char *argv[]) {

  int i;

  char ifStr[196] = {0};
  time_t lastTime;

  /*
   * Initialize all global run-time parameters to reasonable values
   */
  initNtopGlobals(argc, argv);

#ifdef MEMORY_DEBUG
  initLeaks();
#endif

  /*
   * Parse command line options to the application via standard system calls
   */
  parseoptions (argc, argv);

  if(webPort == 0) {
#ifdef HAVE_OPENSSL
    if(myGlobals.sslPort == 0) {
      traceEvent(TRACE_ERROR,
		 "FATAL ERROR: both -W and -w can't be set to 0.\n");
      exit(-1);
    }
#else
    traceEvent(TRACE_ERROR,
	       "FATAL ERROR: -w can't be set to 0.\n");
    exit(-1);
#endif
  }

  /* ***************************** */

#ifdef HAVE_OPENSSL
  if(myGlobals.sslPort == 0)
    traceEvent(TRACE_INFO, "SSL is present but https is disabled: "
	       "use -W <https port> for enabling it\n");
#endif

  initIPServices();

  initLogger(); /* Do not call this function before myGlobals.dbPath
		   is initialized */

  initGlobalValues();

#ifndef MICRO_NTOP
  reportValues(&lastTime);
#endif /* MICRO_NTOP */

  postCommandLineArgumentsInitialization(&lastTime);

  initGdbm();

  initDevices(devices);

  printf("Wait please: ntop is coming up...\n");

  traceEvent(TRACE_INFO, "ntop v.%s %s [%s] (%s build)",
	     version, THREAD_MODE, osName, buildDate);

  if(myGlobals.rFileName != NULL)
    strncpy(ifStr, PCAP_NW_INTERFACE, sizeof(ifStr));
  else
    for(i=0; i<myGlobals.numDevices; i++) {
      char tmpBuf[48];

      if(i>0) {
	if(snprintf(tmpBuf, sizeof(tmpBuf), ",%s", myGlobals.device[i].name)  < 0)
	  traceEvent(TRACE_ERROR, "Buffer overflow!");
      } else {
	if(snprintf(tmpBuf, sizeof(tmpBuf), "%s", myGlobals.device[i].name) < 0)
	  traceEvent(TRACE_ERROR, "Buffer overflow!");
      }
      strncat(ifStr, tmpBuf, sizeof(ifStr)-strlen(ifStr)-1)[sizeof(ifStr)-1] = '\0';
    }

  traceEvent(TRACE_INFO, "Listening on [%s]", ifStr);
  traceEvent(TRACE_INFO, "Copyright 1998-2002 by %s\n", author);
  traceEvent(TRACE_INFO, "Get the freshest ntop from http://www.ntop.org/\n");
  traceEvent(TRACE_INFO, "Initializing...\n");

  initLibpcap(rulesFile, myGlobals.numDevices);

#ifndef MICRO_NTOP
  loadPlugins();
#endif

  /*
    Code fragment below courtesy of
    Andreas Pfaller <apfaller@yahoo.com.au>
  */
#ifndef WIN32
  if((getuid() != geteuid()) || (getgid() != getegid())) {
    /* setuid binary, drop privileges */
    if (setgid(getgid())!=0 || setuid(getuid())!=0) {
      traceEvent(TRACE_ERROR,
		 "FATAL ERROR: Unable to drop privileges.\n");
      exit(-1);
    }
  }

  if((userId != 0) || (groupId != 0)){
    /* user id specified on commandline */
    if ((setgid(groupId) != 0) || (setuid(userId) != 0)) {
      traceEvent(TRACE_ERROR, "FATAL ERROR: Unable to change user ID.\n");
      exit(-1);
    }
  }

  if((geteuid() == 0) || (getegid() == 0)) {
    traceEvent(TRACE_INFO, "WARNING: For security reasons it is STRONGLY recommended to");
    traceEvent(TRACE_INFO, "WARNING: run ntop as unprivileged user by using the -u option!");
  }
#endif

  if(localAddresses != NULL) {
    handleLocalAddresses(localAddresses);
    free(localAddresses);
    localAddresses = NULL;
  }

  initDeviceDatalink();

  if(myGlobals.currentFilterExpression != NULL)
    parseTrafficFilter();
  else
    myGlobals.currentFilterExpression = strdup(""); /* so that it isn't NULL! */

  /* Handle flows (if any) */
  if(flowSpecs != NULL) {
    if(flowSpecs[0] != '\0')
      handleFlowsSpecs(flowSpecs);
    free(flowSpecs);
  }

  /* Patch courtesy of Burton M. Strauss III <BStrauss3@attbi.com> */
  if(protoSpecs != NULL) {
    if(protoSpecs[0] != '\0')
      handleProtocols(protoSpecs);
    free(protoSpecs);
  }

  /*
     Moved from initialize.c (postCommandLineArgumentsInitialization) so that we
     don't add the defaults if the user has given us at least SOMETHING to monitor

     Fix courtesy of Burton M. Strauss III <BStrauss3@attbi.com>
  */
  if(myGlobals.numIpProtosToMonitor == 0)
    addDefaultProtocols();

  initCounters(myGlobals.mergeInterfaces);
  initApps();
  initSignals();

  initThreads(enableThUpdate, enableIdleHosts, enableDBsupport);

#ifndef MICRO_NTOP
  startPlugins();
#endif

  initWeb(webPort, webAddr, sslAddr);

  traceEvent(TRACE_INFO, "Sniffying...\n");

#ifdef MEMORY_DEBUG
  resetLeaks();
#endif

  /*
    In multithread mode, a separate thread handles
    packet sniffing
  */
#ifndef MULTITHREADED
  packetCaptureLoop(&lastTime, refreshRate);
#else
  startSniffer();
#endif

#ifndef WIN32
  pause();
#endif
  while(!myGlobals.endNtop)
    sleep(30);

  return(0);
}
