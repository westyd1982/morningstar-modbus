/*
 *  dailygraphs.c - Create a web page with all the past daily graphs
 *
 
 Copyright 2013 Tom Rinehart.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see http://www.gnu.org/licenses/.
 *
 *  Created by Tom Rinehart on 7/14/2009.
 *  Revised to get year string from localtime for directory and html file name on 12/30/2010.
 *  Revised to use a common header file for file paths on 3/22/2013.
 *
 *  Compile with: cc dailygraphs.c -o dailygraphs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>

#include "powersystem.h"

int int_cmp(const void *a, const void *b);

int main(void)
{
	int i, j, idate, date[366];
	FILE *htmlfile;
	DIR *dh;				//directory handle
	struct dirent *file;	//a 'directory entity' AKA file    
	struct stat info;		//info about the file.
	
	time_t lclTime;
	struct tm *now;
	char yearString[5];
	char htmldirString[64];
	char htmlfileString[64];
	
	/* Create a time stamp for sample result */
	lclTime = time(NULL);
	now = localtime(&lclTime);
	
	strftime(yearString, 5, "%Y", now);
	sprintf(htmldirString,"%s/%s",WEBPAGEFILEPATH,yearString);
	sprintf(htmlfileString,"%s/%s/%sdailygraphs.html",WEBPAGEFILEPATH,yearString,yearString);
	
	i=0;
	dh=opendir(htmldirString);
	while(file=readdir(dh)) {
		if(file->d_name[0]=='2') {
			stat(file->d_name,&info);
			if(!S_ISDIR(info.st_mode)) {
				sscanf(file->d_name,"%8d%*4c",&idate);
				if (idate > 10000000) {					// Only copy file names that are a date (e.g., 20130324.png)
					date[i]=idate;
					i++;
				}
			}
		}
	}
	closedir(dh);
	
	qsort(date, i, sizeof(int), int_cmp);
	
	/* Write data to html file */
	if ((htmlfile = fopen(htmlfileString, "w")) == NULL) { 
		printf("Can't create %sdailygraphs.html file.\n",yearString);
		exit(1);
	}
	
	fprintf(htmlfile,"<html>\n<head>\n<title>%s Daily Power System Graphs</title>\n</head>\n",yearString);
	fprintf(htmlfile,"<body bgcolor=\"#6699FF\" text=\"#000000\" link=\"#330099\" vlink=\"#336633\" alink=\"#FFCC00\">\n");
	fprintf(htmlfile,"<font face=\"Comic Sans MS, Arial, Helvetica\">\n");
	fprintf(htmlfile,"<h3><font color=\"#663300\">%s Daily Power System Graphs</font></h3>\n",yearString);
	for (j=0;j<i;j++) {		// Start at j=1 to skip today's graph
		fprintf(htmlfile,"<img src=\"%d.png\"><br><br>\n",date[j]);
	}
	fprintf(htmlfile,"</body>\n</html>\n");

	fclose(htmlfile);
	
	return 0;
}

/* qsort int comparison function */
int int_cmp(const void *a, const void *b)
{
    const int *ia = (const int *)a; // casting pointer types
    const int *ib = (const int *)b;
//    return *ia  - *ib;	// ascending order
    return *ib  - *ia;		// decending order
	/* integer comparison: returns negative if b > a 
	and positive if a > b */
}
