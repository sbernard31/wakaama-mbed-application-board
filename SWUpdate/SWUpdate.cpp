
// Software Update via Ethernet from forum -
// http://mbed.org/forum/mbed/topic/1183/
//
#include "mbed.h"
#include "SWUpdate.h"
//#include "HTTPClient.h"
#include "HTTPText.h"
#include "HTTPFile.h"
#include <stdio.h>

extern "C" void mbed_reset();

#define DEBUG "SWup"
#include <cstdio>
#if (defined(DEBUG) && !defined(TARGET_LPC11U24))
#define DBG(x, ...)  std::printf("[DBG %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define WARN(x, ...) std::printf("[WRN %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define ERR(x, ...)  std::printf("[ERR %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#define INFO(x, ...) std::printf("[INF %s %3d] "x"\r\n", DEBUG, __LINE__, ##__VA_ARGS__);
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#define INFO(x, ...)
#endif

static HTTPResult HTTPErrorCode;

static bool PassesIntegrityCheck(const char * fname, int cksum, int fsize)
{
    int res = false;    // assume things go wrong...
    int newCksum = 0;
    int newFSize = 0;
    FILE *fh = fopen(fname, "rb");

    INFO("IntegrityCheck(%s,%d,%d)", fname, cksum, fsize);
    if (fh) {
        char buf;
        while (fread(&buf, 1, 1, fh)) {
            newCksum = (newCksum + buf) & 0xFFFF;
            newFSize++;
        }
        fclose(fh);
        INFO("COMPUTED: %d , EXPECTED: %d", newCksum, cksum);
        INFO("      Check(...,%d,%d)", newCksum, newFSize);
        if (newCksum == cksum && newFSize == fsize)
            res = true;
    } else {
        WARN("failed to open %s.", fname);
    }
    res = true ;
    return res;
}

/// mytolower exists because not all compiler libraries have this function
///
/// This takes a character and if it is upper-case, it converts it to
/// lower-case and returns it.
///
/// @note this only works for characters in the range 'A' - 'Z'.
///
/// @param a is the character to convert
/// @returns the lower case equivalent to the supplied character.
///
static char mytolower(char a)
{
    if (a >= 'A' && a <= 'Z')
        return (a - 'A' + 'a');
    else
        return a;
}

/// mystrnicmp exists because not all compiler libraries have this function.
///
/// Some have strnicmp, others _strnicmp, and others have C++ methods, which
/// is outside the scope of this C-portable set of functions.
///
/// @param l is a pointer to the string on the left
/// @param r is a pointer to the string on the right
/// @param n is the number of characters to compare
/// @returns -1 if l < r
/// @returns 0 if l == r
/// @returns +1 if l > r
///
static int mystrnicmp(const char *l, const char *r, size_t n)
{
    int result = 0;

    if (n != 0) {
        do {
            result = mytolower(*l++) - mytolower(*r++);
        } while ((result == 0) && (*l != '\0') && (--n > 0));
    }
    if (result < -1)
        result = -1;
    else if (result > 1)
        result = 1;
    return result;
}


// Scan the local file system for any .bin files and
// if they don't match the current one, remove them.
//
static bool RemoveOtherBinFiles(const char * name, int ver)
{
    char curbin[SW_MAX_FQFN];
    DIR *d;
    struct dirent *p;
    bool noFailed = true;

    snprintf(curbin, SW_MAX_FQFN, "%s%02d.bin", name, (ver % 100));
    INFO("Remove bin files excluding {%s}", curbin);
    d = opendir("/local/");
    // Get a directory handle
    if ( d != NULL ) {
        // Walk the directory
        while ( (p = readdir(d)) != NULL ) {
            INFO("  check {%s}", p->d_name);
            // if the file is .bin and not curbin
            if (0 == mystrnicmp(p->d_name + strlen(p->d_name) - 4, ".bin", 4)
                    && (0 != mystrnicmp(p->d_name, curbin, strlen(curbin)))) {
                // remove the file
                char toremove[SW_MAX_FQFN];
                snprintf(toremove, SW_MAX_FQFN, "/local/%s", p->d_name);
                INFO("    removing %s.", toremove);
                if (remove(toremove)) {
                    // set flag if it could not be removed
                    noFailed = false;
                }
            }
        }
        closedir(d);
    }
    return noFailed;
}

HTTPResult SoftwareUpdateGetHTTPErrorCode(void)
{
    return HTTPErrorCode;
}

SWUpdate_T SoftwareUpdate(const char *url, const char * name, Reboot_T action)
{
    HTTPClient http;
    //http.setTimeout( 15000 );
    char fqurl[SW_MAX_URL];    // fully qualified url
    char verfn[SW_MAX_FQFN];     // local version file
    char fwfn[SW_MAX_FQFN];
    char nameroot[7];
    uint16_t result = SWUP_OK;    // starting out quite optimistic, for all the things that can go wrong
    char buf[50];           // long enough for 3 comma separated numbers...

    INFO("SoftwareUpdate(%s,%s)", url, name);
    strncpy(nameroot, name, 6);
    nameroot[6] = '\0';
    snprintf(verfn, SW_MAX_FQFN, "/local/%s.ver", nameroot);

    /* Read installed version string */
    int inst_ver = -1;
    FILE *fv = fopen(verfn, "r");
    if (fv) {
        fscanf(fv, "%d", &inst_ver);
        fclose(fv);
    }
    INFO("  Installed version: %d", inst_ver);

    /* Download latest version string */
    HTTPText server_ver(buf, 50);
    snprintf(fqurl, SW_MAX_URL, "%s/%s.txt", url, name);
    HTTPErrorCode = http.get(fqurl, &server_ver);
    if (HTTPErrorCode == HTTP_OK) {
        INFO("Download %s --> %d", fqurl, HTTPErrorCode);
        INFO("File contents: %s", buf);
        int latest_ver = -1;
        int cksum = 0;
        int fsize = 0;
        int parseCount;
        parseCount = sscanf(buf, "%d,%d,%d", &latest_ver, &cksum, &fsize);
        INFO("PARSE OK %d", parseCount);
        if (parseCount == 3) {
            INFO("        web version: %d", latest_ver);
            INFO("           checksum: %d", cksum);
            INFO("          file size: %d", fsize);
            if (true || inst_ver != latest_ver) {  // HACK benj remove version checking for demo purposes
                INFO("  Downloading firmware ver %d ...", latest_ver);
                sprintf(fwfn, "/local/%s%02d.BIN", nameroot, (latest_ver % 100));
                snprintf(fqurl, 150, "%s/%s.bin", url, name);

                HTTPFile latest(fwfn);
                HTTPErrorCode = http.get(fqurl, &latest);
                if (HTTPErrorCode == HTTP_OK) {
                    if (PassesIntegrityCheck(fwfn, cksum, fsize)) {
                        if (!RemoveOtherBinFiles(nameroot, latest_ver)) {
                            ERR("  *** Failed to remove old version(s). ***");
                            result |= SWUP_OLD_STUCK;
                        }
                        INFO("Updating stored version number.");
                        fv = fopen(verfn, "w");
                        if (fv) {
                            int fr = fputs(buf, fv);
                            if (fr < 0) {
                                ERR("Failed (%d) to update stored version number.", fr);
                                fclose( fv );
                                result |= SWUP_VER_STUCK;
                            } else {
                                fclose( fv );
                                if (action == AUTO_REBOOT) {
                                    WARN("Resetting...\n");
                                    wait_ms(200);
                                    mbed_reset();
                                }
                            }
                        } else {
                            WARN("Failed to update local version info in %s.", verfn);
                            result |= SWUP_VWRITE_FAILED;
                        }
                    } else {
                        WARN("New file {%s} did not pass integrity check.", fwfn);
                        result |= SWUP_INTEGRITY_FAILED;
                    }
                } else {
                    WARN("Failed to download lastest firmware.");
                    result |= SWUP_HTTP_BIN;
                }
            } else {
                INFO("Online version is same as installed version.");
                result |= SWUP_SAME_VER;
            }
        }
    } else {
        WARN("Failed accessing server. Extended Error Code = %d", HTTPErrorCode);
        result |= SWUP_HTTP_VER;
    }
    return (SWUpdate_T)result;
}
