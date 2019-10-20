
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <selinux/android.h>

#define TEST

#define DEFAULT_CONTEXT "u:r:system_app:s0"    
#define SS_CONTEXT "u:r:system_server:s0"
#define DEFAULT_UID 1000

#define PROGNAME "runas_system"

#define LOG  printf


__noreturn void panic(const char* format, ...)
{
    va_list args;
    int e = errno;

    fprintf(stdout, "%s: ", PROGNAME);
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    exit(e ? -e : 1);
}

void get_uidgid_from_pid(pid_t pid, uid_t* uid, gid_t *gid)
{
    FILE *fp;
    char filename[128], line[256];

    sprintf(filename, "/proc/%d/status", pid);
    if ((fp = fopen(filename, "r")) == NULL)
        panic("Error to open status file(%s): %s\n", filename, strerror(errno) );

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (!strncmp(line, "Uid:", 4)) {
            sscanf(line + 5, "%u", uid);
        } else if (!strncmp(line, "Gid:", 4)) {
            sscanf(line + 5, "%u", gid);
            break;
        }
    }
    fclose(fp);
}

void usage()
{
    LOG("Usage:\n  " PROGNAME " [-p pid] [command args]\n");
    LOG("  if no command, a shell will be runned\n");
    LOG("  the default context is '" DEFAULT_CONTEXT "', use [-p pid] to change it.\n");
}

int main(int argc, char **argv)
{
    int commandArgvOfs = 1;
    uid_t pid, uid, gid;
    char *ctx_str = "null";

    if (argc==2 && (!strncmp(argv[1], "-h", 2) || !strncmp(argv[1], "--help", 6)) ) {
        usage();
        exit(0);
    }

    if( argc>=3 && argv[1][0]=='-' && argv[1][1]=='p') {  // pid
        commandArgvOfs += 2;

        pid = atoi(argv[2]);
        if (pid==0) {
            LOG("parameter error!");
            usage();
            exit(1);
        }

        get_uidgid_from_pid(pid, &uid, &gid);
        if (getpidcon(pid, &ctx_str) < 0) {
            panic("get selinux contest of pid(%d) failed:%s\n", pid, strerror(errno));
        }
    } else { // use default config
        uid = gid = DEFAULT_UID; // system
        ctx_str = DEFAULT_CONTEXT;
    }

    LOG("target uid=%d, context=%s\n", uid, ctx_str);

    if(setresgid(gid,gid,gid) || setresuid(uid,uid,uid)) {
        panic("Permission denied to setuid/setgid\n");
    }
    if (setcon(ctx_str) < 0){
        panic("Error in set context %s: %s\n", ctx_str, strerror(errno) );
    }


    /* User specified command for exec. */
    if ((argc >= commandArgvOfs + 1) &&
        (execvp(argv[commandArgvOfs], argv+commandArgvOfs) < 0)) 
    {
        panic("Exec '%s' failed: %s\n", argv[commandArgvOfs], strerror(errno));
    }

    /* Default exec shell. */
    execlp("/system/bin/sh", "sh", NULL);
    panic("run shell failed: %s\n", strerror(errno));  

    return 0;
}
