//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#include "compat.h"
#include "baselayer.h"

#include "scriptfile.h"
#include "cache1d.h"
#include "crc32.h"

#include "duke3d.h"
#include "common_game.h"
#include "grpscan.h"

//static void process_vaca13(int32_t crcval);
static void process_vacapp15(int32_t crcval);

// custom GRP support for the startup window, file format reflects the structure below
#define GAMELISTFILE "games.list"
//    name                                     crc          size      flags                         dependency  scriptname     postprocessing
static internalgrpinfo_t const internalgrpfiles[] =
{
    //{ "Duke Nukem 3D",                         DUKE13_CRC,  26524524, GAMEFLAG_DUKE,                         0, NULL, NULL},
    //{ "Duke Nukem 3D (South Korean Censored)", DUKEKR_CRC,  26385383, GAMEFLAG_DUKE,                         0, NULL, NULL},
    { "Duke Nukem 3D: Atomic Edition",         DUKE15_CRC,  44356548, GAMEFLAG_DUKE,                         0, NULL, NULL},
    { "Duke Nukem 3D: Atomic Edition (WT)",    DUKEWT_CRC,  44356548, GAMEFLAG_DUKE,                         0, NULL, NULL},
    { "Duke Nukem 3D: Plutonium Pak",          DUKEPP_CRC,  44348015, GAMEFLAG_DUKE,                         0, NULL, NULL},
    //{ "Duke Nukem 3D Shareware 0.99",          DUKE099_CRC, 9690241,  GAMEFLAG_DUKE|GAMEFLAG_DUKEBETA,       0, NULL, NULL},
    //{ "Duke Nukem 3D Shareware 1.0",           DUKE10_CRC,  10429258, GAMEFLAG_DUKE|GAMEFLAG_SHAREWARE,      0, NULL, NULL},
    //{ "Duke Nukem 3D Shareware 1.1",           DUKE11_CRC,  10442980, GAMEFLAG_DUKE|GAMEFLAG_SHAREWARE,      0, NULL, NULL},
    //{ "Duke Nukem 3D Shareware 1.3D",          DUKESW_CRC,  11035779, GAMEFLAG_DUKE|GAMEFLAG_SHAREWARE,      0, NULL, NULL},
    //{ "Duke Nukem 3D Mac Demo",                DUKEMD_CRC,  10444391, GAMEFLAG_DUKE|GAMEFLAG_SHAREWARE,      0, NULL, NULL},
    //{ "Duke Nukem 3D MacUser Demo",            DUKEMD2_CRC, 10628573, GAMEFLAG_DUKE|GAMEFLAG_SHAREWARE,      0, NULL, NULL },
    //{ "Duke it out in D.C. (1.3D)",            DUKEDC13_CRC, 7926624, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE13_CRC, NULL, NULL},
    { "Duke it out in D.C.",                   DUKEDCPP_CRC, 8225517, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, NULL, NULL},
    { "Duke it out in D.C.",                   DUKEDC_CRC,  8410183,  GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, NULL, NULL},
    { "Duke it out in D.C.",                   (int32_t) 0x39A692BF,  8410187, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "DUKEDC.CON", NULL},
    { "Duke it out in D.C.",                   (int32_t) 0xC63B6A8B,  8410149, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "DUKEDC.CON", NULL},
    //{ "Duke Caribbean: Life's a Beach (1.3D)", VACA13_CRC,  23559381, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE13_CRC, NULL, process_vaca13},
    { "Duke Caribbean: Life's a Beach (PPak)", VACAPP_CRC,  22551333, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKEPP_CRC, NULL, process_vacapp15},
    { "Duke Caribbean: Life's a Beach",        VACA15_CRC,  22521880, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, NULL, process_vacapp15},
    { "Duke Caribbean: Life's a Beach",        DUKECB_CRC,  22213819, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, NULL, NULL},
    { "Duke Caribbean: Life's a Beach",        (int32_t) 0x65B5F690, 22397273, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "VACATION.CON", NULL},
    { "Duke Caribbean: Life's a Beach",        (int32_t) 0x64CF2351, 22213795, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "VACATION.CON", NULL},
    { "Duke: Nuclear Winter",                  DUKENW_CRC,  16169365, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "NWINTER.CON", NULL},
    { "Duke: Nuclear Winter Demo",             (int32_t) 0xC7EFBFA9,  10965909, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "NWINTER.CON", NULL},
   // { "Duke!ZONE II (1.3D)",                   DZ2_13_CRC,  26135388, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE13_CRC, "DZ-GAME.CON", NULL},
    { "Duke!ZONE II",                          DZ2_PP_CRC,  44100411, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "DZ-GAME.CON", NULL},
    { "Duke!ZONE II",                          (int32_t) 0x1E9516F1,  3186656, GAMEFLAG_DUKE|GAMEFLAG_ADDON, DUKE15_CRC, "DZ-GAME.CON", NULL},
    { "NAM",                                   NAM_CRC,     43448927, GAMEFLAG_NAM,                          0, NULL, NULL},
    { "NAPALM",                                NAPALM_CRC,  44365728, GAMEFLAG_NAM|GAMEFLAG_NAPALM,          0, NULL, NULL},
    { "WWII GI",                               WW2GI_CRC,   77939508, GAMEFLAG_WW2GI,                        0, NULL, NULL},
    { "Platoon Leader",                        PLATOONL_CRC, 37852572, GAMEFLAG_WW2GI|GAMEFLAG_ADDON,        WW2GI_CRC, "PLATOONL.DEF", NULL},
    { "Redneck Rampage",                       RR_CRC,     141174222, GAMEFLAG_RR,                           0, NULL, NULL },
    { "Redneck Rampage: Rides Again",          RRRA_CRC,   191798609, GAMEFLAG_RR|GAMEFLAG_RRRA,             0, NULL, NULL },
};

struct grpfile_t *foundgrps = NULL;
struct grpinfo_t *listgrps = NULL;
struct grpinfo_t *dn64grpinfo = NULL;

static void LoadList(const char * filename)
{
    scriptfile *script = scriptfile_fromfile(filename);

    if (!script)
        return;

    scriptfile_addsymbolvalue("GAMEFLAG_DUKE", GAMEFLAG_DUKE);
    scriptfile_addsymbolvalue("GAMEFLAG_ADDON", GAMEFLAG_DUKE|GAMEFLAG_ADDON);
    scriptfile_addsymbolvalue("GAMEFLAG_NAM", GAMEFLAG_NAM);
    scriptfile_addsymbolvalue("GAMEFLAG_NAPALM", GAMEFLAG_NAM|GAMEFLAG_NAPALM);
    scriptfile_addsymbolvalue("GAMEFLAG_RR", GAMEFLAG_RR);
    scriptfile_addsymbolvalue("GAMEFLAG_RRRA", GAMEFLAG_RR|GAMEFLAG_RRRA);
    scriptfile_addsymbolvalue("DUKE15_CRC", DUKE15_CRC);
    scriptfile_addsymbolvalue("DUKEPP_CRC", DUKEPP_CRC);
    scriptfile_addsymbolvalue("DUKE13_CRC", DUKE13_CRC);
    scriptfile_addsymbolvalue("DUKEDC13_CRC", DUKEDC13_CRC);
    scriptfile_addsymbolvalue("DUKEDCPP_CRC", DUKEDCPP_CRC);
    scriptfile_addsymbolvalue("DUKEDC_CRC", DUKEDC_CRC);
    scriptfile_addsymbolvalue("VACA13_CRC", VACA13_CRC);
    scriptfile_addsymbolvalue("VACAPP_CRC", VACAPP_CRC);
    scriptfile_addsymbolvalue("VACA15_CRC", VACA15_CRC);
    scriptfile_addsymbolvalue("DUKECB_CRC", DUKECB_CRC);
    scriptfile_addsymbolvalue("DUKENW_CRC", DUKENW_CRC);
    scriptfile_addsymbolvalue("DZ2_13_CRC", DZ2_13_CRC);
    scriptfile_addsymbolvalue("DZ2_PP_CRC", DZ2_PP_CRC);
    scriptfile_addsymbolvalue("NAM_CRC", NAM_CRC);
    scriptfile_addsymbolvalue("NAPALM_CRC", NAPALM_CRC);
    scriptfile_addsymbolvalue("WW2GI_CRC", WW2GI_CRC);

    while (!scriptfile_eof(script))
    {
        enum
        {
            T_GRPINFO,
            T_GAMENAME,
            T_CRC,
            T_SIZE,
            T_DEPCRC,
            T_SCRIPTNAME,
            T_DEFNAME,
            T_FLAGS,
            T_RTSNAME,
        };

        static const tokenlist profiletokens[] =
        {
            { "grpinfo",            T_GRPINFO },
        };

        int32_t token = getatoken(script,profiletokens,ARRAY_SIZE(profiletokens));
        switch (token)
        {
        case T_GRPINFO:
        {
            int32_t gsize = 0, gcrcval = 0, gflags = GAMEFLAG_DUKE, gdepcrc = DUKE15_CRC;
            char *gname = NULL, *gscript = NULL, *gdef = NULL;
            char *grts = NULL;
            char *grpend = NULL;

            static const tokenlist grpinfotokens[] =
            {
                { "name",           T_GAMENAME },
                { "scriptname",     T_SCRIPTNAME },
                { "defname",        T_DEFNAME },
                { "rtsname",        T_RTSNAME },
                { "crc",            T_CRC },
                { "dependency",     T_DEPCRC },
                { "size",           T_SIZE },
                { "flags",          T_FLAGS },

            };

            if (scriptfile_getbraces(script,&grpend)) break;

            while (script->textptr < grpend)
            {
                int32_t token = getatoken(script,grpinfotokens,ARRAY_SIZE(grpinfotokens));

                switch (token)
                {
                case T_GAMENAME:
                    scriptfile_getstring(script,&gname); break;
                case T_SCRIPTNAME:
                    scriptfile_getstring(script,&gscript); break;
                case T_DEFNAME:
                    scriptfile_getstring(script,&gdef); break;
                case T_RTSNAME:
                    scriptfile_getstring(script,&grts); break;

                case T_FLAGS:
                    scriptfile_getsymbol(script,&gflags); gflags &= GAMEFLAGMASK; break;
                case T_DEPCRC:
                    scriptfile_getsymbol(script,&gdepcrc); break;
                case T_CRC:
                    scriptfile_getsymbol(script,&gcrcval); break;
                case T_SIZE:
                    scriptfile_getnumber(script,&gsize); break;
                default:
                    break;
                }

                grpinfo_t * const fg = (grpinfo_t *)Xcalloc(1, sizeof(grpinfo_t));
                fg->next = listgrps;
                listgrps = fg;

                if (gname)
                    fg->name = Xstrdup(gname);

                fg->size = gsize;
                fg->crcval = gcrcval;
                fg->dependency = gdepcrc;
                fg->game = gflags;

                if (gscript)
                    fg->scriptname = dup_filename(gscript);

                if (gdef)
                    fg->defname = dup_filename(gdef);

                if (grts)
                    fg->rtsname = dup_filename(grts);
            }
            break;
        }

        default:
            break;
        }
    }

    scriptfile_close(script);
    scriptfile_clearsymbols();
}

static void LoadGameList(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(internalgrpfiles); i++)
    {
        grpinfo_t * const fg = (grpinfo_t *)Xcalloc(1, sizeof(grpinfo_t));

        fg->name = Xstrdup(internalgrpfiles[i].name);
        fg->crcval = internalgrpfiles[i].crcval;
        fg->size = internalgrpfiles[i].size;
        fg->game = internalgrpfiles[i].game;
        fg->dependency = internalgrpfiles[i].dependency;

        if (internalgrpfiles[i].scriptname)
            fg->scriptname = dup_filename(internalgrpfiles[i].scriptname);

        fg->postprocessing = internalgrpfiles[i].postprocessing;

        fg->next = listgrps;
        listgrps = fg;
    }

    BUILDVFS_FIND_REC * const srch = klistpath("/", "*.grpinfo", BUILDVFS_FIND_FILE);

    for (BUILDVFS_FIND_REC *sidx = srch; sidx; sidx = sidx->next)
        LoadList(sidx->name);

    dn64grpinfo = (grpinfo_t *)Xcalloc(1, sizeof(grpinfo_t));

    dn64grpinfo->name = Xstrdup("Duke Nukem 64");
    dn64grpinfo->crcval = 0;
    dn64grpinfo->size = -1;
    dn64grpinfo->game = GAMEFLAG_DUKE | GAMEFLAG_REALITY;
    dn64grpinfo->dependency = 0;
    dn64grpinfo->scriptname = dup_filename("duke64.def");
    dn64grpinfo->postprocessing = NULL;
    dn64grpinfo->next = listgrps;
    listgrps = dn64grpinfo;

    klistfree(srch);
}

static void FreeGameList(void)
{
    while (listgrps)
    {
        Xfree(listgrps->name);
        Xfree(listgrps->scriptname);
        Xfree(listgrps->defname);
        Xfree(listgrps->rtsname);

        grpinfo_t * const fg = listgrps->next;
        Xfree(listgrps);
        listgrps = fg;
    }
}


#define GRPCACHEFILE "grpfiles.cache"
static struct grpcache
{
    struct grpcache *next;
    int32_t size;
    int32_t mtime;
    int32_t crcval;
    char name[BMAX_PATH];
}
*grpcache = NULL, *usedgrpcache = NULL;

static int32_t LoadGroupsCache(void)
{
    struct grpcache *fg;

    int32_t fsize, fmtime, fcrcval;
    char *fname;

    scriptfile *script;

    script = scriptfile_fromfile(GRPCACHEFILE);
    if (!script) return -1;

    while (!scriptfile_eof(script))
    {
        if (scriptfile_getstring(script, &fname)) break;    // filename
        if (scriptfile_getnumber(script, &fsize)) break;    // filesize
        if (scriptfile_getnumber(script, &fmtime)) break;   // modification time
        if (scriptfile_getnumber(script, &fcrcval)) break;  // crc checksum

        fg = (struct grpcache *)Xcalloc(1, sizeof(struct grpcache));
        fg->next = grpcache;
        grpcache = fg;

        Bstrncpy(fg->name, fname, BMAX_PATH);
        fg->size = fsize;
        fg->mtime = fmtime;
        fg->crcval = fcrcval;
    }

    scriptfile_close(script);
    return 0;
}

static void FreeGroupsCache(void)
{
    while (grpcache)
    {
        struct grpcache * const fg = grpcache->next;
        Xfree(grpcache);
        grpcache = fg;
    }
}

static void RemoveGroup(grpfile_t *igrp)
{
    for (grpfile_t *prev = NULL, *grp = foundgrps; grp; grp=grp->next)
    {
        if (grp == igrp)
        {
            if (grp == foundgrps)
                foundgrps = grp->next;
            else
                prev->next = grp->next;

            Xfree((char *)grp->filename);
            Xfree(grp);

            return;
        }

        prev = grp;
    }
}

grpfile_t * FindGroup(int32_t crcval)
{
    grpfile_t *grp;

    for (grp = foundgrps; grp; grp=grp->next)
    {
        if (grp->type->crcval == crcval)
            return grp;
    }

    return NULL;
}

static grpinfo_t const * FindGrpInfo(int32_t crcval, int32_t size)
{
    grpinfo_t *grpinfo;

    for (grpinfo = listgrps; grpinfo; grpinfo=grpinfo->next)
    {
        if (grpinfo->crcval == crcval && grpinfo->size == size)
            return grpinfo;
    }

    return NULL;
}

static void ProcessGroups(BUILDVFS_FIND_REC *srch)
{
    BUILDVFS_FIND_REC *sidx;
    struct grpcache *fg, *fgg;
    char *fn;
    struct Bstat st;

#define BUFFER_SIZE (1024 * 1024 * 8)
    uint8_t *buf = (uint8_t *)Xmalloc(BUFFER_SIZE);

    for (sidx = srch; sidx; sidx = sidx->next)
    {
        for (fg = grpcache; fg; fg = fg->next)
        {
            if (!Bstrcmp(fg->name, sidx->name)) break;
        }

        if (fg)
        {
            if (findfrompath(sidx->name, &fn)) continue; // failed to resolve the filename
            if (Bstat(fn, &st))
            {
                Xfree(fn);
                continue;
            } // failed to stat the file
            Xfree(fn);
            if (fg->size == (int32_t)st.st_size && fg->mtime == (int32_t)st.st_mtime)
            {
                grpinfo_t const * const grptype = FindGrpInfo(fg->crcval, fg->size);
                if (grptype)
                {
                    grpfile_t * const grp = (grpfile_t *)Xcalloc(1, sizeof(grpfile_t));
                    grp->filename = Xstrdup(sidx->name);
                    grp->type = grptype;
                    grp->next = foundgrps;
                    foundgrps = grp;
                }

                fgg = (struct grpcache *)Xcalloc(1, sizeof(struct grpcache));
                strcpy(fgg->name, fg->name);
                fgg->size = fg->size;
                fgg->mtime = fg->mtime;
                fgg->crcval = fg->crcval;
                fgg->next = usedgrpcache;
                usedgrpcache = fgg;
                continue;
            }
        }

        {
            int32_t b, fh;
            int32_t crcval = 0;

            fh = openfrompath(sidx->name, BO_RDONLY|BO_BINARY, BS_IREAD);
            if (fh < 0) continue;
            if (Bfstat(fh, &st)) continue;

            initprintf(" Checksumming %s...", sidx->name);
            do
            {
                b = read(fh, buf, BUFFER_SIZE);
                if (b > 0) crcval = Bcrc32((uint8_t *)buf, b, crcval);
            }
            while (b == BUFFER_SIZE);
            close(fh);
            initprintf(" Done\n");

            grpinfo_t const * const grptype = FindGrpInfo(crcval, st.st_size);
            if (grptype)
            {
                grpfile_t * const grp = (grpfile_t *)Xcalloc(1, sizeof(grpfile_t));
                grp->filename = Xstrdup(sidx->name);
                grp->type = grptype;
                grp->next = foundgrps;
                foundgrps = grp;
            }

            fgg = (struct grpcache *)Xcalloc(1, sizeof(struct grpcache));
            Bstrncpy(fgg->name, sidx->name, BMAX_PATH);
            fgg->size = st.st_size;
            fgg->mtime = st.st_mtime;
            fgg->crcval = crcval;
            fgg->next = usedgrpcache;
            usedgrpcache = fgg;
        }
    }

    Xfree(buf);
}

static void ProcessDN64Groups(void)
{

    static char const *n64extensions[] =
    {
        "*.z64",
        "*.n64",
        "*.v64",
    };

    for (size_t i = 0; i < ARRAY_SIZE(n64extensions); i++)
    {
        struct Bstat st;
        BUILDVFS_FIND_REC *srch = klistpath("/", n64extensions[i], BUILDVFS_FIND_FILE);
        for (BUILDVFS_FIND_REC *sidx = srch; sidx; sidx = sidx->next)
        {
            char title[20];
            char country[3];
            int32_t fh = openfrompath(sidx->name, BO_RDONLY|BO_BINARY, BS_IREAD);
            if (fh < 0) continue;
            if (fstat(fh, &st)) continue;
            if (st.st_size != 0x800000)
            {
                close(fh);
                continue;
            }
            lseek(fh, 32, SEEK_SET);
            read(fh, title, 20);
            lseek(fh, 61, SEEK_SET);
            read(fh, &country, 3);
            close(fh);
            if ((!memcmp(title, "DUKE NUKEM", 10) && (country[1] == 'E' || country[1] == 'P'))
             || (!memcmp(title, "UDEKN KUME", 10) && (country[2] == 'E' || country[0] == 'P'))
             || (!memcmp(title, "EKUDKUN ", 8) && (country[0] == 'E' || country[0] == 'P')))
            {
                grpfile_t * const grp = (grpfile_t *)Xcalloc(1, sizeof(grpfile_t));
                grp->filename = Xstrdup(sidx->name);
                grp->type = dn64grpinfo;
                grp->next = foundgrps;
                foundgrps = grp;
            }
            else
                continue;
        }
        klistfree(srch);
    }
}

int32_t ScanGroups(void)
{
    struct grpcache *fg, *fgg;

    initprintf("Searching for game data...\n");

    LoadGameList();
    LoadGroupsCache();

    static char const *extensions[] =
    {
        "*.grp",
        "*.ssi",
        "*.dat",
    };

    for (char const *extension : extensions)
    {
        BUILDVFS_FIND_REC *srch = klistpath("/", extension, BUILDVFS_FIND_FILE);
        ProcessGroups(srch);
        klistfree(srch);
    }

    ProcessDN64Groups();

    FreeGroupsCache();

    for (grpfile_t *grp = foundgrps; grp; grp=grp->next)
    {
        if (grp->type->dependency)
        {
            if (FindGroup(grp->type->dependency) == NULL) // couldn't find dependency
            {
                //initprintf("removing %s\n", grp->name);
                RemoveGroup(grp);
                grp = foundgrps;
                // start from the beginning so we can remove anything that depended on this grp
                continue;
            }
        }
    }

    if (usedgrpcache)
    {
        int32_t i = 0;
        FILE *fp;
        fp = fopen(GRPCACHEFILE, "wt");
        if (fp)
        {
            for (fg = usedgrpcache; fg; fg=fgg)
            {
                fgg = fg->next;
                fprintf(fp, "\"%s\" %d %d %d\n", fg->name, fg->size, fg->mtime, fg->crcval);
                Xfree(fg);
                i++;
            }
            fclose(fp);
        }
//        initprintf("Found %d recognized GRP %s.\n",i,i>1?"files":"file");

        return 0;
    }

    initprintf("Found no recognized game data!\n");

    return 0;
}


void FreeGroups(void)
{
    while (foundgrps)
    {
        Xfree((char *)foundgrps->filename);
        grpfile_t * const fg = foundgrps->next;
        Xfree(foundgrps);
        foundgrps = fg;
    }

    FreeGameList();
}

static void process_vacapp15(int32_t crcval)
{
    krename(crcval, 5, "DEFS.CON");
    krename(crcval, 6, "GAME.CON");
    krename(crcval, 7, "USER.CON");
    krename(crcval, 8, "DEMO1.DMO");
    krename(crcval, 9, "DEMO2.DMO");
    krename(crcval, 10, "DEMO3.DMO");

    initgroupfile("VACATION.PRG");
}
