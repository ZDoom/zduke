#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#include "types.h"
#include "file_lib.h"

bool SafeFileExists ( const char * filename )
{
	struct stat buff;

	return stat (filename, &buff) == 0 && !(buff.st_mode & _S_IFDIR);
}

int32 SafeOpenRead ( const char * filename, int32 filetype )
{
	int handle, oflag;

	if (filetype != filetype_text)
	{
		oflag = _O_BINARY;
	}

	handle = open (filename, oflag | _O_RDONLY);
	return handle;
}

void SafeRead (int32 handle, void *buffer, int32 count)
{
	read (handle, buffer, count);
}
