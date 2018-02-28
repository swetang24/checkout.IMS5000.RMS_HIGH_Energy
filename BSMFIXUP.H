/******************************************************************************
	BSMFIXUPS Class

	Handles pointer->index and index->pointer mapping for the IMPSave and
	LoadNativeIMP functions.  When CDEV marks are stored in files, there
	BSM pointers must be stored as indexes into the collection of marks
	immediatly following the main marks collection in the file.  When loaded
	these bsm marks must be loaded and the indexes in the parent marks
	must fixed up to pointers to the bsm marks.
******************************************************************************/
class BSMFIXUPS
	{
	IMPMARKS *pmarks; // Parent marks collection
	int sizefixups;   // Number of fixups allocated
	int nfixups;      // Number of fixups used
	IMPMARK **fixups; // Collection of fixups
public:
	BSMFIXUPS(IMPMARKS *marks);
	~BSMFIXUPS();
	void toindex();
	void topointer();
	BOOL writebsms(HANDLE fh);
	BOOL readbsms(HANDLE fh,WORD ver);
private:
	long add(IMPMARK *mark);
	long find(IMPMARK *mark);
	int CountUniqueBSMs();
	};
