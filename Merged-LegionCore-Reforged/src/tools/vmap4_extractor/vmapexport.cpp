/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "adtfile.h"
#include "Banner.h"
#include "Common.h"
#include "cascfile.h"
#include "DB2CascFileSource.h"
#include "ExtractorDB2LoadInfo.h"
#include "StringFormat.h"
#include "vmapexport.h"
#include "wdtfile.h"
#include "wmo.h"
#include <CascLib.h>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/operations.hpp>
#include <atomic>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#ifdef WIN32
    #include <direct.h>
    #define mkdir _mkdir
#else
    #define ERROR_PATH_NOT_FOUND ERROR_FILE_NOT_FOUND
#endif

//------------------------------------------------------------------------------
// Defines

#define MPQ_BLOCK_SIZE 0x1000

//-----------------------------------------------------------------------------

thread_local CASC::StorageHandle CascStorage;

// Mutexes for thread-safe access to shared state
std::mutex g_wmoDoodadsMutex;
std::mutex g_uniqueIdsMutex;
std::recursive_mutex g_extractMutex;
std::mutex g_dirfileMutex;
// Per-thread dir_bin path
thread_local std::string g_dirBinPath;

int32 FirstLocale = -1;

struct map_info
{
    char name[64];
    int32 parent_id;
};

std::map<uint32, map_info> map_ids;
std::unordered_set<uint32> maps_that_are_parents;
boost::filesystem::path input_path;
bool preciseVectorData = false;
std::unordered_map<std::string, WMODoodadData> WmoDoodads;

// Constants

const char* szWorkDirWmo = "./ClientData/Buildings";

#define CASC_LOCALES_COUNT 17
char const* CascLocaleNames[CASC_LOCALES_COUNT] =
{
    "none", "enUS",
    "koKR", "unknown",
    "frFR", "deDE",
    "zhCN", "esES",
    "zhTW", "enGB",
    "enCN", "enTW",
    "esMX", "ruRU",
    "ptBR", "itIT",
    "ptPT"
};

uint32 WowLocaleToCascLocaleFlags[12] =
{
    CASC_LOCALE_ENUS | CASC_LOCALE_ENGB,
    CASC_LOCALE_KOKR,
    CASC_LOCALE_FRFR,
    CASC_LOCALE_DEDE,
    CASC_LOCALE_ZHCN,
    CASC_LOCALE_ZHTW,
    CASC_LOCALE_ESES,
    CASC_LOCALE_ESMX,
    CASC_LOCALE_RURU,
    0,
    CASC_LOCALE_PTBR | CASC_LOCALE_PTPT,
    CASC_LOCALE_ITIT,
};

bool OpenCascStorage(int locale)
{
    try
    {
        boost::filesystem::path const storage_dir(boost::filesystem::canonical(input_path) / "Data");
        CascStorage = CASC::OpenStorage(storage_dir, WowLocaleToCascLocaleFlags[locale]);
        if (!CascStorage)
        {
            printf("error opening casc storage '%s' locale %s\n", storage_dir.string().c_str(), localeNames[locale]);
            return false;
        }

        return true;
    }
    catch (std::exception const& error)
    {
        printf("error opening casc storage : %s\n", error.what());
        return false;
    }
}

uint32 GetInstalledLocalesMask()
{
    try
    {
        boost::filesystem::path const storage_dir(boost::filesystem::canonical(input_path) / "Data");
        CASC::StorageHandle storage = CASC::OpenStorage(storage_dir, 0);
        if (!storage)
            return false;

        return CASC::GetInstalledLocalesMask(storage);
    }
    catch (std::exception const& error)
    {
        printf("Unable to determine installed locales mask: %s\n", error.what());
    }

    return 0;
}

std::map<std::pair<uint32, uint16>, uint32> uniqueObjectIds;

uint32 GenerateUniqueObjectId(uint32 clientId, uint16 clientDoodadId)
{
    std::lock_guard<std::mutex> lock(g_uniqueIdsMutex);
    return uniqueObjectIds.emplace(std::make_pair(clientId, clientDoodadId), uniqueObjectIds.size() + 1).first->second;
}

// Local testing functions
bool FileExists(const char* file)
{
    if (FILE* n = fopen(file, "rb"))
    {
        fclose(n);
        return true;
    }
    return false;
}

std::string GetDirBinPath()
{
    if (!g_dirBinPath.empty())
        return g_dirBinPath;
    return std::string(szWorkDirWmo) + "/dir_bin";
}

bool ExtractSingleWmo(std::string& fname)
{
    // Copy files from archive
    std::string originalName = fname;

    char szLocalFile[1024];
    char* plain_name = GetPlainName(&fname[0]);
    FixNameCase(plain_name, strlen(plain_name));
    FixNameSpaces(plain_name, strlen(plain_name));
    sprintf(szLocalFile, "%s/%s", szWorkDirWmo, plain_name);

    // Protect file existence check + extraction to avoid duplicate work
    std::lock_guard<std::recursive_mutex> extractLock(g_extractMutex);

    if (FileExists(szLocalFile))
        return true;

    int p = 0;
    // Select root wmo files
    char const* rchr = strrchr(plain_name, '_');
    if (rchr != NULL)
    {
        char cpy[4];
        memcpy(cpy, rchr, 4);
        for (int i = 0; i < 4; ++i)
        {
            int m = cpy[i];
            if (isdigit(m))
                p++;
        }
    }

    if (p == 3)
        return true;

    bool file_ok = true;
    printf("Extracting %s\n", originalName.c_str());
    WMORoot froot(originalName);
    if (!froot.open())
    {
        printf("Couldn't open RootWmo!!!\n");
        return true;
    }
    FILE *output = fopen(szLocalFile,"wb");
    if(!output)
    {
        printf("couldn't open %s for writing!\n", szLocalFile);
        return false;
    }
    froot.ConvertToVMAPRootWmo(output);
    int Wmo_nVertices = 0;
    {
        std::lock_guard<std::mutex> doodadLock(g_wmoDoodadsMutex);
        WMODoodadData& doodads = WmoDoodads[plain_name];
        std::swap(doodads, froot.DoodadData);
        for (std::size_t i = 0; i < froot.groupFileDataIDs.size(); ++i)
        {
            std::string s = Trinity::StringFormat("FILE%08X.xxx", froot.groupFileDataIDs[i]);
            WMOGroup fgroup(s);
            if (!fgroup.open(&froot))
            {
                printf("Could not open all Group file for: %s\n", plain_name);
                file_ok = false;
                break;
            }

            Wmo_nVertices += fgroup.ConvertToVMAPGroupWmo(output, preciseVectorData);
            for (uint16 groupReference : fgroup.DoodadReferences)
            {
                if (groupReference >= doodads.Spawns.size())
                    continue;

                uint32 doodadNameIndex = doodads.Spawns[groupReference].NameIndex;
                if (froot.ValidDoodadNames.find(doodadNameIndex) == froot.ValidDoodadNames.end())
                    continue;

                doodads.References.insert(groupReference);
            }
        }
    }

    fseek(output, 8, SEEK_SET); // store the correct no of vertices
    fwrite(&Wmo_nVertices,sizeof(int),1,output);
    fclose(output);

    // Delete the extracted file in the case of an error
    if (!file_ok)
        remove(szLocalFile);
    return true;
}

void ParsMapFiles()
{
    // Phase 1: Initialize all WDTs on main thread (extract global WMOs, write to dir_bin)
    struct MapTask
    {
        uint32 mapId;
        int32 parentId;
        char mapName[64];
        bool isParent;
    };

    std::vector<MapTask> mapTasks;

    printf("Initializing WDT files...\n");
    {
        std::unordered_map<uint32, WDTFile> wdts;
        auto getWDT = [&wdts](uint32 mapId) -> WDTFile*
        {
            auto itr = wdts.find(mapId);
            if (itr == wdts.end())
            {
                char fn[512];
                char* name = map_ids[mapId].name;
                sprintf(fn, "World\\Maps\\%s\\%s.wdt", name, name);
                itr = wdts.emplace(std::piecewise_construct, std::forward_as_tuple(mapId), std::forward_as_tuple(fn, name, false)).first;
                if (!itr->second.init(mapId))
                {
                    wdts.erase(itr);
                    return nullptr;
                }
            }
            return &itr->second;
        };

        for (auto& [mapId, info] : map_ids)
        {
            if (!getWDT(mapId))
                continue;

            // Also ensure parent WDT is initialized
            if (info.parent_id >= 0)
                getWDT(info.parent_id);

            MapTask task;
            task.mapId = mapId;
            task.parentId = info.parent_id;
            strncpy(task.mapName, info.name, sizeof(task.mapName));
            task.mapName[sizeof(task.mapName) - 1] = '\0';
            task.isParent = maps_that_are_parents.count(mapId) > 0;
            mapTasks.push_back(task);
        }
    }
    printf("WDT initialization complete (%zu maps)\n", mapTasks.size());

    // Phase 2: Process tiles in parallel
    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency());
    printf("Processing map tiles using %u threads...\n", numThreads);

    std::atomic<uint32> nextIndex{0};
    uint32 totalMaps = static_cast<uint32>(mapTasks.size());
    std::atomic<uint32> mapsProcessed{0};

    std::vector<std::thread> workers;
    std::vector<std::string> threadDirBins(numThreads);

    for (unsigned int t = 0; t < numThreads; ++t)
    {
        threadDirBins[t] = std::string(szWorkDirWmo) + "/dir_bin_t" + std::to_string(t);
        workers.emplace_back([&, t]()
        {
            // Open thread's own CASC storage
            try
            {
                boost::filesystem::path storageDir(boost::filesystem::canonical(input_path) / "Data");
                CascStorage = CASC::OpenStorage(storageDir, WowLocaleToCascLocaleFlags[FirstLocale]);
            }
            catch (std::exception const& error)
            {
                printf("Thread %u: Failed to open CASC storage: %s\n", t, error.what());
                return;
            }
            if (!CascStorage)
            {
                printf("Thread %u: Failed to open CASC storage\n", t);
                return;
            }

            g_dirBinPath = threadDirBins[t];

            // Per-thread parent WDT cache (for GetMap tile access only, no init)
            std::unordered_map<int32, std::unique_ptr<WDTFile>> parentWdtCache;

            uint32 idx;
            while ((idx = nextIndex.fetch_add(1)) < totalMaps)
            {
                MapTask& task = mapTasks[idx];

                // Create WDT for tile access (skip init — already done in Phase 1)
                char fn[512];
                sprintf(fn, "World\\Maps\\%s\\%s.wdt", task.mapName, task.mapName);
                WDTFile mapWdt(fn, task.mapName, false);

                // Get parent WDT for fallback tiles
                WDTFile* parentWdt = nullptr;
                if (task.parentId >= 0)
                {
                    auto it = parentWdtCache.find(task.parentId);
                    if (it == parentWdtCache.end())
                    {
                        auto parentIt = map_ids.find(task.parentId);
                        if (parentIt != map_ids.end())
                        {
                            char parentFn[512];
                            sprintf(parentFn, "World\\Maps\\%s\\%s.wdt",
                                parentIt->second.name, parentIt->second.name);
                            auto pw = std::make_unique<WDTFile>(parentFn, parentIt->second.name, true);
                            parentWdt = pw.get();
                            parentWdtCache[task.parentId] = std::move(pw);
                        }
                    }
                    else
                    {
                        parentWdt = it->second.get();
                    }
                }

                for (int32 x = 0; x < 64; ++x)
                {
                    for (int32 y = 0; y < 64; ++y)
                    {
                        bool success = false;
                        if (ADTFile* ADT = mapWdt.GetMap(x, y))
                        {
                            success = ADT->init(task.mapId, task.mapId);
                            mapWdt.FreeADT(ADT);
                        }
                        if (!success && parentWdt)
                        {
                            if (ADTFile* ADT = parentWdt->GetMap(x, y))
                            {
                                ADT->init(task.mapId, task.parentId);
                                parentWdt->FreeADT(ADT);
                            }
                        }
                    }
                }

                uint32 done = mapsProcessed.fetch_add(1) + 1;
                printf("Map %u (%s) complete [%u/%u]\n", task.mapId, task.mapName, done, totalMaps);
            }

            CascStorage.reset();
        });
    }

    for (auto& w : workers)
        w.join();

    // Phase 3: Concatenate per-thread dir_bin files into main dir_bin
    std::string finalDirBin = std::string(szWorkDirWmo) + "/dir_bin";
    FILE* finalFile = fopen(finalDirBin.c_str(), "ab");
    if (finalFile)
    {
        for (unsigned int t = 0; t < numThreads; ++t)
        {
            FILE* src = fopen(threadDirBins[t].c_str(), "rb");
            if (src)
            {
                char buf[65536];
                size_t n;
                while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
                    fwrite(buf, 1, n, finalFile);
                fclose(src);
                remove(threadDirBins[t].c_str());
            }
        }
        fclose(finalFile);
    }
}

bool processArgv(int argc, char ** argv)
{
    bool result = true;
    preciseVectorData = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp("-s", argv[i]) == 0)
        {
            preciseVectorData = false;
        }
        else if (strcmp("-d", argv[i]) == 0)
        {
            if ((i + 1) < argc)
            {
                input_path = boost::filesystem::path(argv[i + 1]);
                ++i;
            }
            else
            {
                result = false;
            }
        }
        else if (strcmp("-?", argv[1]) == 0)
        {
            result = false;
        }
        else if(strcmp("-l",argv[i]) == 0)
        {
            preciseVectorData = true;
        }
        else
        {
            result = false;
            break;
        }
    }

    if (!result)
    {
        printf("%s [-?][-s][-l][-d <path>]\n", argv[0]);
        printf("   -s : (default) small size (data size optimization), ~500MB less vmap data.\n");
        printf("   -l : large size, ~500MB more vmap data. (might contain more details)\n");
        printf("   -d <path>: Path to the vector data source folder.\n");
        printf("   -? : This message.\n");
    }

    return result;
}

static bool RetardCheck()
{
    try
    {
        boost::filesystem::path storageDir(boost::filesystem::canonical(input_path) / "Data");
        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator itr(storageDir); itr != end; ++itr)
        {
            if (itr->path().extension() == ".MPQ")
            {
                printf("MPQ files found in Data directory!\n");
                printf("This tool works only with World of Warcraft: Legion\n");
                printf("\n");
                printf("To extract maps for Wrath of the Lich King, rebuild tools using 3.3.5 branch!\n");
                printf("\n");
                printf("Press ENTER to exit...\n");
                getchar();
                return false;
            }
        }
    }
    catch (std::exception const& error)
    {
        printf("Error checking client version: %s\n", error.what());
    }

    return true;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Main
//
// The program must be run with two command line arguments
//
// Arg1 - The source MPQ name (for testing reading and file find)
// Arg2 - Listfile name
//

int main(int argc, char ** argv)
{
    unsigned int hwCores = std::thread::hardware_concurrency();
    unsigned int usedThreads = hwCores > 0 ? hwCores : 1;
    printf("\n  Extractor Tools v1.0.1 - Copyright (C)2026 Apheleos\n  - Multicore/Multithreading support\n  - Legion 7.3.5 (build 26972)\n\n  Hardware: %u logical processors detected\n  Using %u threads for extraction\n\n", hwCores, usedThreads);
    for (int i = 3; i > 0; --i) { printf("  Starting in %d...\r", i); fflush(stdout); std::this_thread::sleep_for(std::chrono::seconds(1)); }
    printf("                    \n");

    bool success = true;

    if (input_path.empty())
        input_path = boost::filesystem::current_path();

    // Use command line arguments, when some
    if (!processArgv(argc, argv))
    {
        system("pause");
        return 1;
    }

    printf("Input path: %s\n", input_path.string().c_str());

    if (!RetardCheck())
    {
        system("pause");
        return 1;
    }

    // some simple check if working dir is dirty
    else
    {
        std::string sdir = std::string(szWorkDirWmo) + "/dir";
        std::string sdir_bin = std::string(szWorkDirWmo) + "/dir_bin";
        struct stat status;
        if (!stat(sdir.c_str(), &status) || !stat(sdir_bin.c_str(), &status))
        {
            printf("Your output directory seems to be polluted, please use an empty directory!\n");
            system("pause");
            return 1;
        }
    }

    printf("Beginning work ....\n\n");
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // Create the working directory
    if (mkdir(szWorkDirWmo
#if defined(__linux__) || defined(__APPLE__)
                    , 0711
#endif
                    ))
            success = (errno == EEXIST);

    FirstLocale = -1;
    for (int i = 0; i < TOTAL_LOCALES; ++i)
    {
        if (i == LOCALE_none)
            continue;

        if (!OpenCascStorage(i))
            continue;

        FirstLocale = i;
        uint32 build = CASC::GetBuildNumber(CascStorage);
        if (!build)
        {
            CascStorage.reset();
            continue;
        }

        printf("Detected client build: %u\n\n", build);
        break;
    }

    if (FirstLocale == -1)
    {
        printf("FATAL ERROR: No locales defined, unable to continue.\n");
        system("pause");
        return 1;
    }

    // Extract models, listed in GameObjectDisplayInfo.dbc
    ExtractGameobjectModels();

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //map.dbc
    if (success)
    {
        printf("Read Map.dbc file... ");

        DB2CascFileSource source(CascStorage, "DBFilesClient\\Map.db2");
        DB2FileLoader db2;
        if (!db2.Load(&source, MapLoadInfo::Instance()))
        {
            printf("Fatal error: Invalid Map.db2 file format! %s\n", CASC::HumanReadableCASCError(GetLastError()));
            system("pause");
            exit(1);
        }

        for (uint32 x = 0; x < db2.GetRecordCount(); ++x)
        {
            DB2Record record = db2.GetRecord(x);
            map_info& m = map_ids[record.GetId()];

            const char* map_name = record.GetString("Directory");
            size_t max_map_name_length = sizeof(m.name);
            if (strlen(map_name) >= max_map_name_length)
            {
                printf("Fatal error: Map name too long!\n");
                system("pause");
                exit(1);
            }

            strncpy(m.name, map_name, max_map_name_length);
            m.name[max_map_name_length - 1] = '\0';
            m.parent_id = int16(record.GetUInt16("ParentMapID"));
            if (m.parent_id >= 0)
                maps_that_are_parents.insert(m.parent_id);
        }

        for (uint32 x = 0; x < db2.GetRecordCopyCount(); ++x)
        {
            DB2RecordCopy copy = db2.GetRecordCopy(x);
            auto itr = map_ids.find(copy.SourceRowId);
            if (itr != map_ids.end())
            {
                map_info& id = map_ids[copy.NewRowId];
                strcpy(id.name, itr->second.name);
                id.parent_id = itr->second.parent_id;
            }
        }

        printf("Done! (" SZFMTD " maps loaded)\n", map_ids.size());
        ParsMapFiles();
    }

    CascStorage.reset();

    printf("\n");
    if (!success)
    {
        printf("ERROR: Work NOT complete.\n   Precise vector data=%d.\n", preciseVectorData);
        system("pause");
        return 1;
    }

    printf("Work complete. No errors.\n");
    system("pause");
    return 0;
}
