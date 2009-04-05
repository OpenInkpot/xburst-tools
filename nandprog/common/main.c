/*
 * Main entry of the program.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "include.h"

static np_data *npdata;

int main(int argc, char *argv[])
{
	if (cmdline(argc, argv, npdata)) return 0;
	npdata=cmdinit();
	if (!npdata) return 0;
	if (cmdexcute(npdata)) return 0;
	if (cmdexit(npdata)) return 0;

	return 0;
}
