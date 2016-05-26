#include "LinuxHostDevice.h"
#include "MainApp.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TAW_CPU_KERNEL_MAX 16 // 最多支持16核
#define ONE_MB (1024 * 1024)

CLinuxHostDevice::CLinuxHostDevice(void)
{
}


CLinuxHostDevice::~CLinuxHostDevice(void)
{
}

// 获取CPU使用率
bool CLinuxHostDevice::GetCpuOverLoad(COLLECT_DATA::MCpuLoad& cpuload)
{
    float cpu_f[16];
    get_cpu_real(cpu_f);
    int cpu_all = (int)(100*cpu_f[0]);
    cpuload.set_load(cpu_all);

    return true;
}

// 获取内存使用率
bool CLinuxHostDevice::GetMemUsage(COLLECT_DATA::MMemInfo& meminfo)
{
    long num_procs = 0;
    long page_size = 0;
    long num_pages = 0;
    long free_pages = 0;
    long long  mem = 0;
    long long  free_mem = 0;

    num_procs = sysconf (_SC_NPROCESSORS_CONF);
    //printf ("CPU 个数为: %ld 个\n", num_procs);

    page_size = sysconf (_SC_PAGESIZE);
    //printf ("系统页面的大小为: %ld K\n", page_size / 1024 );

    num_pages = sysconf (_SC_PHYS_PAGES);
    //printf ("系统中物理页数个数: %ld 个\n", num_pages);

    free_pages = sysconf (_SC_AVPHYS_PAGES);
    //printf ("系统中可用的页面个数为: %ld 个\n", free_pages);

    mem = (long long) ((long long)num_pages * (long long)page_size);
    mem /= ONE_MB;

    free_mem = (long long)free_pages * (long long)page_size;
    free_mem /= ONE_MB;

    //printf ("总共有 %lld MB 的物理内存, 空闲的物理内存有: %lld MB\n", mem, free_mem);
    meminfo.set_total(mem);
    meminfo.set_free(free_mem);
    float used = mem - free_mem;
    float usage = (used / mem) * 100;
    meminfo.set_usage(usage);

    return true;
}

// 获取磁盘空间
bool CLinuxHostDevice::GetDiskInfo(COLLECT_DATA::MDiskInfo& diskinfo)
{
    char cmd[32] = {"df"};
    char sbuf[READ_BUF_SIZE] = {0};

    char tmp[256] = {0}, space[32] = {0},used[32] = {0},avail[32] = {0},use[32] = {0},mount[256] = {0};

    FILE *fp = popen(cmd,"r");
    fgets(sbuf,sizeof(sbuf),fp);  //discard title 
    while( fgets(sbuf,sizeof(sbuf),fp) )
    {
        COLLECT_DATA::MDiskInfoItem *item = diskinfo.add_diskinfo();
        char *p = strrchr(sbuf,'\n');
        *p = 0;

        sscanf(sbuf,"%s%s%s%s%s%s",tmp,space,used,avail,use,mount);

        float fspace = atof(space)/1024;
        float fused = atof(used)/1024;
        float favail = atof(avail)/1024;
        float fusage = (fused / fspace) * 100;

        item->set_space(fspace);
        item->set_used(fused);
        item->set_available(favail);
        item->set_usage(fusage);
        item->set_mount_point(mount);
        //printf("space=%-8s used=%-8s avail=%-8s use=%-8s mount=%-8s\n",space,used,avail,use,mount);        
    }

    return true;
}


int CLinuxHostDevice::get_cpu_load_new( )
{
    char cpu_load[1024] = {0x00};
    int buflen = sizeof(cpu_load);
    unsigned int total = 0, user = 0, nice = 0, system = 0, idle = 0;

    char cpu[32] = {0x00};
    char text[512] = {0x00};

    FILE *fp = NULL;
    fp = fopen("/proc/stat", "r");
    if ( fp == NULL )
        return -1;

    while (fgets(text, 200, fp))
    {
        if (strstr(text, "cpu") == NULL )
            break;

        sscanf(text, "%s %u %u %u %u", cpu, &user, &nice, &system, &idle);
        total = (user + nice + system + idle);

        snprintf(cpu_load+strlen(cpu_load), buflen-strlen(cpu_load),
            "%s %u %u %u %u %u\n",
            cpu, user, nice, system, idle,total);
        memset( text, 0x00, sizeof(text) );
    }
    fclose(fp);
    fp = NULL;

    FILE *fp_cpu = NULL;
    fp_cpu = fopen("/tmp/taw_cpu_load_info", "w+");
    if ( fp_cpu == NULL )
        return -1;
    fwrite( cpu_load, 1, strlen( cpu_load), fp_cpu );
    fclose(fp_cpu);
    fp_cpu = NULL;

    return 0;
}

#define TAW_CPU_KERNEL_MAX 16 // 最多支持16核
int CLinuxHostDevice::get_cpu_real( float *cpu_load )
{
    FILE *fp = NULL;
    char text[512] = {0x00};
    char cpu[32] = {0x00};
    int i = 0;
    unsigned int total_old[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int total_new[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int user_old[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int user_new[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int nice_old[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int nice_new[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int system_old[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int system_new[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int idle_old[TAW_CPU_KERNEL_MAX ] = {0x00};
    unsigned int idle_new[TAW_CPU_KERNEL_MAX ] = {0x00};


    // 读取上次cpu信息
    fp = fopen("/tmp/taw_cpu_load_info", "r");
    i=0;
    while ( fp != NULL && fgets(text, 200, fp) )
    {
        if (strstr(text, "cpu") == NULL )
            break;

        sscanf(text, "%s %u %u %u %u %u", cpu, &user_old[i],
            &nice_old[i], &system_old[i], &idle_old[i], &total_old[i] );
        i++;
    }
    if ( fp != NULL )
    {
        fclose(fp);
        fp = NULL;
    }

    // 读取新的cpu信息
    get_cpu_load_new();
    fp = fopen("/tmp/taw_cpu_load_info", "r");
    if ( fp == NULL )
        return -1;

    i = 0;
    while (fgets(text, 200, fp))
    {
        if (strstr(text, "cpu") == NULL )
            break;

        sscanf(text, "%s %u %u %u %u %u", cpu, &user_new[i], &nice_new[i],
            &system_new[i], &idle_new[i], &total_new[i] );

        if (total_new[i] == total_old[i])
            cpu_load[i] = 0.0f;
        else
        {
            cpu_load[i] = ( (user_new[i] + nice_new[i] + system_new[i] ) -
                (user_old[i] + nice_old[i] + system_old[i] ) ) / (float)( total_new[i] - total_old[i] );
        }

        i++;
    }
    fclose(fp);
    fp = NULL;

    return 0;
}
