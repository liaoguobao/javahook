// ElfUtil.h

#ifndef _ELF_UTIL_H_
#define _ELF_UTIL_H_

#include "platform.h"
#if defined(_WIN32)
#include "elf.hpp"
#endif

#if defined(__LP64__) || defined(_WIN64)
#define ElfW(type) Elf64_ ## type
#else
#define ElfW(type) Elf32_ ## type
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
    static const ElfW(Phdr)* GetPhdr(const unsigned char *elf, unsigned p_type/*PT_xxx*/, int index = 0)
    {
        if(!elf)
            return 0;
        const ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)elf;
        const ElfW(Phdr) *phdr = (ElfW(Phdr) *)(elf + ehdr->e_phoff);

        for(ElfW(Half) i = 0; i < ehdr->e_phnum; ++i, ++phdr)
        {
            if(phdr->p_type != p_type)
                continue;
            if(index-- > 0)
                continue;

            return phdr;
        }
        return 0;
    }
    static const ElfW(Dyn)* GetDyn(const unsigned char *elf, int d_tag/*DT_xxx*/)
    {
        if(!elf)
            return 0;
        const ElfW(Phdr) *phdr_dyna = GetPhdr(elf, PT_DYNAMIC);
        if(!phdr_dyna)
            return 0;

        const ElfW(Dyn) *dyn = (ElfW(Dyn) *)(elf + phdr_dyna->p_offset);
        for(ElfW(Word) i = 0; i < phdr_dyna->p_filesz / sizeof(ElfW(Dyn)); ++i, ++dyn)
        {
            if(dyn->d_tag != d_tag)
                continue;

            return dyn;
        }
        return 0;
    }
    static const ElfW(Shdr)* GetShdr(const unsigned char *elf, const char *sh_name)
    {
        if(!elf || !sh_name || !*sh_name)
            return 0;
        const ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)elf;
        const ElfW(Shdr) *shdr = (ElfW(Shdr) *)(elf + ehdr->e_shoff);
        const char *strtab_sh = (char *)elf + shdr[ehdr->e_shstrndx].sh_offset;

        for(ElfW(Half) i = 0; i < ehdr->e_shnum; ++i, ++shdr)
        {
            const char *sh_name_ = strtab_sh + shdr->sh_name;
            if(strcmp(sh_name_, sh_name))
                continue;

            return shdr;
        }
        return 0;
    }
    static const ElfW(Shdr)* GetShdr(const unsigned char *elf, unsigned sh_type/*SHT_xxx*/)
    {
        if(!elf || !sh_type)
            return 0;
        const ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)elf;
        const ElfW(Shdr) *shdr = (ElfW(Shdr) *)(elf + ehdr->e_shoff);

        for(ElfW(Half) i = 0; i < ehdr->e_shnum; ++i, ++shdr)
        {
            if(shdr->sh_type != sh_type)
                continue;

            return shdr;
        }
        return 0;
    }
    static const ElfW(Sym)* GetSym(const unsigned char *elf, const char *st_name, int isdynsym = 1)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Shdr) *shdr_dynsym = isdynsym ? GetShdr(elf, ".dynsym") : GetShdr(elf, ".symtab");
        const ElfW(Shdr) *shdr_dynstr = isdynsym ? GetShdr(elf, ".dynstr") : GetShdr(elf, ".strtab");
        if(!shdr_dynsym || !shdr_dynstr)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const ElfW(Sym) *sym = (ElfW(Sym) *)(elf + shdr_dynsym->sh_offset);

        for(ElfW(Word) i = 0; i < shdr_dynsym->sh_size / sizeof(ElfW(Sym)); ++i, ++sym)
        {
            const char *st_name_ = strtab_dyn + sym->st_name;
            if(strcmp(st_name_, st_name))
                continue;

            return sym;
        }
        return 0;
    }
    static const ElfW(Sym)* GetDynSym(const unsigned char *elf)
    {
        if(!elf)
            return 0;

        const ElfW(Dyn) *dyn_symtab = GetDyn(elf, DT_SYMTAB);
        if(dyn_symtab)
            return (ElfW(Sym) *)(elf + dyn_symtab->d_un.d_ptr);

        const ElfW(Shdr) *shdr_dynsym = GetShdr(elf, ".dynsym");
        if(shdr_dynsym)
            return (ElfW(Sym) *)(elf + shdr_dynsym->sh_offset);

        const ElfW(Shdr) *shdr_symtab = GetShdr(elf, SHT_SYMTAB);
        if(shdr_symtab)
            return (ElfW(Sym) *)(elf + shdr_symtab->sh_offset);

        return 0;
    }
    static const char* GetDynStr(const unsigned char *elf)
    {
        if(!elf)
            return 0;

        const ElfW(Dyn) *dyn_strtab = GetDyn(elf, DT_STRTAB);
        if(dyn_strtab)
            return (char *)(elf + dyn_strtab->d_un.d_ptr);

        const ElfW(Shdr) *shdr_dynstr = GetShdr(elf, ".dynstr");
        if(shdr_dynstr)
            return (char *)(elf + shdr_dynstr->sh_offset);

        const ElfW(Shdr) *shdr_strtab = GetShdr(elf, SHT_STRTAB);
        if(shdr_strtab)
            return (char *)(elf + shdr_strtab->sh_offset);

        return 0;
    }
    static ElfW(Addr) GetSym_offset(const unsigned char *elf, const char *st_name)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Phdr) *phdr_dyna = GetPhdr(elf, PT_LOAD);
        if(!phdr_dyna)
            return 0;

        const ElfW(Sym) *sym = 0;
        if(!sym)
            sym = GetSym(elf, st_name, 1);
        if(!sym)
            sym = GetSym(elf, st_name, 0);

        ElfW(Addr) addr = sym ? sym->st_value-phdr_dyna->p_vaddr : 0;
        return addr;
    }
    static const ElfW(Rel)* GetRel(const unsigned char *elf, const char *st_name, int isplt = 1)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Shdr) *shdr_dynsym = GetShdr(elf, ".dynsym");
        const ElfW(Shdr) *shdr_dynstr = GetShdr(elf, ".dynstr");
        const ElfW(Shdr) *shdr_rel = GetShdr(elf, isplt?".rel.plt":".rel.dyn");
        if(!shdr_rel || !shdr_dynstr || !shdr_dynsym)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const ElfW(Rel) *rel = (ElfW(Rel) *)(elf + shdr_rel->sh_offset);
        const ElfW(Sym) *sym = (ElfW(Sym) *)(elf + shdr_dynsym->sh_offset);

        for(ElfW(Word) i = 0; i < shdr_rel->sh_size / sizeof(ElfW(Rel)); ++i, ++rel)
        {
#if defined(__LP64__) || defined(_WIN64)
            ElfW(Off) rsym = ELF64_R_SYM(rel->r_info);
#else
            ElfW(Off) rsym = ELF32_R_SYM(rel->r_info);
#endif
            const char *st_name_ = strtab_dyn + sym[rsym].st_name;
            if(strcmp(st_name_, st_name))
                continue;

            return rel;
        }
        return 0;
    }
    static ElfW(Addr) GetRel_offset(const unsigned char *elf, const char *st_name)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Phdr) *phdr_dyna = GetPhdr(elf, PT_LOAD);
        if(!phdr_dyna)
            return 0;

        const ElfW(Rel) *rel = 0;
        if(!rel)
            rel = GetRel(elf, st_name, 1);
        if(!rel)
            rel = GetRel(elf, st_name, 0);

        ElfW(Addr) addr = rel ? rel->r_offset-phdr_dyna->p_vaddr : 0;
        return addr;
    }
    static const ElfW(Rela)* GetRela(const unsigned char *elf, const char *st_name, int isplt = 1)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Shdr) *shdr_dynsym = GetShdr(elf, ".dynsym");
        const ElfW(Shdr) *shdr_dynstr = GetShdr(elf, ".dynstr");
        const ElfW(Shdr) *shdr_rela = GetShdr(elf, isplt?".rela.plt":".rela.dyn");
        if(!shdr_rela || !shdr_dynstr || !shdr_dynsym)
            return 0;

        const char *strtab_dyn = (char *)elf + shdr_dynstr->sh_offset;
        const ElfW(Rela) *rela = (ElfW(Rela) *)(elf + shdr_rela->sh_offset);
        const ElfW(Sym) *sym = (ElfW(Sym) *)(elf + shdr_dynsym->sh_offset);

        for(ElfW(Word) i = 0; i < shdr_rela->sh_size / sizeof(ElfW(Rela)); ++i, ++rela)
        {
#if defined(__LP64__) || defined(_WIN64)
            ElfW(Off) rsym = ELF64_R_SYM(rela->r_info);
#else
            ElfW(Off) rsym = ELF32_R_SYM(rela->r_info);
#endif
            const char *st_name_ = strtab_dyn + sym[rsym].st_name;
            if(strcmp(st_name_, st_name))
                continue;

            return rela;
        }
        return 0;
    }
    static ElfW(Addr) GetRela_offset(const unsigned char *elf, const char *st_name)
    {
        if(!elf || !st_name || !*st_name)
            return 0;
        const ElfW(Phdr) *phdr_dyna = GetPhdr(elf, PT_LOAD);
        if(!phdr_dyna)
            return 0;

        const ElfW(Rela) *rela = 0;
        if(!rela)
            rela = GetRela(elf, st_name, 1);
        if(!rela)
            rela = GetRela(elf, st_name, 0);

        ElfW(Addr) addr = rela ? rela->r_offset-phdr_dyna->p_vaddr : 0;
        return addr;
    }
    static const ElfW(Dyn)* GetDyn_needed(const unsigned char *elf, const char *so_name)
    {
        if(!elf || !so_name || !*so_name)
            return 0;
        const ElfW(Phdr) *phdr_dyna = GetPhdr(elf, PT_DYNAMIC);
        if(!phdr_dyna)
            return 0;
        const char *strtab_dyn = GetDynStr(elf);
        if(!strtab_dyn)
            return 0;

        const ElfW(Dyn) *dyn = (ElfW(Dyn) *)(elf + phdr_dyna->p_offset);
        for(ElfW(Word) i = 0; i < phdr_dyna->p_filesz / sizeof(ElfW(Dyn)); ++i, ++dyn)
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
