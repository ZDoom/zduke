struct find_t : public _finddata_t
{
	int SearchHandle;
};

inline _dos_findfirst (const char *pattern, int attrib, find_t *finder)
{
	finder->SearchHandle = _findfirst (pattern, finder);
	return finder->SearchHandle == -1;
}

inline _dos_findnext (find_t *finder)
{
	return _findnext (finder->SearchHandle, finder);
}

inline _dos_findclose (find_t *finder)
{
	return _findclose (finder->SearchHandle);
}

#define Z_AvailHeap()			(32*1024*1024)

static void limitrate() {}