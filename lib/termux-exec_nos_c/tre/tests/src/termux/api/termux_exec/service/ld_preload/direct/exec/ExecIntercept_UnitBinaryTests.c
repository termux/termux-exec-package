#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/exec/ExecIntercept.h>



void test__inspectFileHeader();
void test__checkElfIsStatic();
void test__modifyExecEnv();



void ExecIntercept_runTests() {
    logVerbose(LOG_TAG, "ExecIntercept_runTests()");

    test__inspectFileHeader();
    test__checkElfIsStatic();
    test__modifyExecEnv();

    int__AEqual(0, errno);
}





void test__inspectFileHeader__Basic();

void test__inspectFileHeader() {
    logVerbose(LOG_TAG, "test__inspectFileHeader()");

    test__inspectFileHeader__Basic();

    int__AEqual(0, errno);
}

void test__inspectFileHeader__Basic() {
    logVVerbose(LOG_TAG, "test__inspectFileHeader__Basic()");

    char header[TERMUX__FILE_HEADER__BUFFER_SIZE];
    size_t hsize = sizeof(header);

    struct TermuxFileHeaderInfo info = {.interpreterArg = NULL};

    snprintf(header, hsize, "#!/bin/sh\n") < 0 ? abort() : (void)0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(!info.isElf);
    state__ATrue(!info.isNonNativeElf);
    string__AEqual(TERMUX__PREFIX__BIN_DIR "/sh", info.interpreterPath);
    state__ATrue(info.interpreterArg == NULL);

    snprintf(header, hsize, "#!/bin/sh -x\n") < 0 ? abort() : (void)0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(!info.isElf);
    state__ATrue(!info.isNonNativeElf);
    string__AEqual(TERMUX__PREFIX__BIN_DIR "/sh", info.interpreterPath);
    string__AEqual("-x", info.interpreterArg);

    snprintf(header, hsize, "#! /bin/sh -x\n") < 0 ? abort() : (void)0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(!info.isElf);
    state__ATrue(!info.isNonNativeElf);
    string__AEqual(TERMUX__PREFIX__BIN_DIR "/sh", info.interpreterPath);
    string__AEqual("-x", info.interpreterArg);

    snprintf(header, hsize, "#!/bin/sh -x \n") < 0 ? abort() : (void)0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(!info.isElf);
    state__ATrue(!info.isNonNativeElf);
    string__AEqual(TERMUX__PREFIX__BIN_DIR "/sh", info.interpreterPath);
    string__AEqual("-x", info.interpreterArg);

    snprintf(header, hsize, "#!/bin/sh -x \n") < 0 ? abort() : (void)0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(!info.isElf);
    state__ATrue(!info.isNonNativeElf);
    string__AEqual(TERMUX__PREFIX__BIN_DIR "/sh", info.interpreterPath);
    string__AEqual("-x", info.interpreterArg);

    info.interpreterPath = NULL;
    info.interpreterArg = NULL;
    // An ELF header for a 32-bit file.
    // See https://en.wikipedia.org/wiki/Executable_and_Linkable_Format#File_header
    snprintf(header, hsize, "\177ELF") < 0 ? abort() : (void)0;
    // Native instruction set.
    header[0x12] = EM_NATIVE;
    header[0x13] = 0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(info.isElf);
    state__ATrue(!info.isNonNativeElf);
    state__ATrue(info.interpreterPath == NULL);
    state__ATrue(info.interpreterArg == NULL);

    info.interpreterPath = NULL;
    info.interpreterArg = NULL;
    // An ELF header for a 64-bit file.
    // See https://en.wikipedia.org/wiki/Executable_and_Linkable_Format#File_header
    snprintf(header, hsize, "\177ELF") < 0 ? abort() : (void)0;
    // 'Fujitsu MMA Multimedia Accelerator' instruction set - likely non-native.
    header[0x12] = 0x36;
    header[0x13] = 0;
    inspectFileHeader(TERMUX__PREFIX, header, sizeof(header) , &info);
    state__ATrue(info.isElf);
    state__ATrue(info.isNonNativeElf);
    state__ATrue(info.interpreterPath == NULL);
    state__ATrue(info.interpreterArg == NULL);
}





void test__modifyExecEnv__unsetLDVars();
void test__modifyExecEnv__unsetLDPreload();
void test__modifyExecEnv__setProcSelfExe();

void test__modifyExecEnv() {
    logVerbose(LOG_TAG, "test__modifyExecEnv()");

    test__modifyExecEnv__unsetLDVars();
    test__modifyExecEnv__unsetLDPreload();
    test__modifyExecEnv__setProcSelfExe();

    int__AEqual(0, errno);
}

void test__modifyExecEnv__unsetLDVars() {
    logVVerbose(LOG_TAG, "test__modifyExecEnv__unsetLDVars()");

    {
    char *testEnv[] = {"MY_ENV=1", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, true, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD "a", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, true, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD "a", "A=B", ENV_PREFIX__LD_LIBRARY_PATH "B", "B=C", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, true, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], "A=B");
    string__AEqual(allocatedEnvp[2], "B=C");
    state__ATrue(allocatedEnvp[3] == NULL);
    free(allocatedEnvp);
    }
}

void test__modifyExecEnv__unsetLDPreload() {
    logVVerbose(LOG_TAG, "test__modifyExecEnv__unsetLDPreload()");

    {
    char *testEnv[] = {"MY_ENV=1", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD "a", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], ENV_PREFIX__LD_PRELOAD "a");
    state__ATrue(allocatedEnvp[2] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD, NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD "a", "A=B", ENV_PREFIX__LD_LIBRARY_PATH "B", "B=C", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], ENV_PREFIX__LD_PRELOAD "a");
    string__AEqual(allocatedEnvp[2], "A=B");
    string__AEqual(allocatedEnvp[3], ENV_PREFIX__LD_LIBRARY_PATH "B");
    string__AEqual(allocatedEnvp[4], "B=C");
    state__ATrue(allocatedEnvp[5] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV_PREFIX__LD_PRELOAD, "A=B", ENV_PREFIX__LD_LIBRARY_PATH "B", "B=C", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], "A=B");
    string__AEqual(allocatedEnvp[2], ENV_PREFIX__LD_LIBRARY_PATH "B");
    string__AEqual(allocatedEnvp[3], "B=C");
    state__ATrue(allocatedEnvp[4] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", "A=B", ENV_PREFIX__LD_LIBRARY_PATH "B", "B=C", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, true);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], "A=B");
    string__AEqual(allocatedEnvp[2], ENV_PREFIX__LD_LIBRARY_PATH "B");
    string__AEqual(allocatedEnvp[3], "B=C");
    state__ATrue(allocatedEnvp[4] == NULL);
    free(allocatedEnvp);
    }
}

void test__modifyExecEnv__setProcSelfExe() {
    logVVerbose(LOG_TAG, "test__modifyExecEnv__setProcSelfExe()");

    {
    char *termuxProcSelfExe = NULL;
    state__ATrue(asprintf(&termuxProcSelfExe, "%s%s", ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE, TERMUX__PREFIX__BIN_DIR "/bash") != -1);

    char *testEnv[] = {"MY_ENV=1", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, &termuxProcSelfExe, false, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], ENV__TERMUX_EXEC__PROC_SELF_EXE "=" TERMUX__PREFIX__BIN_DIR "/bash");
    free(termuxProcSelfExe);
    free(allocatedEnvp);
    }

    {
    char *termuxProcSelfExe = NULL;
    state__ATrue(asprintf(&termuxProcSelfExe, "%s%s", ENV_PREFIX__TERMUX_EXEC__PROC_SELF_EXE, TERMUX__PREFIX__BIN_DIR "/bash") != -1);

    char *testEnv[] = {"MY_ENV=1", ENV__TERMUX_EXEC__PROC_SELF_EXE "=" TERMUX__PREFIX__BIN_DIR "/python", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, &termuxProcSelfExe, false, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    string__AEqual(allocatedEnvp[1], ENV__TERMUX_EXEC__PROC_SELF_EXE "=" TERMUX__PREFIX__BIN_DIR "/bash");
    free(termuxProcSelfExe);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }

    {
    char *testEnv[] = {"MY_ENV=1", ENV__TERMUX_EXEC__PROC_SELF_EXE "=" TERMUX__PREFIX__BIN_DIR "/python", NULL};
    char **allocatedEnvp;
    modifyExecEnv(testEnv, &allocatedEnvp, NULL, false, false);
    state__ATrue(allocatedEnvp != NULL);
    string__AEqual(allocatedEnvp[0], "MY_ENV=1");
    state__ATrue(allocatedEnvp[1] == NULL);
    free(allocatedEnvp);
    }
}





static void initSyntheticElf64(char *buf, size_t bufSize, uint64_t phoff, uint16_t phnum, bool includePtInterp) {
    memset(buf, 0, bufSize);

    Elf64_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_NATIVE;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phoff = phoff;
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = phnum;
    memcpy(buf, &ehdr, sizeof(ehdr));

    if (phnum > 0 && phoff + ((size_t)phnum * sizeof(Elf64_Phdr)) <= bufSize) {
        for (uint16_t i = 0; i < phnum; i++) {
            Elf64_Phdr phdr;
            memset(&phdr, 0, sizeof(phdr));
            phdr.p_type = (includePtInterp && i == 0) ? PT_INTERP : PT_LOAD;
            memcpy(buf + phoff + ((size_t)i * sizeof(Elf64_Phdr)), &phdr, sizeof(phdr));
        }
    }
}

static void initSyntheticElf32(char *buf, size_t bufSize, uint32_t phoff, uint16_t phnum, bool includePtInterp) {
    memset(buf, 0, bufSize);

    Elf32_Ehdr ehdr;
    memset(&ehdr, 0, sizeof(ehdr));
    memcpy(ehdr.e_ident, ELFMAG, SELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_NATIVE;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    ehdr.e_phoff = phoff;
    ehdr.e_phentsize = sizeof(Elf32_Phdr);
    ehdr.e_phnum = phnum;
    memcpy(buf, &ehdr, sizeof(ehdr));

    if (phnum > 0 && phoff + ((size_t)phnum * sizeof(Elf32_Phdr)) <= bufSize) {
        for (uint16_t i = 0; i < phnum; i++) {
            Elf32_Phdr phdr;
            memset(&phdr, 0, sizeof(phdr));
            phdr.p_type = (includePtInterp && i == 0) ? PT_INTERP : PT_LOAD;
            memcpy(buf + phoff + ((size_t)i * sizeof(Elf32_Phdr)), &phdr, sizeof(phdr));
        }
    }
}

void test__checkElfIsStatic() {
    logVerbose(LOG_TAG, "test__checkElfIsStatic()");

    bool isStaticElf = false;

    // Non-ELF script header must return 0 with isStaticElf = false.
    const char *script = "#!/bin/sh\n";
    int__AEqual(0, checkElfIsStatic(NULL, script, strlen(script), &isStaticElf));
    state__ATrue(!isStaticElf);

    // 64-bit ELF with PT_INTERP inside buffer.
    char buf64[sizeof(Elf64_Ehdr) + (2 * sizeof(Elf64_Phdr))];
    initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, true);
    int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
    state__ATrue(!isStaticElf);

    // 64-bit ELF without PT_INTERP inside buffer (static).
    initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
    int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
    state__ATrue(isStaticElf);

    // 32-bit ELF with PT_INTERP inside buffer.
    char buf32[sizeof(Elf32_Ehdr) + (2 * sizeof(Elf32_Phdr))];
    initSyntheticElf32(buf32, sizeof(buf32), sizeof(Elf32_Ehdr), 2, true);
    int__AEqual(0, checkElfIsStatic(NULL, buf32, sizeof(buf32), &isStaticElf));
    state__ATrue(!isStaticElf);

    // 32-bit ELF without PT_INTERP inside buffer (static).
    initSyntheticElf32(buf32, sizeof(buf32), sizeof(Elf32_Ehdr), 2, false);
    int__AEqual(0, checkElfIsStatic(NULL, buf32, sizeof(buf32), &isStaticElf));
    state__ATrue(isStaticElf);

    // NULL output pointer must fail with EINVAL.
    int__AEqual(-1, checkElfIsStatic(NULL, buf64, sizeof(buf64), NULL));
    int__AEqual(EINVAL, errno);
    errno = 0;

    // NULL header + NULL executablePath must cleanly return 0 with isStaticElf = false.
    int__AEqual(0, checkElfIsStatic(NULL, NULL, 0, &isStaticElf));
    state__ATrue(!isStaticElf);

    // Truncated ELF magic / incomplete headers must return 0 with isStaticElf = false.
    {
        const char truncMagic[4] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
        int__AEqual(0, checkElfIsStatic(NULL, truncMagic, sizeof(truncMagic), &isStaticElf));
        state__ATrue(!isStaticElf);

        char trunc64[sizeof(Elf64_Ehdr) - 1];
        memset(trunc64, 0, sizeof(trunc64));
        memcpy(trunc64, ELFMAG, SELFMAG);
        trunc64[EI_CLASS] = ELFCLASS64;
        trunc64[EI_DATA] = ELFDATA2LSB;
        int__AEqual(0, checkElfIsStatic(NULL, trunc64, sizeof(trunc64), &isStaticElf));
        state__ATrue(!isStaticElf);

        char trunc32[sizeof(Elf32_Ehdr) - 1];
        memset(trunc32, 0, sizeof(trunc32));
        memcpy(trunc32, ELFMAG, SELFMAG);
        trunc32[EI_CLASS] = ELFCLASS32;
        trunc32[EI_DATA] = ELFDATA2LSB;
        int__AEqual(0, checkElfIsStatic(NULL, trunc32, sizeof(trunc32), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Wrong endianness and invalid EI_CLASS must return 0 with isStaticElf = false.
    {
        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
        buf64[EI_DATA] = ELFDATA2MSB;
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);

        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
        buf64[EI_CLASS] = ELFCLASSNONE;
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Overflow / corrupt e_phoff guard must not crash and must return isStaticElf = false.
    initSyntheticElf64(buf64, sizeof(buf64), UINT64_MAX - 16, 2, false);
    int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
    state__ATrue(!isStaticElf);

    // e_phnum == 0 -> static (true); e_phoff == 0 with e_phnum > 0 -> malformed (false).
    {
        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 0, false);
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(isStaticElf);

        initSyntheticElf64(buf64, sizeof(buf64), 0, 2, false);
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Malformed phentsize < sizeof(Elf64_Phdr) -> false; oversized table > 1 MiB -> false.
    {
        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
        Elf64_Ehdr badEhdr;
        memcpy(&badEhdr, buf64, sizeof(badEhdr));
        badEhdr.e_phentsize = sizeof(Elf64_Phdr) - 1;
        memcpy(buf64, &badEhdr, sizeof(badEhdr));
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);

        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
        memcpy(&badEhdr, buf64, sizeof(badEhdr));
        badEhdr.e_phnum = 30000; // 30000 * 56 > 1 MiB limit
        memcpy(buf64, &badEhdr, sizeof(badEhdr));
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Non-standard larger phentsize stride (72 bytes) with and without PT_INTERP.
    {
        const uint16_t customEntSize = 72;
        char strideBuf[sizeof(Elf64_Ehdr) + (2 * 72)];
        memset(strideBuf, 0, sizeof(strideBuf));
        initSyntheticElf64(strideBuf, sizeof(Elf64_Ehdr), sizeof(Elf64_Ehdr), 2, false);
        Elf64_Ehdr sEhdr;
        memcpy(&sEhdr, strideBuf, sizeof(sEhdr));
        sEhdr.e_phentsize = customEntSize;
        memcpy(strideBuf, &sEhdr, sizeof(sEhdr));

        Elf64_Phdr sPhdr;
        memset(&sPhdr, 0, sizeof(sPhdr));
        sPhdr.p_type = PT_LOAD;
        memcpy(strideBuf + sizeof(Elf64_Ehdr), &sPhdr, sizeof(sPhdr));
        memcpy(strideBuf + sizeof(Elf64_Ehdr) + customEntSize, &sPhdr, sizeof(sPhdr));
        int__AEqual(0, checkElfIsStatic(NULL, strideBuf, sizeof(strideBuf), &isStaticElf));
        state__ATrue(isStaticElf);

        sPhdr.p_type = PT_INTERP;
        memcpy(strideBuf + sizeof(Elf64_Ehdr) + customEntSize, &sPhdr, sizeof(sPhdr));
        int__AEqual(0, checkElfIsStatic(NULL, strideBuf, sizeof(strideBuf), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Non-native e_machine must return 0 with isStaticElf = false.
    {
        initSyntheticElf64(buf64, sizeof(buf64), sizeof(Elf64_Ehdr), 2, false);
        Elf64_Ehdr foreignEhdr;
        memcpy(&foreignEhdr, buf64, sizeof(foreignEhdr));
        foreignEhdr.e_machine = (uint16_t)(EM_NATIVE ^ 1);
        memcpy(buf64, &foreignEhdr, sizeof(foreignEhdr));
        int__AEqual(0, checkElfIsStatic(NULL, buf64, sizeof(buf64), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // Partial-buffer fast-path:
    // 1) PT_INTERP within initial bytes resolves dynamically without path.
    // 2) No PT_INTERP in cached prefix with NULL path remains conservatively false.
    {
        char partialBuf[sizeof(Elf64_Ehdr) + (3 * sizeof(Elf64_Phdr))];
        initSyntheticElf64(partialBuf, sizeof(partialBuf), sizeof(Elf64_Ehdr), 3, false);
        Elf64_Ehdr pEhdr;
        memcpy(&pEhdr, partialBuf, sizeof(pEhdr));
        pEhdr.e_phnum = 8;
        memcpy(partialBuf, &pEhdr, sizeof(pEhdr));

        int__AEqual(0, checkElfIsStatic(NULL, partialBuf, sizeof(partialBuf), &isStaticElf));
        state__ATrue(!isStaticElf);

        // Put PT_INTERP at index 1 in the cached prefix -> must detect dynamic without path.
        Elf64_Phdr interpPhdr;
        memset(&interpPhdr, 0, sizeof(interpPhdr));
        interpPhdr.p_type = PT_INTERP;
        memcpy(partialBuf + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr), &interpPhdr, sizeof(interpPhdr));

        int__AEqual(0, checkElfIsStatic(NULL, partialBuf, sizeof(partialBuf), &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // File-backed pread() fallback tests (single-fd, heap >1024B phdrs, last-entry PT_INTERP, truncated EOF).
    const char *tmpDir = getenv("TMPDIR");
    if (tmpDir == NULL || tmpDir[0] == '\0') {
        tmpDir = "/tmp";
    }
    char tmpPath[PATH_MAX];
    snprintf(tmpPath, sizeof(tmpPath), "%s/termux-exec-static-elf-test-XXXXXX", tmpDir);
    int fd = mkstemp(tmpPath);
    state__ATrue(fd >= 0);

    // 1) Phdrs placed beyond TERMUX__FILE_HEADER__BUFFER_SIZE (offset 384).
    char fileBuf[512];
    initSyntheticElf64(fileBuf, sizeof(fileBuf), 384, 2, false);
    ssize_t written = write(fd, fileBuf, sizeof(fileBuf));
    state__ATrue(written == (ssize_t)sizeof(fileBuf));
    close(fd);

    int__AEqual(0, checkElfIsStatic(tmpPath, NULL, 0, &isStaticElf));
    state__ATrue(isStaticElf);

    fd = open(tmpPath, O_WRONLY | O_TRUNC);
    state__ATrue(fd >= 0);
    initSyntheticElf64(fileBuf, sizeof(fileBuf), 384, 2, true);
    written = write(fd, fileBuf, sizeof(fileBuf));
    state__ATrue(written == (ssize_t)sizeof(fileBuf));
    close(fd);

    int__AEqual(0, checkElfIsStatic(tmpPath, NULL, 0, &isStaticElf));
    state__ATrue(!isStaticElf);

    // 2) Large phdr table (30 entries = 1680 bytes > 1024B stackPhdrBuf -> malloc fallback),
    //    both static and with PT_INTERP at the very last entry (index 29).
    {
        char bigBuf[sizeof(Elf64_Ehdr) + (30 * sizeof(Elf64_Phdr))];
        initSyntheticElf64(bigBuf, sizeof(bigBuf), sizeof(Elf64_Ehdr), 30, false);

        int bigFd = open(tmpPath, O_WRONLY | O_TRUNC);
        state__ATrue(bigFd >= 0);
        ssize_t bigWritten = write(bigFd, bigBuf, sizeof(bigBuf));
        state__ATrue(bigWritten == (ssize_t)sizeof(bigBuf));
        close(bigFd);

        int__AEqual(0, checkElfIsStatic(tmpPath, NULL, 0, &isStaticElf));
        state__ATrue(isStaticElf);
        // Also test when caller supplies first 339 bytes (cached partial prefix + trailing malloc pread).
        int__AEqual(0, checkElfIsStatic(tmpPath, bigBuf, TERMUX__FILE_HEADER__BUFFER_SIZE - 1, &isStaticElf));
        state__ATrue(isStaticElf);

        // Place PT_INTERP at index 29 (final entry).
        Elf64_Phdr lastInterp;
        memset(&lastInterp, 0, sizeof(lastInterp));
        lastInterp.p_type = PT_INTERP;
        memcpy(bigBuf + sizeof(Elf64_Ehdr) + (29 * sizeof(Elf64_Phdr)), &lastInterp, sizeof(lastInterp));

        bigFd = open(tmpPath, O_WRONLY | O_TRUNC);
        state__ATrue(bigFd >= 0);
        bigWritten = write(bigFd, bigBuf, sizeof(bigBuf));
        state__ATrue(bigWritten == (ssize_t)sizeof(bigBuf));
        close(bigFd);

        int__AEqual(0, checkElfIsStatic(tmpPath, NULL, 0, &isStaticElf));
        state__ATrue(!isStaticElf);
        int__AEqual(0, checkElfIsStatic(tmpPath, bigBuf, TERMUX__FILE_HEADER__BUFFER_SIZE - 1, &isStaticElf));
        state__ATrue(!isStaticElf);
    }

    // 3) Truncated phdr table extending past EOF -> preadExact returns -1 with ENOEXEC and isStaticElf = false.
    {
        char truncFileBuf[sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr)];
        initSyntheticElf64(truncFileBuf, sizeof(truncFileBuf), sizeof(Elf64_Ehdr), 1, false);
        Elf64_Ehdr truncEhdr;
        memcpy(&truncEhdr, truncFileBuf, sizeof(truncEhdr));
        truncEhdr.e_phnum = 4; // Claims 4 phdrs, file only holds 1.
        memcpy(truncFileBuf, &truncEhdr, sizeof(truncEhdr));

        int truncFd = open(tmpPath, O_WRONLY | O_TRUNC);
        state__ATrue(truncFd >= 0);
        ssize_t truncWritten = write(truncFd, truncFileBuf, sizeof(truncFileBuf));
        state__ATrue(truncWritten == (ssize_t)sizeof(truncFileBuf));
        close(truncFd);

        int__AEqual(-1, checkElfIsStatic(tmpPath, NULL, 0, &isStaticElf));
        int__AEqual(ENOEXEC, errno);
        state__ATrue(!isStaticElf);
        errno = 0;
    }

    unlink(tmpPath);

    int__AEqual(0, errno);
}
