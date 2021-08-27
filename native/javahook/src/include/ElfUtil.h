// ElfUtil.h

#ifndef _ELF_UTIL_H_
#define _ELF_UTIL_H_

#include "platform.h"
#if defined(_WIN32)
#include "elf.hpp"
#endif

class CElfUtil
{
protected:
    CElfUtil()
    {
    }
    ~CElfUtil()
    {
    }
public:
    static const Elf32_Phdr* GetPhdr(const unsigned char *elf, unsigned p_type, int index = 0)
    {
        if(!elf)
            return 0;
        const Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf;
        const Elf32_Phdr *phdr = (Elf32_Phdr *)(elf + ehdr->e_phoff);

        for(Elf32_Half i = 0; i < ehdr->e_phnum; ++i, ++phdr)
        {
            if(phdr->p_type != p_type)
                continue;
            if(index-- > 0)
                continue;

            return phdr;
        }
        return 0;
    }
    static const Elf32_Dyn* GetDyn(const unsigned char *elf, int d_tag)
    {
        if(!elf)
            return 0;
        const Elf32_Phdr *phdr_dyna = GetPhdr(elf, PT_DYNAMIC);
        if(!phdr_dyna)
            return 0;

        const Elf32_Dyn *dyn = (Elf32_Dyn *)(elf + phdr_dyna->p_offset);
        for(Elf32_Word i = 0; i < phdr_dyna->p_filesz / sizeof(Elf32_Dyn); ++i, ++dyn)
        {
            if(dyn->d_tag != d_tag)
                continue;

            return dyn;
        }
        return 0;
    }
    static const Elf32_Shdr* GetShdr(const unsigned char *elf, const char *sh_name)
    {
        if(!elf || !sh_name || !*sh_name)
            return 0;
        const Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf;
        const Elf32_Shdr *shdr = (Elf32_Shdr *)(elf + ehdr->e_shoff);
        const char *strtab_sh = (char *)elf + shdr[ehdr->e_shstrndx].sh_offset;

        for(Elf32_Half i = 0; i < ehdr->e_shnum; ++i, ++shdr)
        {
            const char *sh_name_ = strtab_sh + shdr->sh_name;
            if(strcmp(sh_name_, sh_name))
                continue;

            return shdr;
        }
        return 0;
    }
    static const Elf32_Sym* GetSym(const unsigned char *elf, const char *st_name, int isdynsym = 1)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const Elf32_Shdr *shdr_dynsym = isdynsym ? GetShdr(elf, ".dynsym") : GetShdr(elf, ".symtab");
        const Elf32_Shdr *shdr_dynstr = isdynsym ? GetShdr(elf, ".dynstr") : GetShdr(elf, ".strtab");
        if(!shdr_dynsym || !shdr_dynstr)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const Elf32_Sym *sym = (Elf32_Sym *)(elf + shdr_dynsym->sh_offset);

        for(Elf32_Word i = 0; i < shdr_dynsym->sh_size / sizeof(Elf32_Sym); ++i, ++sym)
        {
            const char *st_name_ = strtab_dyn + sym->st_name;
            if(strcmp(st_name_, st_name))
                continue;

            return sym;
        }
        return 0;
    }
    static Elf32_Addr GetSym_offset(const unsigned char *elf, const char *st_name)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const Elf32_Phdr *phdr_dyna = GetPhdr(elf, PT_LOAD);
        if(!phdr_dyna)
            return 0;

        const Elf32_Sym *sym = 0;
        if(!sym)
            sym = GetSym(elf, st_name, 1);
        if(!sym)
            sym = GetSym(elf, st_name, 0);

        Elf32_Addr addr = sym ? sym->st_value-phdr_dyna->p_vaddr : 0;
        return addr;
    }
    static const Elf32_Rel* GetRel(const unsigned char *elf, const char *st_name, int isplt = 1)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const Elf32_Shdr *shdr_dynsym = GetShdr(elf, ".dynsym");
        const Elf32_Shdr *shdr_dynstr = GetShdr(elf, ".dynstr");
        const Elf32_Shdr *shdr_rel = GetShdr(elf, isplt?".rel.plt":".rel.dyn");
        if(!shdr_rel || !shdr_dynstr || !shdr_dynsym)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const Elf32_Rel *rel = (Elf32_Rel *)(elf + shdr_rel->sh_offset);
        const Elf32_Sym *sym = (Elf32_Sym *)(elf + shdr_dynsym->sh_offset);

        for(Elf32_Word i = 0; i < shdr_rel->sh_size / sizeof(Elf32_Rel); ++i, ++rel)
        {
            const char *st_name_ = strtab_dyn + sym[rel->r_info>>8].st_name;
            if(strcmp(st_name_, st_name))
                continue;

            return rel;
        }
        return 0;
    }
    static Elf32_Addr GetRel_offset(const unsigned char *elf, const char *st_name)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const Elf32_Phdr *phdr_dyna = GetPhdr(elf, PT_LOAD);
        if(!phdr_dyna)
            return 0;

        const Elf32_Rel *rel = 0;
        if(!rel)
            rel = GetRel(elf, st_name, 1);
        if(!rel)
            rel = GetRel(elf, st_name, 0);

        Elf32_Addr addr = rel ? rel->r_offset-phdr_dyna->p_vaddr : 0;
        return addr;
    }
    static const Elf32_Dyn* GetDyn_needed(const unsigned char *elf, const char *so_name)
    {
        if(!elf || !so_name || !*so_name)
            return 0;
        const Elf32_Phdr *phdr_dyna = GetPhdr(elf, PT_DYNAMIC);
        if(!phdr_dyna)
            return 0;
        const Elf32_Shdr *shdr_dynstr = GetShdr(elf, ".dynstr");
        if(!shdr_dynstr)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const Elf32_Dyn *dyn = (Elf32_Dyn *)(elf + phdr_dyna->p_offset);
        for(Elf32_Word i = 0; i < phdr_dyna->p_filesz / sizeof(Elf32_Dyn); ++i, ++dyn)
        {
            if(dyn->d_tag != DT_NEEDED)
                continue;
            const char *so_name_ = strtab_dyn + dyn->d_un.d_val;
            if(strcmp(so_name_, so_name))
                continue;

            return dyn;
        }
        return 0;
    }
};

#endif
