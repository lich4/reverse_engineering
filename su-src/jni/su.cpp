/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <errno.h>

#include <getopt.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <pwd.h>



#define AID_ROOT             0  /* traditional unix root user */

#define AID_SYSTEM        1000  /* system server */

#define AID_SHELL         2000  /* adb and debug shell user */

#define AID_NOBODY        9999

#define AID_APP          10000  /* first app user */

#define AID_USER        100000  /* offset for uid ranges for each user */





void pwtoid(const char* tok, uid_t* uid, gid_t* gid) {

    struct passwd* pw = getpwnam(tok);

    if (pw)

    {

        if (uid) *uid = pw->pw_uid;

        if (gid) *gid = pw->pw_gid;

    }

    else

    {

        char* end;

        errno = 0;

        uid_t tmpid = strtoul(tok, &end, 10);

        if (errno != 0 || end == tok)

                printf("invalid uid/gid '%s'", tok);

        if (uid) *uid = tmpid;

        if (gid) *gid = tmpid;

    }

}



void extract_uidgids(const char* uidgids, uid_t* uid, gid_t* gid, gid_t* gids, int* gids_count) {

    char *clobberablegids;

    char *nexttok;

    char *tok;

    int gids_found;



    if (!uidgids || !*uidgids)

    {

        *gid = *uid = 0;

        *gids_count = 0;

        return;

    }



    clobberablegids = strdup(uidgids);

    strcpy(clobberablegids, uidgids);

    nexttok = clobberablegids;

    tok = strsep(&nexttok, ",");

    pwtoid(tok, uid, gid);

    tok = strsep(&nexttok, ",");

    if (!tok)

    {

        /* gid is already set above */

        *gids_count = 0;

        free(clobberablegids);

        return;

    }

    pwtoid(tok, NULL, gid);

    gids_found = 0;

    while ((gids_found < *gids_count) && (tok = strsep(&nexttok, ",")))

    {

        pwtoid(tok, NULL, gids);

        gids_found++;

        gids++;

    }

    if (nexttok && gids_found == *gids_count)

    {

        fprintf(stderr, "too many group ids\n");

    }

    *gids_count = gids_found;

    free(clobberablegids);

}



int main(int argc, char** argv)

{

    uid_t current_uid = getuid();



    // Handle -h and --help.

    ++argv;

    if (*argv && (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0))

    {

        fprintf(stderr,

                "usage: su [UID[,GID[,GID2]...]] [COMMAND [ARG...]]\n"

                "\n"

                "Switch to WHO (default 'root') and run the given command (default sh).\n"

                "\n"

                "where WHO is a comma-separated list of user, group,\n"

                "and supplementary groups in that order.\n"

                "\n");

        return 0;

    }



    uid_t uid = 0;

    gid_t gid = 0;



    if (*argv)

    {

        gid_t gids[10];

        int gids_count = sizeof(gids)/sizeof(gids[0]);

        extract_uidgids(*argv, &uid, &gid, gids, &gids_count);

        if (gids_count) {

            if (setgroups(gids_count, gids))

            {

                printf("setgroups failed");

            }

        }

        ++argv;

    }



    if (setgid(gid))

            printf("setgid failed");

    if (setuid(uid))

            printf("setuid failed");



    //设置环境变量

    setenv("PATH", "/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin", 1);

    setenv("LD_LIBRARY_PATH", "/vendor/lib:/system/lib", 1);

    setenv("ANDROID_BOOTLOGO", "1", 1);

    setenv("ANDROID_ROOT", "/system", 1);

    setenv("ANDROID_DATA", "/data", 1);

    setenv("ANDROID_ASSETS", "/system/app", 1);

    setenv("EXTERNAL_STORAGE", "/sdcard", 1);

    setenv("ASEC_MOUNTPOINT", "/mnt/asec", 1);

    setenv("LOOP_MOUNTPOINT", "/mnt/obb", 1);

    char* exec_args[argc + 1];

    size_t i = 0;

    for (; *argv != NULL; ++i)

    {

      exec_args[i] = *argv++;

    }



    if (i == 0) exec_args[i++] = "/system/bin/sh";

    exec_args[i] = NULL;



    execvp(exec_args[0], exec_args);

    printf("failed to exec %s", exec_args[0]);

}

