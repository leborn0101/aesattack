//
// Created by f on 2016/11/9.
//

#ifndef DPLSTEE_TEST_AES_GADDR_H
#define DPLSTEE_TEST_AES_GADDR_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libflush/mylog.h>
#define    page_map_file     "/proc/self/pagemap"
#define    PFN_MASK          ((((uint64_t)1)<<55)-1)
#define    PFN_PRESENT_FLAG  (((uint64_t)1)<<63)

int mem_addr_vir2phy(void* viraddr, void** phyaddr)
{
    //LOGI("mem_addr_vir2phy viraddr : %p", viraddr);
    uintptr_t vir = (uintptr_t)viraddr;
    int fd;
    int page_size=getpagesize();
    unsigned long vir_page_idx = vir/page_size;
    unsigned long pfn_item_offset = vir_page_idx*sizeof(uint64_t);
    uint64_t pfn_item;

    fd = open(page_map_file, O_RDONLY);
    if (fd<0)
    {
        LOGI("open %s failed", page_map_file);
        return -1;
    }


    if ((off_t)-1 == lseek(fd, pfn_item_offset, SEEK_SET))
    {
        LOGI("lseek %s", page_map_file);
        return -1;
    }


    if (sizeof(uint64_t) != read(fd, &pfn_item, sizeof(uint64_t)))
    {
        LOGI("read %s failed", page_map_file);
        return -1;
    }


    if (0==(pfn_item & PFN_PRESENT_FLAG))
    {
        LOGI("page is not present");
        return -1;
    }


    *phyaddr = (void*)((pfn_item & PFN_MASK)*page_size + (vir % page_size));
    //LOGI("mem_addr_vir2phy phyaddr : %p", *phyaddr);
    close(fd);
    return 0;


}

#endif //DPLSTEE_TEST_AES_GADDR_H
