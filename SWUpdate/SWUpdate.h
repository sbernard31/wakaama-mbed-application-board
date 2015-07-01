/// @file 
/// Automatic Software Update via the network.
///
/// This library provides a reasonably simple interface to updating sofware
/// semi-automatically via the network. It does that by querying a web server 
/// upon which you have placed an updated version of the embedded software 
/// application. Upon finding something there, it will determine if it is
/// a different version than the one you current have installed and it
/// will try to download the software. If that succeeds, then it can
/// optionally reboot to activate the new software.
///
/// While the name shown in the examples here is "myprog", this is unimportant.
/// Your application will have a name of your choosing, which you will 
/// use in the API.
///
/// @note Your binary file name should not exceed 6 characters. This leaves
///     room for a 2-digit sequence number. Since this is using the local
///     file system, it does not support long filenames when accessed from
///     the mbed. So, SomeLongFile23.bin becomes somefi~1.bin and is then 
///     erased, leaving no file.
///
/// @note This was tested with firmware version 16457. Others may/not work.
///     To check your version, open the mbed.htm file in a text editor,
///     and look for "&firmware=16457&"
///
/// Local File System Files:
///
/// The files of interest on the local file system are as follows:
///
///   @li myprog23.bin - The actual application binary file that is currently
///             executing. In this case, this is the 23rd version that
///             has been installed. You can go to 99 after which you might
///             want to start over.
///   @li myprog.ver - A text file, maintained by this software update
///             application, that was downloaded from the server with
///             application version 23. 
///
/// If "myprog.ver" does not exist, it will assume that the server has a 
/// newer application, so it will be downloaded and activated (even if all
/// it does is to replace the existing myprog23.bin file).
///
/// Web Server Files:
///
/// The files on the web server are as follows. 
///
///   @li myprog.bin - The latest version of the application binary file. 
///             Note that this file does not have any version number
///             embedded into its filename as is the case on the local
///             file system.
///   @li myprog.txt - A corresponding text file. The root name must match
///             that of the binary file.
///
/// The myprog.txt file shall have 3 comma-separated numbers in it.
///   version,checksum,filesize (e.g. "23,41384,107996").
///
///   @li version is a simple number. If the number is different than
///             what is stored on the local file system, then the program
///             will be updated (even if the server number is lower).
///             This bidirectional "update" can let you downgrade.
///   @li checksum is the decimal representation of a simple 16-bit checksum.
///   @li filesize is the decimal representation of the size of the file.
///
/// Variations:
///
///   Within that single web server folder, you could have several apps - 
///     @li Sensor.bin, Sensor.txt
///     @li SensrB.bin, SensrB.txt
///     @li SensrD.bin, SensrD.txt
///
///   In this example, perhaps your first version was called SensorNode, but
///   with a small hardware design change, you have a new "model B" version.
///   This example also assumes that you have a need to maintain two separate
///   applications, one for each hardware version.
///
/// Creating the Version and Integrity Check data:
///
/// You can create the server "myprog.txt" file with this perl script (not
/// every detail is shown, but it should be easy to figure out).
/// @code
/// # Read current .txt file
/// open (FT, "<$txt") || die("Can't read $txt.");
/// $ver = <FT>; chomp $ver; close FT;
/// $ver =~ s/(\d+),.*/$1/;
/// print "Current Version is {$ver}\n";
/// 
/// # Read the [assumed new] .bin file
/// open (FB, "<$bin") || die("Can't read $bin.");
/// binmode FB;
/// while (sysread(FB, $c, 1))
///     {
///     $cksum = ($cksum + ord($c)) & 0xFFFF;
///     $byteCount++;
///     }
/// close FB;
/// # Advance version number and write the new .txt file
/// $ver++; print "$ver Checksum is $cksum over $byteCount bytes.\n";
/// open (FT, ">$txt") || die("Can't write update to $txt.");
/// printf(FT "%d,%d,%d\n", $ver, $cksum,$byteCount);
/// close FT;
/// @endcode
///

#ifndef SWUPDATE_H
#define SWUPDATE_H

#include "mbed.h"
#include "HTTPClient.h"

// This defines the maximum string length for a fully qualified
// filename. Usually, this will be pretty short 
// (e.g. "/local/myprog.bin"), which should be 19 max with 8.3 filename.
#define SW_MAX_FQFN 20

// This defines the maximum string length for the url, including
// the base filename of interest.
#define SW_MAX_URL 150

/// After downloading, the user can choose what happens next.
typedef enum {
    DEFER_REBOOT,   ///< Do not reboot to activate the new firmware.
    AUTO_REBOOT     ///< Automatically reboot to activate the new firmware.
} Reboot_T;

/// Bit-Field return codes from the SoftwareUpdate API.
///
/// Various things can go wrong in the software update process. The return
/// value is a bit-field that flags the possibilities, although usually there
/// will only be a single bit set.
///
typedef enum {
    SWUP_OK               = 0x00,   ///< Software Update succeeded as planned.
    SWUP_SAME_VER         = 0x01,   ///< Online version is the same as the installed version.
    SWUP_HTTP_BIN         = 0x02,   ///< HTTP get returned an error while trying to fetch the bin file.
    SWUP_OLD_STUCK        = 0x04,   ///< Old file could not be removed.
    SWUP_VER_STUCK        = 0x08,   ///< Old version number could not be updated.
    SWUP_VWRITE_FAILED    = 0x10,   ///< Can't open for write the version tracking file.
    SWUP_INTEGRITY_FAILED = 0x20,   ///< Integrity check of downloaded file failed.
    SWUP_HTTP_VER         = 0x40,   ///< HTTP get returned an error while trying to fetch the version file.
} SWUpdate_T;


/// To perform the software update, we simply give this API the web 
/// server URL that is hosting the embedded software. We also give it 
/// the "root" name of the file of interest, which permits you to
/// have different applications served from the same location. 
/// 
/// Note that the root name can be a long filename, as is typically 
/// produced from the cloud-based build process. This name will
/// be truncated to the first 6 characters when installed on the
/// mbed local file system, in order to retain space for a 2-digit
/// version number.
///
/// One optional parameter lets you decide what happens if a new 
/// version is installed - automatically reboot to launch it, or
/// return to the calling program which may perform a more orderly
/// reboot.
///
/// @code
///     ...
///     if (NowIsTheTimeToCheckForSoftwareUpdates()) {
///         if (SWUP_OK == SoftwareUpdate("http://192.168.1.200/path/to/file", "myprog_LPC1768", DEFER_REBOOT)) {
///             printf("Software updated, rebooting now...\r\n");
///             wait_ms(5000);
///             mbed_reset();
///         }
///     }
///     ...
/// @endcode
///
/// @param url is a pointer to a text string of the url from which to download.
/// @param name is the base filename of the binary file.
/// @param action determines whether to automatically reboot to activate the new bin.
/// @return SWUpdate_T code indicating if the update succeeded, otherwise it returns a bitmask
///         of failure flags. Also, note that if it succeeded, and it was set for AUTO_REBOOT
///         that it will not return from this function.
///
SWUpdate_T SoftwareUpdate(const char *url, const char * name, Reboot_T action = AUTO_REBOOT);

/// Get the HTTP transaction return code.
///
/// If something goes wrong with the communications with the server, SoftwareUpdate will
/// respond with an SWUP_HTTP_VER or SWUP_HTTP_BIN return value. To learn more about 
/// what went wrong, this API will provide the actual return code from the HTTP transaction.
///
/// @returns @ref HTTPResult code from the server transaction.
///
HTTPResult SoftwareUpdateGetHTTPErrorCode(void);


#endif // SWUPDATE_H
